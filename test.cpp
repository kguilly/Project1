#include <iostream>
#include "stdlib.h"
#include "stdio.h"
#include "string"

/*
Cmd: g++ test.cpp -o test.o ; ./test.o
*/

int numPeople = 5;
int mailboxCapacity = 5;

struct mailbox{
    int owner;
    char* messages;
    int numMessages;
};
mailbox * mailboxArr; // [mailbox owner][message slots]

int main(){
    mailboxArr = (mailbox *)malloc(sizeof(int)*numPeople);
    for (int i=0; i<=mailboxCapacity; i++){
        mailboxArr[i].messages = (char *)malloc(sizeof(int)*mailboxCapacity);
        mailboxArr[i].owner = i;
        mailboxArr[i].numMessages = 0;
    }
    mailboxArr[3].numMessages = 55;
    std::cout << mailboxArr[3].numMessages << std::endl;
    std::cout << mailboxArr[2].numMessages << std::endl;

    free(mailboxArr);
}