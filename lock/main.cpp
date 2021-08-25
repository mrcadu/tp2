#include <atomic>
#include <thread>
#include <vector>
#include <iostream>
#include <cmath>

using namespace std;
std::atomic_flag lock = ATOMIC_FLAG_INIT;
int sum = 0;

void acquireLock(){
    while(lock.test_and_set());
}

void releaseLock(){
    lock.clear();
}

void add_to_sum(const uint8_t number){
    acquireLock();
    sum += number;
    releaseLock();
}

void* thread_sum(int first, int last, vector<uint8_t> *numbers){
    for (int i = first; i < last;i++) {
        add_to_sum(numbers->at(i));
    }
    pthread_exit(nullptr);
}
void sum_with_values(int amount_of_threads, double amount_of_numbers){
    sum = 0;
    vector<thread> threads;
    vector<uint8_t> numbers = vector<uint8_t>(100);
    numbers.reserve(int(amount_of_numbers));
    int amount_per_thread = int(amount_of_numbers) / amount_of_threads;
    for (int i = 0; i < amount_of_numbers; ++i) {
        numbers.push_back(100);
    }
    threads.reserve(amount_of_threads);
    for (int i=0; i<amount_of_threads; i++) {
        vector<uint8_t> *pointer = &numbers;
        threads.push_back(thread(thread_sum, i*amount_per_thread, (i+1)*amount_per_thread, pointer));
    }
    for (auto& th : threads) th.join();
}
int main() {
    int threads_values[9] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
    double number_values[3] = {pow(10,7),pow(10,8),pow(10,9)};
    for (int threads: threads_values) {
        for (double values: number_values) {
            const auto begin_time = chrono::system_clock::now();
            sum_with_values(threads,values);
            const auto end_time = chrono::system_clock::now();
            chrono::duration<double> interval = end_time - begin_time;
            printf("Para os valores N=%f e K=%d o tempo obtido foi %f\n",values,threads,interval.count());
        }
    }
}
