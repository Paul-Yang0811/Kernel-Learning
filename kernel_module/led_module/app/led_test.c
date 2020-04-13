#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static char usr_data[] = {"user hello data!"};

int main(int argc, const char *argv[])
{
    int ret = 0;
    char read_buf[100];
    char write_buf[100];
    const char *filename = argv[1];
    
    if (argc != 3) {
        printf("usage:/n");
        printf("LED_OFF:  ./a.out filename 0/n");
        printf("LED_ON: ./a.out filename 1/n");
        return -1;
    }

    int fd = open(filename, O_RDWR);
    if (fd < 0) {
        printf("open %s failed\n", filename);
        return -1;
    }

    if (atoi(argv[2]) == 1) {
        write_buf[0] = 1;
        ret = write(fd, write_buf, 1);
        if (ret < 0) {
            printf("read %s failed\n", filename);
        }
    } else if (atoi(argv[2]) == 0) {
        write_buf[0] = 0;
        ret = write(fd, write_buf, 1);
        if (ret < 0) {
            printf("write %s failed\n", filename);
        }
    } else {
        printf("usage: 1-->on, 0-->off\n");
    }

    ret = close(fd);
    if (ret < 0) {
        printf("close %s failed\n", filename);
        return -1;
    }

    return 0;                                                                                                                                                                                                                                                                                                                                                                                           
}