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
#include "task2.h"
#include "task3.h"
using namespace std;

/* cmd line: g++ main.cpp task1.cpp task2.cpp task3.cpp -o main.o ; ./main.o -A <num>
*/

using namespace std;

int main(int argc, char* argv[]){
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
            // run task 1
            runT1();
        }
        else if(strcmp(argv[2], "2") == 0){
            runT2();
        }
        else if(strcmp(argv[2], "3") == 0){
            // run task 3
            runT3();            
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

