#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>

const int numPhilosophers = 5;

std::mutex forks[numPhilosophers];
std::condition_variable forksCV[numPhilosophers];
std::mutex printMutex;

size_t randTime(size_t min, size_t max)
{
    static std::mt19937 rnd(std::time(nullptr));
    return std::uniform_int_distribution<>(min, max)(rnd);
}

//Based on Chandy/Misra algorithm
void philosopher(int id) {
    int leftFork = id;
    int rightFork = (id + 1) % numPhilosophers;


    while (true) {
        // Thinking
        {
            std::lock_guard<std::mutex> printLock(printMutex);
            std::cout << "Philosopher " << id << " is thinking." << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(randTime(400, 1000)));

        // Pick up forks
        {
            std::unique_lock<std::mutex> leftForkLock(forks[leftFork]);
            std::unique_lock<std::mutex> rightForkLock(forks[rightFork], std::try_to_lock);

            //If right fork is occupied, release left fork
            if(!rightForkLock.owns_lock()) {
                forksCV[leftFork].notify_one();
                //forksCV[rightFork].notify_one();
                continue;
            }
            // Eating
            {
                std::lock_guard<std::mutex> printLock(printMutex);
                std::cout << "      Philosopher " << id << " is eating." << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(randTime(400, 1000)));
        }

        // Put down forks
        forksCV[leftFork].notify_one();
        forksCV[rightFork].notify_one();
    }
}


int main()
{
    std::jthread philosophers[numPhilosophers];

    for (int i = 0; i < numPhilosophers; ++i)
    {
        philosophers[i] = std::jthread(philosopher, i);
    }

    return 0;
}