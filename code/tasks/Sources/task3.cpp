#include "../Headers/task3.h"

#include "iostream"
#include "stdlib.h"
#include "stdio.h"
#include "sched.h"
#include "time.h"
#include "pthread.h"
#include "limits"
#include "semaphore.h"
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


Task3::Task3(){
    this->readcount = 0; // controls when to wait and signal the area
    this->readerNum = 0; this->writerNum = 0; // identifying info for readers and writers
    this->stopSignal = 0; // signal given from writers to let readers know that all reads are done and to stop 
    this->totalReads = 0; // reps the total reads in the buffer at once
    this->timesWritten = 0; // keep track of the amount of writers that have come through

}

void Task3::getInput(int &readers, int& writers, int &maxReadersAtOnce){
    cout << ("Enter the number of readers (1-10,000): ");
    cin >> readers;
    while(1){
        if(cin.fail()){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid goofball, try again: ";
            cin >> readers;
        }
        else if((readers > 10000) | (readers < 1)){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Enter a number in the range: ";
            cin >> readers;

        }
        else if(!(cin.fail())){
            break;
        }
    }

    cout << ("Enter the number of writers (1-10,000): ");
    cin >> writers;
    while(1){
        if(cin.fail()){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Come on now try again. 1 to 10k: ";
            cin >> writers;
        }
        else if((writers > 10000) | (writers < 1)){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "You're better than that. From 1 to 10k: ";
            cin >> writers;
        }
        else if(!(cin.fail())){
            break;
        }
    }
    
    cout << "Enter the total number of readers at a time (1-" << readers << "):";
    cin >> maxReadersAtOnce;
    while(1){
        if(cin.fail()){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid. Enter a number between 1 and " << readers << "):";
            cin >> maxReadersAtOnce;
        }
        else if((maxReadersAtOnce > readers) | (maxReadersAtOnce < 1)){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid. Enter a number between 1 and " << readers << "):";
            cin >> maxReadersAtOnce;
        }
        else if(!(cin.fail())){
            break;
        }
    }
}

void Task3::validateSems(){
    if(sem_init(&cmdWindow, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&area, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&mutex, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&argSem, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&readersWait, 0, maxReadersAtOnce) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&totalReadsSem, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&writersWait, 0, 0) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&timesWrittenSem, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
}

void Task3::garbageCollection(){

     if(sem_destroy(&cmdWindow)== -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if(sem_destroy(&area)== -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if(sem_destroy(&mutex)== -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if(sem_destroy(&argSem) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if(sem_destroy(&readersWait) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if(sem_destroy(&totalReadsSem) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if(sem_destroy(&writersWait) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if(sem_destroy(&timesWrittenSem) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }

}

void * Task3::read(void * arg){
    sem_wait(&argSem);
    // start reading 
    int readerNum_ = (*(int *)arg)++;
    sem_post(&argSem);

    while(1){
        // check the stop signal
        if (stopSignal == 1) break;

        // if the buffer is at the max num of allowed reads, make remaining readers wait
        sem_wait(&readersWait);
        
        // before reading, check the stop signal
        if (stopSignal==1) break;

        sem_wait(&mutex);
        readcount++;
        if(readcount == 1) sem_wait(&area);
        sem_post(&mutex);

        // read
        sem_wait(&cmdWindow);
        cout << CYAN << "--Reader" << readerNum_ << " is reading." << RESET << endl;
        sem_post(&cmdWindow);

        srand(time(0));
        int randint = (rand() % 4) + 3; // random num b/w 3 and 6
        for(int i=0; i<=randint ; i++){
            sched_yield();
        }

        //release
        sem_wait(&mutex);
        readcount --;

        // reader has finished reading
        sem_wait(&cmdWindow);
        sem_wait(&totalReadsSem);
        cout<< CYAN << "--Reader" << readerNum_ << " has finished reading. Total Reads: " << ++totalReads << endl;
        sem_post(&totalReadsSem);
        sem_post(&cmdWindow);
        if(readcount == 0) sem_post(&area);
        sem_post(&mutex);

        // if the number of reads at once is reached, let a writer go
        if (totalReads == maxReadersAtOnce) sem_post(&writersWait);
    }
    pthread_exit(0);
}

void * Task3::write(void * arg){
    sem_wait(&argSem);
    int writerNum_ = (*(int *)arg)++;
    sem_post(&argSem);

    // wait to write until maxNumReadersinBuffer is reached
    sem_wait(&writersWait);

    sem_wait(&area);
    // write
    sem_wait(&cmdWindow);
    cout << MAGENTA << "---WriterNum" << writerNum_ << " writes to buffer." << RESET << endl;
    sem_post(&cmdWindow);

    srand(time(0));
    int randint = (rand() % 4) + 3; // random num b/w 3 and 6
    for(int i=0; i<=randint ; i++){
        sched_yield();
    }

    sem_post(&area);

    // Writer has finished writing, hand it back over to the readers
    sem_wait(&cmdWindow);
    cout << MAGENTA << "---WriterNum" << writerNum_ << " is finished writing." << RESET << endl;
    sem_post(&cmdWindow);

    // increment timeswritten
    sem_wait(&timesWrittenSem);
    timesWritten++;
    sem_post(&timesWrittenSem);

    sem_wait(&totalReadsSem);
    totalReads = 0;
    sem_post(&totalReadsSem);
    for(int i=0; i<maxReadersAtOnce; i++){
        sem_post(&readersWait);
    }
    
    // if all writers have finished writing, send the stop signal to stop reading
    if (timesWritten >= writers){
        stopSignal = 1;
        // release any other readers that may be stuck in sem queue
        for(int i=0; i<readers; i++) sem_post(&readersWait);
    } 
    pthread_exit(0);
}

