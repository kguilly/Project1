/*
POST OFFICE SIMULATION
*/
#include "iostream"
#include "stdlib.h"
#include "stdio.h"
#include "sched.h"
#include "time.h"
#include "pthread.h"
#include "limits"
#include "semaphore.h"

using namespace std;

// Define global vars
int numPeople; // N from task sheet
int mailboxCapacity; // S from task sheet
int messagesToSend; // M from the task sheet

struct mailbox
{
    int person;
    int * messages;
};

mailbox * mailboxArr; // [mailbox owner][message slots]




// Define semaphores

//Define functions
void getInput();
void validateSemaphores();

void garbageCollection();

int main(){
    printf("\n----------------------------------------\nPost Office Simulation (Semaphores)\n");

    // Grab input from the user
    getInput();

    // initialize and validate your semaphores 
    validateSemaphores();

    //init mailbox arr
    mailboxArr = (mailbox *)malloc(sizeof(int)*numPeople);
    for (int i=0; i<=mailboxCapacity; i++){
        mailboxArr[i].messages = (int *)malloc(sizeof(int)*mailboxCapacity);
    }



    // reallocate memory, destroy sems
    garbageCollection();


}

void getInput(){
    cout << ("Enter the number of people coming into the post office(1 to 10k): ");
    cin >> numPeople;
    while(1){
        if(cin.fail()){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid goofball, try again: ";
            cin >> numPeople;
        }
        else if((numPeople > 10000) | (numPeople < 1)){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Enter a number in the range: ";
            cin >> numPeople;

        }
        else if(!(cin.fail())){
            break;
        }
    }

    cout << ("How many messages can a person's mailbox hold? (1-10,000): ");
    cin >> mailboxCapacity;
    while(1){
        if(cin.fail()){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Come on now try again. 1 to 10k: ";
            cin >> mailboxCapacity;
        }
        else if((mailboxCapacity > 10000) | (mailboxCapacity < 1)){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "You're better than that. From 1 to 10k: ";
            cin >> mailboxCapacity;
        }
        else if(!(cin.fail())){
            break;
        }
    }

    cout << ("Enter the number of messages to be sent before the simulation ends (1 to 10k): ");
    cin >> messagesToSend;
    while(1){
        if(cin.fail()){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid goofball, try again: ";
            cin >> messagesToSend;
        }
        else if((messagesToSend > 10000) | (messagesToSend < 1)){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Enter a number in the range: ";
            cin >> messagesToSend;

        }
        else if(!(cin.fail())){
            break;
        }
    }
}

void validateSemaphores(){

}

void garbageCollection(){
    for (int i=0; i<=mailboxCapacity; i++){
        free(mailboxArr[i].messages);
    }
    free(mailboxArr);
}

