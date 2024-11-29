
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h> 
#include <string.h>

#include <mpi.h>
#include <assert.h>

//#define assert(x)



struct segTree {
    int *tree;
    int seg_size;
    int N;
    void *start_address;
    void *end_address;
};

#define lengthNum  14
#define maxEdgeListSize 100



static int bindLength[lengthNum] = {1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7, 1 << 8, 1 << 9, 1 << 10, 1 << 11, 1 << 12,
    1 << 13, 1 << 14, 1 << 15, 1 << 16};

int hasInit = 0;

struct segTree *lists[lengthNum][maxEdgeListSize];
int listsSize[lengthNum];

int nextListsSize[lengthNum];

#define maxm 100
int used[maxm];
void* addss;

struct segTree *buildSetTree(int bind_length, int seg_size) {
    void *new_address = __real_malloc(1ll * bind_length * seg_size);
    assert(new_address);
    struct segTree *now_tree = (struct segTree *) __real_malloc(sizeof(struct segTree));
    now_tree->start_address = new_address;
    long p1 = now_tree->start_address;
    now_tree->end_address = (void *)(p1 + 1ll * bind_length * seg_size);
    now_tree->seg_size = seg_size;
    int tree_size = seg_size << 1;
    now_tree->tree = (int *) __real_malloc(tree_size * sizeof(int));
    for(int i = 0; i < tree_size; i++) now_tree->tree[i] = -1;
    for (int i = 1; i <= seg_size; ++i)
        now_tree->tree[i] = 0;
    return now_tree;
}

void updateTree(struct segTree *now_tree, int pos, int value) {
    now_tree->tree[pos] += value;
}

int queryOne(struct segTree *now_tree, int pos) {
    return now_tree->tree[pos];
}

int queryTree(struct segTree *now_tree) {
    int res = -1;
    for(int i = 1; i <= now_tree->seg_size; i++) {
        if(now_tree->tree[i] == 0) {
            res = i;
            break;
        }
    }
    return res;
}

void init() {
    for (int i = 0; i < lengthNum; i++) {
        listsSize[i] = 0;
        struct segTree *now_tree = buildSetTree(bindLength[i], 8);
        lists[i][listsSize[i]++] = now_tree;
        nextListsSize[i] = 8 << 1;
    }
    hasInit = 1;
}

void *getAddress2(int seg_pos) {
    long p1 = addss;
    long add_len = 1ll * 1024 * seg_pos;
    return (void*) (p1 + add_len);
}

int getSegPos2(void *free_address) {
    long p1 = free_address;
    long p2 = addss;
    long p3 = p1 + 1ll * 1024 * maxm;
    if(p1 < p2 || p1 >= p3) return -1;
    assert(p1 >= p2 && p1 < p3);
    assert((p1 - p2) % 1024 == 0);
    return (p1 - p2) / 1024;
}


void* mannual_malloc(size_t size) {

    static int cnt = 0;
    if(cnt == 0) {
        for(int i = 0; i < maxm; i++) used[i] = 0;
        addss = __real_malloc(maxm * 1024);
    }
    cnt++;
    
    for(int i = 0; i < maxm; i++) {
        if(used[i] == 0) {
            used[i] = 1;
            return getAddress2(i);
        }
    }
    return NULL;
}



void* __attribute__((noinline)) __wrap_malloc(size_t size) {
    void *ptr;
    if (size > (1 << 10) || size < 8) ptr = __real_malloc(size);
    //if (1) ptr = __real_malloc(size);
    else ptr = mannual_malloc(size);
    if(ptr == NULL) ptr = __real_malloc(size);
    return ptr;
}


void __attribute__((noinline)) __wrap_free(void *ptr) {
    int pos = getSegPos2(ptr);
    if(pos == -1) __real_free(ptr);
    else used[pos] = 0;
}

void* __attribute__((noinline)) __wrap_realloc(void* ptr, size_t size) {
    int pos = getSegPos2(ptr);
    if(pos == -1) return __real_realloc(ptr, size);
    if(size > 1024) {
        used[pos] = 0;
        void* new_ptr = __real_malloc(size);
        memcpy(new_ptr, ptr, 1024);
        return new_ptr;
    } else {
        // if size <= old_size
        // if old_size <= size <= 1024
        return getAddress2(pos);
    }
}


//void* __attribute__((noinline)) __wrap_calloc(size_t nmemb, size_t size) {
//    void *ptr = __real_calloc(nmemb, size);
//    assert(ptr);
//    return ptr;
//}
//void* __attribute__((noinline)) __wrap_valloc(size_t size) {
//    int alignment = 8192; // page_size
//    void *ptr = __real_malloc(size + alignment - 1);
//    // void *ptr = __real_valloc(size);
//    assert(ptr);
//    // ptr = (void*) (((long) ptr + alignment - 1) / alignment * alignment);
//    return ptr;
//}
//
//int __attribute__((noinline)) __wrap_posix_memalign(void **memptr, size_t alignment, size_t size) {
//
//    int ret = __real_posix_memalign(memptr, alignment, size);
//    assert(!ret);
//    return ret;
//}


