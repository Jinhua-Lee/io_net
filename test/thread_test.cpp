#include <pthread.h>
#include <iostream>
#include <unistd.h>

#define MAX_COUNT 10000

pthread_mutex_t mut;
int g_num = 0;

void *counter(void *args);

void test_thread_print() {
    pthread_t c_t1;
    pthread_t c_t2;
    int t1_num = 1;
    int t2_num = 2;
    pthread_create(&c_t1, nullptr, counter, &t1_num);
    pthread_create(&c_t2, nullptr, counter, &t2_num);

    // 夯住
    getchar();
}

void *counter(void *args) {
    int i = 1;
    while (i < MAX_COUNT / 4) {
        pthread_mutex_lock(&mut);
        g_num++;
        std::cout << "[counter " << *(int *)args << "] g_num = " << g_num << std::endl;
        pthread_mutex_unlock(&mut);
        usleep(1);
        i++;
    }
}