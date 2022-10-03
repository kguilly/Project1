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

// colored output
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"



// Define global vars
int numPeople; // N from task sheet
int mailboxCapacity; // S from task sheet
int messagesToSend; // M from the task sheet
int messagesLeftToSend; // keep track of how many messages can still be sent
int count = 0;

string ** mailboxArr; // [mailbox owner][message slots]


// Define semaphores
sem_t cmdWindow; sem_t argSem; sem_t *fullSpacesSem; sem_t *emptySpacesSem;
sem_t numMessagesLeftToSendSem; sem_t *mailboxProtectionSem;

//Define functions
void getInput();
void validateSemaphores();
void *postOffice(void * arg);
void garbageCollection();
int chooseRandPerson(int personNum);

int main(){
    printf("\n-------------------------------------------\n");
    printf(  "   Post Office Simulation (Semaphores)  \n\n");

    // Grab input from the user
    getInput();
    messagesLeftToSend = messagesToSend;

    // initialize and validate your semaphores 
    validateSemaphores();

    //init mailbox arr
    mailboxArr = (string **)malloc(sizeof(string)*numPeople);
    for (int i=0; i<=numPeople; i++){
        // init each mailbox to have the total capacity that the mailboxes are allowed
        mailboxArr[i] = (string *)malloc(sizeof(string)*mailboxCapacity); 
    }

    // threadd the mail
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
    mailboxProtectionSem = (sem_t *)malloc(sizeof(sem_t)*numPeople);
    for(int i=0; i<numPeople; i++){
        if(sem_init(&mailboxProtectionSem[i], 0, 1) == -1){
            perror("sem_init");
            exit(EXIT_FAILURE);
        }
    }

    fullSpacesSem = (sem_t *)malloc(sizeof(sem_t)*numPeople);
    for(int i=0; i<numPeople; i++){
        if(sem_init(&fullSpacesSem[i], 0, 0) == -1){
            perror("sem_init");
            exit(EXIT_FAILURE);
        }
    }
    emptySpacesSem = (sem_t *)malloc(sizeof(sem_t)*numPeople);
    for(int i=0; i<numPeople; i++){
        if(sem_init(&emptySpacesSem[i], 0, mailboxCapacity) == -1){
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
    cout << RED << "Person " << countVal << " has entered the post office." << RESET << endl;
    sem_post(&cmdWindow);

    int fullSpacesVal = 0; //going to check the value of full spaces
    while(1){
        

        sem_wait(&cmdWindow);
        sem_getvalue(&fullSpacesSem[countVal], &fullSpacesVal);
        cout << GREEN <<  "--Person " << countVal << " checks -- he has " << fullSpacesVal;
        cout << " letters in his mailbox." << RESET << endl;
        sem_post(&cmdWindow);

        sem_getvalue(&fullSpacesSem[countVal], &fullSpacesVal);
        if(fullSpacesVal > 0){
            // read the thing -- CONSUMER
            sem_wait(&fullSpacesSem[countVal]);
            sem_getvalue(&fullSpacesSem[countVal], &fullSpacesVal);
            sem_wait(&cmdWindow);
            cout << YELLOW << "----Person " << countVal << " reads msg " << fullSpacesVal;
            cout << ": " << mailboxArr[countVal][fullSpacesVal] << RESET << endl;
            sem_post(&cmdWindow);

            // take the message out of the mailbox
            sem_wait(&mailboxProtectionSem[countVal]);
            mailboxArr[countVal][fullSpacesVal] = " ";
            sem_post(&mailboxProtectionSem[countVal]);

            sem_post(&emptySpacesSem[countVal]);
            
        }
        else{
            // compose a message addressed to someone else -- PRODUCER
            srand(time(0));
            int randPerson = chooseRandPerson(countVal); // choose a random person
            
            // check to see if more messages can be sent, if not break
            if (messagesLeftToSend > 0){
                sem_wait(&cmdWindow);
                cout << BLUE << "------Person " << countVal << " tries to send message to Person ";
                cout << randPerson << RESET << endl;
                sem_post(&cmdWindow);

                // check to see if their mailbox is at capacity yet
                int spacesLeft;
                sem_getvalue(&emptySpacesSem[randPerson], &spacesLeft);
                if(spacesLeft > 0){
                    // send them a message
                    string messageNum = to_string(messagesToSend - messagesLeftToSend);
                    string sentMessage = "Message " + messageNum + " to person " + to_string(randPerson);
                    
                    // check again to make sure that messages can still be sent
                    if (messagesLeftToSend > 0){
                        //send the message
                        sem_wait(&mailboxProtectionSem[randPerson]);
                        int mailboxSpot;
                        sem_getvalue(&fullSpacesSem[randPerson],&mailboxSpot);
                        mailboxArr[randPerson][mailboxSpot] = sentMessage; // put mail in the right spot
                        sem_post(&fullSpacesSem[randPerson]); // add one to full space
                        sem_wait(&emptySpacesSem[randPerson]); // subtract one from empty space
                        sem_post(&mailboxProtectionSem[randPerson]);
                        
                        sem_wait(&cmdWindow);
                        cout << MAGENTA << "------Person " << countVal << " sent msg: " << sentMessage << RESET << endl;
                        sem_post(&cmdWindow);       

                        // decrement the number of messages left to send
                        sem_wait(&numMessagesLeftToSendSem);
                        messagesLeftToSend--;
                        sem_post(&numMessagesLeftToSendSem);

                    }
                    else{
                        sem_wait(&cmdWindow);
                        cout << "--------Person " << countVal<< " cannot send message, there are no more to send." << endl;
                        sem_post(&cmdWindow);
                    }                    
                                     
                    
                    

                    // decrement the number of messages left to send
                    sem_wait(&numMessagesLeftToSendSem);
                    messagesLeftToSend--;
                    sem_post(&numMessagesLeftToSendSem);
                }
                else{
                    // tell the cmd window that their mailbox is at capacity
                    sem_wait(&cmdWindow);
                    cout << CYAN << "---------Person " << randPerson << "'s mailbox is at capacity." << RESET << endl;
                    sem_post(&cmdWindow);

                    sched_yield();
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
    
    for (int i=0; i<numPeople; i++){
        if(sem_destroy(&mailboxProtectionSem[i]) == -1){
            perror("sem_destroy");
            exit(EXIT_FAILURE);
        }
    }
    free(mailboxProtectionSem);

    for(int i=0; i<numPeople; i++){
        free(mailboxArr[i]);
    }
    free(mailboxArr);
    if (sem_destroy(&cmdWindow) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&argSem) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    for (int i=0; i<numPeople; i++){
        if(sem_destroy(&fullSpacesSem[i]) == -1){
            perror("sem_destroy");
            exit(EXIT_FAILURE);
        }
    }
    free(fullSpacesSem);
    for(int i=0; i<numPeople; i++){
        if(sem_destroy(&emptySpacesSem[i]) == -1){
            perror("sem_destroy");
            exit(EXIT_FAILURE);
        }
    }
    free(emptySpacesSem);
    if(sem_destroy(&numMessagesLeftToSendSem)== -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
}

int chooseRandPerson(int personNum){
    srand(time(0));
    int randNum;
    randNum = rand() % numPeople;
    if (personNum == randNum){
        // gotta try again
        if(personNum == (numPeople-1)) randNum = rand() % (numPeople - 1); 
        else if (personNum == 0) randNum = (rand() % (numPeople-1)) + 1;
        else{
            int upperRand = (rand() % ((numPeople-1) - personNum)) + (personNum + 1);
            int lowerRand = (rand() % personNum);
            if(rand() % 2 == 0) randNum = upperRand;
            else randNum = lowerRand;
        }
    }
    return randNum;
}