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

int main(int argc, char* argv[]) {
	if (argc < 4) {
		cerr << "Usage: ./matcher %fileList4query% %dir4db% %resultFile%" << endl;
		return -1;
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

	struct timeval start, end;
	gettimeofday(&start, NULL);

	Searcher searcher;
	searcher.BuildIndex(dir4db);
	cout<<"build index done"<<endl;

	ofstream fout;
	fout.open(result_file, fstream::out);
	searcher.Search(query_list, fout);
	fout.close();

	gettimeofday(&end, NULL);
	int time_use = (end.tv_sec - start.tv_sec);
	cout<<"Time: "<<time_use<<endl;

	return 0;
}
