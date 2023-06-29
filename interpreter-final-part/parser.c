#include "value.h"
#include "linkedlist.h"
#include "tokenizer.h"
#include "talloc.h"
#include <stdio.h>

// Takes a list of tokens from a Scheme program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens){
    Value *stack = makeNull();
    int openParens = 0;
    while (tokens->type != NULL_TYPE){
        if (tokens->c.car->type != CLOSE_TYPE){
            if (tokens->c.car->type == OPEN_TYPE){
                openParens++;
            }
            stack = cons(tokens->c.car, stack);
            tokens = tokens->c.cdr;
        }else{
            if (openParens == 0){
                printf("Syntax error: too many close parentheses\n");
                texit(0);
            }else{
                openParens--;
                tokens = tokens->c.cdr;
                Value *subtree = makeNull();
                if(stack->c.car->type == OPEN_TYPE){
                    subtree = cons(makeNull(), subtree);
                }else{
                    while (stack->c.car->type != OPEN_TYPE){
                        subtree = cons(stack->c.car, subtree);
                        stack = stack->c.cdr;
                    }
                }
                stack = stack->c.cdr;
                stack = cons(subtree, stack);
            }
        }
    }
    if (openParens > 0){
        printf("Syntax error: not enough close parentheses\n");
        texit(0);
    }
    stack = reverse(stack);
    return stack;
}


// Prints the tree to the screen in a readable fashion. It should look just like
// Scheme code; use parentheses to indicate subtrees.
void printTree(Value *tree){
    if (tree->type == CONS_TYPE){
        Value *left = tree->c.car;
        if (left->type == CONS_TYPE){
            printf("(");
            printTree(left);
            
            tree = tree->c.cdr;
            printf(") ");
            printTree(tree);
        }else{
        printTree(left);
        if(left->type != CONS_TYPE && tree->c.cdr->type != CONS_TYPE && tree->c.cdr->type != NULL_TYPE){
            printf(". ");
        }
        tree = tree->c.cdr;
        printTree(tree);
        }
    }else{
        switch (tree->type) {
        case INT_TYPE:
            printf("%i ", tree->i);
            break;
        case DOUBLE_TYPE:
            printf("%f ", tree->d);
            break;
        case STR_TYPE:
            printf("%s ", tree->s);
            break;
        case BOOL_TYPE:
            if(tree->i == 0){
                printf("#f ");
            }else{
                printf("#t ");
            }
            break;
        case SYMBOL_TYPE:
            printf("%s ", tree->s);
            break;
        case NULL_TYPE: 
        case PTR_TYPE:
        case OPEN_TYPE:
        case CLOSE_TYPE:
        case CONS_TYPE:
        case VOID_TYPE:
        case CLOSURE_TYPE:
        case UNSPECIFIED_TYPE:
        case PRIMITIVE_TYPE:
            break;
        }
    }   
}
