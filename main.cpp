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
void * clientThread(void*);
void * serverThread(void*);
pthread_mutex_t arriveMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t replacementMutex = PTHREAD_MUTEX_INITIALIZER;

sem_t srv; // For server client enumeration synchronization.
sem_t * cli; // For client to select again
int arrivalCounter = 1;
vector<pthread_mutex_t> mutexs;

int * selectionArray;



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
    assignments = (int *) calloc(numOfSeats,sizeof(int));
    if(!assignments){
        cerr << "Could not allocate memory with calloc for assignments array" << endl;
        exit(1);
    }
    for(int x = 1; x  <= numOfSeats; x++) {
        mutexs.push_back(PTHREAD_MUTEX_INITIALIZER);
    }
    if (sem_init(&srv,0,0) == -1)
        printf("%s\n",strerror(errno));

    selectionArray  = (int*) calloc(numOfSeats,sizeof(int));
    if(!selectionArray){
        fprintf(stderr,"Unable to allocate memory for selection array, cannot proceed");
        exit(1);
    }
    if (sem_init(&srv,0,0) == -1)
        printf("%s\n",strerror(errno));


    cli  = (sem_t*) calloc(numOfSeats,sizeof(sem_t));
    if(!cli){
        fprintf(stderr,"Unable to allocate memory for cli semaphor array, cannot proceed");
        exit(1);
    }
    for(int i = 0;i< numOfSeats;i++){
        if (sem_init(&cli[i],0,1) == -1)
            printf("%s\n",strerror(errno));
    }
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

    cout << "Number of total seats: " << numOfSeats << endl;
    for(int x = 0; x < numOfSeats; x++){
        cout << "Client" << x+1 << " reserves Seat" << assignments[numOfSeats] << endl;
    }
    cout << "All seats are reserved." << endl;
    // Output ^

    pthread_mutex_destroy(&arriveMutex);
    pthread_exit(0);
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
    int *myId = (int*) calloc(1,sizeof(int));
    if(!myId){
        fprintf(stderr,"Unable to allocate memory for id integer, cannot proceed");
        exit(1);
    }
    *myId = arrivalCounter;
    rc = pthread_create(&server, nullptr,serverThread,myId);
    // Create associated server, pass the client id according to arrive time starting from 1

    if (rc) {
        printf("ERROR; return code from pthread_create() in client is %d\n", rc);
        exit(-1);
    }
    if (sem_wait(&srv) != 0)
        printf("%s\n",strerror(errno));
    pthread_mutex_unlock(&arriveMutex);

    while(true){
        if (sem_wait(&cli[*myId-1]) != 0)
            printf("%s\n",strerror(errno));

        uniform_int_distribution<int> seatSelectorDist(1,numOfSeats);
        int selectedSeat = seatSelectorDist(generator);
        selectionArray[*myId-1] = selectedSeat;


        if(assignments[*myId-1])
            pthread_exit(nullptr);
    }
}
void * serverThread(void * clientId){

    int id = *(int*)clientId;
    arrivalCounter++;
    free(clientId);
    if (sem_post(&srv) != 0)
        printf("%s\n",strerror(errno));

    while(true){
        if(selectionArray[id-1]) {
            pthread_mutex_lock(&replacementMutex);
            if(!assignments[selectionArray[id-1]-1]){
                assignments[selectionArray[id-1]-1] = id;
            }else{
                selectionArray[id-1] = 0;
                if (sem_post(&cli[id-1]) != 0)
                    printf("%s\n",strerror(errno));
            }
            pthread_mutex_unlock(&replacementMutex);
            pthread_exit(nullptr);
        }
    }
}

