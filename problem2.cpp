/*
 *  Minotaur's Crystal Vase
 *
 *  Author: Ethan Woollet
 *  Date: Feb 2023 
 * 
 *  Professor: Juan Parra
 *  Class: Parallel and Distributed Processing (COP4520)
 * 
 */


#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>
#include <mutex>
#include <queue>
#include <thread>
#include <string>
#include <ctime>
#include <condition_variable>

using namespace std;
using namespace std::chrono;


// This is a basic thread safe wrapper class for a queue. It is specifically written for this program.
class ThreadSafeQueue {

    queue<int> queue;
    mutex mut_q;

    bool empty() {
        return queue.empty();
    }

    public:
    
        ThreadSafeQueue() = default;

        void push(int val) {
            
            lock_guard<mutex> lock(mut_q); // scoped lock

            queue.push(val);

            return;
        }

        int pop() { // could try to pop an empty queue, thus optional return type

            lock_guard<mutex> lock(mut_q); // scoped lock
            
            if(queue.empty()) { // can't pop, return an invalid value for this problem.
                return -1;
            }

            int val = queue.front();
            queue.pop();

            return val;
        }
};


// function prototypes
void start_party(int n);
void guest_function(int id);


// globals
int n_guests = 20; // Default # of Threads
int runs = 0; // number of times people saw the vase
int next_guest = -1; // id of the current thread (-1 means no thread)
ThreadSafeQueue party_queue; // queue to store the order of partygoers
condition_variable cv_next_guest; // Used for conditional waits
int guests_finished = 0; // number of guests who have finished 
bool party_started = false; // a variable used to stop the spawned threads from instantly entering their main loop


// mutexes
mutex mut_room;


int main(int argc, char **argv) {

    // take command line input for number of guests
    // if(argc != 2) {
    //     printf("Invalid input. Please enter a number of guests.\n`./problem2 <number of guests>`\n");
    //     return 1; // error
    // }

    // n_guests = stoi(argv[1]);
    printf("Number of guests: %d\n", n_guests);

    // seed the rng
    auto time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    srand(time);

    // run simulation
    auto start = high_resolution_clock::now(); // start time
    start_party(n_guests);
    auto end = high_resolution_clock::now(); // end time
    auto total_time = duration_cast<milliseconds>(end - start);

    cout << "\nThat party took " << runs << " runs, and " << total_time.count() << "ms.\nThanks for coming!\n\n~ The Minotaur\n\n";

    return 0;
}


bool check_id(int id) {
    return id == next_guest;
}


void start_party(int n) {

    queue<thread> pool;

    mut_room.lock(); // lock the room to start

    // spawn all the partygoers
    for(int i = 0; i < n; i++) { // spawn all threads in the pool
        thread t1(guest_function, i);
        pool.push(move(t1));
    }

    cout << "All guests are ready.\n";

    next_guest = party_queue.pop();
    while(next_guest < 0) {
        next_guest = party_queue.pop();
    }
    mut_room.unlock();

    party_started = true;
    cv_next_guest.notify_all(); // start the guests going through the room

    while(!pool.empty()) { // wait for all threads to finish, then destroy them (with shark lasers)
        pool.front().join();
        pool.pop();
    }

    return;
}


void guest_function(int id) {

    auto time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    srand(time & id);

    int has_ticket = true;

    party_queue.push(id);

    while(!party_started) { 
        // Nothing
    }

    while(has_ticket) {

        while(next_guest != id) {

            if(next_guest < 0) {
                return;
            }

            unique_lock<mutex> lock(mut_room);
            cv_next_guest.wait(lock, [&]{ return check_id(id); }); // waits for notification from current guest
        }

        // enter room
        unique_lock<mutex> lock(mut_room);

        // view vase

        // notify next guest and leave
        next_guest = party_queue.pop();

        if(next_guest < 0) {

            cv_next_guest.notify_all();
            break;
        }

        runs++;
        cv_next_guest.notify_all();

        if((rand())%2  == 1) { // 50% chance to requeue

            party_queue.push(id);
        }
        else {
            has_ticket = false; // guest gets rid of ticket
        }

    }
    
    return;
}
