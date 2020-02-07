#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

void run_test(int num_proc, int nice_val, int test_time) {
    int i, wstatus, pid, ndn_pid; // non-default nice pid
    long tot_usec, ndn_usec;
    struct rusage ndn_ru, tot_ru;

    for(i=0; i<num_proc; i++) {
        switch(pid=fork()) {
            case -1:
                dprintf(2, "err fork\n");
                exit(EXIT_FAILURE);
                break;
            case 0:
                if(!i) {
                    errno = 0;
                    if(nice(nice_val)<0 && errno) {
                        dprintf(2, "err setting nice\n");
                        // TODO: make this exit parent as well
                        exit(EXIT_FAILURE);
                    }
                }
                while(1);
            default:
                if(!i)
                    ndn_pid = pid;
        }
    }

    // since this is called after forks began, total time may be slightly larger
    // than expected total time (i.e., (# processors)*(expected time))
    if(sleep(test_time)) {
        dprintf(2, "err sleep interrupted. angery!\n");
        exit(EXIT_FAILURE);
    }

    // ignore sigterm in parent
    signal(SIGTERM, SIG_IGN);

    // get ndn pid; unfortunately, may cause this to exit slightly earlier than
    // others;
    kill(ndn_pid, SIGTERM);
    waitpid(ndn_pid, &wstatus, 0);
    getrusage(RUSAGE_CHILDREN, &ndn_ru);

    // kill rest of children and get rusage
    kill(0, SIGTERM);
    for(i=1; i<num_proc; i++)
        wait(&wstatus);
    getrusage(RUSAGE_CHILDREN, &tot_ru);

    ndn_usec = (ndn_ru.ru_utime.tv_sec+ndn_ru.ru_stime.tv_sec)*1000000
               +ndn_ru.ru_utime.tv_usec+ndn_ru.ru_stime.tv_usec;
    tot_usec = (tot_ru.ru_utime.tv_sec+ndn_ru.ru_stime.tv_sec)*1000000
               +tot_ru.ru_utime.tv_usec+ndn_ru.ru_stime.tv_usec;
    dprintf(2, "=====\nnicetest.c\n=====\n"
               "Total CPU time:\t%ldus\n"
               "Task0 CPU time:\t%ldus\n"
               "Task0 CPU %:\t%lf%\n",
            tot_usec, ndn_usec, ((double)ndn_usec)/tot_usec*100);
}

int main(int argc, char **argv) {
    // time in seconds, nice values from -20 to 19
    // nice values will be validated; time (in seconds) will only checked to be
    // positive but should be a value >5 to see discernible results
    int num_proc, nice_val, test_time;

    if(argc<3) {
        dprintf(2, "Usage: nicetest [num_proc] [nice_val] [test_time]\n");
        exit(EXIT_FAILURE);
    }

    num_proc = atoi(argv[1]);
    if(num_proc<1) {
        dprintf(2, "num_proc must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }

    nice_val = atoi(argv[2]);
    if(nice_val<-20 || nice_val>19) {
        dprintf(2, "nice number be integer in range [-20,19].\n");
        exit(EXIT_FAILURE);
    }

    test_time = atoi(argv[3]);
    if(test_time<0) {
        dprintf(2, "time must be a positive integer.\n");
        exit(EXIT_FAILURE);
    }

    run_test(num_proc, nice_val, test_time);
    exit(EXIT_SUCCESS);
}