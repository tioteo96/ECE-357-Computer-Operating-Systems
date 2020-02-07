#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <sched.h>
#include <sys/wait.h>
#include "tas.h"

#define NSLOTS 10
#define CPUnum 10           //CPU(s): 8
#define OPnum 100000

int* retry;

struct dll{
    int value;
    struct dll *fwd, *rev;
};

struct slab{
    char freemap[NSLOTS];       //0:empty  1:filled
    struct dll slots[NSLOTS];
    char lock;
    char lock2;
};

int perr(char* op, char* file, char* para);

//Problem 1
void spin_lock(char* lock);
void spin_unlock(char* lock);
void problem1(int* balance, char* lock, bool testnum); //true:with spinlock   false:without spinlock

//Problem 2 & 3
void *slab_alloc(struct slab *slab);
int slab_dealloc(struct slab *slab, void *object);

//Problem 4
struct dll *dll_insert(struct dll *anchor, int vlaue, struct slab *slab, bool isix);
void dll_delete(struct dll *anchor, struct dll *node, struct slab *slab, bool isix);
struct dll *dll_find(struct slab *slab, struct dll *anchor, int value);

//Problem 5 & 6
void problem_5_6(bool isix); //false: problem5   true: problem6
void slab_init(struct slab *slab);

//Problem 6
void write_seqlock(char* lock, int* counter);
void write_sequnlock(char* lock, int* counter);
int read_seqbegin(int* counter);
bool read_seqretry(int* counter, int check);
struct dll *dll_find_seq(struct slab *slab, struct dll *anchor, int value);



int main(void){
    int* balance;
    char* lock;
    if((lock = mmap(NULL,sizeof(char),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0))<0)
        perr("mmap","lock","");
    if((balance = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0))<0)
        perr("mmap","balance","");

    problem1(balance,lock,true);
    *balance = 0;
    problem1(balance,lock,false);

    if(munmap(lock,sizeof(char))<0)
        perr("munmap","lock","");
    if(munmap(balance,sizeof(int))<0)
        perr("mmunap","balance","");

    struct timeval tv1;
    struct timeval tv2;
    struct timeval result;

    fprintf(stderr, "Problem 5:\n");
    if(gettimeofday(&tv1,NULL)<0) perr("gettimeofday","tv1","");
    problem_5_6(false);
    if(gettimeofday(&tv2,NULL)<0) perr("gettimeofday","tv2","");
    timersub(&tv2,&tv1,&result); //ERRORS: No errors are defined
    fprintf(stderr,"time: %ld.%06ldsec\n\n",result.tv_sec,result.tv_usec);

    fprintf(stderr, "Problem 6:\n");
    if(gettimeofday(&tv1,NULL)<0) perr("gettimeofday","tv1","");
    problem_5_6(true);
    if(gettimeofday(&tv2,NULL)<0) perr("gettimeofday","tv2","");
    timersub(&tv2,&tv1,&result); //ERRORS: No errors are defined
    fprintf(stderr,"time: %ld.%06ldsec\n\n",result.tv_sec,result.tv_usec);
}



int perr(char* op, char* file, char* para){//error printing and exiting function
    if(errno != 0){
        fprintf(stderr, "Error: Failed to %s [%s] %s: %s\n", op, file, para, strerror(errno));
        exit(errno);
    }
    return 0;
}

void spin_lock(char* lock){//Problem 1
    while(tas(lock)!=0);
}

void spin_unlock(char* lock){//Problem 1
    *lock = 0;
}

void write_seqlock(char* lock, int* counter){//Problem 6
    spin_lock(lock);
    *counter = *counter + 1;
}

void write_sequnlock(char* lock, int* counter){//Problem 6
    *counter = *counter + 1;
    spin_unlock(lock);
}

int read_seqbegin(int* counter){//Problem 6
    while((*counter)%2){
        if(sched_yield()<0) perr("sched_yield","","");
    }
    return *counter;
}

bool read_seqretry(int* counter, int check){//Problem 6
    if(*counter != check){
        *retry += 1;
        return true;
    }
    return false;
}

void problem1(int* balance, char* lock, bool testnum){          //testnum == true: with spinlock
    pid_t pid;                                                  //testnum == false: without spinlock
    int i, cpu;
    for(cpu=0;cpu<CPUnum;cpu++){
        switch(pid=fork()){
            case -1:
                perr("fork","child process","");
                break;
            case 0:
                if(testnum) spin_lock(lock);
                for(i=0;i<1000000;i++)
                    *balance += 1;
                if(testnum) spin_unlock(lock);
                exit(EXIT_SUCCESS);
                break;
        }
    }
    for(cpu=0;cpu<CPUnum;cpu++){
        if(wait(NULL)<0) perr("wait","child process","");
    }

    if(testnum) fprintf(stderr, "\nProblem 1 (With    Spinlock): balance = %d\n", *balance);
    else fprintf(stderr, "Problem 1 (Without Spinlock): balance = %d\n\n", *balance);
}

void *slab_alloc(struct slab *slab){//Problem 2
    int i;
    spin_lock(&slab->lock);
    for(i=0;i<NSLOTS;i++){
        if(slab->freemap[i] == 0){
            slab->freemap[i] = 1;
            spin_unlock(&slab->lock);
            return &slab->slots[i];
        }
    }
    spin_unlock(&slab->lock);
    return NULL;
}

int slab_dealloc(struct slab *slab, void *object){//Problem 2
    int i;
    spin_lock(&slab->lock);
    for(i=0;i<NSLOTS;i++){
        if((&(slab->slots[i])==object)&&(slab->freemap[i]==1)){
            slab->freemap[i] = 0;
            spin_unlock(&slab->lock);
            return 1;
        }
    }
    spin_unlock(&slab->lock);
    return -1;
}

struct dll *dll_insert(struct dll *anchor, int value, struct slab *slab, bool isix){//Problem 4 & 6
    struct dll *new, *cur;

    if(isix) write_seqlock(&slab->lock2, &anchor->value);
    else spin_lock(&slab->lock2);

    if((new=slab_alloc(slab))!=NULL){
        cur = anchor->fwd;
        while((cur != anchor)&&(value > (cur->value))){
            cur = cur->fwd;
        }

        new->value = value;

        new->rev = cur->rev;
        cur->rev->fwd = new;
        new->fwd = cur;
        cur->rev = new;

        if(isix) write_sequnlock(&slab->lock2, &anchor->value);
        else spin_unlock(&slab->lock2);

        return new;
    }

    if(isix) write_sequnlock(&slab->lock2, &anchor->value);
    else spin_unlock(&slab->lock2);

    return NULL;
}

void dll_delete(struct dll *anchor, struct dll *node, struct slab *slab, bool isix){//Problem 4 & 6
    if(isix) write_seqlock(&slab->lock2, &anchor->value);
    else spin_lock(&slab->lock2);

    if((slab_dealloc(slab,node)!=-1)&&(node!=anchor)){
        node->fwd->rev = node->rev;
        node->rev->fwd = node->fwd;
        node->fwd = node;
        node->rev = node;
    }
    if(isix) write_sequnlock(&slab->lock2, &anchor->value);
    else spin_unlock(&slab->lock2);
}

//The slab was added in as an additional parameter for the find function.
//This was because if not, I needed to create the lock inside the dll struct, which would
//make locks for each dll, which would never be used, besides the anchor's
//So ended up making two locks inside the slab and adding a paramter for the find function.

struct dll *dll_find(struct slab *slab, struct dll *anchor, int value){//Problem 4
    struct dll *cur;
    bool dir = true;
    spin_lock(&slab->lock2);
    if(abs(anchor->fwd->value - value) < abs(anchor->rev->value - value)) dir = false;

    if(dir) cur = anchor->fwd;
    else cur = anchor->rev;
    while(cur != anchor){
        if(cur->value == value){
            spin_unlock(&slab->lock2);
            return cur;
        }
        else{
            if(dir){
                if(cur->value > value) break;
                else cur = cur->fwd;
            }
            else{
                if(cur->value < value) break;
                else cur = cur->rev;
            }
        }
    }
    spin_unlock(&slab->lock2);
    return NULL;
}

struct dll *dll_find_seq(struct slab *slab, struct dll *anchor, int value){//Problem 6
    struct dll *cur;
    bool dir = true;
    int check;
    do{
        check = read_seqbegin(&anchor->value);

        if(abs(anchor->fwd->value - value) < abs(anchor->rev->value - value)) dir = false;
        if(dir) cur = anchor->fwd;
        else cur = anchor->rev;

        while(cur != anchor){
            if(cur->value == value){
                if(read_seqretry(&anchor->value,check)) break;
                else return cur;
            }
            else{
                if(dir){
                    if(cur->value > value) break;
                    else cur = cur->fwd;
                }
                else{
                    if(cur->value < value) break;
                    else cur = cur->rev;
                }
            }
        }
    }while(read_seqretry(&anchor->value,check));
    return NULL;
}

void slab_init(struct slab *slab){
    int i;
    for(i=0;i<NSLOTS;i++){
        slab->slots[i].fwd = &slab->slots[i];
        slab->slots[i].rev = &slab->slots[i];
    }
}

void problem_5_6(bool isix){                //isix == true: Problem 6
    struct slab *SLAB;                      //isix == false: Problem 5
    struct dll *temp;
    struct dll *anchor;

    if((SLAB = mmap(NULL,sizeof(struct slab),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0))<0)
        perr("mmap","SLAB","");
    if(isix){
        if((retry = mmap(NULL,sizeof(int),PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0))<0)
            perr("mmap","retry","");
    }
    slab_init(SLAB);
    anchor = slab_alloc(SLAB);

    pid_t pid;
    int cpu, op, task, num;

    for(cpu=1;cpu<=CPUnum;cpu++){
        switch(pid=fork()){
            case -1:
                perr("fork","child process","");
                break;
            case 0:
                srand(cpu);
                for(op=0;op<OPnum;op++){
                    num = rand()%100;
                    switch(task=rand()%2){
                        case 0:
                            dll_insert(anchor,num,SLAB,isix);
                            break;
                        case 1:
                            if(isix) temp=dll_find_seq(SLAB,anchor,num);
                            else temp=dll_find(SLAB,anchor,num);
                            if(temp!=NULL) dll_delete(anchor,temp,SLAB,isix);
                            break;
                    }
                }
                exit(EXIT_SUCCESS);
                break;
        }
    }

    for(cpu=1;cpu<=CPUnum;cpu++){
        if(wait(NULL)<0) perr("wait","child process","");
    }

    if(isix) fprintf(stderr, "Number of RETRY: %d\n", *retry);
    temp = anchor->fwd;
    int j = 1;
    while(temp != anchor){
        fprintf(stderr, "[%d]\t",temp->value);
        j++;
        temp = temp->fwd;
    }
    fprintf(stderr, "\n");

    if(munmap(SLAB,sizeof(struct slab))<0) perr("munmap","SLAB","");
    if(isix){
        if(munmap(retry,sizeof(int))<0) perr("munmap","retry","");
    }
}