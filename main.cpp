#include <iostream>
#include <pthread.h>
#include <chrono>
#include <vector>

using namespace std;
int numOfSeats; // number of seats
int *assignments; // results will be written here
void createClients();
void * clientThread(void*);

vector<pthread_mutex_t> mutexs;

int main(int argc, char** argv) {

    if(argc!=2){
        cerr << "Usage: (executable) (number of seats)" << endl;
        exit(1);
    }else if(atoi(argv[1]) == 0){
        cerr << "Either num of seats was 0 or invalid."<<endl << "Enter a number between 50 and 100 please" << endl;
        exit(1);
    }
    else if(!(atoi(argv[1])<= 100 && atoi(argv[1])>=50)){
        cerr << "The seat number should be between 50 and 100 (both inclusive)" << endl;
        exit(1);
    }else{
        numOfSeats = atoi(argv[1]);
    }
    // basic input process into seats integer ^

    vector<pthread_t> myThreads;
    assignments = (int *) malloc(sizeof(int)*numOfSeats);
    if(!assignments){
        cerr << "Could not allocate memory with malloc for assignments array" << endl;
        exit(1);
    }
    for(int x = 1; x  <= numOfSeats; x++) {
        mutexs.push_back(PTHREAD_MUTEX_INITIALIZER);
    }
    // Some initialization ^

    for(int x = 1; x  <= numOfSeats; x++){
        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tid,&attr, clientThread, nullptr);
        myThreads.push_back(tid);
    }
    // Thread creations ^
    for(int x = 1; x <= numOfSeats; x++)
        pthread_join(myThreads[x-1],nullptr);

    // Wait for them to finish ^

    cout << "Number of total seats: " << numOfSeats << endl;
    for(int x = 0; x < numOfSeats; x++){
        cout << "Client" << x+1 << " reserves Seat" << assignments[numOfSeats] << endl;
    }
    cout << "All seats are reserved." << endl;
    // Output ^

    return 0;
}

void * clientThread(void * param){
    cout << "Thread created"  << endl;
    pthread_exit(nullptr);
}

