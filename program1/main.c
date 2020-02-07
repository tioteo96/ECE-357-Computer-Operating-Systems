#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

void report(int bytes, int o, int r, int w, char* filename, char* buf){ //reporting to stderr
    int binary;
    for(int n=0;n<bytes;n++){
        binary = buf[n];
        if(!(isprint(binary)||isspace(binary))){
            fprintf(stderr,"<ALERT:BINARY FILE>\n");
            break;
        }
    }
    fprintf(stderr, "<%s>: (%d bytes)\nsystem calls:\n",filename,bytes);
    fprintf(stderr,"%d open/ %d read/ %d write\n",o,r,w);
}

int perr(char* op, char* file, char* para, char*err){//printing error
    if(errno != 0){
        fprintf(stderr, "Error: Failed to %s [%s] %s: %s\n", op, file, para, err);
        exit(errno);
        return -1;
    }
    return 0;
}

void rnw(int c, char argv[], int fd){
    int bytes=0, o=0, r=0, w=0;
    int lim = 4096;
    int tempfd, i, j;//i = read // j = write
    char *buf = (char*)malloc(4096*sizeof(char));
    switch (c)
    {
        case 1: //reading from infile writing to stdout
            fd = open(argv,O_RDONLY,0666);
            o++;
            if(fd<0) perr("open",argv,"for reading",strerror(errno));
            while((i=read(fd,buf,lim))>0){
                r++;
                perr("read",argv,"",strerror(errno));
                while((j=write(1,buf,i))<i) {write(1,&buf[j],i-j); w++;} //partial write
                bytes = bytes+j;
                w++;
                perr("write",argv,"to stdout",strerror(errno));
            }
            close(fd);
            perr("close",argv,"",strerror(errno));
            report(bytes,o,r,w,argv,buf);
            break;
        case 2: //reading from stdin writing to stdout
            while((i=read(0,buf,lim))>0){
                r++;
                perr("read","stdin","",strerror(errno));
                while((j=write(1,buf,i))<i) {write(1,&buf[j],i-j); w++;}
                bytes = bytes+j;
                w++;
                perr("write","stdin","to stdout",strerror(errno));
            }
            report(bytes,o,r,w,"standard input",buf);
            break;
        case 3: //reading from infile writing to outfile
            tempfd = open(argv,O_RDONLY,0666);
            o++;
            if(tempfd<0) perr("open",argv,"for reading",strerror(errno));
            while((i=read(tempfd,buf,lim))>0){
                r++;
                perr("read",argv,NULL,strerror(errno));
                while((j=write(fd,buf,i))<i) {write(fd,&buf[j],i-j); w++;}
                bytes = bytes+j;
                w++;
                perr("write",argv,"to outfile",strerror(errno));
            }
            close(tempfd);
            perr("close","infile",NULL,strerror(errno));
            report(bytes,o,r,w,argv,buf);
            break;
        case 4: //reading from stdin writing to outfile
            while((i=read(0,buf,lim))>0){
                r++;
                perr("read","stdin",NULL,strerror(errno));
                while((j=write(fd,buf,i))<i) {write(fd,&buf[j],i-j); w++;}
                bytes = bytes+j;
                w++;
                perr("write","stdin","to outfile",strerror(errno));
            }
            report(bytes,o,r,w,"standard input",buf);
            break;
    }
    free(buf);
}

int main(int argc, char *argv[]){
    int fd, option;
    int o = 0;
    while((option = getopt(argc,argv,":o")) != -1){ //argument checking
        switch (option)
        {
            case 'o':
                if(o>0) {fprintf(stderr,"Error: only one outfile can be specified\n"); exit(errno);}
                o++;
                break;
            case '?':
                fprintf(stderr, "Error: %c is not a valid argument\n", optopt);
                exit(errno);
                break;
        }
    }
    if(argc==1) rnw(2,NULL,0); // outfile and infile not specified
    else if(argc>1){
        if(strcmp(argv[1],"-o")==0){ // outfile specified
            fd = open(argv[2], O_RDWR|O_APPEND|O_CREAT|O_TRUNC,0666);
            if(argc==3) rnw(4,NULL,fd); //infile not specified
            if(argc>3){
                for(int n=3;n<(argc);n++){
                    if(strcmp(argv[n],"-")==0) rnw(4,NULL,fd); // infile specified(-)
                    else rnw(3,argv[n],fd); // infile specified
                }
            }
            close(fd);
            perr("close","outfile",NULL,strerror(errno));
        }
        else{ //outfile not specified
            for(int n=1;n<(argc);n++){
                if(strcmp(argv[n],"-")==0) rnw(2,NULL,0); // infile specified(-)
                else rnw(1,argv[n],0); // infile specified
            }
        }
    }
    return 0;
}
