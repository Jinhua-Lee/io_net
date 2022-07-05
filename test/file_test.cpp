//
// Created by jinhua on 2022/6/29.
//

/**
 * 1. 缓冲IO
 */
void test_buffered_io() {
    FILE *p_file = fopen("/home/jinhua/赤壁赋.txt", "r");
    // 将文件内部的指针指向文件末尾
    fseek(p_file, 0, SEEK_END);

    // 获取文件长度，（得到文件位置指针当前位置相对于文件首的偏移字节数）
    long file_len = ftell(p_file);

    // 将文件内部的指针重新指向一个流的开头
    rewind(p_file);

    // 申请内存空间，file_len * sizeof(char)是为了更严谨，16位上char占一个字符，其他机器上可能变化
    char *pread = (char *) malloc(file_len * sizeof(char) + 1);

    //用malloc申请的内存是没有初始值的，如果不赋值会导致写入的时候找不到结束标志符而出现内存比实际申请值大，写入数据后面跟随乱码的情况

    // 将内存空间都赋值为‘\0’
    memset(pread, '\0', file_len * sizeof(char) + 1);

    // 将p_file中内容读入pread指向内存中
    fread(pread, 1, file_len, p_file);
    std::cout << pread << std::endl;
    fclose(p_file);
    free(pread);
    pread = nullptr;
}

/**
 * 2. 直接IO
 */
void test_direct_io() {

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