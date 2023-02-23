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
int n_guests = 20;
bool is_cupcake = true; // cupcake at the end of the labyrinth
bool all_visited = false; // flag for the minotaur to know everyone has gone at least once
bool in_maze = false; // a flag for if someone is in the maze currently
int current_guest = -1; // how the minotaur announces who goes


// global locks
mutex mut_maze;

int main(int argc, char **argv) {

    // take command line input for number of guests
    // if(argc != 2) {
    //     printf("Invalid input. Please enter a number of guests.\n`./problem1 <number of guests>`\n");
    //     return 1; // error
    // }

    // n_guests = stoi(argv[1]);
    printf("Number of guests: %d\n", n_guests);

    // seed the rng
    auto time = chrono::system_clock::to_time_t(chrono::system_clock::now());
    srand(time);

    // run simulation
    auto start = high_resolution_clock::now(); // start time
    int runs = start_party(n_guests);
    auto end = high_resolution_clock::now(); // end time

    auto total_time = duration_cast<milliseconds>(end - start);

    cout << "\nThat party took " << runs << " runs, and " << total_time.count() << "ms.\nThanks for coming!\n\n~ The Minotaur\n\n";

    return 0;
}


void guest_function(int id) {

    bool has_visited = false;

    while(true) {

        if(all_visited) { // minotaur says stop
            break;
        }

        mut_maze.lock(); // block main thread
        
        if(current_guest == id) { // enter maze

            in_maze = true; // alert main thread that someone has entered

            if(!is_cupcake && !has_visited) { 

                // call servant for new cupcake
                is_cupcake = true;
                has_visited = true;
            }

            current_guest = -1;
        }

        mut_maze.unlock(); // stop blocking main thread
    }
    
    return;
}


void leader_function(int n_guests) {

    int count = 0;
    int id = 0;

    while(true) {

        if(count >= n_guests) {
            all_visited = true; // alert all threads to end
            break;
        }

        mut_maze.lock(); // block main thread
        if(current_guest == id) { // enter maze

            in_maze = true; // alert main thread that someone has entered

            if(is_cupcake) { 

                // eat the cupcake
                is_cupcake = false;
                count++;

            }

            current_guest = -1;
        }
        mut_maze.unlock(); // unblock
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

    // for debugging
    cout << "All guests are ready.\n";
    int i = 0;

    mut_maze.lock(); // block other threads from entering the maze
    while(!all_visited){

        current_guest = rand()%n; // select next guest
        in_maze = false; // indicate that someone needs to enter the maze

        mut_maze.unlock(); // open the maze
        i++;

        do {
            // NOTHING
            if(all_visited) {
                break;
            }
        }
        while(!in_maze); // the selected person entered the maze

        mut_maze.lock(); // wait for thread to leave and drop lock
    }
    mut_maze.unlock();

    while(!pool.empty()) { // wait for all threads to finish, then destroy them (with laser sharks)
        pool.front().join();
        pool.pop();
    }

    return i;
}