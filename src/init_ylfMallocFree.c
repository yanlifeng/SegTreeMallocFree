#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
//#include <mpi.h>



struct segTree {
    int *tree;
    int seg_size;
    int N;
    long *start_address;
    long *end_address;
};

#define lengthNum  14
#define maxEdgeListSize 100

//#define ZZ_malloc malloc
#define ZZ_malloc __real_malloc

//#define ZZ_free free
#define ZZ_free __real_free

static int bindLength[lengthNum] = {1 << 3, 1 << 4, 1 << 5, 1 << 6, 1 << 7, 1 << 8, 1 << 9, 1 << 10, 1 << 11, 1 << 12,
                                    1 << 13, 1 << 14, 1 << 15, 1 << 16};

int hasInit = 0;

struct segTree *lists[lengthNum][maxEdgeListSize];
int listsSize[lengthNum];

int nextListsSize[lengthNum];

struct segTree *buildSetTree(int bind_length, int seg_size) {
    void *new_address = ZZ_malloc(bind_length * seg_size);
    struct segTree *now_tree = (struct segTree *) ZZ_malloc(sizeof(struct segTree));
    now_tree->start_address = (long *) new_address;
    now_tree->end_address = (long *)((char *)new_address + bind_length * seg_size);
    //int real_size = 1 << __lg(seg_size + 5) + 1;
    //TODO
    now_tree->N = seg_size - 1;
    now_tree->seg_size = seg_size;
    int tree_size = seg_size << 1;
    now_tree->tree = (int *) ZZ_malloc(tree_size * sizeof(int));
    for (int i = 1; i <= seg_size; ++i)
        now_tree->tree[i + now_tree->N] = 0;
    for (int i = now_tree->N; i; --i)
        now_tree->tree[i] = now_tree->tree[i << 1] + now_tree->tree[i << 1 | 1];
    return now_tree;
}

void updateTree(struct segTree *now_tree, int pos, int value) {
    for (int i = pos + now_tree->N; i; i >>= 1)
        now_tree->tree[i] += value;
}

int queryOne(struct segTree *now_tree, int pos) {
    return now_tree->tree[pos + now_tree->N];
}

int queryTree(struct segTree *now_tree) {
    int l = 1, r = now_tree->seg_size;
    if (now_tree->tree[1] == now_tree->seg_size) return -1;
    for (int i = 1; l != r;) {
        int mid = (l + r) / 2;
        if (now_tree->tree[i << 1] < (mid - l + 1)) {
            r = mid;
            i = i << 1;
        } else if (now_tree->tree[i << 1 | 1] < (r - mid)) {
            l = mid + 1;
            i = i << 1 | 1;
        } else {
            assert(0);
        }
    }
    return l;
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

void *getAddress(struct segTree *now_tree, int bind_length, int seg_pos) {
    assert(seg_pos >= 1);
    return (void *) ((char*)now_tree->start_address + bind_length * (seg_pos - 1));
}

int getSegPos(struct segTree *now_tree, int bind_length, void *free_address) {
    long p1 = (long *)free_address;
    long p2 = now_tree->start_address;
    return (p1 - p2) /bind_length + 1;
}

void* __attribute__((noinline)) __wrap_malloc(size_t size) {
//void *__attribute__((noinline)) mallocc(size_t size) {

    //init
    if (hasInit == 0) init();
//    printf("size init : %zukb\n", size / 1024);

    if (size > (1 << 16) || size < 8) return ZZ_malloc(size);


    size_t real_size = 1 << (size_t) ceil(log2(size));
    int length_type = (size_t) ceil(log2(size)) - 3;
    //printf("size %zu\n", size);
//    printf("real_size %zu\n", real_size);
//    printf("length type %d\n", length_type);

    // Look in the spaces already applied for
    int now_tree_num = listsSize[length_type];
    int find_pos = -1;
    int first_zero_pos = -1;
    for (int i = 0; i < now_tree_num; i++) {
        struct segTree *now_tree = lists[length_type][i];
        first_zero_pos = queryTree(now_tree);
        if (first_zero_pos != -1) {
            find_pos = i;
            break;
        }
    }
    // The trees that are already occupied, open a new one
    if (find_pos == -1) {
        struct segTree *now_tree = buildSetTree(bindLength[length_type], nextListsSize[length_type]);
        nextListsSize[length_type] = nextListsSize[length_type] << 1;
        lists[length_type][listsSize[length_type]++] = now_tree;
        assert(listsSize[length_type] <= maxEdgeListSize);
        find_pos = listsSize[length_type] - 1;
        first_zero_pos = 1;
    }
    assert(find_pos != -1 && first_zero_pos != -1);

    // Find the first_zero_pos of the find_pos tree which is 0, return this address and modify the tree
    void *res_address = getAddress(lists[length_type][find_pos], bindLength[length_type], first_zero_pos);
    int aaa = queryOne(lists[length_type][find_pos], first_zero_pos);
    assert(aaa == 0);
    updateTree(lists[length_type][find_pos], first_zero_pos, 1);

    return res_address;
}


void __attribute__((noinline)) __wrap_free(void *ptr) {
//void __attribute__((noinline)) freee(void *ptr) {

    //init
    if (hasInit == 0) init();
//    printf("now free %p\n", ptr);
    //TODO
    int pos1 = -1;
    int pos2 = -1;
    for (int i = 0; i < lengthNum; i++) {
        int now_tree_num = listsSize[i];
        for (int j = 0; j < now_tree_num; j++) {
            struct segTree *now_tree = lists[i][j];
            long p1 = lists[i][j]->start_address;
            long p2 = lists[i][j]->end_address;
            long p3 = ptr;
            if (p3 >= p1 && p3 < p2) {
                pos1 = i;
                pos2 = j;
                break;
            }
        }
        if(pos1 != -1) break;
    }
    if (pos1 == -1 && pos2 == -1) {
        ZZ_free(ptr);
        return;
    }
    int free_seg_pos = getSegPos(lists[pos1][pos2], bindLength[pos1], ptr);
//    printf(" --- free_seg_pos %d\n", free_seg_pos);
    updateTree(lists[pos1][pos2], free_seg_pos, -1);
}

