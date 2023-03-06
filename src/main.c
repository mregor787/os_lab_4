#include "unistd.h"
#include "string.h"
#include "stdio.h"
#include "ctype.h"
#include "sys/mman.h"
#include "sys/wait.h"

int main() {
    char str[1000] = {};
    char *msg = "Enter the string (use ~ as end of input):\n";
    write(1, msg, sizeof(char) * strlen(msg));
    for (int i = 0; i < 1000; ++i) {
        read(0, &str[i], sizeof(char));
        if (str[i] == '~') {
            char c;
            read(0, &c, sizeof(char));
            break;
        }
    }
    int len = strlen(str);
    char* ptr = mmap(NULL, sizeof(char) * len, PROT_READ | PROT_WRITE, 
    MAP_SHARED | MAP_ANON, 0, 0);
    if (ptr == MAP_FAILED) {
        perror("Mapping error\n");
        return -1;
    }
    for (int i = 0; i < len; ++i)
        ptr[i] = str[i];

    int ch1 = fork();
    if (ch1 == -1) {
        perror("Child1 error\n");
        return -1;
    }
    else if (ch1 == 0) {
        for (int i = 0; i < strlen(ptr); ++i)
            ptr[i] = tolower(ptr[i]);
    }
    else {
        waitpid(ch1, NULL, 0);
        int ch2 = fork();
        if (ch2 == -1) {
            perror("Child2 error\n");
            return -1;
        }
        else if (ch2 == 0) {
            int i = 1;
            while (i < strlen(ptr)) {
                if (ptr[i - 1] == 32 && ptr[i] == 32) {
                    memmove(&ptr[i], &ptr[i + 1], strlen(ptr) - i);
                    --i;
                }
                ++i;
            }
        }
        else {
            waitpid(ch2, NULL, 0);
            char *msg = "Result string:\n";
            write(1, msg, sizeof(char) * strlen(msg));
            int i = 0;
            while (ptr[i] != '~') {
                write(1, &ptr[i], sizeof(char));
                ++i;
            }
            write(1, "\n", sizeof(char));
        }
    }
    int err = munmap(ptr, strlen(ptr) * sizeof(char));
    if (err != 0) {
        perror("Unmapping error\n");
        return -1;
    }
    return 0;
}