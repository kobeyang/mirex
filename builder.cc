#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include <cstring>
#include <map>
#include <ctime>
#include <cstdlib>
#include <sndfile.h>
#include "mp3-file-reader.h"
#include "finger-extractor.h"
#include "util.h"
#include "unistd.h"
#include "sys/time.h"

using namespace std;

int THREAD_NUM = 8;

bool CreateWave(string mp3_file, string output) {
	MpegFileReader * reader = new MpegFileReader(mp3_file, 22050, 1, 4096, 4096);
	if(reader == NULL) {
		cerr << "reader is null!" << endl;
		return false;
	}
	if (reader->initialize() == false) {
		cerr << "reader initialize failed!" << endl;
		return false;
	}

	vector<float> all_samples;
	float ** buf = reader->nextBlock();
	int count_block = 0;
	while (buf) {
		count_block ++;
		for (int i = 0; i < 4096; i++)
			all_samples.push_back(buf[0][i]);
		buf = reader->nextBlock();
	}

	SNDFILE *outfile = NULL;
	SF_INFO sfinfo;
	memset(&sfinfo, 0, sizeof(SF_INFO));
	sfinfo.samplerate = (int)reader->getSampleRate();
	sfinfo.channels = 1;
	sfinfo.format = SF_FORMAT_PCM_16 | SF_FORMAT_WAV;

	if ((outfile = sf_open(output.c_str(), SFM_WRITE, &sfinfo)) == NULL) {
		cerr << "unable to create output file: " << output << endl;
		return false;
	}

	float* buffer = (float *)malloc(all_samples.size() * sizeof(float));
	for (int i = 0; i < all_samples.size(); i++)
		buffer[i] = all_samples[i];
	sf_writef_float(outfile, buffer, all_samples.size());

	free(buffer);
	sf_close(outfile);
	delete reader;
	return true;
}

void ExtractFingerprint(const vector<string>& allFiles,
	const string fingerprint_path, int thread_idx) {
	FingerExtractor extractor;
	for(int i = 0; i < (signed)allFiles.size(); i++) {
		if (i % THREAD_NUM != thread_idx)
			continue;
		int slash = allFiles[i].rfind("/");
		string filename = allFiles[i].substr(slash + 1, allFiles[i].rfind(".") - slash - 1);
    cerr << allFiles[i] << " is being processed..." << endl;

		string tmp_file = "./tmp/" + to_string(thread_idx) + ".wav";
		if (CreateWave(allFiles[i], tmp_file) == false) {
			cerr << "Create wave failed: " << tmp_file << endl;
			return;
		}
    cerr << "Convert to wave success!" << endl; 
		if (extractor.CalcFingerprint(tmp_file) == false) {
      cerr << "Calculate fingerprints failed!" << endl;
      return;
    }
    string output_path = fingerprint_path + "/" + filename + ".txt";
		if (extractor.PrintFingerToFile(allFiles[i], output_path) == false) {
      cerr << "Output fingerprints error!" << endl;
      return;
    }
		cout << allFiles[i] << " is finished." << endl;
    cout << endl;
	}
}

int main(int argc, char* argv[]) {	
	if (argc < 3) {
		cerr << "Usage: ./builder %fileList4db% %dir4db%" << endl;
		return -1;
	}
	string filelist = string(argv[1]);
	string dir4db = string(argv[2]);

  if (argc == 5) {
    THREAD_NUM = stoi(argv[4]);
    cerr << "Set thread num to " << stoi(argv[4]) << endl;
  }
	
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

	system("mkdir tmp");
	string cmd = "mkdir " + dir4db;
	system(cmd.c_str());

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
