#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pscheduler.h"


void main(long long int argc, char** argv){
    char* proc_file = NULL;
    char* sch_alg = NULL;
    long long int quantum = 10;
    char* mem_alloc = NULL;
    long long int total_mem_size = 0;
    /*check the flags to check cases*/
    for(long long int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-f") == 0){
            proc_file = argv[i+1];
        }
        if(strcmp(argv[i], "-a") == 0){
            sch_alg = argv[i+1];
        }
        if(strcmp(argv[i], "-q") == 0){
            quantum = atoi(argv[i+1]);
        }
        if(strcmp(argv[i], "-m") == 0){
            mem_alloc = argv[i+1];
        }
        if(strcmp(argv[i], "-s") == 0){
            total_mem_size = atoi(argv[i+1]);
        }
    }
    /*open the case file, read each line and put them into a format of list*/
    FILE* processes;
    char line[100];
    processes = fopen(proc_file, "r");
    if(processes == NULL) {
        fprintf(stderr, "Error opening file");
        exit(1);
    }
    node_t* processList = NULL;
    while(fgets(line, 100, processes) != NULL){
        if(processList == NULL){
            processList = createNode(line);
        }else{
            node_t* temp = createNode(line);
            processList = addNode(processList, temp);
            free(temp);
        }
    }
    fclose(processes);
    sort(processList);
    node_t* arriving = NULL;
    node_t* curr_proc = NULL;
    node_t* pre_proc = NULL;
    page_t* evicted_pages = NULL;
    long long int time = 0;
    long long int curr_proc_time = 0;
    long long int mem_manager = 1;
    long long int curr_load_time = 0;
    pageList_t* pages = initPages(total_mem_size);
    long long int check_load = -1;
    if(!mem_alloc || (strcmp(mem_alloc, "u") == 0)){
        mem_manager = -1;
    }
    /*[min proc_num, max proc_num, interval, interval proc_num,
    Total turnaround time, max overhead, total overhead, proc_num]*/
    double statistics[8] = {LONG_MAX, LONG_MIN, 1, 0, 0, LONG_MIN, 0, 0};
    double* result = NULL;
    /*cases when memory management not required*/
    if(mem_manager < 0){
        while(processList || arriving || curr_proc){
            processArrival(&processList, &arriving, time, &pre_proc);
            if(strcmp(sch_alg, "cs") == 0){
                findQuickestProc(arriving);
            }
            checkFinished(&curr_proc, &curr_proc_time, arriving, time);
            checkStart(&curr_proc, curr_proc_time, &arriving, time);
            processScheduler(sch_alg, quantum, &curr_proc,
            &curr_proc_time, &pre_proc);
            if(curr_proc && curr_proc->data.job_time == 0){
                result = countStatistics(time, curr_proc, statistics);
            }
            time++;
        }
    }else{
        long long int curr_load_time = 0;
        long long int check_load = -1;
        long long int page_alloced = 0;
        long long int page_existed = 0;
        while(processList || arriving || curr_proc || pre_proc){
            processArrival(&processList, &arriving, time, &pre_proc);
            if(strcmp(sch_alg, "cs") == 0){
                findQuickestProc(arriving);
            }
            if(strcmp(mem_alloc, "cm") == 0){
                findSmallestProc(arriving);
            }
            checkMemFinished(&curr_proc, &curr_proc_time, arriving,
            time, pages, &evicted_pages);
            checkMemStart(&curr_proc, curr_proc_time, &arriving,
            time, &curr_load_time, total_mem_size, &page_alloced);
            /*if pages are not been allocated*/
            if(page_alloced == 0 && curr_proc){
                /*page_existed is the part of process
                already in the memory before allocation*/
                page_existed = checkProcInPages(pages, curr_proc);
                page_alloced = allocPages(pages, &curr_proc,
                mem_alloc, time, &evicted_pages);
            }
            if(page_existed >= 0){
                check_load = loading(&curr_load_time,
                curr_proc, page_alloced);
            }
            /*if loading finished*/
            if(check_load == 1){
                processScheduler(sch_alg, quantum,
                &curr_proc, &curr_proc_time, &pre_proc);
            }
            /*count the statistics everytime*/
            if(curr_proc && curr_proc->data.job_time == 0){
                result = countStatistics(time, curr_proc, statistics);
            }
            pagesClock(pages);
            pagesFreqCount(pages);
            time++;
        }
        freePages(pages);
    }
    if(result){
        printStatistics(result, time);
    }
}
