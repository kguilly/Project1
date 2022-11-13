#include "iostream"
#include "stdlib.h"
#include "stdio.h"
#include "sched.h"
#include "time.h"
#include "pthread.h"
#include "limits"
#include "semaphore.h"
#include <cstring>

#include "task2.h"
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
int count2;

string ** mailboxArr; // [mailbox owner][message slots]


// Define semaphores
sem_t cmdWindow2; sem_t argSem; sem_t *fullSpacesSem; sem_t *emptySpacesSem;
sem_t numMessagesLeftToSendSem; sem_t *mailboxProtectionSem;

void getInput();
void validateSemaphores();
void * postOffice(void *);
int chooseRandPerson(int);
void garbageCollection2();

int runT2(){
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
        pthread_create(&threadArr[i], NULL, postOffice, &count2);
    }
    for (int i=0; i< numPeople; i++){
        pthread_join(threadArr[i], NULL);
    }


    // reallocate memory, destroy sems
    garbageCollection2();
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
        else if(numPeople == 1){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Simulation will not work with only one person, try again: ";
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
    if(sem_init(&cmdWindow2, 0, 1) == -1){
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

void *postOffice(void * arg){
    sem_wait(&argSem);
    int count2Val = (*(int *)arg)++;
    sem_post(&argSem);


    int fullSpacesVal = 0; //going to check the value of full spaces
    while(1){
        sem_wait(&cmdWindow2);
        cout << RED << "Person " << count2Val << " has entered the post office." << RESET << endl;
        sem_post(&cmdWindow2);
        

        sem_wait(&cmdWindow2);
        sem_getvalue(&fullSpacesSem[count2Val], &fullSpacesVal);
        cout << GREEN <<  "--Person " << count2Val << " checks -- he has " << fullSpacesVal;
        cout << " letters in his mailbox." << RESET << endl;
        sem_post(&cmdWindow2);

        sem_wait(&mailboxProtectionSem[count2Val]);
        sem_getvalue(&fullSpacesSem[count2Val], &fullSpacesVal);

        // if they have messages left in their mailbox, read them
        if(fullSpacesVal > 0){
            // read all messages until there are none left to read
            while(fullSpacesVal > 0){

                // read the thing -- CONSUMER
                sem_wait(&fullSpacesSem[count2Val]);
                sem_getvalue(&fullSpacesSem[count2Val], &fullSpacesVal);
                sem_wait(&cmdWindow2);
                cout << YELLOW << "----Person " << count2Val << " reads msg " << fullSpacesVal;
                cout << ": " << mailboxArr[count2Val][fullSpacesVal] << RESET << endl;
                sem_post(&cmdWindow2);
                
                // take the message out of the mailbox
                mailboxArr[count2Val][fullSpacesVal] = " ";
                sem_post(&emptySpacesSem[count2Val]);
                sem_post(&mailboxProtectionSem[count2Val]); // release, let something else write to box if need be

                // read a message, now yield
                sched_yield();

                sem_wait(&mailboxProtectionSem[count2Val]);
                sem_getvalue(&fullSpacesSem[count2Val], &fullSpacesVal);

            }
            sem_post(&mailboxProtectionSem[count2Val]);

            // once they finish reading their messages, leave the post office
            sem_wait(&cmdWindow2);
            cout << "Person " << count2Val << " leaves the post office." <<endl;
            sem_post(&cmdWindow2);

            // after they leave the post office, yield for 3 to 6 cycles
            srand(time(0));
            int randint = (rand() % 4) + 3; // random num b/w 3 and 6
            for(int i=0; i<=randint ; i++){
                sched_yield();
            }


        }
        else{
            sem_post(&mailboxProtectionSem[count2Val]); // waited before if block, need to release if no messages

            // compose a message addressed to someone else -- PRODUCER
            srand(time(0));
            int randPerson = chooseRandPerson(count2Val); // choose a random person
            
            // check to see if more messages can be sent, if not break
            if (messagesLeftToSend > 0){
                sem_wait(&cmdWindow2);
                cout << BLUE << "------Person " << count2Val << " tries to send message to Person ";
                cout << randPerson << RESET << endl;
                sem_post(&cmdWindow2);

                // check to see if their mailbox is at capacity yet
                int spacesLeft;
                sem_wait(&numMessagesLeftToSendSem);
                sem_getvalue(&emptySpacesSem[randPerson], &spacesLeft);
                if(spacesLeft > 0){
                    // send them a message
                    string messageNum = to_string(messagesToSend+1 - messagesLeftToSend);
                    string sentMessage = "Message " + messageNum + " to person " + to_string(randPerson);
                    
                    // check again to make sure that messages can still be sent
                    if (messagesLeftToSend > 0){
                        // sem_post(&numMessagesLeftToSendSem);
                        //send the message
                        sem_wait(&mailboxProtectionSem[randPerson]);
                        int mailboxSpot;
                        sem_getvalue(&fullSpacesSem[randPerson],&mailboxSpot);
                        mailboxArr[randPerson][mailboxSpot] = sentMessage; // put mail in the right spot
                        sem_post(&fullSpacesSem[randPerson]); // add one to full space
                        sem_wait(&emptySpacesSem[randPerson]); // subtract one from empty space
                        sem_post(&mailboxProtectionSem[randPerson]);
                        
                        sem_wait(&cmdWindow2);
                        cout << MAGENTA << "------Person " << count2Val << " sent msg: " << sentMessage << RESET << endl;
                        sem_post(&cmdWindow2);       

                        // decrement the number of messages left to send
                        // sem_wait(&numMessagesLeftToSendSem);
                        messagesLeftToSend--;
                        sem_post(&numMessagesLeftToSendSem);

                    }
                    else{
                        sem_post(&numMessagesLeftToSendSem);

                        sem_wait(&cmdWindow2);
                        cout << "--------Person " << count2Val<< " cannot send message, there are no more to send." << endl;
                        sem_post(&cmdWindow2);
                    }                    

                }
                else{
                    sem_post(&numMessagesLeftToSendSem);
                    // tell the cmd window that their mailbox is at capacity
                    sem_wait(&cmdWindow2);
                    cout << CYAN << "---------Person " << randPerson << "'s mailbox is at capacity." << RESET << endl;
                    sem_post(&cmdWindow2);

                    sched_yield();sched_yield();
                }
                
            }
            else{
                // if there are no more messages to be sent and their mailbox has no messages, leave
                sem_wait(&mailboxProtectionSem[count2Val]);
                sem_getvalue(&fullSpacesSem[count2Val], &fullSpacesVal);

                if(fullSpacesVal==0){
                    sem_post(&mailboxProtectionSem[count2Val]);
                    sem_wait(&cmdWindow2);
                    cout << "Person " << count2Val << " leaves the post office." <<endl;
                    sem_post(&cmdWindow2);
                    break;

                }
                sem_post(&mailboxProtectionSem[count2Val]);
            }

            // once they finish sending a message, leave the post office
            sem_wait(&cmdWindow2);
            cout << "Person " << count2Val << " leaves the post office." <<endl;
            sem_post(&cmdWindow2);

            // after they leave the post office, yield for 3 to 6 cycles
            srand(time(0));
            int randint = (rand() % 4) + 3; // random num b/w 3 and 6
            for(int i=0; i<=randint ; i++){
                sched_yield();
            }

            
        }

    }

    pthread_exit(0);
}

int chooseRandPerson(int personNum){
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

void garbageCollection2(){
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
    if (sem_destroy(&cmdWindow2) == -1){
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




