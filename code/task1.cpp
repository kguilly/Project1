#include "iostream"
#include "stdlib.h"
#include "stdio.h"
#include "sched.h"
#include "time.h"
#include "pthread.h"
#include "limits"
#include "semaphore.h"
#include <cstring>

#include "task1.h"
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
enum philState{
    thinking = 0,
    hungry = 1,
    eating = 2,
    waiting = 3
};

int phils; int meals; int count1; int philsWaiting;
        // init semaphores
sem_t sitting;sem_t waitToSit;sem_t *chopstickArr; sem_t pickUpSticks; sem_t beginEat;
sem_t endEat; sem_t getUp; sem_t leaving; sem_t cmdWindow1; sem_t mealSem;


void *diningTable(void * arg);
void getInput(int&phils, int &meals);     
void eat(int philNum, philState &philState);
void semaphoreCheck();
void garbageCollection1();

int runT1(){
    printf("\n------------------------------------------\n");
    printf(  "   Dining Philosophers (Semapahores)\n\n");

    //prompt user and validate input
    getInput(phils, meals);
    cout << "\nTonight's dinner party will serve::\n" << phils << " philosophers ";
    cout << meals << " total meals. \n------------------------------------------\n"; 

    // for the report
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Initialize Semaphores
    semaphoreCheck();

    // init phil arr
    pthread_t philArr[phils];
    for(int i=0; i< phils; i++){
        pthread_create(&philArr[i], NULL, diningTable, &count1);
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
    garbageCollection1();

    return 0;
} 

void getInput(int&phils, int &meals){
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

void semaphoreCheck(){
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
    if(sem_init(&cmdWindow1, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    if(sem_init(&mealSem, 0, 1) == -1){
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
}

void*diningTable(void*arg){
    sem_wait(&sitting);
    int count1val = (*((int *)arg))++;
    sem_wait(&cmdWindow1);
    cout << GREEN << "-Philosopher #" << count1val << " has arrived" << RESET << endl;
    sem_post(&cmdWindow1);
    sem_post(&sitting);
    
    //Wait until all the phils have sat down
    if((count1val + 1) == phils){
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
        sem_getvalue(&chopstickArr[count1val], &leftChop);
        sem_getvalue(&chopstickArr[(count1val+1)%phils], &rightChop);
        

        if((leftChop == 1) & (rightChop == 1) & (meals > 0)){
            // pick up the chopsticks, eat, and release the pickup variable
            sem_wait(&cmdWindow1);
            sem_wait(&chopstickArr[count1val]);
            cout << RED << "---Philosopher #" << count1val << " picks up his LEFT chopstick. " << RESET << endl;
            sem_post(&cmdWindow1);

            // sched_yield(); // report 


            sem_wait(&cmdWindow1);
            sem_wait(&chopstickArr[(count1val+1)%phils]);
            cout << RED << "---Philosopher #" << count1val << " picks up his RIGHT chopstick." << RESET << endl;
            sem_post(&cmdWindow1);
            sem_post(&pickUpSticks);
            eat(count1val, philState);

        }
        else if((leftChop == 1) & (rightChop == 1) & (meals == 0)){
            sem_post(&pickUpSticks);
            sem_wait(&cmdWindow1);
            cout << YELLOW << "There are no meals available for philosopher #" << count1val<< "." << RESET << endl;
            sem_post(&cmdWindow1);
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

    sem_wait(&cmdWindow1);
    cout << BLUE << "---------Philosopher #" << count1val << " has gotten up and left. " << RESET << endl;
    sem_post(&cmdWindow1);

    pthread_exit(0);
}

void eat(int philNum, philState &philState){
    // phil has picked up his chopsticks, but double check that he can still eat
    sem_wait(&mealSem);
    if(meals <= 0){
        // he cannot eat
        sem_post(&mealSem);
        // say that he can't eat, put down his chopsticks, and leave
        sem_wait(&cmdWindow1);
        cout << YELLOW << "There are no meals available for philosopher #" << philNum << "." << RESET << endl;
        sem_post(&cmdWindow1);

        sem_wait(&pickUpSticks);
        sem_wait(&cmdWindow1);
        sem_post(&chopstickArr[philNum]);
        cout << CYAN << "--------Philosopher #" << philNum << " puts down LEFT chopstick." << RESET << endl;
        sem_post(&cmdWindow1);

        sem_wait(&cmdWindow1);
        sem_post(&chopstickArr[(philNum + 1) % phils]);
        cout << CYAN << "--------Philosopher #" << philNum << " puts down RIGHT chopstick." << RESET << endl;
        sem_post(&cmdWindow1);
        sem_post(&pickUpSticks);

        return;

    }
    sem_wait(&cmdWindow1);
    meals--; // phil has claimed a meal
    sem_post(&mealSem);
    cout << MAGENTA << "------Philosopher #" << philNum << " is eating. " << RESET << endl;
    philState = eating;
    cout << "------Meals left: " << meals << endl;
    sem_post(&cmdWindow1);
    

    // eat for a lil while
    srand(time(0));
    int randint = (rand() % 4) + 3; // random num b/w 3 and 6
    for(int i=0; i<=randint ; i++){
        sched_yield();
    }
    
    //phil has finished his meal, put down the chopsticks and begin thinking
    sem_wait(&pickUpSticks);
    sem_wait(&cmdWindow1);
    sem_post(&chopstickArr[philNum]);
    cout << CYAN << "--------Philosopher #" << philNum << " puts down LEFT chopstick." << RESET << endl;
    sem_post(&cmdWindow1);

    sem_wait(&cmdWindow1);
    sem_post(&chopstickArr[(philNum + 1) % phils]);
    cout << CYAN << "--------Philosopher #" << philNum << " puts down RIGHT chopstick." << RESET << endl;
    cout << "--------Philosopher #" << philNum << " begins to think." << endl;
    sem_post(&cmdWindow1);
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

void garbageCollection1(){
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
    if (sem_destroy(&cmdWindow1) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
    if (sem_destroy(&mealSem) == -1){
        perror("sem_destroy");
        exit(EXIT_FAILURE);
    }
}


