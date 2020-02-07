#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

void sig_handler(int signum){
    fprintf(stderr,"Signal [%d: %s] received\n", signum, strsignal(signum));
    exit(signum);
}

int makefile(int length){
    int fd, i, w;
    char* filename = "testfile";

    if((fd=open(filename,O_RDWR|O_CREAT|O_TRUNC,0666))<0){
        fprintf(stderr,"Error: Failed to open testfile\n");
        exit(255);
    }

    for(i=0;i<length;i++){
        if((w=write(fd,"A",1))<0){
            fprintf(stderr,"Error: Failed to write to testfile\n");
            exit(255);
        }
    }
    return fd;
}

void test(int testnum, int prot, int flag){
    int fd;
    int length = 4096;
    char* map;

    if(testnum==4) length = 4500;
    fd = makefile(length);

    if((map = mmap(NULL,length,prot,flag,fd,0))<0){
        fprintf(stderr, "Error: Failed to mmap testfile\n");
        exit(255);
    }

    if(testnum==1){
        fprintf(stderr, "map[5]=='%c'\n",map[5]);
        fprintf(stderr, "writing a 'B'\n");
        map[5] = 'B';
    }
    else if(testnum==2||testnum==3){
        char buf[1];

        fprintf(stderr, "map[5] == '%c'\n", map[5]);
        fprintf(stderr, "writing a 'B' to map[5]\n");
        map[5] = 'B';

        fprintf(stderr, "looking for a byte change 'B' in file\n");
        if((lseek(fd,5,SEEK_SET))<0){
            fprintf(stderr, "Error: Failed to lseek\n");
            exit(255);
        }
        if((read(fd,buf,1))<0){
            fprintf(stderr, "Error: Failed to read from testfile\n");
            exit(255);
        }

        if(buf[0]=='B')
            exit(0);
        else
            exit(1);
    }
    else if(testnum==4){
        char buf[1];
        fprintf(stderr, "writing a 'X' to end of map\n");
        map[length] = 'X';

        fprintf(stderr, "creating a hole near the end of file\n");
        if((lseek(fd,15,SEEK_END))<0){
            fprintf(stderr, "Error: Failed to lseek\n");
            exit(255);
        }
        if((write(fd,"B",1))<0){
            fprintf(stderr,"Error: Failed to write to testfile\n");
            exit(255);
        }

        fprintf(stderr, "reading 'X' from the file\n");
        if((lseek(fd,length,SEEK_SET))<0){
            fprintf(stderr, "Error: Failed to lseek\n");
            exit(255);
        }
        if((read(fd,buf,1))<0){
            fprintf(stderr, "Error: Failed to read from testfile\n");
            exit(255);
        }

        if(buf[0]=='X')
            exit(0);
        else
            exit(1);
    }

    if((munmap(map,length))<0){
        fprintf(stderr, "Error: failed to munmap\n");
        exit(255);
    }
}

int main(int argc, char* argv[]){
    if(argc!=2){
        fprintf(stderr, "Usage: ttest [1-4]\n");
        exit(255);
    }
    int signum, t, fd;

    for(signum=1;signum<32;signum++){
        signal(signum, sig_handler);
    }

    t = atoi(argv[1]);
    switch(t){
        case 1:
            fprintf(stderr, "Executing Test #1 (PROT_READ Violation)\n");
            test(1, PROT_READ, MAP_SHARED);
            break;
        case 2:
            fprintf(stderr, "Executing Test #2 (writing to MAP_SHARED)\n");
            test(2, PROT_READ|PROT_WRITE, MAP_SHARED);
            break;
        case 3:
            fprintf(stderr, "Executing Test #3 (writing to MAP_PRIVATE)\n");
            test(3, PROT_READ|PROT_WRITE, MAP_PRIVATE);
            break;
        case 4:
            fprintf(stderr, "Executing Test #4 (PROT_READ Violation)\n");
            test(4, PROT_READ|PROT_WRITE, MAP_SHARED);
            break;
        default:
            fprintf(stderr, "Usage: ttest [1-4]\n");
            exit(255);
            break;
    }
    exit(0);
}