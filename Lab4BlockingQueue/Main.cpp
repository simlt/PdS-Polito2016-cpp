#include "BlockingQueue.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <chrono>


void insert(BlockingQueue<int> &bq, int v)
{
	std::stringstream msg;
	try
	{
		msg << " -> Init insert v: " << v << " [n: " << bq.conteggio() << "]\n";
		std::cout << msg.str();
		//msg.clear();
		bq.inserisci(v);
		msg << " -> Done insert v: " << v << " [n: " << bq.conteggio() << "]\n";
		std::cout << msg.str();
	}
	catch (std::exception e)
	{
		msg << "EXCEPTION: " << e.what() << " [n: " << bq.conteggio() << "]\n";
		std::cout << msg.str();
	}
}

int extract(BlockingQueue<int>& bq)
{
	bool result;
	int v;
	std::stringstream msg;

	msg << "<-  Init extract v. [n: " << bq.conteggio() << "]\n";
	std::cout << msg.str();
	//msg.clear();
	result = bq.preleva(v);
	if (result)
		msg << "<-  Done extract v: " << v << " [n: " << bq.conteggio() << "]\n";
	else
		msg << "<-  Extract failed (closed queue). [n: " << bq.conteggio() << "]\n";
	std::cout << msg.str();
	return v;
}

int main()
{
	BlockingQueue<int> bq1(2); // Thread-Safe BlockingQueue of size 2
	BlockingQueue<int> bq2(2); // Bigger BlockingQueue for heavy test (try to change size to test full or empty queues)

	int i;
	std::vector<std::thread> threads;

	using namespace std::chrono_literals;

	std::cout << "\nPut 6 elements (4 should be waiting)\n";
	for (i = 0; i < 6; ++i)
		threads.push_back(std::thread(insert, std::ref(bq1), i));
	std::this_thread::sleep_for(100ms);

	std::cout << "\nGet 5 elements (3 are inserted, 5 are extracted, 1 insertion is still waiting)\n";
	for (i = 0; i < 5; ++i)
		threads.push_back(std::thread(extract, std::ref(bq1)));
	std::this_thread::sleep_for(100ms);

	std::cout << "\nGet 2 more elements (1 is extracted and 1 extraction is waiting)\n";
	for (i = 5; i < 7; ++i)
		threads.push_back(std::thread(extract, std::ref(bq1)));
	std::this_thread::sleep_for(100ms);

	std::cout << "\nPut 4 more elements (3 are insterted, 1 is extracted and 1 insertion is waiting)\n";
	for (i = 6; i < 10; ++i)
		threads.push_back(std::thread(insert, std::ref(bq1), i));
	std::this_thread::sleep_for(100ms);

	std::cout << "\nClosing the queue!\n Should get an exception from the waiting insertion\n";
	bq1.chiudi();
	std::this_thread::sleep_for(100ms);

	std::cout << "\nGet remaining results (should all fail)\n";
	for (i = 7; i < 10; ++i)
		threads.push_back(std::thread(extract, std::ref(bq1)));

	// Synchronize all threads
	for (i = 0; i < 20; ++i)
		threads[i].join();

	std::cout << "\nSimple test ended. Press key to start heavy test...\n";
	std::cin.get();

	// Now test with random scheduling
	threads.clear();

	// Schedule threads
	for (i = 0; i < 100; ++i)
	{
		threads.push_back(std::thread(insert, std::ref(bq2), i));
		threads.push_back(std::thread(extract, std::ref(bq2)));
	}

	// Synchronize all threads
	for (i = 0; i < 200; ++i)
		threads[i].join();

	std::cout << "\nHeavy test ended. Press key to end...\n";
	std::cin.get();
	return EXIT_SUCCESS;
}
