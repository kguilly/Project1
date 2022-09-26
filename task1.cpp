#include "iostream"
#include "stdlib.h"
#include "stdio.h"
#include "sched.h"
#include "time.h"
#include "pthread.h"
#include "limits"
#include "semaphore.h"

using namespace std;
/* Command line arg: g++ -pthread -Wall task1.cpp -o task1.o; ./task1.o  */
//init vars
enum philState{
    thinking = 0,
    hungry = 1,
    eating = 2,
    waiting = 3 
};
int phils; int meals; int count =0; int philsWaiting = 0;
// init semaphores
sem_t sitting;sem_t waitToSit;sem_t *chopstickArr; sem_t pickUpSticks; sem_t beginEat;
sem_t endEat; sem_t getUp; sem_t leaving; sem_t cmdWindow;


void *diningTable(void * arg);
void getInput(int&phils, int &meals);
void eat(int philNum, philState &philState);

int main(){
    printf("\n----------------------------------------\nDining Philosophers (Semapahores)\n");

    //prompt user and validate input
    getInput(phils, meals);
    cout << "\nTonight's dinner party will serve::\n" << phils << " philosophers ";
    cout << meals << " total meals. \n----------------------------------------\n----------------------------------------\n"; 

    // init chopstick array
    chopstickArr = (sem_t *)malloc(sizeof(sem_t)*phils);
    for (int i=0; i< phils; i++){
        sem_init(&chopstickArr[i], 0, 1);
    }

    // wait until the phils have sat down
    sem_init(&sitting, 0, 1);
    sem_init(&waitToSit, 0, 0);
    sem_init(&pickUpSticks, 0, 1);
    sem_init(&beginEat, 0, 1);
    sem_init(&endEat, 0, 1);
    sem_init(&getUp, 0, 1);
    sem_init(&leaving, 0, 0);
    sem_init(&cmdWindow, 0, 1);
    // init phil arr
    pthread_t philArr[phils];
    for(int i=0; i< phils; i++){
        pthread_create(&philArr[i], NULL, diningTable, &count);
    }

    for(int i=0; i<phils; i++){
        pthread_join(philArr[i], NULL);
    }


    //deallocate 
    free(chopstickArr);
    for (int i=0; i<phils; i++){
        sem_destroy(&chopstickArr[i]);
    }
    sem_destroy(&sitting);
    sem_destroy(&waitToSit);
    sem_destroy(&pickUpSticks);
    sem_destroy(&beginEat);
    sem_destroy(&endEat);
    sem_destroy(&getUp);
    sem_destroy(&leaving);
    sem_destroy(&cmdWindow);
    return 0;

}

void getInput(int &phils, int &meals){
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

void * diningTable(void * arg){
    sem_wait(&sitting);
    int countval = (*((int *)arg))++;
    cout << "-Philosopher #" << countval << " has arrived" << endl;
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
        sem_wait(&cmdWindow);
        cout << "---Philosopher #" << countval << "'s LEFT chopstick";
        cout << ((leftChop==1) ? " is available" : " is not available.") << endl;
        sem_post(&cmdWindow);

        sem_wait(&cmdWindow);
        sem_getvalue(&chopstickArr[(countval+1)%phils], &rightChop);
        cout << "---Philosopher #" << countval << "'s RIGHT chopstick";
        cout << ((rightChop==1) ? " is available" : " is not available.") << endl;
        sem_post(&cmdWindow);

        if((leftChop == 1) & (rightChop == 1) & (meals > 0)){
            // pick up the chopsticks, eat, and release the pickup variable
            sem_wait(&chopstickArr[countval]);
            sem_wait(&chopstickArr[(countval+1)%phils]);
            sem_post(&pickUpSticks);
            eat(countval, philState);

        }
        else if((leftChop == 1) & (rightChop == 1) & (meals == 0)){
            sem_wait(&cmdWindow);
            cout << "There are no meals available for philosopher #" << countval<< "." << endl;
            sem_post(&cmdWindow);
            sem_post(&pickUpSticks);
        }   
        else{
            sem_post(&pickUpSticks);
            sched_yield();
        }
    }

    philState = waiting;
    // if all phils are ready to leave, let em go
    sem_wait(&getUp);
    philsWaiting++;
    sem_post(&getUp);

    if(philsWaiting == phils){
        cout << "--------All philosophers are ready to leave the table" << endl;
        sem_post(&leaving);
    }
    sem_wait(&leaving);
    cout << "---------Philosopher #" << countval << " has gotten up and left. " << endl;
    sem_post(&leaving);

    pthread_exit(0);

}

void eat(int philNum, philState &philState){
    sem_wait(&cmdWindow);
    cout << "------Philosopher #" << philNum << " is eating. " << endl;
    philState = eating;
    //phil has claimed a meal, decrement
    meals--;
    cout << "------Meals left: " << meals << endl;
    sem_post(&cmdWindow);

    // eat for a lil while
    srand(time(0));
    int randint = (rand() % 4) + 3; // random num b/w 3 and 6
    for(int i=0; i<=randint ; i++){
        sched_yield();
    }
    
    //phil has finished his meal, put down the chopsticks and begin thinking
    sem_post(&chopstickArr[philNum]);
    sem_wait(&cmdWindow);
    cout << "--------Philosopher #" << philNum << " puts down LEFT chopstick." << endl;
    sem_post(&cmdWindow);

    sem_post(&chopstickArr[(philNum + 1) % phils]);
    sem_wait(&cmdWindow);
    cout << "--------Philosopher #" << philNum << " puts down RIGHT chopstick." << endl;
    cout << "--------Philosopher #" << philNum << " begins to think." << endl;
    sem_post(&cmdWindow);

    // think for a lil while
    philState = thinking;
    int randint2 = (rand() % 4) + 3; // random num b/w 3 and 6
    for(int i=0; i<=randint2 ; i++){
        sched_yield();
    } 

    philState = hungry;

}

