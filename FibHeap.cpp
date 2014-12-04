#include <iostream>
#include <cmath>
#include "FibHeap.h"

using namespace std;

static unsigned id = 0;

FibHeap::~FibHeap()
{
    if (min)
        FibDeleteHeap(min);
}

void
FibHeap::FibDeleteHeap(FibNodePtr x)
{
    FibNodePtr ptr = x;

    if (ptr) {
        do {
            FibNodePtr tmp = ptr;
            ptr = ptr->right;
            FibDeleteHeap(tmp->child);
            delete tmp;
        } while (ptr != x);
    }
}

inline void
FibHeap::FibMoveToRoot(FibNodePtr node)
{
    FibNodePtr min = this->min;

    node->left = min->left;
    node->right = min;
    min->left->right = node;
    min->left = node;
}

int
FibHeap::FibInsertNode(FibNodePtr node)
{
    FibNodePtr heap_min = this->min;

    if (!heap_min) {
        this->min = node;
    } else {
        this->FibMoveToRoot(node);
        if (node->key < heap_min->key)
            this->min = node;
    }

    this->numNodes++;
    return 0;
}

FibNodePtr
FibHeap::FibCreateNode(int key)
{
    FibNodePtr node = new(nothrow) FibNode;
    if (node) {
        node->id = id++;
        node->key = key;
        node->degree = 0;
        node->mark = 0;
        node->left = node;
        node->right = node;
        node->child = nullptr;  // explicit init due to valgrind
        node->parent = nullptr; // explicit init due to valgrind
    }

    return node;
}

FibNodePtr
FibHeap::FibExtractMin()
{
    FibNodePtr heap_min = this->min;
    FibNodePtr ptr = nullptr;
    FibNodePtr tmp = nullptr;

    if (heap_min) {
        FibNodePtr child = heap_min->child;
        if (child) {
            ptr = child;
            do {
                tmp = child->right;
                this->FibMoveToRoot(child);
                child->parent = nullptr;
                child = tmp;
            } while(child != ptr);
        }

        /* remove minimum from the root list of H */
        heap_min->left->right = heap_min->right;
        heap_min->right->left = heap_min->left;

        if (heap_min == heap_min->right) {
            this->min = nullptr;
        } else {
            this->min = heap_min->right;
            this->FibConsolidate();
        }

        this->numNodes--;
    }

    return heap_min;
}

int
FibHeap::FibConsolidate()
{
    FibNodePtr ptr = this->min;
    FibNodePtr ptr_x = ptr;
    FibNodePtr ptr_y = nullptr;
    double phi = ((1 + sqrt(5)) / 2); // golden ratio used for logarithm base
    int deg = -1;
    int max_degree =
        static_cast<int>(floor(
                               log(static_cast<double>(this->numNodes)) /
                               log(phi)));

    /* auxiliary array to keep track of roots according to their degrees */
    vector<FibNodePtr> ax_array(max_degree, nullptr);

    if (!ptr || ptr == ptr->right)
        return 0;

    do {
        deg = ptr_x->degree;
        while (ax_array[deg]) {
            ptr_y = ax_array[deg];
            if (ptr_x->key > ptr_y->key) {
                SWAP(ptr_x, ptr_y);
            }

            /* when looping through the rootlist, we strongly rely on the
             * current minimal value node, because the loop does not
             * finish unless minimal value node found, which we
             * definitely procesed at the beginning. In case that our
             * minimal value node needs to be swapped and linked as child
             * of his next neighbour, we need to update the minimal value
             * node to his neighbour, otherwise we completely loose track
             * of which nodes still need to be procesed, as by linking
             * we do update all pointers.
             */
            if (ptr == ptr_y)
                ptr = ptr_x;

            /* make y child of x */
            this->FibHeapLink(ptr_y, ptr_x);
            ax_array[deg] = nullptr;
            deg++;
        }

        ax_array[deg] = ptr_x;
        ptr_x = ptr_x->right;
    } while (ptr_x != ptr);

    this->min = nullptr;
    ptr = this->min;

    /* create a new rootlist */
    for (unsigned i = 0; i < ax_array.size(); i++) {
        if (ax_array[i]) {
            if (!ptr) {

                /* create a root list for H containing jus A[i] */
                ax_array[i]->left = ax_array[i];
                ax_array[i]->right = ax_array[i];
                this->min = ax_array[i];
                ptr = this->min;
            } else {
                this->FibMoveToRoot(ax_array[i]);
                if (ax_array[i]->key < ptr->key)
                    this->min = ax_array[i];
            }
        }
    }

    return 0;
}

int
FibHeap::FibHeapLink(FibNodePtr y, FibNodePtr x)
{
    if (!x)
        return -1;

    FibNodePtr *childptr = &(x->child);

    /* remove y from the root list of H */
    y->right->left = y->left;
    y->left->right = y->right;
    y->left = y;
    y->right = y;
    y->parent = x;

    /* make y a child of x */
    if (!(*childptr))
        (*childptr) = y;

    (*childptr)->left->right = y;
    y->left = (*childptr)->left;
    (*childptr)->left = y;
    y->right = (*childptr);

    x->degree++;
    y->mark = false;

    return 0;
}

int
FibHeap::FibDecreaseKey(FibNodePtr x, int key)
{
    FibNodePtr heap_min = this->min;
    FibNodePtr y = nullptr;
    int ret = -1;

    if (!x) {
        cerr << "cannot find node\n" << endl;
        goto error;
    }

    if (key > x->key) {
        cerr << "new key is greater than current key" << endl;
        goto error;
    }

    x->key = key;
    y = x->parent;
    if (y && (x->key < y->key)) {
        this->FibCut(x, y);
        this->FibCascadingCut(y);
    }

    if (x->key < heap_min->key)
        this->min = x;

    ret = 0;
 error:
    return ret;
}

int
FibHeap::FibCut(FibNodePtr x, FibNodePtr y)
{
    if (!x)
        return -1;

    // remove x from child list of y
    if (!(x == x->right)) {
        x->left->right = x->right;
        x->right->left = x->left;
        y->child = x->right;
    } else {
        y->child = nullptr;
    }

    // add x to the root list of H
    this->FibMoveToRoot(x);

    y->degree--;
    x->parent = nullptr;
    x->mark = false;

    return 0;
}

int
FibHeap::FibCascadingCut(FibNodePtr y)
{
    FibNodePtr ptr = y->parent;

    if (!ptr)
        return -1;

    if (!(y->mark)) {
        y->mark = true;
    } else {
        this->FibCut(y, ptr);
        this->FibCascadingCut(ptr);
    }

    return 0;
}

int
FibHeap::FibDeleteNode(FibNodePtr node)
{
    FibNodePtr retnode = nullptr;/////////////////////////////
    int ret = -1;
    if ((ret = this->FibDecreaseKey(node, INT_MIN)) < 0) {
        cerr << "Delete: cannot decreasekey\n";/////////////////////
        goto cleanup;
    }

    if (!(retnode = this->FibExtractMin())) {
        cerr << "Delete: cannot extract min\n";///////////////////////
        goto cleanup;
    }

    ret = 0;
 cleanup:
    delete retnode;
    return ret;
}

FibNodePtr
FibHeap::FibFindNode(unsigned id)
{
    FibNodePtr ret = nullptr;
    FibNodePtr ptr = this->min;

    ret = this->FibFindImpl(ptr, id);
    return ret;
}

FibNodePtr
FibHeap::FibFindImpl(FibNodePtr x, unsigned id)
{
    FibNodePtr ptr = x;
    FibNodePtr tmp = ptr;

    if (ptr) {
        do {
            if (ptr->id == id)
                return ptr;
            tmp = ptr;
            ptr = ptr->right;
            if ((tmp = this->FibFindImpl(tmp->child, id)))
                return tmp;
        } while (ptr != x);
    }

    return nullptr;
}

FibHeapPtr
FibUnion(FibHeap &heap1, FibHeap &heap2) {
    FibHeap *heap_ptr = nullptr;
    FibNodePtr h_min = nullptr;
    FibNodePtr h_min1 = heap1.min;
    FibNodePtr h_min2 = heap2.min;
    FibNodePtr tmp = nullptr;

    heap_ptr = new(nothrow) FibHeap;
    if (!heap_ptr)
        goto cleanup;

    heap_ptr->min = h_min1;
    h_min = heap_ptr->min;

    // concatenate the root list of H2 with root list of H
    h_min->left->right = h_min2;
    tmp = h_min->left;
    h_min->left = h_min2->left;
    h_min2->left->right = h_min;
    h_min2->left = tmp;
    if (!h_min1 || (h_min2 && h_min2->key < h_min1->key))
        heap_ptr->min = h_min2;

    heap_ptr->numNodes = heap1.numNodes + heap2.numNodes;

    /* because we now have all the references in the new created heap,
     * we can remove the references from the original heaps, but still
     * leaving the caller with the responsibility to properly deallocate
     * all dynamic memory
     */
    heap1.min = nullptr;
    heap2.min = nullptr;
 cleanup:
    return heap_ptr;
}
