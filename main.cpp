#include <iostream>
#include <pthread.h>
#include <chrono>
#include <vector>
#include <random>
#include <unistd.h>
#include <stack>
#include <semaphore.h>
#include <cstring>

using namespace std;
int numOfSeats; // number of seats
int *assignments; // results will be written here
void createClients();
void * clientThread(void*);
void * serverThread(void*);
pthread_mutex_t arriveMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t serverIdReceiveMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t s; // For server client enumeration synchronization.
int arrivalCounter = 1;
vector<pthread_mutex_t> mutexs;

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
    assignments = (int *) malloc(sizeof(int)*numOfSeats);
    if(!assignments){
        cerr << "Could not allocate memory with malloc for assignments array" << endl;
        exit(1);
    }
    for(int x = 1; x  <= numOfSeats; x++) {
        mutexs.push_back(PTHREAD_MUTEX_INITIALIZER);
    }
    if (sem_init(&s,0,0) == -1)
        printf("%s\n",strerror(errno));
    // Some initialization ^

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    for(int x = 1; x  <= numOfSeats; x++){
        pthread_t tid;
        rc = pthread_create(&tid,&attr, clientThread, nullptr);
        if (rc) {
            fprintf(stderr,"ERROR while creating clients; return code from pthread_create() in main is %d\n", rc);
            exit(-1);
        }
        myThreads.push_back(tid);
    }
    pthread_attr_destroy(&attr); // When done, free library resources used by the attribute with pthread_attr_destroy()

    // Thread creations ^

    for(int x = 0; x < numOfSeats; x++) {
        rc = pthread_join(myThreads[x], &status);
        if (rc) {
            printf("ERROR; return code from pthread_join() is %d\n", rc);
            exit(-1);
        }
      //  printf("Main: completed join with thread %d having a status of %ld\n",x,(long)status);
    }
    // Wait for them to finish by joining ^

   /* cout << "Number of total seats: " << numOfSeats << endl;
    for(int x = 0; x < numOfSeats; x++){
        cout << "Client" << x+1 << " reserves Seat" << assignments[numOfSeats] << endl;
    }
    cout << "All seats are reserved." << endl;*/
    // Output ^

    pthread_mutex_destroy(&arriveMutex);
    pthread_exit(0);
    return 0;
}

void * clientThread(void * param){

    int rc;
    //source:  http://www.cplusplus.com/reference/random/uniform_int_distribution/operator()

    // construct a trivial random generator engine from a time-based seed
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);
    uniform_int_distribution<int> distribution(50,200);
    int timeToSleep = distribution(generator);
    usleep(timeToSleep *1000); // usleep takes microseconds so multiply with 1000
    pthread_t server;

    pthread_mutex_lock(&arriveMutex);
    int *myId = (int*) malloc(sizeof(int));
    *myId = arrivalCounter;
    rc = pthread_create(&server, nullptr,serverThread,myId);
    if (rc) {
        printf("ERROR; return code from pthread_create() in client is %d\n", rc);
        exit(-1);
    }
    if (sem_wait(&s) != 0)
        printf("%s\n",strerror(errno));
    pthread_mutex_unlock(&arriveMutex);

    uniform_int_distribution<int> seatSelectorDist(1,numOfSeats);
    int selectedSeat = seatSelectorDist(generator);




    // Create associated server, pass the client id according to arrive time starting from 1

   // cout << "Thread created, Time sleeped is: " << timeToSleep <<   endl;
    pthread_exit(nullptr);
}
void * serverThread(void * clientId){

    int id = *(int*)clientId;
    arrivalCounter++;
    free(clientId);
    if (sem_post(&s) != 0)
        printf("%s\n",strerror(errno));


    cout << "I am server thread for client " << id << endl;
    pthread_exit(0);
}

