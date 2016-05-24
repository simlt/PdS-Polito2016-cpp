#pragma once
#include <queue>
#include <mutex>
#include <thread>
#include <cassert>


template <class T>
class BlockingQueue
{
	enum class BQStatus {
		OPEN,
		CLOSED
	};
	int capacity;
	int current_size;
	std::queue<T> queue;
	BQStatus status;
	// Syncronization objects
	std::mutex mutex;
	std::condition_variable cv_queue_empty;
	std::condition_variable cv_queue_full;

public:
	BlockingQueue(int size);
	virtual ~BlockingQueue() {}
	bool preleva(T& res);
	void inserisci(T val);
	// NEVER call conteggio() while mutex is locked to avoid a deadlock!
	int conteggio();
	void chiudi();
};

// Keep implementation of template class in header to avoid linker error (LNK2019)
template<class T>
BlockingQueue<T>::BlockingQueue(int size)
	: capacity(size), current_size(0), status(BQStatus::OPEN)
{
	assert(size > 0);
}

template<class T>
bool BlockingQueue<T>::preleva(T& res)
{
	std::unique_lock<std::mutex> lock(mutex);
	if (status == BQStatus::CLOSED)
		return false;
	// Check if queue is empty and it's open (status may change after a wait)
	while ((current_size == 0) && (status == BQStatus::OPEN))
	{
#if _DEBUG
		std::cout << "[BQ] - Waiting to extract...\n";
#endif // DEBUG
		cv_queue_empty.wait(lock);
	}
	// Queue may have been closed while waiting, so check again
	if (status == BQStatus::CLOSED)
		return false;

	// Add some checks
	assert(current_size > 0);
	assert(current_size <= capacity);
	assert(status == BQStatus::OPEN);
	// Now we can extract an element
	res = queue.front(); // std::move(queue.pop());
	queue.pop();
	current_size--;
	// And unlock a thread waiting on full queue
	cv_queue_full.notify_one();
	return true;
}

template<class T>
void BlockingQueue<T>::inserisci(T val)
{
	std::unique_lock<std::mutex> lock(mutex);
	if (status == BQStatus::CLOSED)
		throw std::exception("Attempted to insert an element in a closed BlockingQueue");
	// Check if queue is full and it's open (status may change after a wait)
	while ((current_size == capacity) && (status == BQStatus::OPEN))
	{
#if _DEBUG
		std::cout << "[BQ] - Waiting to insert...\n";
#endif // DEBUG
		cv_queue_full.wait(lock);
	}
	// Queue may have been closed while waiting, so check again
	if (status == BQStatus::CLOSED)
		throw std::exception("BlockingQueue was closed while waiting for an insert");

	// Add some checks
	assert(current_size >= 0);
	assert(current_size < capacity);
	assert(status == BQStatus::OPEN);
	// Now we can insert an element
	queue.push(val);
	current_size++;
	// And unlock a thread waiting on full queue
	cv_queue_empty.notify_one();
}

template<class T>
int BlockingQueue<T>::conteggio()
{
	std::lock_guard<std::mutex> lock(mutex);
	return current_size;
}

template<class T>
void BlockingQueue<T>::chiudi()
{
	std::lock_guard<std::mutex> lock(mutex);
	status = BQStatus::CLOSED;
	// Now release all waiting threads (full and empty)
	cv_queue_empty.notify_all();
	cv_queue_full.notify_all();
}
