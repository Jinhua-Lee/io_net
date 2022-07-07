#include <cstdio>
#include <cstring> // for memcpy()
#include <fcntl.h> // for open()
#include <sys/mman.h>
#include <unistd.h> // for close()

#include "mmapper.h"

using namespace mem;

Writer::Writer(size_t size_lim, std::string file_path)
        : size_lim_(size_lim), file_path_(file_path), cur_pos_(0), pending_(0) {
    int fd = open(file_path_.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        printf("Open file %s failed.\n", file_path_.c_str());
        exit(-1);
    }

    // solve the bus error problem:
    // we should allocate space for the file first.
    // lseek用于移动文件读写位置，这里是将位置设置到 size_lim_ -1
    lseek(fd, size_lim_ - 1, SEEK_SET);
    write(fd, "", 1);

    // 映射fd文件的 [0, size_lim_] 位置到文件中，供读写，操作位置为fd偏移0（文件开头）.
    mem_file_ptr_ = mmap(0, size_lim_, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if (mem_file_ptr_ == MAP_FAILED) {
        printf("Map failed.\n");
        exit(-1);
    }

    spinlock_.clear(std::memory_order_release);
    close(fd);
}

Writer::~Writer() {
    // 取消文件的映射
    if (munmap(mem_file_ptr_, size_lim_) == -1) {
        printf("Unmap failed.\n");
    }
    // resize the file to actual size
    // 将文件恢复真实大小
    truncate(file_path_.c_str(), cur_pos_);
    printf("Safely quit mmap\n");
    mem_file_ptr_ = nullptr;
}

void Writer::write_data(const char *data, size_t len) {
    /* 这两条语句中间可能被打断，并不是安全的
    size_t old_pos = cur_pos_.load();
    cur_pos_ += len;
    */
    // 自选锁 start
    while (spinlock_.test_and_set(std::memory_order_acquire)) {
        // acquire spinlock
        // atomically set flag to true and return previous value
    }

    // 保证旧位置的线程安全性
    size_t old_pos = cur_pos_;
    cur_pos_ += len;
    // 位置到到末尾，扩容1倍
    if (cur_pos_ > size_lim_) {
        remap(size_lim_ << 1);
    }
    // 自选锁 end
    spinlock_.clear(std::memory_order_release); // release spinlock

    pending_ += 1;
    // 直接操作文件指针位置
    // 将data的前len长度，复制到【文件映射内存的地址mem_file_ptr_ + old_pos】位置
    std::memcpy((char *) mem_file_ptr_ + old_pos, data, len);
    pending_ -= 1;
}

void Writer::remap(size_t new_size) {
    // should be very time consuming, try to avoid

    // wait for all pending memcpy
    while (pending_ != 0) {
    }

    void *new_addr = mremap(mem_file_ptr_, size_lim_, new_size, MREMAP_MAYMOVE);
    if (new_addr == MAP_FAILED) {
        printf("Panic when try to remap...\n");
        return;
    }

    // extend file
    int fd = open(file_path_.c_str(), O_RDWR);
    if (fd == -1) {
        printf("Open file %s failed.\n", file_path_.c_str());
        exit(-1);
    }
    lseek(fd, new_size - 1, SEEK_SET);
    write(fd, "", 1);
    close(fd);

    if (new_addr != mem_file_ptr_) {
        printf("REMAP: map address changed from %p to %p...\n", mem_file_ptr_,
               new_addr);
        mem_file_ptr_ = new_addr;
    }
    size_lim_ = new_size;

    printf("REMAP: extend limit to %08x\n", new_size);
}
