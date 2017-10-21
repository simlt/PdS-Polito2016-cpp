// Lab3Thread.cpp : definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include "Lab3Thread.h"

#include <cpprest/http_client.h>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <future>


#define BASEURL		"https://it.wikipedia.org/w/api.php"


int main()
{
	std::string line;
	std::string word;
	while (1)
	{
		std::unordered_map<std::string, std::future<int>> tasks;
		std::vector<std::pair<std::string, int>> results;

		std::cout << " > ";
		std::getline(std::cin, line);
		std::istringstream iss(line);
		// Extract each word from line
		while (iss >> word)
		{
			if (tasks.count(word) > 0)
				continue; // Skip already added word
			tasks[word] = std::async(getSearchCount, word);
		}
		// Wait for all queries to return a result
		for (auto itr = tasks.begin(); itr != tasks.end(); ++itr)
			results.push_back(std::make_pair(itr->first, itr->second.get()));

		// Sort the vector in descending order
		std::sort(results.begin(), results.end(), [] (const std::pair<std::string, int> & a, const std::pair<std::string, int> & b) { return a.second > b.second; });

		// Print results
		for (auto itr : results)
			std::cout << "- " << itr.first << ": " << itr.second << std::endl;
	}
}

int getSearchCount(const std::string & word)
{
	web::uri_builder ub;
	web::http::client::http_client client(U(BASEURL));
	// Prepare query URL
	ub.set_query(U("format=json&action=query&list=search&srwhat=text&srinfo=totalhits&srprop=&srlimit=1"));
	ub.append_query(U("srsearch"), word.c_str());

	// Query the server for response
	pplx::task<int> t = client.request(web::http::methods::GET, ub.to_string()).then([](web::http::http_response response)
	{
		if (response.status_code() == web::http::status_codes::OK)
			return response.extract_json();
		else
			throw web::http::http_exception::exception("Invalid query response");
	}).then([=](web::json::value json)
	{
		return json[U("query")][U("searchinfo")][U("totalhits")].as_integer();
	});

	try
	{
		return t.get();
	}
	catch(const std::exception& e)
	{
		std::cout << word << " generated exception: " << e.what() << std::endl;
	}
	return -1;
}
