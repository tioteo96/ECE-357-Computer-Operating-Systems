#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <setjmp.h>
#include <signal.h>

#define MAXSIZE 65536

jmp_buf report_jb;
int byte=0;
int n=2;
pid_t g, m, p;

int perr(char* op, char* file, char* para){
    if(errno != 0){
        fprintf(stderr, "Error: Failed to %s [%s] %s: %s\n", op, file, para, strerror(errno));
        exit(errno);
    }
    return 0;
}

void kitty(int from, char* filename, int pipe_fg[], int pipe_gm[], struct sigaction report){
    int r, w, jmp;
    char buf[MAXSIZE];
    int s = 0;
    if((close(pipe_gm[1]))==-1) perr("close","pipe_gm[1]","");
    if((close(pipe_gm[0]))==-1) perr("close","pipe_gm[0]","");
    if((close(pipe_fg[0]))==-1) perr("close","pipe_fg[0]","");
    while((r=read(from,buf,MAXSIZE))!=0){
        if((setjmp(report_jb))!=0){
            if((sigprocmask(SIG_UNBLOCK,&report.sa_mask,NULL))==-1)
                perr("sigprocmask","report","for SIG_UNBLOCK");
            break;
        }
        if(r<0) perr("read",filename,"for reading");
        w=write(pipe_fg[1],buf,r);
        if(w<0) perr("write",filename,"for writing");

        while(w<r){
            int pw=0;
            if((pw = write(pipe_fg[1],buf+w,r-w))==-1)
                perr("write",filename,"into pipe");
            w = w+pw;
        }
        byte = byte+w;
    }

    if(s==0){
        if((close(pipe_fg[1]))==-1) perr("close","pipe_fg[1]","");//
        if(close(from)==-1) perr("close",filename,"");
        s++;
        if((waitpid(g, NULL, 0))==-1) perr("waitpid","g","");
        if((waitpid(m, NULL, 0))==-1) perr("waitpid","m","");
    }
}

void more(int pipe_fg[], int pipe_gm[]){
    char *args[] = {"more", NULL};

    if((close(pipe_fg[0]))==-1) perr("close","pipe_fg[0]","");
    if((close(pipe_fg[1]))==-1) perr("close","pipe_fg[1]","");
    if((close(pipe_gm[1]))==-1) perr("close","pipe_gm[1]","");

    //stdin redirect
    if((dup2(pipe_gm[0],0))==-1) perr("dup2","pipe_gm[0]","to stdin");
    if((close(pipe_gm[0]))==-1) perr("close","pipe_gm[0]","");

    if((execvp(args[0],args))==-1)perr("execvp","more","");
}

void grep(int pipe_fg[], int pipe_gm[], char* pattern){
    char *args[] = {"grep", pattern, NULL};

    if((close(pipe_fg[1]))==-1) perr("close","pipe_fg[1]","");
    if((close(pipe_gm[0]))==-1) perr("close","pipe_gm[0]","");

    //stdin redirect
    if((dup2(pipe_fg[0],0))==-1) perr("dup2","pipe_fg[0]","to stdin");
    if((close(pipe_fg[0]))==-1) perr("close","pipe_fg[0]","");
    //stdout redirect
    if((dup2(pipe_gm[1],1))==-1) perr("dup2","pipe_gm[1]","to stdout");
    if((close(pipe_gm[1]))==-1) perr("close","pipe_gm[1]","");

    if((execvp(args[0],args))==-1) perr("execvp","grep","");
}

void sig_report(int sn){
    if(sn==SIGINT){
        fprintf(stderr,"\nNumber of files: %d\n", n-1);
        fprintf(stderr,"Number of bytes: %d\n\n", byte);
    }
    longjmp(report_jb, 1);
}

int main(int argc, char* argv[]){
    int fd;

    struct sigaction report;
    report.sa_handler = sig_report;
    report.sa_flags = 0;
    if((sigemptyset(&report.sa_mask))==-1) perr("sigemptyset","report","");
    if((sigaddset(&report.sa_mask, SIGINT))==-1) perr("sigaddset","SIGINT","to report");
    if((sigaddset(&report.sa_mask, SIGPIPE))==-1) perr("sigaddset","SIGPIPE","to report");
    if((sigaction(SIGINT,&report,0))==-1) perr("sigaction","SIGINT","");
    if((sigaction(SIGPIPE,&report,0))==-1) perr("sigaction","SIGPIPE","");

    if(argc<3){
        fprintf(stderr, "Error: need more argument\n");
        exit(1);
    }
    else{
        for(n;n<argc;n++){//for each infile
            if((fd=open(argv[n],O_RDONLY))==-1) perr("open", argv[n], "for reading");

            int pipe_fg[2];
            int pipe_gm[2];
            if(pipe(pipe_fg)==-1) perr("pipe","pipe_fg","");
            if(pipe(pipe_gm)==-1) perr("pipe","pipe_gm","");

            switch(m=fork()){
                case -1:
                    perr("fork","m","");
                    break;
                case 0://child more
                    if(close(fd)==-1)
                        perr("close", argv[n], "");
                    more(pipe_fg, pipe_gm);
                    exit(EXIT_SUCCESS);
                    break;
            }
            switch(g=fork()){
                case -1:
                    perr("fork","g","");
                    break;
                case 0://child grep
                    if(close(fd)==-1)
                        perr("close", argv[n], "");
                    grep(pipe_fg, pipe_gm, argv[1]);
                    exit(EXIT_SUCCESS);
                    break;
            }
            //parent
            kitty(fd,argv[n],pipe_fg,pipe_gm,report);
        }
    }
    return 0;
}
