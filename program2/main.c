#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <sys/param.h>
#include <stdbool.h>
#include <ctype.h>


void permission(struct stat st){
    static char permission[10];
    if((st.st_mode&S_IFMT)==S_IFDIR) permission[0] = 'd';
    else if((st.st_mode&S_IFMT)==S_IFREG) permission[0] = '-';
    else if((st.st_mode&S_IFMT)==S_IFLNK) permission[0] = 'l';
    else if((st.st_mode&S_IFMT)==S_IFCHR) permission[0] = 'c';
    else if((st.st_mode&S_IFMT)==S_IFBLK) permission[0] = 'b';
    else if((st.st_mode&S_IFMT)==S_IFIFO) permission[0] = 'p';
    else if((st.st_mode&S_IFMT)==S_IFSOCK) permission[0] = 's';

    for(int i=1;i<10;i++) permission[i] = '-';
    if(st.st_mode&S_IRUSR) permission[1] = 'r';
    if(st.st_mode&S_IWUSR) permission[2] = 'w';
    if(st.st_mode&S_ISUID){
        if(st.st_mode&S_IXUSR) permission[3] = 's';
        else if(!(st.st_mode&S_IXUSR)) permission[3] = 'S';
    }
    else if(!(st.st_mode&S_ISUID)&&(st.st_mode&S_IXUSR)) permission[3] = 'x';
    if(st.st_mode&S_IRGRP) permission[4] = 'r';
    if(st.st_mode&S_IWGRP) permission[5] = 'w';
    if(st.st_mode&S_ISGID){
        if(st.st_mode&S_IXGRP) permission[6] = 's';
        else if(!(st.st_mode&S_IXGRP)) permission[6] = 'S';
    }
    else if(!(st.st_mode&S_ISGID)&&(st.st_mode&S_IXGRP)) permission[6] = 'x';
    if(st.st_mode&S_IROTH) permission[7] = 'r';
    if(st.st_mode&S_IWOTH) permission[8] = 'w';
    if(st.st_mode&S_ISVTX){
        if(st.st_mode&S_IXOTH) permission[9] = 't';
        else if(!(st.st_mode&S_IXOTH)) permission[9] = 'T';
    }
    else if(!(st.st_mode&S_ISVTX)&&(st.st_mode&S_IXOTH)) permission[9] = 'x';

    printf("%s\t", permission);
}

void username(struct stat st){
    struct passwd *p;
    if((p = getpwuid(st.st_uid))==NULL){
        if(errno == 0) printf("%d\t", st.st_uid);
        else perror("Error: getpwuid():\n");
    }
    else printf("%s\t", p->pw_name);
}

void groupname(struct stat st){
    struct group *g;
    if((g = getgrgid(st.st_gid))==NULL){
        if(errno == 0) printf("%d\t", st.st_gid);
        else perror("Error: getgrgid():\n");
    }
    else printf("%s\t", g->gr_name);
}

void size(struct stat st){
    if(((st.st_mode&S_IFMT)==S_IFBLK)||((st.st_mode&S_IFMT)==S_IFCHR)){
        printf("%d, %d\t", major(st.st_rdev), minor(st.st_rdev));
    }
    else printf("%lld\t", st.st_size);
}

void ttime(struct stat st){
    char time[20];
    time_t t = st.st_mtime;
    struct tm lt;
    localtime_r(&t, &lt);
    strftime(time, 20, "%b %d %H:%M", &lt);
    printf("%s\t", time);
}

void printls(struct stat st, char *path){
    printf("%llu\t", st.st_ino);
    printf("%lld\t", st.st_blocks/2);
    permission(st);
    printf("%d\t", st.st_nlink);
    username(st);
    groupname(st);
    size(st);
    ttime(st);
    printf("%s", path);
    if((st.st_mode&S_IFMT)==S_IFLNK){
        char symlink[MAXPATHLEN] = {};
        ssize_t length;
        if((length = readlink(path, symlink, sizeof(symlink)-1))==-1){
            perror("Error: failed to read symbolic link path\n");
        }
        else printf(" -> %s\n", symlink);
    }
    else printf("\n");
}

bool diff_mount(struct stat st, int v){
    if(st.st_dev != v) return true;
    return false;
}

bool checktime(time_t t, struct stat st){
    time_t currenttime;
    time(&currenttime);
    if(t==0) return true;
    else if(t>0){
        if(st.st_mtime<currenttime-t) return true;
    }
    else if(t<0){
        if(st.st_mtime>currenttime+t) return true;
    }
    return false;
}

void find(char argv[], int v, time_t t){
    struct dirent *entry;
    DIR *dir;
    struct stat st;

    if((dir = opendir(argv))==NULL){
        fprintf(stderr, "Error: failed to open %s: %s\n", argv, strerror(errno));
        return;
    }
    errno = 0;
    while((entry = readdir(dir))){
        char path[MAXPATHLEN];

        snprintf(path, MAXPATHLEN, "%s/%s", argv, entry->d_name);
        if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
            if(lstat(path,&st)==-1){
                fprintf(stderr, "Error: failed to get status of %s:%s\n", argv, strerror(errno));
                return;
            }
            if(checktime(t,st)) printls(st,path);
            if((st.st_mode&S_IFMT)==S_IFDIR){
                if(v==-1) find(path, v, t);
                else if(diff_mount(st,v)) printf("note: not crossing mount point at %s\n", path);
                else if(!diff_mount(st,v)) find(path,v, t);
            }
        }
    }
    if(errno != 0) fprintf(stderr, "Error: failed to read %s:%s\n", argv, strerror(errno));
    if(closedir(dir)==-1){
        fprintf(stderr, "Error: failed to close %s:%s\n", argv, strerror(errno));
    }
}

int main(int argc, char *argv[]){
    int cmd;
    int v = -1;
    int argcount = 2;
    time_t t = 0;
    opterr = 0;
    struct stat st;
    lstat(argv[argc-1],&st);

    while((cmd=getopt(argc,argv,"m:v"))!=-1){
        switch(cmd){
            case 'm':
                for(int i=0;i<strlen(optarg);i++){
                    if(((isdigit(optarg[i]))==0)&&(optarg[0]!='-')){
                        fprintf(stderr, "Error: %c is not a valid number\n", optarg[i]);
                        return 0;
                    }
                }
                t = atoi(optarg);
                argcount += 2;
                break;
            case 'v':
                v = st.st_dev;
                argcount++;
                break;
            case '?':
                printf("usage: find [-m time| -v] path\n");
                exit(errno);
        }
    }
    if(argc<=1||argc!=argcount){
        fprintf(stderr, "Error: not enough arguments\n");
        printf("usage: find [-m time| -v] path\n");
        exit(errno);
    }
    else if(argc>1 && argc==argcount){
        printf("inode#\tblock#\tfile permission\tnlink\towner\tgroup\tsize\ttime\t\tpathname\n");
        if((st.st_mode&S_IFMT) == S_IFDIR){
            printls(st, argv[argc-1]);
            find(argv[argc-1], v, t);
            return 0;
        }
        else printls(st, argv[argc-1]);
        printf("\n");
    }
    return 0;
}

