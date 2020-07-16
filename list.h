#define BLANK " "
#define PAGE_SIZE 4
#define LONG_MAX 9223372036854775807
#define LONG_MIN -9223372036854775807

typedef struct data{
    long long int arr_time;
    long long int proc_id;
    long long int mem_size;
    long long int job_time;
    long long int total_time;
}data_t;

typedef struct node{
    data_t data;
    struct node* next;
} node_t;

typedef struct page{
    long long int page_id;
    long long int proc_id;
    long long int proc_time;
    long long int proc_freq;
    struct page* next;
} page_t;

typedef struct page_list{
    page_t* head;
    page_t* tail;
    long long int page_size;
} pageList_t;




node_t* addNode(node_t* head, node_t* newNode);
page_t* addPage(page_t* head, page_t* newPage);
node_t* createNode(char* line);
node_t* pop(node_t** head);
void findQuickestProc(node_t* head);
void findSmallestProc(node_t* head);
void sort(node_t* head);
void swap(node_t* curr, node_t* nextNode);
long long int countListLength(node_t* list);
pageList_t* initPages(long long int mem_size);
void freePages(pageList_t* pages);
void printEvictedPages(long long int time, page_t** evicted_pages);
void printPages(pageList_t* pages, node_t* curr_proc);
void printStart(pageList_t* pages, page_t** evicted_pages, long long int time,
node_t* curr_proc, long long int page_alloced, long long int proc_existed);
void evictProc(pageList_t* pages, node_t* curr_proc, page_t** evicted_pages);
void evictPages(pageList_t* pages, long long int* evict_list,
page_t** evicted_pages, long long int list_size);
void pagesClock(pageList_t* pages);
void pagesFreqCount(pageList_t* pages);
long long int checkProcInPages(pageList_t* pages, node_t* curr_proc);
long long int checkFullPages(pageList_t* pages);
long long int* findLRUPageToEvict(pageList_t* pages, long long int evict_size,
long long int* page_to_evict, node_t* curr_proc);
long long int* findLFUPageToEvict(pageList_t* pages, long long int evict_size,
long long int* page_to_evict, node_t* curr_proc);
long long int checkProcInMem(pageList_t* pages, node_t* proc);