#include "value.h"
#include "linkedlist.h"
#include "talloc.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



Value *eval(Value *expr, Frame *frame);
Value *lookUpSymbol(Value *expr, Frame *frame);

void print(Value *curr){
    switch (curr->type) {
        case INT_TYPE:
            printf("%i\n", curr->i);
            break;
        case DOUBLE_TYPE:
            printf("%f\n", curr->d);
            break;
        case STR_TYPE:
            printf("%s\n", curr->s);
            break;
        case BOOL_TYPE:
            if(curr->i) {
                printf("#t\n");
            }else{
                printf("#f\n");
            }
            break;
        case PTR_TYPE:
            printf("%p", curr->p);
            break;
        case NULL_TYPE:
        case OPEN_TYPE:
        case CLOSE_TYPE: 
        case PRIMITIVE_TYPE:
        case UNSPECIFIED_TYPE:
        case VOID_TYPE:
            break;
        case CLOSURE_TYPE:
            printf("#<procedure>\n");
            break;
        case SYMBOL_TYPE:
            printf("%s\n", curr->s);
            break;
        case CONS_TYPE:
            printf("(");
            printTree(curr);
            printf(")\n");
            break;
        }
}
    

//Evaluates an if statement, returns the cdr if the conditional is true, the cdr of the cdr if it is not
Value *evalIf(Value *args, Frame *frame) {
    if(args->c.cdr->type == NULL_TYPE || args->c.cdr->c.cdr->type == NULL_TYPE) {
        printf("Evaluation Error: if has fewer than 3 arguments\n");
        texit(0);
    }
    Value *result = eval(args->c.car, frame);
    if(result->type == BOOL_TYPE && !result->i) {
        return eval(args->c.cdr->c.cdr->c.car, frame);
    } else {
        return eval(args->c.cdr->c.car, frame);
        
    }
}

Value *makeBindings(Value *args, Frame *frame) {
    Value *final = makeNull();
    while(args->type != NULL_TYPE) {
        if(args->c.car->type == NULL_TYPE || args->c.car->c.car->type != SYMBOL_TYPE || args->c.car->c.cdr->type == NULL_TYPE) {
            printf("Evaluation Error: Invalid argument for 'let' expression\n");
            texit(0);
        }
        Value *temp = final;
        while(temp->type != NULL_TYPE) {
            if(!strcmp(temp->c.car->c.car->s, args->c.car->c.car->s)) {
                printf("Evaluation Error: Duplicate symbol used in 'let' expression\n");
                texit(0);
            }
            temp = temp->c.cdr;
        }
        final = cons(cons(args->c.car->c.car, eval(args->c.car->c.cdr->c.car, frame)), final);
        args = args->c.cdr;
    }
    return final;
}

Value *evalLet(Value *args, Frame *parentFrame) {
    if(args->c.car->type != CONS_TYPE) {
        printf("Evaluation Error: Invalid 'let' expression\n");
        texit(0);
    }
    if(args->c.car->c.car->type != CONS_TYPE && args->c.car->c.car->type != NULL_TYPE) {
        printf("Evaluation Error: Invalid grouping in 'let' phrase\n");
        texit(0);
    }
    if(args->c.car->c.car->type != NULL_TYPE && args->c.car->c.car->c.cdr->type == NULL_TYPE) {
        printf("Evaluation Error: Invalid 'let' expression\n");
        texit(0);
    }
    if(args->c.car->c.car->type == NULL_TYPE) {
        Frame *newFrame = talloc(sizeof(Frame));
        newFrame->parent = parentFrame;
        newFrame->bindings = makeNull();
        args = args->c.cdr;
        if(args->type == NULL_TYPE) {
            printf("Evaluation error: no args following the bindings in let\n");
            texit(0);
        }
        while(args->c.cdr->type != NULL_TYPE) {
            args = args->c.cdr;
        }
        return eval(args->c.car, newFrame);
    }
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = parentFrame;
    newFrame->bindings = makeBindings(args->c.car, parentFrame);
    args = args->c.cdr;
    if(args->type == NULL_TYPE) {
        printf("Evaluation error: no args following the bindings in let\n");
        texit(0);
    }
    while(args->c.cdr->type != NULL_TYPE) {
        eval(args->c.car, newFrame);
        args = args->c.cdr;
    }
    return eval(args->c.car, newFrame);
}

Value *evalDef(Value *args, Frame *frame) {
    if(args->c.cdr->type == NULL_TYPE) {
        printf("Evaluation Error: No body in lambda expression\n");
        texit(0);
    }
    if(args->c.car->type != SYMBOL_TYPE && args->c.car->type != STR_TYPE) {
        printf("Evaluation Error: Wrong type to apply\n");
        texit(0);
    }

    Value *final = makeNull();
    frame->bindings = cons(cons(args->c.car, eval(args->c.cdr->c.car, frame)), frame->bindings);

    Value *result = talloc(sizeof(Value));
    result->type = VOID_TYPE;
    return result;
}

Value *evalLambda(Value *args, Frame *frame){
    if(args->type == NULL_TYPE) {
        printf("Evaluation Error: No Arguments in lambda expression\n");
        texit(0);
    }
    if(args->c.car->type != CONS_TYPE){
        printf("Evaluation Error: invalid arguments\n");
        texit(0);
    }
    Value *newClosure = talloc(sizeof(Value));
    newClosure->type = CLOSURE_TYPE;
    newClosure->cl.frame = frame;
    Value *temp = args->c.car;
    Value *params = makeNull();
    while(temp->type == CONS_TYPE && temp->c.car->type != NULL_TYPE) {
        Value *tempParams = params;
        while(tempParams->type != NULL_TYPE) {
            if(!strcmp(temp->c.car->s,params->c.car->s)) {
                printf("Evaluation Error: Duplicate Parameter in Lambda expression\n");
                texit(0);
            } else {
                tempParams = tempParams->c.cdr;
            }
        }
        if(temp->c.car->type != SYMBOL_TYPE && temp->c.car->type != STR_TYPE) {
                printf("Evaluation Error: Invalid argument for lambda expression\n");
                texit(0);
            }
        params = cons(temp->c.car, params);
        temp = temp->c.cdr;
    }
    newClosure->cl.paramNames = params;
    if(args->c.cdr->type == NULL_TYPE) {
        printf("Evaluation Error: No body in lambda phrase\n");
        texit(0);
    }
    newClosure->cl.functionCode = args->c.cdr->c.car;

    return newClosure;
}

Value *evalLetStar(Value *args, Frame *parentFrame){
    if(args->c.car->type != CONS_TYPE) {
        printf("Evaluation Error: Invalid 'let' expression\n");
        texit(0);
    }
    if(args->c.car->c.car->type != CONS_TYPE && args->c.car->c.car->type != NULL_TYPE) {
        printf("Evaluation Error: Invalid grouping in 'let' phrase\n");
        texit(0);
    }
    if(args->c.car->c.car->type != NULL_TYPE && args->c.car->c.car->c.cdr->type == NULL_TYPE) {
        printf("Evaluation Error: Invalid 'let' expression\n");
        texit(0);
    }

    if(args->c.car->c.car->type == NULL_TYPE) {
        Frame *newFrame = talloc(sizeof(Frame));
        newFrame->parent = parentFrame;
        newFrame->bindings = makeNull();
        args = args->c.cdr;
        if(args->type == NULL_TYPE) {
            printf("Evaluation error: no args following the bindings in let\n");
            texit(0);
        }
        return eval(args->c.car, newFrame);
    }

    Value *temp = args->c.car;
    Frame *tempFrame = parentFrame;
    Frame *newFrame;
    while(temp->type != NULL_TYPE  ){
        newFrame = talloc(sizeof(Frame));
        newFrame->parent = tempFrame;
        newFrame->bindings = cons(cons(temp->c.car->c.car, eval(temp->c.car->c.cdr->c.car, newFrame->parent)), makeNull());
        temp = temp->c.cdr;
        tempFrame = newFrame;
    }
    args = args->c.cdr;
    if(args->type == NULL_TYPE) {
        printf("Evaluation error: no args following the bindings in let\n");
        texit(0);
    }
    return eval(args->c.car, newFrame);
}

Value *evalLetRec(Value *args, Frame *parentFrame){
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = parentFrame;
    newFrame->bindings = makeNull();
    Value *actual = makeNull();
    Value *formal = makeNull();
    Value *temp = args->c.car;
    while(temp->type != NULL_TYPE){
        actual = cons(eval(temp->c.car->c.cdr->c.car, newFrame), actual);
        formal = cons(temp->c.car->c.car, formal);
        temp = temp->c.cdr;
    }
    actual = reverse(actual);
    formal = reverse(formal);
    while(formal->type != NULL_TYPE){
        newFrame->bindings = cons(cons(formal->c.car, actual->c.car), newFrame->bindings);
        actual = actual->c.cdr;
        formal = formal->c.cdr;
    }
    return eval(args->c.cdr->c.car, newFrame);
}

Value *evalSet(Value *args, Frame *frame){
    Value *newVal = eval(args->c.cdr->c.car, frame);
    Frame *tempFrame = frame;
    Value *temp = tempFrame->bindings;
    while(tempFrame->parent != NULL){
        while(temp->type != NULL_TYPE) {
            if(!strcmp(temp->c.car->c.car->s, args->c.car->s)) {
                temp->c.car->c.cdr = newVal;
                return makeNull();
            }
            temp = temp->c.cdr;
        }
        tempFrame = tempFrame->parent;
        temp = tempFrame->bindings;
    }
    while(temp->type != NULL_TYPE) {
        if(!strcmp(temp->c.car->c.car->s, args->c.car->s)) {
            temp->c.car->c.cdr = newVal;
            return makeNull();
        }
        temp = temp->c.cdr;
    }
    return makeNull();
}

Value *evalBegin(Value *args, Frame *frame){
    Value *result;
    if(args->type == NULL_TYPE){
        result = talloc(sizeof(Value));
        result->type = VOID_TYPE;
        return result;
    }

    while(args->type != NULL_TYPE){
        result = eval(args->c.car, frame);
        args = args->c.cdr;
    }
    return result;
}

Value *evalAnd(Value *args, Frame *frame){
    Value *result;
    if(args->type == NULL_TYPE){
        result = talloc(sizeof(Value));
        result->type = BOOL_TYPE;
        result->i = 1;
        return result;
    }
    while(args->type != NULL_TYPE){
        result = eval(args->c.car, frame);
        if(result->type == BOOL_TYPE && result->i == 0){
            return result;
        }
        args = args->c.cdr;
    }
    result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    result->i = 1;
    return result;
}

Value *evalOr(Value *args, Frame *frame){
    Value *result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    if(args->type == NULL_TYPE){
        result->i = 0;
        return result;
    }
    while(args->type != NULL_TYPE){
        Value *val = eval(args->c.car, frame);
        if(val->type != BOOL_TYPE){
            result->i = 1;
            return result;
        }else if(val->type == BOOL_TYPE && val->i == 1){
            result->i = 1;
            return result;
        }
        args = args->c.cdr;
    }
    result->i = 0;
    return result;
}

Value *evalCond(Value *args, Frame *frame){
    Value *sym = talloc(sizeof(Value));
    sym->type = SYMBOL_TYPE;
    sym->s = "else";
    Value *val = talloc(sizeof(Value));
    val->type = BOOL_TYPE;
    val->i = 1;
    frame->bindings = cons(cons(sym, val), frame->bindings);
    while(args->type != NULL_TYPE){
        Value *test = eval(args->c.car->c.car, frame);
        if(test->type == BOOL_TYPE && test->i == 1){
            return eval(args->c.car->c.cdr->c.car, frame);
        }else if(test->type == BOOL_TYPE && test->i == 0){
        }else if(test->type == SYMBOL_TYPE && !strcmp(test->s, "else")){ // else case
            return eval(args->c.car->c.cdr->c.car, frame);
        }
        args = args->c.cdr;
    }
    Value *result = talloc(sizeof(Value));
    result->type = VOID_TYPE;
    return result;
}

Value *apply(Value *function, Value *args){
    if(function->type != CLOSURE_TYPE) {
        printf("Evaluation Error: Invalid function\n");
        texit(0);
    }
    Frame *newFrame = talloc(sizeof(Frame));
    newFrame->parent = function->cl.frame;
    Value *tempParam = function->cl.paramNames;
    Value *tempArg = args;
    Value *final = makeNull();
    if(tempArg->type == NULL_TYPE && tempParam->type == NULL_TYPE) {
        newFrame->bindings = makeNull();
        return eval(function->cl.functionCode, newFrame);
    } else {
        while(tempParam->type != NULL_TYPE) {
            if(tempArg->type == NULL_TYPE) {
                printf("Evaluation Error: invalid number of arguments1\n");
                texit(0);
            }
            final = cons(cons(tempParam->c.car, tempArg->c.car), final);
            tempArg = tempArg->c.cdr;
            tempParam = tempParam->c.cdr;
        }
        if(tempArg->type != NULL_TYPE) {
            printf("Evalutation Error: Invalid number of arguments2\n");
            texit(0);
        }
        newFrame->bindings = final;

        return eval(function->cl.functionCode, newFrame);
    }
}

Value *primPlus(Value *args){
    Value *final;
    if (args->type == NULL_TYPE){
        final = talloc(sizeof(Value));
        final->type = INT_TYPE;
        final->i = 0;
        return final;
    }
    double result = 0;
    int real = 0;
    while(args->type != NULL_TYPE){
        if (args->c.car->type != INT_TYPE && args->c.car->type != DOUBLE_TYPE){
            printf("Evaluation error: Wrong type to apply\n");
            texit(0);
        }
        if(args->c.car->type == INT_TYPE){
            result = result + args->c.car->i;
        }else{
            result = result + args->c.car->d;
            real = 1;
        }
        args = args->c.cdr;
    }
    final = talloc(sizeof(Value));
    if(real){
        final->type = DOUBLE_TYPE;
        final->d = result;
    } else {
        final->type = INT_TYPE;
        final->i = result;
    }
    return final;
}

Value *primNull(Value *args){ 
    if(args->type == NULL_TYPE || args->c.cdr->type != NULL_TYPE){
        printf("Evaluation Error: Invalid number of arguments\n");
        texit(0);
    } 

    if(args->c.car->type == CONS_TYPE && args->c.car->c.car->type == NULL_TYPE){
        Value *trueVal = talloc(sizeof(Value));
        trueVal->type = BOOL_TYPE;
        trueVal->i = 1;
        return trueVal;
    } else if(args->c.car->type != NULL_TYPE){
        Value *trueVal = talloc(sizeof(Value));
        trueVal->type = BOOL_TYPE;
        trueVal->i = 0;
        return trueVal;
    } else{
        Value *trueVal = talloc(sizeof(Value));
        trueVal->type = BOOL_TYPE;
        trueVal->i = 1;
        return trueVal;
    }
}

Value *primCar(Value *args){ 
    if(args->type == NULL_TYPE || args->c.cdr->type != NULL_TYPE){
        printf("Evaluation Error: Invalid number of arguments\n");
        texit(0);
        return makeNull();
    }
    if(args->c.car->type != CONS_TYPE){
        printf("Evaluation Error: Wrong type to apply\n");
        texit(0);
        return makeNull();
    } else {
        return args->c.car->c.car;
    }
}

Value *primCdr(Value *args){ 
    if(args->type == NULL_TYPE || args->c.cdr->type != NULL_TYPE){
        printf("Evaluation Error: Invalid number of arguments\n");
        texit(0);
        return makeNull();
    }
    if(args->c.car->type != CONS_TYPE){
        printf("Evaluation Error: Wrong type to apply\n");
        texit(0);
        return makeNull();
    }else{
        return args->c.car->c.cdr;
    }
}

Value *primCons(Value *args){ 
    if(args->type == NULL_TYPE || args->c.cdr->type == NULL_TYPE || args->c.cdr->c.cdr->type != NULL_TYPE){
        printf("Evaluation Error: Invalid number of arguments\n");
        texit(0);
    }

    Value *newCons = talloc(sizeof(Value));
    newCons->type = CONS_TYPE;
    newCons->c.car = args->c.car;
    newCons->c.cdr = args->c.cdr->c.car;
    return newCons;
}

Value *primMinus(Value *args){
    Value *final;
    if (args->type == NULL_TYPE){
        final = talloc(sizeof(Value));
        final->type = INT_TYPE;
        final->i = 0;
        return final;
    }
    double doubleStart;
    int intStart;
    int real = 0;
    if (args->c.car->type != INT_TYPE && args->c.car->type != DOUBLE_TYPE){
        printf("Evaluation error: Wrong type to apply\n");
        texit(0);
    }

    if(args->c.car->type == INT_TYPE){
        intStart = args->c.car->i;
    }else{
        doubleStart = args->c.car->d;
        real = 1;
    }

    if(args->c.cdr->c.car->type == NULL_TYPE){
        final = talloc(sizeof(Value));
        if(real){
            final->type = DOUBLE_TYPE;
            final->d = - doubleStart;
        } else {
            final->type = INT_TYPE;
            final->i = - intStart;
        }
        return final;
    }

    Value *temp = args->c.cdr;
    while(temp->type != NULL_TYPE){
        if (temp->c.car->type != INT_TYPE && temp->c.car->type != DOUBLE_TYPE){
            printf("Evaluation error: Wrong type to apply\n");
            texit(0);
        }
        if(real){
            if(args->c.car->type == INT_TYPE){
                doubleStart = doubleStart - temp->c.car->i;
            }else{
                doubleStart = doubleStart - temp->c.car->d;
                real = 1;
            }
        }else{
            if(args->c.car->type == INT_TYPE){
                intStart = intStart - temp->c.car->i;
            }else{
                intStart = intStart - temp->c.car->d;
                real = 1;
            }
        }
        temp = temp->c.cdr;
    }
    
    final = talloc(sizeof(Value));
    if(real){
        final->type = DOUBLE_TYPE;
        final->d = doubleStart;
    } else {
        final->type = INT_TYPE;
        final->i = intStart;
    }
    return final;
}

Value *primLess(Value *args){
    if(args->type == NULL_TYPE || args->c.cdr->type == NULL_TYPE ){
        printf("Evaluation Error: Invalid number of arguments\n");
        texit(0);
    }

    double valOne;
    double valTwo;
    while(args->c.cdr->type != NULL_TYPE){
        if (args->c.car->type != INT_TYPE && args->c.car->type != DOUBLE_TYPE){
            printf("Evaluation error: Wrong type to apply\n");
            texit(0);
        }
        if(args->c.car->type == DOUBLE_TYPE){
            valOne = args->c.car->d;
        } else{
            valOne = args->c.car->i;
        }
        if(args->c.cdr->c.car->type == DOUBLE_TYPE){
            valTwo = args->c.cdr->c.car->d;
        } else{
            valTwo = args->c.cdr->c.car->i;
        }
        if(valTwo < valOne){
            Value *result = talloc(sizeof(Value));
            result->type = BOOL_TYPE;
            result->i = 0;
            return result;
        }
        args = args->c.cdr;
    }

    if(valOne == valTwo){
        Value *result = talloc(sizeof(Value));
        result->type = BOOL_TYPE;
        result->i = 0;
        return result;
    }
    Value *result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    result->i = 1;
    return result;
}

Value *primMore(Value *args){
    if(args->type == NULL_TYPE || args->c.cdr->type == NULL_TYPE ){
        printf("Evaluation Error: Invalid number of arguments\n");
        texit(0);
    }

    double valOne;
    double valTwo;
    while(args->c.cdr->type != NULL_TYPE){
        if (args->c.car->type != INT_TYPE && args->c.car->type != DOUBLE_TYPE){
            printf("Evaluation error: Wrong type to apply\n");
            texit(0);
        }
        if(args->c.car->type == DOUBLE_TYPE){
            valOne = args->c.car->d;
        } else{
            valOne = args->c.car->i;
        }
        if(args->c.cdr->c.car->type == DOUBLE_TYPE){
            valTwo = args->c.cdr->c.car->d;
        } else{
            valTwo = args->c.cdr->c.car->i;
        }
        if(valTwo > valOne){
            Value *result = talloc(sizeof(Value));
            result->type = BOOL_TYPE;
            result->i = 0;
            return result;
        }
        args = args->c.cdr;
    }

    if(valOne == valTwo){
        Value *result = talloc(sizeof(Value));
        result->type = BOOL_TYPE;
        result->i = 0;
        return result;
    }
    Value *result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    result->i = 1;
    return result;
}

Value *primEqual(Value *args){
    if(args->type == NULL_TYPE){
        printf("Evaluation Error: Invalid number of arguments\n");
        texit(0);
    }
    
    double valOne;
    double valTwo;
    while(args->c.cdr->type != NULL_TYPE){
        if (args->c.car->type != INT_TYPE && args->c.car->type != DOUBLE_TYPE){
            printf("Evaluation error: Wrong type to apply1\n");
            texit(0);
        }
        if (args->c.cdr->c.car->type != INT_TYPE && args->c.cdr->c.car->type != DOUBLE_TYPE){
            printf("Evaluation error: Wrong type to apply2\n");
            texit(0);
        }
        if(args->c.car->type == DOUBLE_TYPE){
            valOne = args->c.car->d;
        }else{
            valOne = args->c.car->i;
        }
        if(args->c.cdr->c.car->type == DOUBLE_TYPE){
            valTwo = args->c.cdr->c.car->d;
        }else{
            valTwo = args->c.cdr->c.car->i;
        }
        if(valOne != valTwo){
            Value *result = talloc(sizeof(Value));
            result->type = BOOL_TYPE;
            result->i = 0;
            return result;
        }
        args = args->c.cdr;
    }
    Value *result = talloc(sizeof(Value));
    result->type = BOOL_TYPE;
    result->i = 1;
    return result;
}

Value *primMult(Value *args){
    Value *result;
    if(args->type == NULL_TYPE){
        result = talloc(sizeof(Value));
        result->type = INT_TYPE;
        result->i = 1;
        return result;
    }
    if(args->c.cdr->type == NULL_TYPE){
        result = talloc(sizeof(Value));
        result->type = INT_TYPE;
        if(args->c.car->type == DOUBLE_TYPE){
            result->i = args->c.car->d;
        } else{
            result->i = args->c.car->i;
        }
        return result;
    }
    double final = 1;
    int real = 0;
    while(args->c.cdr->type != NULL_TYPE){
        if (args->c.car->type != INT_TYPE && args->c.car->type != DOUBLE_TYPE){
            printf("Evaluation error: Wrong type to apply\n");
            texit(0);
        }
        if (args->c.cdr->c.car->type != INT_TYPE && args->c.cdr->c.car->type != DOUBLE_TYPE){
            printf("Evaluation error: Wrong type to apply\n");
            texit(0);
        }
        if(args->c.car->type == DOUBLE_TYPE){
            real = 1;
            final = final * args->c.car->d;
        } else{
            final = final * args->c.car->i;
        }
        args = args->c.cdr;
    }
    if(args->c.car->type == DOUBLE_TYPE){
        real = 1;
        final = final * args->c.car->d;
    } else{
        final = final * args->c.car->i;
    }
    if(real){
        result = talloc(sizeof(Value));
        result->type = DOUBLE_TYPE;
        result->d = final;
    } else{
        result = talloc(sizeof(Value));
        result->type = INT_TYPE;
        result->i = final;
    }
    return result;
}

Value *primDiv(Value *args){
    if(args->type == NULL_TYPE){
        printf("Evaluation Error: Invalid number of arguments\n");
        texit(0);
    }
    Value *result;
    if(args->c.cdr->type == NULL_TYPE){
        result = talloc(sizeof(Value));
        if(args->c.car->type == DOUBLE_TYPE){
            result->type = DOUBLE_TYPE;
            result->d = 1 / args->c.car->d;
        } else{
            result->type = DOUBLE_TYPE;
            result->d = 1 / args->c.car->i;
        }
        return result;
    }
    double divisor;
    double num;
    if(args->c.car->type == DOUBLE_TYPE){
        num = args->c.car->d;
    } else{
        num = args->c.car->i;
    }
    args = args->c.cdr;
    if(args->c.car->type == DOUBLE_TYPE){
        divisor = args->c.car->d;
    } else{
        divisor = args->c.car->i;
    }
    
    result = talloc(sizeof(Value)); 
    double val = num / divisor;   
    if(val - (int)val == 0){
        val = (int)val;
        result->type = INT_TYPE;
        result->i = val;
    } else{
        result->type = DOUBLE_TYPE;
        result->d = val;
    }
    return result;
}

Value *primMod(Value *args){
    if(args->type == NULL_TYPE || args->c.cdr->type == NULL_TYPE || args->c.cdr->c.cdr->type != NULL_TYPE){
        printf("Evaluation Error: Invalid number of arguments\n");
        texit(0);
    }
    int final = args->c.car->i % args->c.cdr->c.car->i;
    Value *result = talloc(sizeof(Value));
    result->type = INT_TYPE;
    result->i = final;
    return result;
}

void bind(char *name, Value *(*function)(Value *), Frame *frame){
    Value *fName = talloc(sizeof(Value));
    fName->type = SYMBOL_TYPE;
    fName->s = name;
    Value *fn = talloc(sizeof(Value));
    fn->type = PRIMITIVE_TYPE;
    fn->pf = *function;
    frame->bindings = cons(cons(fName, fn), frame->bindings);
}

Value *lookUpSymbol(Value *expr, Frame *frame) {
    if(frame->bindings->type == NULL_TYPE && frame->parent == NULL) {
        printf("Evaluation error: Unbound Variable %s\n", expr->s);
        texit(0);
    }
    Value *temp = frame->bindings;
    while(temp->type != NULL_TYPE) {
        if(!strcmp(temp->c.car->c.car->s, expr->s)) {
            return temp->c.car->c.cdr;
        }
        temp = temp->c.cdr;
    }
    if(frame->parent == NULL) {
        printf("Evaluation error: Unbound Variable %s\n", expr->s);
        texit(0);
    } else {
        return lookUpSymbol(expr, frame->parent);
    }
    return makeNull();
}

Value *evalQuote(Value *arg){
    if(arg->type == NULL_TYPE){
        printf("Evaluation error: invalid quote arguments\n");
        texit(0);
    }

    return arg->c.car;
}

void interpret(Value *tree) {
    Frame *topLevel = talloc(sizeof(Frame));
    topLevel->bindings = makeNull();
    topLevel->parent = NULL;
    
    bind("+", primPlus, topLevel);
    bind("null?", primNull, topLevel);
    bind("car", primCar, topLevel);
    bind("cdr", primCdr, topLevel);
    bind("cons", primCons, topLevel);
    bind("-", primMinus, topLevel);
    bind("<", primLess, topLevel);
    bind(">", primMore, topLevel);
    bind("=", primEqual, topLevel);
    bind("*", primMult, topLevel);
    bind("/", primDiv, topLevel);
    bind("modulo", primMod, topLevel);

    Value *curr;
    while(tree->type != NULL_TYPE) {
        curr = eval(tree->c.car, topLevel);
        print(curr);
        tree = tree->c.cdr;
    }    
}

Value *eval(Value *expr, Frame *frame) {
    Value *first;
    Value *args;
    Value *result;
    switch (expr->type) {
        case INT_TYPE:
            return expr;
            break;
        case DOUBLE_TYPE:
            return expr;
            break;
        case STR_TYPE:
            return expr;
            break;
        case BOOL_TYPE:
            return expr;
            break;
        case NULL_TYPE:
        case PTR_TYPE:
        case OPEN_TYPE:
        case PRIMITIVE_TYPE:
        case UNSPECIFIED_TYPE:
        case VOID_TYPE:
        case CLOSE_TYPE:
            return makeNull();
            break; 
        case CLOSURE_TYPE:
            return eval(apply(expr, expr->c.car), expr->cl.frame);
            break;
        case SYMBOL_TYPE:
            return lookUpSymbol(expr, frame);
            break;
        case CONS_TYPE:
            first = expr->c.car;
            args = expr->c.cdr;

            if (!strcmp(first->s, "if")) {
                Value *result;
                result = evalIf(args, frame);
                return result;
            } else if (!strcmp(first->s,"let")) {
                Value *result;
                result = evalLet(args, frame);
                return result;
            } else if (!strcmp(first->s,"quote")){
                if(args->type == NULL_TYPE || args->c.cdr->type != NULL_TYPE){
                    printf("Evaluation error: Invalid number of arguments in quote\n");
                    texit(0);
                }
                return evalQuote(args);
            } else if (!strcmp(first->s,"define")){
                if(args->type == NULL_TYPE) {
                    printf("Evaluation Error: No args following define\n");
                    texit(0);
                }
                Value *result = evalDef(args, frame);
                return result;
            } else if (!strcmp(first->s,"lambda")){
                Value *result = evalLambda(args, frame);
                return result;
            } else if (!strcmp(first->s,"let*")){ 
                Value *result = evalLetStar(args, frame);
                return result;
            } else if (!strcmp(first->s,"letrec")){ 
                Value *result = evalLetRec(args, frame);
                return result;
            } else if (!strcmp(first->s,"set!")){ 
                Value *result = evalSet(args, frame);
                return result;
            } else if (!strcmp(first->s,"begin")){ 
                Value *result = evalBegin(args, frame);
                return result;
            } else if (!strcmp(first->s,"and")){ 
                Value *result = evalAnd(args, frame);
                return result;
            } else if (!strcmp(first->s,"or")){ 
                Value *result = evalOr(args, frame);
                return result;
            } else if (!strcmp(first->s,"cond")){ 
                Value *result = evalCond(args, frame);
                return result;
            } else {    
                Value *result = eval(first, frame);
                if(result->type == PRIMITIVE_TYPE) {
                    Value *argOne = makeNull();
                    Value *temp = args;
                    while(temp->type != NULL_TYPE) {
                        argOne = cons(eval(temp->c.car, frame), argOne);
                        temp = temp->c.cdr;
                    }
                    argOne = reverse(argOne);
                    Value *final = result->pf(argOne);
                    return final;
                }
                Value *remainder = makeNull();
                while(args->type != NULL_TYPE) {
                    remainder = cons(eval(args->c.car, frame), remainder);
                    args = args->c.cdr;
                }
                return apply(result, remainder);
            }
            break;
        }    
        return makeNull();
    }
