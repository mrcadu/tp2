#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <math.h>

using namespace std;
using namespace std::chrono;

#include<string>
using std::string;


class MutexCount
{
public:
    MutexCount()
    {
        this->count = 0;
    }

    int getCount()
    {
        mutex.lock();
        int val = this->count;
        mutex.unlock();
        return val;
    }

    void addCount()
    {
        mutex.lock();
        this->count++;
        mutex.unlock();
    }

private:
    std::mutex mutex;
    int count;
};

class Semaphore
{
public:
  Semaphore(int count_ = 0)
      : count(count_)
  {
  }

  inline void signal()
  {
    std::unique_lock<std::mutex> lock(mtx);
    count++;
    cv.notify_one();
  }
  inline void wait()
  {
    std::unique_lock<std::mutex> lock(mtx);
    while (count == 0)
      cv.wait(lock);
    count--;
  }

private:
  std::mutex mtx;
  std::condition_variable cv;
  int count;
};

int random_int()
{
    return rand() % (int)10000000 + 1;
}

string checkNumberPrime(int n)
{
    for (unsigned int i = 2; i < sqrt(n); i++)
        if ((n % i) == 0)
            return "não! ";
    return "sim! ";
}

int main(int argc, char **argv)
{
  if ((argc != 4))
  {
    std::cerr << "Argumento inválido" << std::endl;
    return 1;
  }

  srand(time(NULL));

  int Np = atoi(argv[1]);
  int Nc = atoi(argv[2]);
  int N = atoi(argv[3]);
  int M = 100000;

  std::vector<std::thread> pro;
  std::vector<std::thread> con;
  MutexCount pro_count;
  MutexCount con_count;

  Semaphore mutex(1);
  Semaphore empty(N);
  Semaphore full(0);

  auto start = high_resolution_clock::now();

  std::vector<int> buffer;
  for (unsigned int i = 0; i < Np; i++)
  {
    pro.push_back(std::thread([&]() {
      while (pro_count.getCount() < M)
      {
        pro_count.addCount();
        empty.wait();
        mutex.wait();
        buffer.push_back(random_int());
        mutex.signal();
        full.signal();
      }
    }));
  }

  for (unsigned int i = 0; i < Nc; i++)
  {
      con.push_back(std::thread([&]() {
      while (con_count.getCount() < M)
      {
        con_count.addCount();
        full.wait();
        mutex.wait();
        int item = buffer.back();
        buffer.pop_back();
        mutex.signal();
        empty.signal();
        std::cout << "O número " << item << " é primo ? " << checkNumberPrime(item) << std::endl;
      }
    }));
  }

  for (unsigned int i = 0; i < Np; i++) {
      pro[i].join();
  }
  for (unsigned int i = 0; i < Nc; i++) {
      con[i].join();
  }

  auto stop = high_resolution_clock::now();
  auto duration = duration_cast<milliseconds>(stop - start);
  cout << "Time spent: "
       << duration.count() << " milliseconds" << endl;
  return 0;
}