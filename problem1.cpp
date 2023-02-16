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

using namespace std;
using namespace std::chrono;

// Thread functions
void guest_function(int id);
void leader_function(int n_guests);
int start_party(int n);


// cupcake at the end of the labyrinth
bool is_cupcake = true;
mutex mut_is_cupcake;

// flag for the minotaur to know everyone has gone at least once
bool all_visited = false;
mutex mut_all_visited;

// how the minotaur announces who goes
mutex mut_current_guest;
int current_guest = -1;

// a flag for if someone is in the maze currently
mutex mut_in_maze;
bool in_maze = false;

int main(int argc, char **argv) {

    if(argc != 2) {
        perror("Invalid input. Please enter a number of guests.\n`./problem1 <number of guests>`");
        return 1;
    }

    int n_guests = stoi(argv[1]);
    printf("Number of guests: %d\n", n_guests);

    int runs = start_party(n_guests);

    cout << "\n\nThat party took " << runs << "runs.\nThanks for coming!\n\n~ The Minotaur";

    return 0;
}


void guest_function(int id) {

    bool has_eaten = false;

    // cout << id << " entering work loop" <<"\n";
    while(true) {

        mut_all_visited.lock();

        if(all_visited) { // minotaur says stop
            mut_all_visited.unlock();
            break;
        }

        mut_all_visited.unlock();

        mut_current_guest.lock();
        
        if(current_guest == id) { // enter maze

            mut_in_maze.lock();
            in_maze = true;
            mut_in_maze.unlock();

            cout << "Guest " << id << " entered the maze.\n";

            // traversing maze

            // end of maze
            // cout << "Guest " << id << " exited the maze.\n";
            mut_is_cupcake.lock();

            if(!is_cupcake && !has_eaten) { 

                // call servant for new cupcake
                is_cupcake = true;
                cout << "Guest " << id << " REPLACED the cupcake.\n";
                // leaves

                mut_is_cupcake.unlock();
            }
            else {
                cout << "Guest " << id << " did NOTHING.\n";
            }

            mut_is_cupcake.unlock();
            // return to start
            current_guest = -1;

            mut_in_maze.lock();
            in_maze = false;
            mut_in_maze.unlock();
        }

        // cout << "~\n";
        mut_current_guest.unlock();
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return;
}


void leader_function(int n_guests) {

    int count = 0;
    int id = 0;

    // cout << id << " entering work loop" <<"\n";
    while(true) {

        if(count >= n_guests) { // minotaur says stop
            mut_all_visited.lock();
            all_visited = true;
            mut_all_visited.unlock();
            return;
        }

        mut_current_guest.lock();
        if(current_guest == id) { // enter maze

            mut_in_maze.lock();
            in_maze = true;
            mut_in_maze.unlock();

            cout << "Guest 0 entered the maze.\n";

            // traversing maze


            // end of maze
            // cout << "Guest 0 exited the maze.\n";
            mut_is_cupcake.lock();

            if(is_cupcake) { 

                // eat the cupcake
                is_cupcake = false;
                count++;
                cout << "Guest 0 ATE the cupcake. Current count: " << count << "\n";
                // leaves

                mut_is_cupcake.unlock();
            }
            else {
                cout << "Guest 0 did NOTHING.\n";
            }
            
            mut_is_cupcake.unlock();
            // return to start
            current_guest = -1;

            mut_in_maze.lock();
            in_maze = false;
            mut_in_maze.unlock();
        }

        // cout << "~\n";
        mut_current_guest.unlock();
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return;
}


int start_party(int n) {

    queue<thread> pool;
    
    // spawn the leader
    // cout << "spawned 0\n";
    thread t(leader_function, n);
    pool.push(move(t));

    // spawn everyone else
    for(int i = 1; i < n; i++) { // spawn all threads in the pool
        // cout << "spawned " << i <<"\n";
        thread t1(guest_function, i);
        pool.push(move(t1));
    }

    // cout << "\n-- AFTER POOLING --\n"; // remove me
    in_maze = true;
    mut_current_guest.lock();
    current_guest = 0;
    cout << "Guest " << current_guest << " is up!\n";
    mut_current_guest.unlock();


    int i = 1;
    mut_all_visited.lock();
    for(; i <= 100 && !all_visited; i++) { // change me
        mut_all_visited.unlock();
        while(true) {
            mut_in_maze.lock();
            if(!in_maze) {
                mut_in_maze.unlock();
                break;
            }
            mut_in_maze.unlock();
        }

        mut_all_visited.lock();
        if(!all_visited) {
            in_maze = true;
            mut_current_guest.lock();
            current_guest = i%n;
            cout << "Guest " << current_guest << " is up!\n";
            mut_current_guest.unlock();
        }
        
    }
    mut_all_visited.unlock();

    // mut_current_guest.lock();
    // current_guest = 0;
    // mut_current_guest.unlock();

    while(!pool.empty()) { // wait for all threads to finish, then destroy them (with lasers)
        pool.front().join();
        pool.pop();
    }

    return i;
}