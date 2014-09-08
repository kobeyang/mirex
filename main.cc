#include <iostream>
#include <string>
#include <cstring>
#include <sndfile.h>
#include "mp3-file-reader.h"

using namespace std;

bool CreateWave(string mp3_file, string output) {
	MpegFileReader * reader = new MpegFileReader(mp3_file, 22050, 1, 4096, 4096);
	if(reader == NULL) {
		cerr << "reader is null!" << endl;
		return -1;
	}
	if (reader->initialize() == false) {
		cerr << "reader initialize failed!" << endl;
		return -1;
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
	delete reader;
	cout << "done!" << endl;

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
	sf_close(outfile);

	return true;
}

int main() {
	if (CreateWave("1.mp3", "1.wav"))
		cout << "Success!" << endl;
	else
		cout << "Error!" << endl;
	return 0;
}
