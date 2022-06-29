//
// Created by jinhua on 2022/6/29.
//

void test_file_open() {
    FILE *logFile = fopen("/home/jinhua/jmeter.log", "r");

    char *line = new char[40];
    char *str = fgets(line, 40, logFile);

    getchar();
}