/*
 *  Minotaur's Birthday Party
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

using namespace std;
using namespace std::chrono;


// Thread functions
void guest_function(int id); // Instructions for any normal guest thread
void leader_function(int n_guests); // instructions for the 'leader' of the party-goers
int start_party(int n); // Runs the simulation


// globals
bool is_cupcake = true; // cupcake at the end of the labyrinth
bool all_visited = false; // flag for the minotaur to know everyone has gone at least once
bool in_maze = false; // a flag for if someone is in the maze currently
int current_guest = -1; // how the minotaur announces who goes


// mutexes for globals
mutex mut_is_cupcake;
mutex mut_all_visited;
mutex mut_in_maze; 
mutex mut_current_guest; 


int main(int argc, char **argv) {

    // take command line input for number of guests
    if(argc != 2) {
        printf("Invalid input. Please enter a number of guests.\n`./problem1 <number of guests>`\n");
        return 1; // error
    }

    int n_guests = stoi(argv[1]);
    printf("Number of guests: %d\n", n_guests);

    // seed the rng
    auto time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    srand(time);

    // run simulation
    auto start = high_resolution_clock::now(); // start time
    int runs = start_party(n_guests);
    auto end = high_resolution_clock::now(); // end time

    auto total_time = duration_cast<milliseconds>(end - start);

    cout << "\n\nThat party took " << runs << " runs, and " << total_time.count() << "ms.\nThanks for coming!\n\n~ The Minotaur\n\n";

    return 0;
}


void guest_function(int id) {

    bool has_visited = false;

    while(true) {

        mut_all_visited.lock();
        if(all_visited) { // minotaur says stop
            mut_all_visited.unlock();
            break;
        }
        mut_all_visited.unlock();

        mut_current_guest.lock();
        if(current_guest == id) { // enter maze

            // traversing maze

            mut_is_cupcake.lock();
            if(!is_cupcake && !has_visited) { 

                // call servant for new cupcake
                is_cupcake = true;
                has_visited = true;
            }

            mut_is_cupcake.unlock();
            // return to start
            current_guest = -1;

            mut_in_maze.lock();
            in_maze = false;
            mut_in_maze.unlock();
        }
        mut_current_guest.unlock();

    }
    
    return;
}



void leader_function(int n_guests) {

    int count = 0;
    int id = 0;

    while(true) {

        if(count >= n_guests) { // minotaur says stop

            mut_all_visited.lock();
            all_visited = true;
            mut_all_visited.unlock();
            break;
        }

        mut_current_guest.lock();
        if(current_guest == id) { // enter maze

            mut_in_maze.lock();
            in_maze = true;
            mut_in_maze.unlock();

            // traversing maze

            mut_is_cupcake.lock();

            if(is_cupcake) { 

                // eat the cupcake
                is_cupcake = false;
                count++;

            }
            
            mut_is_cupcake.unlock();
            // return to start
            current_guest = -1;

            mut_in_maze.lock();
            in_maze = false;
            mut_in_maze.unlock();
        }

        mut_current_guest.unlock();
    }

    return;
}


int start_party(int n) {

    queue<thread> pool;
    
    // spawn the leader
    thread t(leader_function, n);
    pool.push(move(t));

    // spawn everyone else
    for(int i = 1; i < n; i++) { // spawn all threads in the pool
        thread t1(guest_function, i);
        pool.push(move(t1));
    }


    int i = 0;

    mut_all_visited.lock();
    do {
        mut_all_visited.unlock();

        mut_in_maze.lock();
        do {
            mut_in_maze.unlock();
            // this_thread::sleep_for(chrono::milliseconds(1) );
            mut_in_maze.lock();
        }
        while(in_maze);
        mut_in_maze.unlock();

        mut_all_visited.lock();
        if(!all_visited) {
            mut_all_visited.unlock();

            mut_in_maze.lock();
            in_maze = true;
            mut_in_maze.unlock();

            mut_current_guest.lock();
            current_guest = rand()%n;
            mut_current_guest.unlock();
            // cout << "Guest " << current_guest << " is up! ";
            
            i++;
        }
        else {
            mut_all_visited.unlock();
        }
        
        mut_all_visited.lock();
    } 
    while (!all_visited);

    mut_all_visited.unlock();

    while(!pool.empty()) { // wait for all threads to finish, then destroy them (with laser sharks)
        pool.front().join();
        pool.pop();
    }

    return i;
}