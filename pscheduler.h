#include "list.h"


void processScheduler(char* scheduler, long long int quantum,
node_t** curr_proc, long long int* curr_proc_time, node_t** pre_proc);
void processArrival(node_t** processList, node_t** arriving,
long long int time, node_t** pre_proc);
void checkStart(node_t** curr_proc, long long int curr_proc_time,
node_t** arriving, long long int time);
void checkMemStart(node_t** curr_proc, long long int curr_proc_time,
node_t** arriving, 
long long int time, long long int* curr_load_time,
long long int total_mem_size, long long int* page_alloced);
void checkFinished(node_t** curr_proc, long long int* curr_proc_time,
node_t* arriving, long long int time);
void checkMemFinished(node_t** curr_proc,
long long int* curr_proc_time, node_t* arriving,
long long int time, pageList_t* pages, page_t** evicted_pages);
long long int loading(long long int* curr_load_time,
node_t* curr_proc, long long int page_alloced);
long long int allocPages(pageList_t* pages, node_t** curr_proc,
char* mem_alloc, long long int time, page_t** evicted_pages);
void swapping(long long int* page_alloced, long long int proc_page_num,
pageList_t* pages, page_t** evicted_pages, node_t* curr_proc);
void virtualMemory(long long int* page_alloced,
long long int proc_page_num, pageList_t* pages,
page_t** evicted_pages, node_t** curr_proc, char* mem_alloc);
double* countStatistics(long long int time,
node_t* curr_proc, double* statistics);
void printStatistics(double* statistics, long long int time);