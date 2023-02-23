## This is a program written by Ethan Woollet for COP4520 with professor Juan Parra at UCF.

There are 2 C++ files in this repository because the assignment had two separate problems to solve and they are named correspondingly.
Also, note that the instructions for this assignment are included in the PDF file in this repository.

*The default number of threads is defined by a value called `n_guests` in both programs and it is set to 20. I have also commented out some code that can be used to change the default value to any number from command line inputs which just need to be uncommented to work. 


# Compilation Instructions

To compile and run this program, download 'problem1.cpp' and 'problem2.cpp' or clone this repo.
Then, navigate the the directory containing that file and perform the following commands

- `g++ -o <executable name> [problem1 | problem2].cpp`
- `./<executable name>`


# Proof of Correctness for Problem 1's Solution

My solution for problem 1 effectively relies on a single thread to perform the job of keeping track of how many unique people have definitely entered the maze. Because of this, there are two unique thread functions that are used.

- The 'Leader' function is the thread that has `id == 0`, just to make it easier to program. This thread is the only thread that is able to actually eat the cupcake, and every time it does so, it add 1 to its internal count.

- For every other 'Guest' thread, they all have a boolean that indicates whether or not they have called for a cupcake before, and their only job is to call for a cupcake if there isnt one when they get there *and* if they haven't called for one previously.

- The main thread, while not one of the spawned processes, still factors in, however. The 'Minotaur' represented by the main thread has the job of indicating to the spawned threads who is supposed to change state next.

Combined, the overall process is as follows:

1. The main thread spawns all of the child threads. One thread uses the 'leader' function, and the rest use the 'guest' function.
2. The main thread selects a random thread id and changes a global variable to that id value.
3. Each spawned thread checks the value selected by the main thread and compares that to their own id. If it matches, they move on. Otherwise, they idle until a new id is chosen by the main thread.
4. Once a thread sees that it's id has been selected, it then checks the state of the cupcake and performs some action based on that and their own state.
5. IF the thread selected is the leader thread, and it sees that it has counted the same number of cupcakes as there are spawned threads, then alert the main thread that everyone has gone. Then, all spawned threads terminate their processes and the main thread finishes up the program.
6. ELSE The selected thread then resets the id value, and lets the main thread take over again and jumps back to step 2.

The program uses a lot of intentional blocking via lcoks in an attempt to stop unnecessry context switching and computation. It also uses a few global variable that are only changed by one the one thread that isn't blocked.

My program cycles through the described process until the leader thread knows that everyone has been through at least once, since every thread can only replace the cupcake a single time and the leader consumes every cupcake including the initial one, which represents the leader having started. Therefore, the program will only end once every thread has gone through *at least* once.


# Notes On The Solution For Problem 1

I rewrote this program many times, with each iteration getting progressivley better, but I reached a point where I just didn't know what I was doing that made it so slow. I know that there is a large startup lag for spawning the threads, but for larger numbers, it becomes incredibly slow. I'm not well versed enough to know why or what I can do to fix that. It's very frustraing to say the least.


# Proof of Correctness for Problem 2's Solution

The strategy I chose to implement for the second problem was the third option, which uses a queue. 

I believe that using a queue is the best of the three choices for multiple reasons. For option 1, guests would be constantly trying to grab a lock to enter the room and all would be vying at the exact same time, which adds a lot of uncertainty to when they would be able to actually enter the room. For option 2, it has the same problem, but they only race to the door once the room has been cleared. For the third option, the lock would never be fought over as the thing that would have the attention is the queue itself. And, once the guests have entered the queue, they will only be idling until they are told that it is their turn to enter. This completely eliminates the 'fighting' over the door or the room. If we were to think of the room as being some resource of importance, I think that It then makes sense to see the queue as being the policy of an operating system (using FIFO, obviously). It regulates the actual performance and switching between the processes, while still having all processes' choose when to attempt to get a time slot.

As for the actual implementation of option 3, I wrote a custom thread-safe queue class that is effectively a wrapper class for std::queue, but using locks to prevent race conditions. 

Using this thread-safe queue, I spawned threads with unique ids that all perform the following process:

1. Queue up once.
2. Conditionally wait until alerted and the currently selected id matches it's own id
3. Once selected, view and pop the front of the queue. Store that id in the global `next_guest` variable
4. IF the guest value popped is -1, then there are no more guests to wait for. Alert all and exit. 
5. ELSE Alert the other threads to wake up and check the condition.
6. Then, randomly select whether or not to requeue. If not, then exit.
7. Return to step 2.

This process ensures that every thread performs at least one queue operation and thus does its process at least once. Therefore they all get to go, and the randomness for requeueing fulfills the extra criteria from the option's description.

The main thread essentially just facilitates the start and cleanup of the rest of the threads. It spawns them all, then it selects the first thread to go from the front of the queue, then it waits for them to finish.


# Notes On The Solution For Problem 2

I used unique locks and conditional waiting for this problem unlike in problem 1, and I believe that if I rewrote the first problem using this approach, it would produce much better results than intentional blocking. I do not, however, have the time to do so, but I might try in the future to revamp this code.


