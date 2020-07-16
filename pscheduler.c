#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "pscheduler.h"


void processScheduler(char* scheduler, long long int quantum,
node_t** curr_proc, long long int* curr_proc_time, node_t** pre_proc){
    /*if the process is not finished, deduct its job time*/
    if((*curr_proc) && (*curr_proc)->data.job_time > 0){
        (*curr_proc_time)++;
        (*curr_proc)->data.job_time--;
    }
    /*this is to check if the job has reached the quantum in rr case*/
    if(strcmp(scheduler, "rr") == 0 &&
    *curr_proc_time == quantum && (*curr_proc)->data.job_time > 0){
        *pre_proc = *curr_proc;
        *curr_proc = NULL;
        *curr_proc_time = 0;
    }
}

/*check if there is a process arriving at this time*/
void processArrival(node_t** processList,
node_t** arriving, long long int time, node_t** pre_proc){
    if((*processList) && (*processList)->data.arr_time == time){
        node_t* process = pop(processList);
        *arriving = addNode(*arriving, process);
        free(process);
    }
    /*if there is a unfinished process(e.g. reached quantum),
    add it to the end of arrived processes list*/
    if(*pre_proc){
        *arriving = addNode(*arriving, *pre_proc);
        free(*pre_proc);
        *pre_proc = NULL;
    }
    
}

void checkStart(node_t** curr_proc, long long int curr_proc_time,
node_t** arriving, long long int time){
    /*when there is no process running,
    and there are processes waiting for execution*/
    if(!*curr_proc && *arriving && curr_proc_time == 0){
        *curr_proc = pop(arriving);
        printf("%lld, RUNNING, id=%lld, remaining-time=%lld\n", time,
        (*curr_proc)->data.proc_id, (*curr_proc)->data.job_time);
    }
}

void checkFinished(node_t** curr_proc, long long int* curr_proc_time,
node_t* arriving, long long int time){
    /*when a process' job_time == 0, means it has been finished*/
    if(*curr_proc && (*curr_proc)->data.job_time == 0){
        printf("%lld, FINISHED, id=%lld, proc-remaining=%lld\n", time,
        (*curr_proc)->data.proc_id, countListLength(arriving));
        *curr_proc = NULL;
        *curr_proc_time = 0;
    }
}


void checkMemStart(node_t** curr_proc, long long int curr_proc_time,
node_t** arriving, long long int time, long long int* curr_load_time,
long long int total_mem_size, long long int* page_alloced){
    /*similar to the checkStart, but it doesn't do the print
    since it needs the page allocation result in this case*/
    if(!*curr_proc && *arriving && curr_proc_time == 0){
        *curr_proc = pop(arriving);
        *curr_load_time = 0;
        *page_alloced = 0;
        if(*curr_proc && (*curr_proc)->data.mem_size > total_mem_size){
            fprintf(stderr,
            "Error, process requires memory size exceeds limit!\n");
            exit(0);
        }
    }
}

void checkMemFinished(node_t** curr_proc,
long long int* curr_proc_time, node_t* arriving,
 long long int time, pageList_t* pages, page_t** evicted_pages){
    if(*curr_proc && (*curr_proc)->data.job_time == 0){
        /*when a process is finished, evict it from pages*/
        if(!(*evicted_pages)){
            evictProc(pages, *curr_proc, evicted_pages);
        }
        if(*evicted_pages){
            printEvictedPages(time, evicted_pages);
        }
        printf("%lld, FINISHED, id=%lld, proc-remaining=%lld\n",
        time, (*curr_proc)->data.proc_id, countListLength(arriving));
        *curr_proc_time = 0;
        free(*curr_proc);
        *curr_proc = NULL;
    }
}

/*check if the process has loaded for require time*/
long long int loading(long long int* curr_load_time,
node_t* curr_proc, long long int page_alloced){
    if(curr_proc){
        if(*curr_load_time < 2*page_alloced){
            (*curr_load_time)++;
            return 0;
        }else if(*curr_load_time == 2*page_alloced){
            return 1;
        }
    }
    return -1;
}

long long int allocPages(pageList_t* pages, node_t** curr_proc,
char* mem_alloc, long long int time, page_t** evicted_pages){
    long long int proc_page_num = ((*curr_proc)->data.mem_size)/PAGE_SIZE;
    page_t* curr = pages->head;
    long long int page_alloced = 0;
    long long int proc_existed = 0;
    /*check any part of the process already in the pages*/
    while(curr){
        if(curr->proc_id == (*curr_proc)->data.proc_id){
            page_alloced++;
            proc_existed++;
        }
        curr = curr->next;
    }
    /*if not all part of the process in the pages,
    check if there are any empty pages,
    and put the not allocated part of the process into the pages*/
    if(page_alloced < proc_page_num){
        curr = pages->head;
        while(curr){
            if(curr->proc_id < 0 && page_alloced < proc_page_num){
                curr->proc_id = (*curr_proc)->data.proc_id;
                page_alloced++;
            }
            curr = curr->next;
        }
    }
    
    /*if the process is still not all in the pages, try evict others*/
    if(page_alloced < proc_page_num){
        if(strcmp(mem_alloc, "p") == 0){
            swapping(&page_alloced, proc_page_num,
            pages, evicted_pages, *curr_proc);
        }
        if(strcmp(mem_alloc, "v") == 0 || strcmp(mem_alloc, "cm") == 0){
            virtualMemory(&page_alloced, proc_page_num,
            pages, evicted_pages, curr_proc, mem_alloc);
        }
    }
    /*if the process can't get all pages it requires, add penalty
    note: the quantum and memory should be large enough to handle big penalty*/
    if((strcmp(mem_alloc, "v") == 0 || strcmp(mem_alloc, "cm") == 0)
    && page_alloced < proc_page_num){
        (*curr_proc)->data.job_time += (proc_page_num - page_alloced);
    }
    /*if pages are been allocated, the process can start to run*/
    if(page_alloced > 0){
        printStart(pages, evicted_pages, time, *curr_proc,
        page_alloced, proc_existed);
    }
    /*in vm case, return page_alloced - proc_existed for checking load time*/
    if((strcmp(mem_alloc, "v") == 0 || strcmp(mem_alloc, "cm") == 0)
    && page_alloced - proc_existed > 0){
        return page_alloced - proc_existed;
    }
    return page_alloced;
}

void swapping(long long int* page_alloced, long long int proc_page_num,
pageList_t* pages, page_t** evicted_pages, node_t* curr_proc){
    if(*page_alloced < proc_page_num){
        page_t* curr = pages->head;
        long long int max = LONG_MIN;
        long long int proc_to_evict = -1;
        /*loop through pages to find out which process to evict*/
        while(curr){
            if(curr->proc_time > max){
                proc_to_evict = curr->proc_id;
                max = curr->proc_time;
            }
            curr = curr->next;
        }
        /*if a proper process' been found*/
        if(max > 0 && proc_to_evict >= 0){
            page_t* evict_curr = pages->head;
            while(evict_curr){
                /*if not all of the current process been allocated,
                put the rest into pages*/
                if(evict_curr->proc_id == proc_to_evict
                && *page_alloced < proc_page_num){
                    *evicted_pages = addPage(*evicted_pages, evict_curr);
                    evict_curr->proc_id = curr_proc->data.proc_id;
                    evict_curr->proc_time = 0;
                    (*page_alloced)++;
                /*else if current process has all been allocated,
                only evict the found process,
                and set the corresponding pages into unused*/
                }else if(evict_curr->proc_id == proc_to_evict
                && *page_alloced == proc_page_num){
                    *evicted_pages = addPage(*evicted_pages, evict_curr);
                    evict_curr->proc_id = -1;
                    evict_curr->proc_time = 0;
                }
                evict_curr = evict_curr->next;
            }
        }
    }
    /*if the curent process is not all been allocated, repeat the process*/
    if((*page_alloced) < proc_page_num){
        swapping(page_alloced, proc_page_num, pages, evicted_pages, curr_proc);
    }
}

void virtualMemory(long long int* page_alloced,
long long int proc_page_num, pageList_t* pages,page_t** evicted_pages,
node_t** curr_proc, char* mem_alloc){
    /*if process been allocated less than 4 pages, start evict,
    otherwise don't evict*/
    long long int evict_size = 0;
    if(*page_alloced < 4){
        evict_size = 4 - *page_alloced;
    }else{
        return;
    }
    /*initialise page_to_evict, which is used to store page_id
    for pages need to be evicted*/
    long long int page_to_evict[evict_size];
    for(long long int i = 0; i < evict_size; i++){
        page_to_evict[i] = -1;;
    }
    long long int* evict_list = NULL;
    if(strcmp(mem_alloc, "cm") == 0){
        evict_list = findLFUPageToEvict(pages, evict_size,
        page_to_evict, *curr_proc);
    }
    if(strcmp(mem_alloc, "v") == 0){
        evict_list = findLRUPageToEvict(pages, evict_size,
        page_to_evict, *curr_proc);
    }
    
    if(evict_list){
        evictPages(pages, evict_list, evicted_pages, evict_size);
    }
    page_t* page_to_add = pages->head;
    /*allocate the process into the emty pages*/
    while(page_to_add){
        if(page_to_add->proc_id < 0 && *page_alloced <= proc_page_num){
            page_to_add->proc_id = (*curr_proc)->data.proc_id;
            (*page_alloced)++;
        }
        page_to_add = page_to_add->next;
    }
}


double* countStatistics(long long int time,
node_t* curr_proc, double* statistics){
    /*if current process is in current interval,
    add one to the process count of interval*/
    if(time / 60 + 1 <= statistics[2]){
        statistics[3] = statistics[3] + 1;
    }
    /*if current interval has less process than the min,
    min = current interval process num*/
    if(statistics[3] < statistics[0]){
        statistics[0] = statistics[3];
    }
    /*if current interval has more process than the max,
    max = current interval process num*/
    if(statistics[3] > statistics[1]){
        statistics[1] = statistics[3];
    }
    /*if the time comes to a new interval*/
    if(time / 60 + 1 > statistics[2]){
        /*if time jump to next interval,
        means there is a interval has no process completed*/
        if((time / 60 > statistics[2])){
            statistics[0] = 0;
        }
        statistics[2] = time / 60 + 1;
        statistics[3] = 1;
    }
    /*accumulate the turnaround time*/
    double turnaround = time + 1 - curr_proc->data.arr_time;
    statistics[4] = statistics[4] + turnaround;
    /*calculate the time overhead*/
    double over_head = turnaround / (curr_proc->data.total_time);
    if(over_head > statistics[5]){
        statistics[5] = over_head;
    }
    statistics[6] = over_head + statistics[6];
    statistics[7]++;
    return statistics;
}

void printStatistics(double* statistics, long long int time){
    //Throughput: avg, min, max
    long long int avg = statistics[7] / statistics[2];
    if((long long int)statistics[7] % (long long int)statistics[2] != 0){
        avg++;
    }
    printf("Throughput %lld, %lld, %lld\n", avg,
    (long long int)statistics[0], (long long int)statistics[1]);
    long long int turnaround = (long long int)(statistics[4]/statistics[7]);
    if(((long long int)statistics[4] % (long long int)statistics[7]) != 0){
        turnaround++;
    }
    printf("Turnaround time %lld\n", turnaround);
    //Time overhead: max, avg
    printf("Time overhead %.2f %.2f\n",
    statistics[5], statistics[6]/statistics[7]);
    printf("Makespan %lld\n", time-1);
}
