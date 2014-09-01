#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include <map>
#include <ctime>
#include "finger-extractor.h"
#include "util.h"
#include "unistd.h"
#include "sys/time.h"

using namespace std;

const int THREAD_NUM = 8;

void ExtractFingerprint(const vector<string>& allFiles,
	const string fingerprint_path, int thread_idx) {
	FingerExtractor extractor;
	for(int i = 0; i < (signed)allFiles.size(); i++) {
		if (i % THREAD_NUM != thread_idx)
			continue;
		int slash = allFiles[i].rfind("/");
		string filename = allFiles[i].substr(slash + 1, allFiles[i].rfind(".") - slash - 1);
		extractor.CalcFingerprint(allFiles[i]);
		extractor.PrintFingerToFile(fingerprint_path + "/" + filename + ".txt");
		cout << allFiles[i] << "   ...done." << endl;
	}
}

int main(int argc, char* argv[]) {	
	if (argc < 3) {
		cerr << "Usage: ./builder %fileList4db% %dir4db%" << endl;
		return -1;
	}
	string filelist = string(argv[1]);
	string dir4db = string(argv[2]);
	
	ifstream fin(filelist, fstream::in);
	vector<string> mp3_files;
	string mp3;
	getline(fin, mp3);
	while (fin.good()) {
		if (!mp3.empty())
			mp3_files.push_back(mp3);
		getline(fin, mp3);
	}
	fin.close();

	struct timeval start, end;
	gettimeofday(&start, NULL);

	vector<thread> threads;
	for (int i = 0; i < THREAD_NUM; i++)
		threads.push_back(thread(ExtractFingerprint, mp3_files, dir4db, i));
	for (int i = 0; i < THREAD_NUM; i++)
		threads[i].join();
	cout<<"Extract Done!"<<endl;

	gettimeofday(&end, NULL);
	int time_use = (end.tv_sec - start.tv_sec);
	cout<<"Time: "<<time_use<<endl;

	return 0;
}
