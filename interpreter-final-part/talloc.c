#include <stdlib.h>
#include "value.h"

struct Node {
    void *item;
    struct Node *next;
};
typedef struct Node Node;

Node *memHead;

// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size){
    if (memHead == NULL){
        Node *newHead = malloc(sizeof(Node));
        newHead->item = malloc(size);
        newHead->next = NULL;
        memHead = newHead;
    }else{
        Node *newHead = malloc(sizeof(Node));
        newHead->item = malloc(size);
        newHead->next = memHead;
        memHead = newHead;
    }
    return memHead->item;
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree(){
    Node *next;
    while(memHead != NULL){
        free(memHead->item);
        next = memHead->next;
        free(memHead);
        memHead = next;
    }
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status){
    tfree();
    exit(0);
}

