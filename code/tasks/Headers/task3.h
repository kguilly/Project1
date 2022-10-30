#ifndef TASK3_H
#define TASK3_H

#include "iostream"
#include "stdlib.h"
#include "stdio.h"
#include "sched.h"
#include "time.h"
#include "pthread.h"
#include "limits"
#include "semaphore.h"
using namespace std;

class Task3{
    public:
        Task3();
        int run();

    private:
        // init vars
        int readers; int writers; int maxReadersAtOnce; // init vars from user
        int readcount; // controls when to wait and signal the area
        int readerNum; int writerNum; // identifying info for readers and writers
        int stopSignal; // signal given from writers to let readers know that all reads are done and to stop 
        int totalReads; // reps the total reads in the buffer at once
        int timesWritten; // keep track of the amount of writers that have come through

        // init sems
        sem_t area; sem_t mutex; sem_t cmdWindow; 
        sem_t argSem; // var that changes the value of the identifing number for readers and writers
        sem_t readersWait; // only allows the specified number of readers in the buffer at once
        sem_t totalReadsSem; // protect the totalReads variable
        sem_t writersWait; // let a writer go only when the maxreadersatonce number is hit
        sem_t timesWrittenSem; // var security

        // funct headers
        void getInput(int &readers, int& writers, int &maxReadersAtOnce);
        void validateSems();
        void garbageCollection();
        void * read(void * arg);
        void * write(void *arg);

    protected:
};



#endif