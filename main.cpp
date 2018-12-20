#include <iostream>
#include <pthread.h>
#include <chrono>
#include <vector>
#include <random>
#include <unistd.h>
#include <stack>
#include <semaphore.h>
#include <cstring>
#include <fstream>

using namespace std;
int numOfSeats; // number of seats
void * clientThread(void*);
void * serverThread(void*);

vector<bool> alreadyAssigned;
vector<pthread_mutex_t> *sel;

int clientCounter = 1;
pthread_mutex_t arriveMutex;
pthread_mutex_t writeLock;
ofstream out;


int main(int argc, char** argv) {
    void *status; // status returned from threads
    int rc;

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

    sel = new vector<pthread_mutex_t>();

    if(!sel){
        fprintf(stderr,"Unable to allocate memory for semaphore array, cannot proceed");
        exit(1);
    }else{
        for(int i =0;i< numOfSeats;i++){
            pthread_mutex_t mut;
            pthread_mutex_init(&mut,nullptr);
            sel->push_back(mut);
        }
    }
    for(int i =0;i< numOfSeats;i++){
        alreadyAssigned.push_back(false);
    }
    pthread_mutex_init(&arriveMutex, nullptr);
    pthread_mutex_init(&writeLock, nullptr);
    out.open("output.txt");

    // Some initialization ^
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    for(int x = 1; x  <= numOfSeats; x++){
        pthread_t tid;
        rc = pthread_create(&tid, &attr, clientThread, nullptr);
        if (rc) {
            fprintf(stderr,"ERROR while creating clients; return code from pthread_create() in main is %d\n", rc);
            exit(-1);
        }
        myThreads.push_back(tid);
    }

    // Thread creations ^
    cout << "Number of total seats: " << numOfSeats << endl;

    for(int x = 0; x < numOfSeats; x++) {
        rc = pthread_join(myThreads[x], &status);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
        //  printf("Main: completed join with thread %d having a status of %ld\n",x,(long)status);
    }
    // Wait for them to finish by joining ^

    // Output ^
    cout << "All seats are reserved." << endl;

    system((std::string("python3 scripts/test.py ") + to_string(numOfSeats)).c_str());
    pthread_exit(0);
}

void * clientThread(void * param){

    //source:  http://www.cplusplus.com/reference/random/uniform_int_distribution/operator()
    // construct a trivial random generator engine from a time-based seed

    int rc;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);
    uniform_int_distribution<int> distribution(50,200);


    int timeToSleep = distribution(generator);
    usleep(timeToSleep *1000); // usleep takes microseconds so multiply with 1000

    pthread_mutex_lock(&arriveMutex);
    int myId = clientCounter;
    clientCounter++;
    pthread_mutex_unlock(&arriveMutex);

    pthread_t server;
    uniform_int_distribution<int> selection(1,numOfSeats);

    bool jobDone = false;
    int selectedSeat = selection(generator);

    while(!jobDone){
        while(alreadyAssigned[selectedSeat-1]){
            selectedSeat = selection(generator);
        }
        pthread_mutex_lock(&sel->at(selectedSeat-1));
        if(alreadyAssigned[selectedSeat-1]){
            pthread_mutex_unlock(&sel->at(selectedSeat-1));
            continue;
        }
        int data[2] = {myId,selectedSeat};
        rc = pthread_create(&server, nullptr , serverThread, &data);
        if (rc) {
            printf("ERROR; return code from pthread_create() in client is %d\n", rc);
            exit(-1);
        }
        pthread_join(server, nullptr);
        jobDone = true;
        pthread_mutex_unlock(&sel->at(selectedSeat-1));
    }
    pthread_exit(nullptr);
}
void * serverThread(void * data){
    int id = *(int*)data;
    int selectedSeat = *((int*)data+1);
    pthread_mutex_lock(&writeLock);
    cout << "Client" << id << " reserves Seat" << selectedSeat << endl;
    out << selectedSeat << endl;
    alreadyAssigned.at(selectedSeat-1) = true;
    pthread_mutex_unlock(&writeLock);
    pthread_exit(nullptr);
}

