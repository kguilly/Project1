#include "iostream"
#include "stdlib.h"
#include "stdio.h"
#include "sched.h"
#include "time.h"
#include "pthread.h"
#include "limits"
#include "semaphore.h"

/* Cmd Line: g++ -pthread -Wall task3.cpp -o task3.o ; ./task3.o       */
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


// init vars
int readers; int writers; int maxReadersAtOnce;

// init sems


// funct headers
void getInput(int &readers, int& writers, int &maxReadersAtOnce);

int main(){
    printf("\n------------------------------------------\n");
    printf(  "   Readers-Writer Problem(Semapahores)\n\n");

    getInput(readers, writers, maxReadersAtOnce);

    return 0;
}

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
}
