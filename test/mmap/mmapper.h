#pragma once

#include <atomic>
#include <string>

namespace mem {
    // mmap方式写数据到文件的工具类
    class Writer {
    public:
        // 构造（映射大小限制，文件路径）
        explicit Writer(size_t size_lim, std::string file_path);

        // 析构
        ~Writer();

        // 写数据，将字符串的len长度写入
        void write_data(const char *data, size_t len);

    private:
        // 大小限制
        size_t size_lim_;
        // 映射到内存的文件指针，
        void *mem_file_ptr_;
        // 文件路径
        std::string file_path_;
        // 文件当前位置
        // UPDATE: use a spinlock, so we don't need it to be atomic
        size_t cur_pos_;

        // ...原子操作相关，保证线程安全性
        // for remap when overflow
        std::atomic_flag spinlock_;
        std::atomic<size_t> pending_;

        // 重新映射一个内存大小
        void remap(size_t new_size);
    };
}
