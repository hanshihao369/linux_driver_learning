#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


//./chrdevbaseAPP /dev/chrdevbase 1 表示从驱动里面读数据
//./chrdevbaseAPP /dev/chrdevbase 2 表示从驱动里面写数据

int main(int argc, char *argv[])
{
    int ret = 0;
    int fd = 0;
    char *filename;
    char readbuf[100],writebuf[100];
    static char usrdata[] = {"user data"};

    if(argc != 3)
    {
        printf("Usage: error");
    }

    filename = argv[1];

    fd = open(filename,O_RDWR);
    if(fd < 0)
    {
        printf("Error opening\r\n");
        return -1;
    }

    if(atoi(argv[2]) == 1)
    {
        ret = read(fd, readbuf, 50);
        if(ret < 0)
        {
            printf("Error reading %s\r\n", filename);
        }
        else
        {
            printf("APP read data: %s\r\n", readbuf);
        }
    }

    if(atoi(argv[2]) == 2)
    {
        memcpy(writebuf,usrdata,sizeof(usrdata));
        ret = write(fd, writebuf, 50);
        if(ret < 0)
        {
            printf("Error writing %s\r\n", filename);
        }
        else
        {
            
        }
    }

    ret = close(fd);
    if(ret < 0)
    {
        printf("Error closing %s\r\n", filename);
    }

    return 0;
}