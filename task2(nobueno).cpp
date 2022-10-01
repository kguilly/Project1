/*
POST OFFICE SIMULATION
Cmd Line: g++ -pthread -Wall task2.cpp -o task2.o ; ./task2.o
*/
#include "iostream"
#include "stdlib.h"
#include "stdio.h"
#include "sched.h"
#include "time.h"
#include "pthread.h"
#include "limits"
#include "semaphore.h"
#include "string"

using namespace std;

// Define global vars
int numPeople; // N from task sheet
int mailboxCapacity; // S from task sheet
int messagesToSend; // M from the task sheet
int messagesLeftToSend; // keep track of how many messages can still be sent
int count = 0;


struct mailbox{
    int owner;
    string * messages;
    int numMessages;
};
mailbox * mailboxArr; // [mailbox owner][message slots]


// Define semaphores
sem_t cmdWindow; sem_t argSem; sem_t *checkMessagesSem; sem_t *numMessagesSem;
sem_t numMessagesLeftToSendSem;

//Define functions
void getInput();
void validateSemaphores();
void *postOffice(void * arg);
void garbageCollection();

int main(){
    printf("\n-------------------------------------------\n");
    printf(  "   Post Office Simulation (Semaphores)  \n\n");

    // Grab input from the user
    getInput();
    messagesLeftToSend = messagesToSend;

    // initialize and validate your semaphores 
    validateSemaphores();

    //init mailbox arr
    mailboxArr = (mailbox *)malloc(sizeof(int)*numPeople);
    for (int i=0; i<=numPeople; i++){
        mailboxArr[i].messages = (string *)malloc(sizeof(int)*mailboxCapacity);
        mailboxArr[i].owner = i;
        mailboxArr[i].numMessages = 0;
    }
    pthread_t threadArr[numPeople];
    for (int i=0; i< numPeople; i++){
        pthread_create(&threadArr[i], NULL, postOffice, &count);
    }
    for (int i=0; i< numPeople; i++){
        pthread_join(threadArr[i], NULL);
    }


    // reallocate memory, destroy sems
    garbageCollection();

    return 0;

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
    cout << "" << endl;
}

void validateSemaphores(){
    // init message read array
    checkMessagesSem = (sem_t *)malloc(sizeof(sem_t)*numPeople);
    for(int i=0; i<numPeople; i++){
        if(sem_init(&checkMessagesSem[i], 0, mailboxCapacity) == -1){
            perror("sem_init");
            exit(EXIT_FAILURE);
        }
    }
    numMessagesSem = (sem_t *)malloc(sizeof(sem_t)*numPeople);
    for(int i=0; i<numPeople; i++){
        if(sem_init(&numMessagesSem[i], 0, 1) == -1){
            perror("sem_init");
            exit(EXIT_FAILURE);
        }
    }
    if(sem_init(&cmdWindow, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&argSem, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&numMessagesLeftToSendSem, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

}

void * postOffice(void * arg){
    sem_wait(&argSem);
    int countVal = (*(int *)arg)++;
    sem_post(&argSem);

    sem_wait(&cmdWindow);
    cout << "Person " << countVal << " has entered the post office." << endl;
    sem_post(&cmdWindow);

    while(1){
        
        sem_wait(&cmdWindow);
        cout << "Person " << countVal << " checks -- he has " << mailboxArr[countVal].numMessages;
        cout << " letters in his mailbox." << endl;
        sem_post(&cmdWindow);
        if (mailboxArr[countVal].numMessages > 0){
            // read the thing
            sem_wait(&cmdWindow);
            cout << "Person " << countVal << " reads msg " << mailboxArr[countVal].numMessages;
            cout << ": " << mailboxArr[countVal].messages[mailboxArr[countVal].numMessages] << endl;
            sem_post(&cmdWindow);
            sem_wait(&checkMessagesSem[countVal]);

            //decrement the number of messages in that mailbox, protected by a semaphore
            sem_wait(&numMessagesSem[countVal]);
            mailboxArr[countVal].numMessages--;
            sem_post(&numMessagesSem[countVal]);

            continue;
        }
        else{
            // compose a message addressed to someone else 
            srand(time(0));
            int randPerson = (rand() % numPeople); // choose a random person
            
            // check to see if more messages can be sent, if not break
            if (messagesLeftToSend > 0){
                sem_wait(&cmdWindow);
                cout << "Person " << countVal << " tries to send message to Person ";
                cout << randPerson << endl;
                sem_post(&cmdWindow);
                 // check to see if their mailbox is at capacity yet
                if(mailboxArr[randPerson].numMessages < mailboxCapacity){
                    // call post as per rubric 
                    sem_post(&checkMessagesSem[randPerson]);

                    // send them a message
                    string messageNum = to_string(messagesToSend - messagesLeftToSend);
                    string sentMessage = "Message " + messageNum + " to person " + to_string(randPerson);
                    // let them know of success
                    sem_wait(&cmdWindow);
                    cout << "Person " << countVal << " sent msg: " << sentMessage << endl;
                    sem_post(&cmdWindow);
                    
                    // increment the number of messages in their mailbox 
                    sem_wait(&numMessagesSem[randPerson]);
                    mailboxArr[randPerson].numMessages++;
                    sem_post(&numMessagesSem[randPerson]);

                    // decrement the number of messages left to send
                    sem_wait(&numMessagesLeftToSendSem);
                    messagesLeftToSend--;
                    sem_post(&numMessagesLeftToSendSem);
                }
                else{
                    // tell the cmd window that their mailbox is at capacity
                    sem_wait(&cmdWindow);
                    cout << "Person " << randPerson << "'s mailbox is at capacity." << endl;
                    sem_post(&cmdWindow);
                }
                
            }
            else{
                break;
            }
            
        }       
         

    }


    sem_wait(&cmdWindow);
    cout << "Person " << countVal << " leaves the post office." << endl;
    sem_post(&cmdWindow);
    pthread_exit(0);

}

void garbageCollection(){
    
    free(mailboxArr);
    if (sem_destroy(&cmdWindow) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&argSem) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    free(checkMessagesSem);
    for (int i=0; i<numPeople; i++){
        sem_destroy(&checkMessagesSem[i]);
    }
    free(numMessagesSem);
    for(int i=0; i<numPeople; i++){
        sem_destroy(&numMessagesSem[i]);
    }
    if(sem_destroy(&numMessagesLeftToSendSem)== -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
}

