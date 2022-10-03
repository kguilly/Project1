#include "iostream"
#include "stdlib.h"
#include "stdio.h"
// #include "tasks/task1.cpp"
// #include "tasks/task2.cpp"
// #include "tasks/task3.cpp"
#include "string.h"
#include "limits"
using namespace std;

/* cmd line: g++ main.cpp -o main.o ; ./main.o -A <num>
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
            system("g++ tasks/task1.cpp -o task1; ./task1");
        }
        else if(strcmp(argv[2], "2") == 0){
            system("g++ tasks/task2.cpp -o task2; ./task2");
        }
        else if(strcmp(argv[2], "3") == 0){
            system("g++ tasks/task3.cpp -o task3; ./task3");
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