#include <stdbool.h>
#include "value.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "talloc.h"

// Create a new NULL_TYPE value node.
Value *makeNull(){
    Value *newNode = talloc(sizeof(Value));
    newNode -> type = NULL_TYPE;
    return newNode;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr){
    Value *newNode = talloc(sizeof(Value));
    newNode -> type = CONS_TYPE;
    newNode->c.car = newCar;
    newNode->c.cdr = newCdr;
    return newNode;
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list){
    if(list -> type == NULL_TYPE) {
        printf("empty list");
    }
    else {
        printf("[ ");
        switch ((list -> c.car)->type) {
        case INT_TYPE:
            printf("%i, ", (list -> c.car)->i);
            break;
        case DOUBLE_TYPE:
            printf("%f, ", (list -> c.car)->d);
            break;
        case STR_TYPE:
            printf("%s, ", (list -> c.car)->s);
            break;   
        case CONS_TYPE:
            break;
        case NULL_TYPE:
            break;  
        case PTR_TYPE:
        case VOID_TYPE:
        case PRIMITIVE_TYPE:
        case UNSPECIFIED_TYPE:
        case CLOSURE_TYPE:
            break;
        case BOOL_TYPE:
            if(list->c.car->i == 0){
                printf("#f, ");
            }else{
                printf("#t, ");
            }
            break;
        case OPEN_TYPE:
            printf("%s, ", (list -> c.car)->s);
            break;
        case CLOSE_TYPE:
            printf("%s, ", (list -> c.car)->s);
            break;
        case SYMBOL_TYPE:
            printf("%s, ", (list -> c.car)->s);
            break;
        }

        Value *restList = list->c.cdr;
        while(restList -> type != NULL_TYPE){
            switch ((restList->c.car)->type) {
                case INT_TYPE:
                    printf("%i, ", (restList->c.car)->i);
                    break;
                case DOUBLE_TYPE:
                    printf("%f, ", (restList->c.car)->d);
                    break;
                case STR_TYPE:
                    printf("%s, ", (restList->c.car)->s);
                    break;  
                case CONS_TYPE:
                    break;
                case NULL_TYPE:
                    break; 
                case PTR_TYPE:
                case VOID_TYPE:
                case PRIMITIVE_TYPE:
                case UNSPECIFIED_TYPE:
                case CLOSURE_TYPE:
                    break;  
                case BOOL_TYPE:
                    if(list->c.car->i == 0){
                        printf("#f, ");
                    }else{
                        printf("#t, ");
                    }
                    break;
                case OPEN_TYPE:
                    printf("%s, ", (list -> c.car)->s);
                    break;
                case CLOSE_TYPE:
                    printf("%s, ", (list -> c.car)->s);
                    break;
                case SYMBOL_TYPE:
                    printf("%s, ", (list -> c.car)->s);
                    break;       
            }
            restList = restList->c.cdr;
        }
        printf("]\n");
    }    
}

// Return a new list that is the reverse of the one that is passed in. All
// content within the list should be duplicated; there should be no shared
// memory whatsoever between the original list and the new one.
//
// FAQ: What if there are nested lists inside that list?
// ANS: There won't be for this assignment. There will be later, but that will
// be after we've got an easier way of managing memory.
Value *reverse(Value *list) {
    
    Value *head = makeNull();
    Value *curr = list;
    while(curr->type != NULL_TYPE){
        head = cons(curr->c.car, head);
        curr = curr->c.cdr;
    }
    return head;

}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list) {
    assert(list->type == CONS_TYPE || list->type == NULL_TYPE);
    return list->c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure 
// that this is a legitimate operation.
Value *cdr(Value *list) {
    assert(list->type == CONS_TYPE);
    return list->c.cdr;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value) {
    assert(value->type == NULL_TYPE || value->type == CONS_TYPE);
    return (value->type == NULL_TYPE);
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value) {
    assert(value->type == NULL_TYPE || value->type == CONS_TYPE);
    if(isNull(value)){
        return 0;
    }else{
        int length = 1;
        Value *restList = value->c.cdr;

        while(restList->type != NULL_TYPE) {        
            length += 1;                         
            restList = restList->c.cdr;
        }                                       

        return length;
    }
}
