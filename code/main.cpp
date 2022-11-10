#include "iostream"
#include "stdlib.h"
#include "stdio.h"
#include "sched.h"
#include "time.h"
#include "pthread.h"
#include "limits"
#include "semaphore.h"
#include <cstring>
using namespace std;

/* cmd line: g++ main.cpp -o main.o ; ./main.o -A <num>
*/

using namespace std;
class Task3{



    // colored output
    #define RESET   "\033[0m"
    #define RED     "\033[31m"
    #define GREEN   "\033[32m"
    #define YELLOW  "\033[33m"
    #define BLUE    "\033[34m"
    #define MAGENTA "\033[35m"
    #define CYAN    "\033[36m"
    #define WHITE   "\033[37m"
    public:
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

        Task3(){
            this->readcount = 0; // controls when to wait and signal the area
            this->readerNum = 0; this->writerNum = 0; // identifying info for readers and writers
            this->stopSignal = 0; // signal given from writers to let readers know that all reads are done and to stop 
            this->totalReads = 0; // reps the total reads in the buffer at once
            this->timesWritten = 0; // keep track of the amount of writers that have come through

        }

        // funct headers
        void getInput(int &readers, int& writers, int &maxReadersAtOnce){
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
        void validateSems(){
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
        void garbageCollection(){
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
        void * read(void * arg){
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
        void * write(void *arg){
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
        void run(){
            printf("\n------------------------------------------\n");
            printf(  "   Readers-Writer Problem(Semapahores)\n\n");


            this->getInput(readers, writers, maxReadersAtOnce);
            this->validateSems();

            pthread_t readerArr[readers];
            pthread_t writerArr[writers];
            for(int i=0; i< readers; i++){
                pthread_create(&readerArr[i], NULL, &Task3::read_helper, &readerNum);
            }
            for(int i=0; i< writers; i++){
                pthread_create(&writerArr[i], NULL, &Task3::write_helper, &writerNum);
            }
            for(int i=0; i<readers; i++){
                pthread_join(readerArr[i], NULL);
            }
            for(int i=0; i<writers; i++){
                pthread_join(writerArr[i], NULL);
            }

            this->garbageCollection();
        }


        static void* read_helper(void * context){
            return ((Task3*)context) -> read(context);
        }

        static void* write_helper(void* context){
            return ((Task3*)context) -> write(context);
        }
    private:
    protected:
};

class Task2{
    // colored output
    #define RESET   "\033[0m"
    #define RED     "\033[31m"
    #define GREEN   "\033[32m"
    #define YELLOW  "\033[33m"
    #define BLUE    "\033[34m"
    #define MAGENTA "\033[35m"
    #define CYAN    "\033[36m"
    #define WHITE   "\033[37m"

    public: 
        // Define global vars
        int numPeople; // N from task sheet
        int mailboxCapacity; // S from task sheet
        int messagesToSend; // M from the task sheet
        int messagesLeftToSend; // keep track of how many messages can still be sent
        int count;

        string ** mailboxArr; // [mailbox owner][message slots]


        // Define semaphores
        sem_t cmdWindow; sem_t argSem; sem_t *fullSpacesSem; sem_t *emptySpacesSem;
        sem_t numMessagesLeftToSendSem; sem_t *mailboxProtectionSem;

        Task2(){
            this->numPeople=0;
            this->mailboxCapacity=0;
            this->messagesLeftToSend=0;
            this->count = 0;
        }
        //Define functions
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
        
        void *postOffice(void * arg){
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
            cout << "Person " << countVal << " leaves the post office." <<endl;
            sem_post(&cmdWindow);
            pthread_exit(0);
        }

        static void* postOffice_helper(void* context){
            return ((Task2 *)context)->postOffice(context);
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
        
        void run(){
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
                pthread_create(&threadArr[i], NULL, &Task2::postOffice_helper, &count);
            }
            for (int i=0; i< numPeople; i++){
                pthread_join(threadArr[i], NULL);
            }


            // reallocate memory, destroy sems
            garbageCollection();
        }

};

class Task1{
    // colored output
    #define RESET   "\033[0m"
    #define RED     "\033[31m"
    #define GREEN   "\033[32m"
    #define YELLOW  "\033[33m"
    #define BLUE    "\033[34m"
    #define MAGENTA "\033[35m"
    #define CYAN    "\033[36m"
    #define WHITE   "\033[37m"

    enum philState{
        thinking = 0,
        hungry = 1,
        eating = 2,
        waiting = 3
    };

    public: 
        int phils; int meals; int count; int philsWaiting;
        // init semaphores
        sem_t sitting;sem_t waitToSit;sem_t *chopstickArr; sem_t pickUpSticks; sem_t beginEat;
        sem_t endEat; sem_t getUp; sem_t leaving; sem_t cmdWindow; sem_t mealSem;

        Task1();
        void *diningTable(void * arg);
        static void * diningTable_helper(void* context);
        void getInput(int&phils, int &meals);     
        void eat(int philNum, philState &philState);
        void semaphoreCheck();
        void garbageCollection();
        void run();
};
Task1::Task1(){
    
    this->phils=0;
    this->meals=0;
    this->count=0;
    this->philsWaiting=0;      
}
void Task1::run(){
    printf("\n------------------------------------------\n");
    printf(  "   Dining Philosophers (Semapahores)\n\n");

    //prompt user and validate input
    Task1::getInput(phils, meals);
    cout << "\nTonight's dinner party will serve::\n" << phils << " philosophers ";
    cout << meals << " total meals. \n------------------------------------------\n"; 

    // for the report
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Initialize Semaphores
    Task1::semaphoreCheck();

    // init phil arr
    pthread_t philArr[phils];
    for(int i=0; i< phils; i++){
        pthread_create(&philArr[i], NULL, &Task1::diningTable_helper, &count);
    }

    for(int i=0; i<phils; i++){
        pthread_join(philArr[i], NULL);
    }

    // for the report
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &end);
    double runtime = (end.tv_sec - start.tv_sec) * 1000.0;
    runtime += (end.tv_nsec - start.tv_nsec) / 1000000.0;
    printf("Runtime in milliseconds = %f\n", runtime);

    //deallocate 
    Task1::garbageCollection();
}
void Task1::garbageCollection(){
    free(chopstickArr);
    for (int i=0; i<phils; i++){
        sem_destroy(&chopstickArr[i]);
    }
    if (sem_destroy(&sitting) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&waitToSit) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&pickUpSticks) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&beginEat) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&endEat) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&getUp) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&leaving) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&cmdWindow) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&mealSem) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
}
void Task1::semaphoreCheck(){
    // init chopstick array
    chopstickArr = (sem_t *)malloc(sizeof(sem_t)*phils);
    for (int i=0; i< phils; i++){
        if(sem_init(&chopstickArr[i], 0, 1) == -1){
            perror("sem_init");
            exit(EXIT_FAILURE);
        }
    }
    if(sem_init(&sitting, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&waitToSit, 0, 0) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&pickUpSticks, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&beginEat, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&endEat, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&getUp, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&leaving, 0, 0) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&cmdWindow, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&mealSem, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
}
void Task1::eat(int philNum, philState &philState){
    // phil has picked up his chopsticks, but double check that he can still eat
    sem_wait(&mealSem);
    if(meals <= 0){
        // he cannot eat
        sem_post(&mealSem);
        // say that he can't eat, put down his chopsticks, and leave
        sem_wait(&cmdWindow);
        cout << YELLOW << "There are no meals available for philosopher #" << philNum << "." << RESET << endl;
        sem_post(&cmdWindow);

        sem_wait(&pickUpSticks);
        sem_wait(&cmdWindow);
        sem_post(&chopstickArr[philNum]);
        cout << CYAN << "--------Philosopher #" << philNum << " puts down LEFT chopstick." << RESET << endl;
        sem_post(&cmdWindow);

        sem_wait(&cmdWindow);
        sem_post(&chopstickArr[(philNum + 1) % phils]);
        cout << CYAN << "--------Philosopher #" << philNum << " puts down RIGHT chopstick." << RESET << endl;
        sem_post(&cmdWindow);
        sem_post(&pickUpSticks);

        return;

    }
    sem_wait(&cmdWindow);
    meals--; // phil has claimed a meal
    sem_post(&mealSem);
    cout << MAGENTA << "------Philosopher #" << philNum << " is eating. " << RESET << endl;
    philState = eating;
    cout << "------Meals left: " << meals << endl;
    sem_post(&cmdWindow);
    

    // eat for a lil while
    srand(time(0));
    int randint = (rand() % 4) + 3; // random num b/w 3 and 6
    for(int i=0; i<=randint ; i++){
        sched_yield();
    }
    
    //phil has finished his meal, put down the chopsticks and begin thinking
    sem_wait(&pickUpSticks);
    sem_wait(&cmdWindow);
    sem_post(&chopstickArr[philNum]);
    cout << CYAN << "--------Philosopher #" << philNum << " puts down LEFT chopstick." << RESET << endl;
    sem_post(&cmdWindow);

    sem_wait(&cmdWindow);
    sem_post(&chopstickArr[(philNum + 1) % phils]);
    cout << CYAN << "--------Philosopher #" << philNum << " puts down RIGHT chopstick." << RESET << endl;
    cout << "--------Philosopher #" << philNum << " begins to think." << endl;
    sem_post(&cmdWindow);
    sem_post(&pickUpSticks);

    // think for a lil while
    philState = thinking;
    int randint2 = (rand() % 4) + 3; // random num b/w 3 and 6
    for(int i=0; i<=randint2 ; i++){
        sched_yield();
    } 

    // he's a hungry guy
    philState = hungry;

}
void Task1::getInput(int&phils, int&meals){
    cout << ("Enter the number of philosophers (2-10,000): ");
    cin >> phils;
    while(1){
        if(cin.fail()){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid goofball, try again: ";
            cin >> phils;
        }
        else if((phils > 10000) | (phils < 2)){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Enter a number in the range: ";
            cin >> phils;

        }
        else if(!(cin.fail())){
            break;
        }
    }

    cout << ("How many meals are these smarties gonna eat? (1-10,000): ");
    cin >> meals;
    while(1){
        if(cin.fail()){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Come on now try again. 1 to 10k: ";
            cin >> meals;
        }
        else if((meals > 10000) | (meals < 1)){
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "You're better than that. From 1 to 10k: ";
            cin >> meals;
        }
        else if(!(cin.fail())){
            break;
        }
    }
}
void*Task1::diningTable(void*arg){
    sem_wait(&sitting);
    int countval = (*((int *)arg))++;
    sem_wait(&cmdWindow);
    cout << GREEN << "-Philosopher #" << countval << " has arrived" << RESET << endl;
    sem_post(&cmdWindow);
    sem_post(&sitting);
    
    //Wait until all the phils have sat down
    if((countval + 1) == phils){
        sem_post(&waitToSit);
        cout << "--All the philosophers have arrived, they sit. " << endl;
    }
    sem_wait(&waitToSit);
    sem_post(&waitToSit);

    // attach a phil state
    philState philState = hungry;
    //init chopstick vars
    int leftChop;
    int rightChop;
    while(meals > 0){
        //try to pick up left and right chopsticks
        sem_wait(&pickUpSticks);
        sem_getvalue(&chopstickArr[countval], &leftChop);
        sem_getvalue(&chopstickArr[(countval+1)%phils], &rightChop);
        

        if((leftChop == 1) & (rightChop == 1) & (meals > 0)){
            // pick up the chopsticks, eat, and release the pickup variable
            sem_wait(&cmdWindow);
            sem_wait(&chopstickArr[countval]);
            cout << RED << "---Philosopher #" << countval << " picks up his LEFT chopstick. " << RESET << endl;
            sem_post(&cmdWindow);

            // sched_yield(); // report 


            sem_wait(&cmdWindow);
            sem_wait(&chopstickArr[(countval+1)%phils]);
            cout << RED << "---Philosopher #" << countval << " picks up his RIGHT chopstick." << RESET << endl;
            sem_post(&cmdWindow);
            sem_post(&pickUpSticks);
            eat(countval, philState);

        }
        else if((leftChop == 1) & (rightChop == 1) & (meals == 0)){
            sem_post(&pickUpSticks);
            sem_wait(&cmdWindow);
            cout << YELLOW << "There are no meals available for philosopher #" << countval<< "." << RESET << endl;
            sem_post(&cmdWindow);
        }   
        else{
            sem_post(&pickUpSticks);
        }
    }

    philState = waiting;
    // once phils are waiting, wait for all of them to be ready to go
    sem_wait(&getUp);
    philsWaiting++;
    sem_post(&getUp);

    // implement barrier
    if(philsWaiting == phils){
        cout << "--------All philosophers are ready to leave the table" << endl;
        sem_post(&leaving);
    }
    sem_wait(&leaving);
    sem_post(&leaving);

    sem_wait(&cmdWindow);
    cout << BLUE << "---------Philosopher #" << countval << " has gotten up and left. " << RESET << endl;
    sem_post(&cmdWindow);

    pthread_exit(0);
}
static void* Task1::diningTable_helper(void*context){
    return ((Task1*)context)->diningTable(context);
}



int main(int argc, char* argv[]){
    // TESTING //
    Task1 t1;
    t1.run();
    
    // check if there's too little arguments
    if (argc <= 1){
        cout << "You cannot have 0 parameters." << endl;
        return 1;
    }
    if (argc <= 2){
        cout << "You must pass another argument." << endl;
        return 1;
    }
    
    // check if there's too many arguments
    if (argc > 3){
        cout << "Error. Too many arguments. Expected 2" << endl;
        return 1;
    }
    

    // if correct entry
    if(strcmp(argv[1],"-A") == 0){
        if(strcmp(argv[2], "1") == 0){
            Task1 t1;
            t1.run();
        }
        else if(strcmp(argv[2], "2") == 0){
            Task2 t2;
            t2.run();
        }
        else if(strcmp(argv[2], "3") == 0){
            // run task 3
            Task3 t3;
            t3.run();
            
        }
        else{
            cout << "Error, incorrect arguments. The options are 1, 2, or 3. " << endl;
            return 1;
        }
        
    }else{
        cout << "\"" << argv[1] << "\" is not a recognized argument, please try again.\n";
        cout << "Did you mean \"-A\"?"<< endl;
    }
     
    return 0;


}

