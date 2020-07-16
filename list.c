#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "list.h"

/*add a new node into process list*/
node_t* addNode(node_t* head, node_t* newNode){
    if(!head && newNode){
        head = malloc(sizeof(node_t));
        head->data = newNode->data;
        head->next = NULL;
        return head;
    }
    /*find the end of the list*/
    node_t* curr = head;
    while(curr->next != NULL){
        curr = curr->next;
    }
    /*add the new node*/
    curr->next = malloc(sizeof(node_t));
    curr->next->data = newNode->data;
    curr->next->next = NULL;
    return head;
}


/*similar to the addNode, but add a page to the end of page list*/
page_t* addPage(page_t* head, page_t* newPage){
    if(!head && newPage){
        head = malloc(sizeof(page_t));
        head->page_id = newPage->page_id;
        head->proc_id = -1;
        head->next = NULL;
        return head;
    }
    page_t* curr = head;
    while(curr->next != NULL){
        curr = curr->next;
    }
    curr->next = malloc(sizeof(page_t));
    curr->next->page_id = newPage->page_id;
    curr->next->proc_id = -1;
    curr->next->next = NULL;
    return head;
}

/*create a new node with data read through line*/
node_t* createNode(char* line){
    node_t* newNode = malloc(sizeof(node_t));
    assert(newNode != NULL);
    newNode->data.arr_time = atoi(strtok(line, BLANK));
    newNode->data.proc_id = atoi(strtok(NULL, BLANK));
    newNode->data.mem_size = atoi(strtok(NULL, BLANK));
    newNode->data.job_time = atoi(strtok(NULL, BLANK));
    newNode->data.total_time = newNode->data.job_time;
    newNode->next = NULL;
    line = NULL;
    return newNode;
}

/*pop out a node from the process list*/
node_t* pop(node_t** head){
    node_t* newNode = NULL;
    if(head){
        newNode = *head;
        *head = newNode->next;
        newNode->next = NULL;
    }
    return newNode;
}

/*sort the arrival list, with job time small first,
for same job time, smaller id first*/
void findQuickestProc(node_t* head){
    if(head == NULL){
        return;
    }
    /*list only contains 1 node. no need to sort*/
    if(head->next == NULL){
        return;
    }
    long long int min = LONG_MAX;
    node_t* min_proc = NULL;
    node_t* curr = head;
    while(curr){
        if(curr->data.job_time < min){
            min = curr->data.job_time;
            min_proc = curr;
        }
        curr = curr->next;
    }
    if(min_proc){
        swap(min_proc, head);
    }
}

/*sort the process list, with arrval time small first,
for processes with same arrival time, proc_id smaller goes first*/
void sort(node_t* head){
    if(head == NULL){
        fprintf(stderr, "Error, list to sort not exist!");
        exit(0);
    }
    /*list only contains 1 node. no need to sort*/
    if(head->next == NULL){
        return;
    }
    node_t* curr = head;
    node_t* nextNode = head->next;
    while(nextNode != NULL){
        if(curr->data.arr_time > nextNode->data.arr_time){
            swap(curr, nextNode);
            sort(head);
        }
        if(curr->data.arr_time == nextNode->data.arr_time){
            if(curr->data.proc_id > nextNode->data.proc_id){
                swap(curr, nextNode);
                sort(head);
            }
        }
        curr = curr->next;
        nextNode = nextNode->next;
    }
}

/*swap two nodes in the process list*/
void swap(node_t* curr, node_t* nextNode){
    data_t temp_data;
    temp_data = curr->data;
    curr->data = nextNode->data;
    nextNode->data = temp_data;
}

/*count the length of a given process list*/
long long int countListLength(node_t* list){
    node_t* curr = list;
    long long int i = 0;
    while(list){
        i++;
        list = list->next;
    }
    return i;

}

/*initialise the page list with given memory size*/
pageList_t* initPages(long long int mem_size){
    pageList_t* pages = malloc(sizeof(pageList_t));
    assert(pages != NULL);
    pages->head = NULL;
    pages->tail = NULL;
    pages->page_size = 0;
    page_t* curr = NULL;
    for(long long int i = 0; i < mem_size/4; i++){
        if(!curr && pages->page_size == 0){
            pages->page_size++;
            pages->head = malloc(sizeof(page_t));
            assert(pages->head != NULL);
            pages->head->page_id = i;
            pages->head->proc_id = -1;
            pages->head->proc_time = 0;
            pages->head->proc_freq = 0;
            pages->head->next = NULL;
            pages->tail = pages->head;
            curr = pages->head;
        }else if(curr){
            pages->page_size++;
            curr->next = malloc(sizeof(page_t));
            assert(curr->next != NULL);
            curr->next->page_id = i;
            curr->next->proc_id = -1;
            curr->next->proc_time = 0;
            curr->next->proc_freq = 0;
            curr->next->next = NULL;
            pages->tail = curr->next;
            curr = curr->next;
        }
    }
    return pages;
}

/*free the page list*/
void freePages(pageList_t* pages){
    page_t* curr = pages->head;
    page_t* pre = NULL;
    while(curr){
        pre = curr;
        curr = curr->next;
        free(pre);
    }
    free(pages);
}

/*print the list of evicted pages*/
void printEvictedPages(long long int time, page_t** evicted_pages){
    printf("%lld, EVICTED, mem-addresses=[", time);
    while(*evicted_pages){
        if(*evicted_pages){
            printf("%lld", (*evicted_pages)->page_id);
        }
        if((*evicted_pages)->next){
            printf(",");
        }
        page_t* temp = *evicted_pages;
        *evicted_pages = (*evicted_pages)->next;
        free(temp);
    }
    printf("]\n");
}

/*print the list of given pages*/
void printPages(pageList_t* pages, node_t* curr_proc){
    page_t* curr = pages->head;
    long long int page_num = 0;
    while(curr){
        if(curr->proc_id == curr_proc->data.proc_id){
            page_num++;
        }
        curr = curr->next;
    }
    curr = pages->head;
    while(curr){
        if(curr->proc_id == curr_proc->data.proc_id){
            printf("%lld", curr->page_id);
            page_num--;
            if(curr->next && page_num > 0){
                printf(",");
            }
        }
        curr = curr->next;
    }
}

/*print the start of a process*/
void printStart(pageList_t* pages, page_t** evicted_pages,
long long int time, node_t* curr_proc, long long int page_alloced,
long long int proc_existed){
    long long int full_pages = checkFullPages(pages);
    if(*evicted_pages){
        printEvictedPages(time, evicted_pages);
    }
    float mem_percent = ((float)(full_pages)/pages->page_size)*100.0;
    long long int round = (full_pages*100)/pages->page_size;
    if(mem_percent - round > 0){
        round++;
    }
    long long int load_time = 2*(page_alloced - proc_existed);
    printf("%lld, RUNNING, id=%lld, remaining-time=%lld, load-time=%lld", 
    time, curr_proc->data.proc_id, curr_proc->data.job_time, load_time);
    printf(", mem-usage=%lld%%, mem-addresses=[", round);
    printPages(pages, curr_proc);
    printf("]\n");
}

/*evict the process from the page list*/
void evictProc(pageList_t* pages, node_t* curr_proc, page_t** evicted_pages){
    page_t* evict_curr = pages->head;
    while(evict_curr){
        if(evict_curr->proc_id == curr_proc->data.proc_id){
            *evicted_pages = addPage(*evicted_pages, evict_curr);
            evict_curr->proc_id = -1;
            evict_curr->proc_time = 0;
        }
        evict_curr = evict_curr->next;
    }
}

/*evict all the pages from page list with page_id in the evict list*/
void evictPages(pageList_t* pages, long long int* evict_list,
page_t** evicted_pages, long long int list_size){
    page_t* evict_curr = pages->head;
    while(evict_curr){
        for(long long int i = 0; i < list_size; i++){
            if(evict_curr->page_id == evict_list[i]){
                *evicted_pages = addPage(*evicted_pages, evict_curr);
                evict_curr->proc_id = -1;
            }
        }
        evict_curr = evict_curr->next;
    }
}

/*accumulate running time for each of the pages*/
void pagesClock(pageList_t* pages){
    page_t* curr_page = pages->head;
    while(curr_page){
        if(curr_page->proc_id != -1){
            (curr_page->proc_time)++;
        }
        curr_page = curr_page->next;
    }
}

/*accumulate frequency of visiting for each of the pages*/
void pagesFreqCount(pageList_t* pages){
    page_t* curr_page = pages->head;
    while(curr_page){
        if(curr_page->proc_id != -1){
            (curr_page->proc_freq)++;
        }
        curr_page = curr_page->next;
    }
}

/*check how many memory already given to the current process*/
long long int checkProcInPages(pageList_t* pages, node_t* curr_proc){
    long long int proc_page_num = (curr_proc->data.mem_size)/PAGE_SIZE;
    long long int page_count = 0;
    page_t* curr_page = pages->head;
    while(curr_page){
        if(curr_page->proc_id == curr_proc->data.proc_id){
            page_count++;
            curr_page->proc_freq++;
        }
        curr_page = curr_page->next;
    }
    if(page_count == proc_page_num){
        return -1;
    }
    return proc_page_num - page_count;
}

/*count how many pages are full in the page list*/
long long int checkFullPages(pageList_t* pages){
    long long int full_pages = 0;
    page_t* curr = pages->head;
    while(curr){
        if(curr->proc_id != -1){
            full_pages++;
        }
        curr = curr->next;
    }
    return full_pages;
}

long long int* findLRUPageToEvict(pageList_t* pages,
long long int evict_size, long long int* page_to_evict, node_t* curr_proc){
    page_t* curr_page = pages->head;
    long long int max = LONG_MIN;
    /*find out the page with max running time*/
    while(curr_page){
        if(curr_page->proc_time > max &&
        curr_page->proc_id != curr_proc->data.proc_id){
            max = curr_page->proc_time;
        }
        curr_page = curr_page->next;
    }
    curr_page = pages->head;
    long long int i = 0;
    while(curr_page && i < evict_size){
        /*if the current page need to be evicted*/
        if(curr_page->proc_time == max){
            /*if the current page_to_evict is full, move to next*/
            if(page_to_evict[i] > 0){
                i++;
                continue;
            }
            /*set current page time to 0 and add it to the evict list*/
            curr_page->proc_time = 0;
            page_to_evict[i] = curr_page->page_id;
            i++;
        }
        curr_page = curr_page->next;
    }
    /*if pages evicted not enough, repeat this process*/
    if(i < evict_size){
        findLRUPageToEvict(pages, evict_size, page_to_evict, curr_proc);
    }
    return page_to_evict;
}


long long int* findLFUPageToEvict(pageList_t* pages, long long int evict_size,
long long int* page_to_evict, node_t* curr_proc){
    page_t* curr_page = pages->head;
    long long int max = LONG_MIN;
    /*find out the page with max running time*/
    while(curr_page){
        if(curr_page->proc_freq > max &&
        curr_page->proc_id != curr_proc->data.proc_id){
            max = curr_page->proc_freq;
        }
        curr_page = curr_page->next;
    }
    curr_page = pages->head;
    long long int i = 0;
    while(curr_page && i < evict_size){
        /*if the current page need to be evicted*/
        if(curr_page->proc_freq == max){
            /*if the current page_to_evict is full, move to next*/
            if(page_to_evict[i] > 0){
                i++;
                continue;
            }
            /*accumulate current page frequency and add it to the evict list*/
            curr_page->proc_freq = 0;
            page_to_evict[i] = curr_page->page_id;
            i++;
        }
        curr_page = curr_page->next;
    }
    /*if pages evicted not enough, repeat this process*/
    if(i < evict_size){
        findLFUPageToEvict(pages, evict_size, page_to_evict, curr_proc);
    }
    return page_to_evict;
}

/*check if current process already exists in memory*/
long long int checkProcInMem(pageList_t* pages, node_t* proc){
    page_t* curr = pages->head;
    while(curr && proc){
        if(curr->proc_id == proc->data.proc_id){
            return 1;
        }
        curr = curr->next;
    }
    return -1;
}


void findSmallestProc(node_t* head){
    if(head == NULL){
        return;
    }
    /*list only contains 1 node. no need to sort*/
    if(head->next == NULL){
        return;
    }
    long long int min = LONG_MAX;
    node_t* min_proc = NULL;
    node_t* curr = head;
    while(curr){
        if(curr->data.mem_size < min){
            min_proc = curr;
            min = curr->data.mem_size;
        }
        curr = curr->next;
    }
    if(min_proc){
        swap(min_proc, head);
    }
}
