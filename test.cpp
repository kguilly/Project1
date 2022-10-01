#include <iostream>
#include "stdlib.h"
#include "stdio.h"
#include "string"
#include "time.h"

/*
Cmd: g++ test.cpp -o test.o ; ./test.o
*/
using namespace std;
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


    std::cout << "\n\n\n\n\n make an rng that does not include your number \n" << std::endl;
    int numPeople = 6;
    int yourNum = 0;
    srand(time(0));
    int randnum;
    if (yourNum ==  numPeople) randnum = rand() % numPeople;
    else if(yourNum == 0) randnum = (rand() % numPeople) + 1;
    else{
        int upperRand = rand() % ((numPeople - yourNum)) + (yourNum + 1);
        int lowerRand = rand() % yourNum;
        cout << upperRand << " " << lowerRand << endl;
        if(rand() % 2 == 0) randnum = upperRand; 
        else randnum = lowerRand;
    }
            cout << randnum ;






    int y = 5;
    int * x = &y;
    *x = 6;

    cout << "\n\n\n\n\n" << *x << " \t" << y << endl;




    return 0;

}