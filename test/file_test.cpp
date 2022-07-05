//
// Created by jinhua on 2022/6/29.
//
#include <fcntl.h>

const char *file_name = "/home/jinhua/赤壁赋.txt";

/**
 * 1. 缓冲IO <p>&emsp;
 * -- FILE结构体 -> 用户缓冲区
 * -- char *p_read -> 用户内存
 */
void test_buffered_io() {
    // 1. 文件获取：这里p_file相当于指向 [用户缓冲区]
    FILE *p_file = fopen(file_name, "r");

    // 2. 获取文件长度
    // 指针指向文件末尾
    fseek(p_file, 0, SEEK_END);
    // 获取文件长度，（相对于文件首的偏移字节数）
    long file_len = ftell(p_file);

    // 3. 将文件内部的指针，重新指向一个流的开头
    rewind(p_file);

    // 4. 内存空间分配，这里即是 [用户内存]
    // file_len * sizeof(char)是为了更严谨，16位上char占一个字符，其他机器上可能变化
    char *pread = (char *) malloc(file_len * sizeof(char) + 1);
    // 内存空间都赋值为‘\0’
    memset(pread, '\0', file_len * sizeof(char) + 1);

    // 5. 读入pread指向内存中
    // fread()会自动分配buffer
    fread(pread, 1, file_len, p_file);

    // 6. 输出信息，释放资源
    std::cout << pread << std::endl;
    fclose(p_file);
    free(pread);
    pread = nullptr;
}

/**
 * 2. 直接IO
 */
void test_direct_io() {
    int in_fd = open(file_name, O_RDONLY, S_IRUSR);
    char buffer[1024];
    if (in_fd == -1) {
        std::cout << "read file " << file_name << " error!";
        return;
    }

    ssize_t flag;
    // read不会分配应用程序buffer
    while ((flag = read(in_fd, buffer, 1024) > 0)) {
        puts(buffer);
        memset(buffer, '\0', flag);
    }
    close(in_fd);
}

/**
 * 3. 内存映射
 */
void test_mmap() {

}

/**
 * 4. 零拷贝
 */
void test_zero_copy() {

}