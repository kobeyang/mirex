#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <bitset>
#include <string>
#include <map>
#include <ctime>
#include "finger-extractor.h"
#include "searcher.h"
#include "util.h"
#include "unistd.h"
#include "sys/time.h"

using namespace std;

int THREAD_NUM = 8;
ofstream fout;
Searcher searcher;

void MultiThreadSearch(const vector<string>& query_files, int thread_id) {
	FingerExtractor extractor;
	
	for(int i = 0 ;i < query_files.size(); i++)	{
		if (i % THREAD_NUM != thread_id)
			continue;
		if (extractor.CalcFingerprint(query_files[i]) == false) {
      cerr << query_files[i] << " -- calculating fingerprint failed!" << endl;
			continue;
    }
		vector<bitset<32>> finger_block = extractor.getQueryFinger();
		string result = searcher.SubSamplingSearch(finger_block);
		string output = query_files[i] + "\t" + result + "\n";
		fout << output;
		fout.flush();
		cerr << query_files[i] << "   ...done." << endl;
	}
}

int main(int argc, char* argv[]) {
	if (argc < 4) {
		cerr << "Usage: ./matcher %fileList4query% %dir4db% %resultFile%" << endl;
		return -1;
	}
  if (argc == 6) {
    THREAD_NUM = stoi(argv[5]);
    cerr << "Set thread num to " << stoi(argv[5]) << endl;
  }
	string query_list_path = string(argv[1]);
	string dir4db = string(argv[2]);
	string result_file = string(argv[3]);

	ifstream fin(query_list_path, fstream::in);
	vector<string> query_list;
	string query;
	getline(fin, query);
	while (fin.good()) {
		if (!query.empty())
			query_list.push_back(query);
		getline(fin, query);
	}
	fin.close();

	cerr << "Building index." << endl;
	searcher.BuildIndex(dir4db);
	cerr << "Building index done." << endl;

	fout.open(result_file, fstream::out);

	vector<thread> threads;
	for (int i = 0; i < THREAD_NUM; i++)
		threads.push_back(thread(MultiThreadSearch, query_list, i));
	for (int i = 0; i < THREAD_NUM; i++)
		threads[i].join();

	fout.close();
	return 0;
}
