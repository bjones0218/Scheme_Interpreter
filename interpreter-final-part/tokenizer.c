#include "value.h"
#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include "talloc.h"
#include "linkedlist.h"

int isDigit(char c) {
    return (c >= '0' && c <= '9');
}

int isLetter(char c) {
    return((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

int isDot(char c) {
    return(c == '.');
}

int isSymbol(char c) {
    return(c ==  '!' || c ==  '$' || c ==  '%' || c ==  '&' || c ==  '*' || c ==  '/' || c ==  ':' || c ==  '<' || c ==  '=' || c ==  '>' || c == '?' || c ==  '~' || c ==  '_' || c ==  '^');
}

int plusMinus(char c) {
    return(c == '+' || c == '-');
}

Value *getNumber(char c) {
    int doubleCheck = 0;
    char *fullNum = talloc(sizeof(int)*300);
    int curSpot = 0;
    if(plusMinus(c)) {
        fullNum[curSpot] = c;
        curSpot++;
        c = (char)fgetc(stdin);
    }
    while(isDigit(c) || isDot(c)) {
        if(isDot(c)) {
            doubleCheck += 1;
        }
        if (doubleCheck >= 2) {
            printf("Syntax Error: unrecognized double type\n");
            texit(0);
        }
        
        fullNum[curSpot] = c;
        curSpot++;        
        c = (char)fgetc(stdin);
        }
        fullNum[curSpot] = '\0';

    if(c != EOF){
        if(c != ' ' && c != '(' && c != ')' && c != '\n'){
            printf("Syntax Error: Invalid Int or Double\n");
            texit(0);
        }
    }
        ungetc(c, stdin);
        Value *num = talloc(sizeof(Value));
        char *useless;
        if(doubleCheck == 0) {
            int ret;
            num->type = INT_TYPE;
            ret = (int) strtol(fullNum, &useless, 10);
            num->i = ret;
            return num;
    }else{
        double ret;
        num->type = DOUBLE_TYPE;
        ret = strtod(fullNum, &useless);
        num->d = ret;
        return num;
    }
}

Value *getsymbol(char c){
    char *fullSym = talloc(sizeof(char)*300);
    int curSpot = 0;

    while (isSymbol(c) || isLetter(c) || isDigit(c) || plusMinus(c) || isDot(c)){
        fullSym[curSpot] = c;       
        c = (char)fgetc(stdin);
        curSpot++;
    }
    fullSym[curSpot] = '\0';
    if (c != EOF){
        if(c != ' ' && c != '(' && c != ')' && c != '\n'){
            printf("Syntax Error: Invalid Symbol\n");
            texit(0);
        }   
    }
    ungetc(c, stdin);
    Value *symbol = talloc(sizeof(Value));
    symbol->type = SYMBOL_TYPE;
    symbol->s = fullSym;
    return symbol;
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list){
    while(list->type != NULL_TYPE){
        if (list->c.car->type == INT_TYPE){
            printf("%i:integer\n", list->c.car->i);
        } else if (list->c.car->type == DOUBLE_TYPE){
            printf("%f:double\n", list->c.car->d);
        } else if (list->c.car->type == SYMBOL_TYPE){
            printf("%s:symbol\n", list->c.car->s);
        } else if (list->c.car->type == BOOL_TYPE){
            if(list->c.car->i == 0){
                printf("#f:boolean\n");
            }else{
                printf("#t:boolean\n");
            }
        } else if (list->c.car->type == OPEN_TYPE){
            printf("%s:open\n", list->c.car->s);
        } else if (list->c.car->type == CLOSE_TYPE){
            printf("%s:close\n", list->c.car->s);
        } else if (list->c.car->type == STR_TYPE){
            printf("%s:string\n", list->c.car->s);
        }
        list = list->c.cdr;
    }
}

// Read all of the input from stdin, and return a linked list consisting of the
// tokens.
Value *tokenize(){
    char charRead;
    Value *list = makeNull();
    charRead = (char)fgetc(stdin);

    while (charRead != EOF) {
        Value *newChar;
        if (charRead == '\n' || charRead == ' '){   //Spaces and New Line

        }else if(charRead == ';'){                   //Comment Stripping
            while(charRead != '\n'){
                charRead = (char)fgetc(stdin);
            }
        }else if(charRead == '(') {                 //Open Parens
            newChar = talloc(sizeof(Value));
            newChar->type = OPEN_TYPE;
            newChar->s = "(";
            list = cons(newChar, list);
        } else if (charRead == ')') {               //Close Parens
            newChar = talloc(sizeof(Value));
            newChar->type = CLOSE_TYPE;
            newChar->s = ")";
            list = cons(newChar, list);
        } else if(charRead == '"'){                 //Strings
            newChar = talloc(sizeof(Value));
            newChar->type = STR_TYPE;
            char *newString = talloc(sizeof(char)*300);
            newString[0] = '"';
            charRead = (char)fgetc(stdin);
            int curSpot = 1;
            while(charRead != '"'){
                newString[curSpot] = charRead;
                charRead = (char)fgetc(stdin);
                curSpot++;
            }
            newString[curSpot] = charRead;
            newString[curSpot+1] = '\0';
            newChar->s = newString;
            list = cons(newChar, list);
        } else if(plusMinus(charRead)) {            //Int and Double starting with +/-
            char sign = charRead;
            charRead = (char)fgetc(stdin);
            if(charRead == ' ' || charRead == ')' || charRead == '(' || charRead == '\n') {
                char *symbol = talloc(sizeof(char)*2);
                newChar = talloc(sizeof(Value));
                newChar->type = SYMBOL_TYPE;
                symbol[0] = sign;
                symbol[1] = '\0';
                newChar->s = symbol;
                ungetc(charRead, stdin);
                list = cons(newChar, list);
            } else if(isDigit(charRead) || isDot(charRead)) {
                ungetc(charRead, stdin);
                newChar = getNumber(sign);
                list = cons(newChar, list);
            } else {
                printf("Syntax Error: Invalid Symbol\n"); // Invalid symbol if sumbol starts with +/-
                texit(0);
            }
        } else if(isDigit(charRead) ||isDot(charRead)) { //Int and Doubles
            newChar = getNumber(charRead);
            list = cons(newChar, list);
        } else if(isSymbol(charRead) || isLetter(charRead)) { //Symbols
            newChar = getsymbol(charRead);
            list = cons(newChar, list);
        } else if(charRead == '#'){                 //Booleans
            charRead = (char)fgetc(stdin);
            if(charRead == 'f'){
                newChar = talloc(sizeof(Value));
                newChar->type = BOOL_TYPE;
                newChar->i = 0;
            }else if (charRead == 't'){
                newChar = talloc(sizeof(Value));
                newChar->type = BOOL_TYPE;
                newChar->i = 1;
            }else{
                printf("Syntax Error: Invalid Symbol\n");
                texit(0);
            }
            list = cons(newChar, list);
        } else{
            printf("Syntax Error: Invalid Symbol\n");
            texit(0);
        }
        
        charRead = (char)fgetc(stdin);
    }
    
    Value *revList = reverse(list);
    return revList;
}

