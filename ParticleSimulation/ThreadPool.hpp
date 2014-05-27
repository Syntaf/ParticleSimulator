#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <deque>
#include <condition_variable>

class ThreadPool;

//worker thread objects
class Worker {
	public:
		Worker(ThreadPool& s): pool(s) {}
		void operator()();
	private:
		ThreadPool &pool;
};

//thread pool
class ThreadPool {
	public:
		ThreadPool(size_t);
		template<class F>
		void enqueue(F f);
		void waitFinished();
		~ThreadPool();
	private:
		friend class Worker;
		//keeps track of threads so we can join
		std::vector< std::thread > workers;
		//task queue
		std::deque< std::function<void()> > tasks;
		//sync
		std::mutex queue_mutex;
		std::condition_variable condition;
		bool stop;
};

void ThreadPool::waitFinished()
{
	while(!tasks.empty())
	{
		std::cout << tasks.size() << std::endl;
	}
}

void Worker::operator()()
{
	std::function<void()> task;
	while(true) {
		{
			std::unique_lock<std::mutex>
				lock(pool.queue_mutex);

			//look for a work item
			while(!pool.stop && pool.tasks.empty()) {
				pool.condition.wait(lock);
			}

			if(pool.stop)
				return;

			//get task from queue
			task = pool.tasks.front();
			pool.tasks.pop_front();
		
		} //release lock
		
		//execute task
		task();
	}
}

//launch x amount of workers
ThreadPool::ThreadPool(size_t threads)
	: stop(false)
{
	for(size_t i = 0; i<threads;++i)
		workers.push_back(std::thread(Worker(*this)));
}

ThreadPool::~ThreadPool()
{
	//stop all threads
	stop = true;
	condition.notify_all();

	//join them
	for(size_t i=0; i<workers.size();++i)
		workers[i].join();
}

template<class F>
void ThreadPool::enqueue(F f)
{
	{//aquire lock

		std::unique_lock<std::mutex> lock(queue_mutex);

		//add task
		tasks.push_back(std::function<void()>(f));
	
	}//release lcok

	//wake up thread
	condition.notify_one();
}

#endif