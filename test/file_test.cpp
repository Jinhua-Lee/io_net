//
// Created by jinhua on 2022/6/29.
//

void test_file_open() {
    FILE *logFile = fopen("/home/jinhua/赤壁赋.txt", "r");

    char *line = new char[240];
    const char *str;
    while ((str = fgets(line, 240, logFile) )!= nullptr) {
        std::cout << str;
    }
}