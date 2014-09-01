#include <iostream>
#include <cstdio>
#include <thread>
#include <vector>
#include <string>
#include <bitset>
#include <cstring>
#include <cmath>
#include <ctime>
#include <algorithm>
#include "finger-extractor.h"
#include "util.h"

using namespace std;

const double freq_bind[] = 
{300.000, 317.752, 336.554, 356.469, 377.563, 
 399.904, 423.568, 448.632, 475.178, 503.296,
 533.078, 564.622, 598.032, 633.419, 670.901,
 710.600, 752.648, 797.185, 844.357, 894.320,
 947.240, 1003.29, 1062.66, 1125.54, 1192.14,
 1262.68, 1337.40, 1416.54, 1500.36, 1589.14,
 1683.17, 1782.77, 1888.27, 2000.00}; //划分的频带 [0, 33]

FingerExtractor::FingerExtractor() {
	wp = new WaveProcessor(5000);
}

void FingerExtractor::CalcFingerprint(const string waveFilePath)
{
	this->wavepath = waveFilePath;
	wp->Clear();
	wp->OpenWaveFile(waveFilePath.c_str());
	wp->MakeTargetSamplesData();
	all_time_data = wp->GetSamplesVector();
	wp->CloseWaveFile();
	_Energying(all_time_data.size());
	_Fingerprinting();
}

int FingerExtractor::_select_bind(double point_freq)
{
	int start = 0;
	int end = 33;
	int mid = 0;
	while(start <= end)
	{
		mid = (end + start) / 2;
		if(point_freq < freq_bind[mid])
			end = mid - 1;
		else if(point_freq > freq_bind[mid + 1])
			start = mid + 1;
		else
			return mid;
	}
	return -1;
}

int FingerExtractor::_Energying(int all_time_data_size) {
	energy.clear();
	frameNum = 0;
	int start = 0;
	int jump_samples = (int)(sampleRate * timeInterval);

	while(start + NumSamplesPerFrameM < all_time_data_size)	{
		short time_data[1850];
		cpxv_t freq_data[2048];
		double bind_energy[33];
		memset(bind_energy, 0, sizeof(double) * 33);
		for(int i = 0; i < NumSamplesPerFrameM; i++) {
			time_data[i] = all_time_data[i + start];
		}

		DoFFT(time_data, freq_data);
		double point_freq = 0;
		for(int j = 0; j < NumBinsInFftWinM; j++)	{
			//FFT结果第n个点代表的频率值
			point_freq = (j + 1) * sampleRate / NumBinsInFftWinM;
			if(point_freq < 300 || point_freq > 2000) {
				continue;
			}	else{
				int bind = _select_bind(point_freq); // [0,32]
				bind_energy[bind] += sqrt((freq_data[j].re * freq_data[j].re + freq_data[j].im * freq_data[j].im));
			}
		}
		vector<double> one_frame_energy;
		for(int i = 0; i < 33; i++)
			one_frame_energy.push_back(bind_energy[i]);
		energy.push_back(one_frame_energy);

		//下一帧
		frameNum++;
		start+=jump_samples;
	}
	return frameNum;
}

void FingerExtractor::_Fingerprinting() {
	fingers.resize(frameNum);
	for (int i = 0; i < frameNum; i++)
		fingers[i].resize(32);
	//第0帧
	for(int j = 0; j < 32; j++)	{
		if(energy[0][j] - energy[0][j+1] > 0)
			fingers[0][j] = '1';
		else
			fingers[0][j] = '0';
	}
	
	for(int i = 1; i < frameNum; i++)	{
		for(int j = 0; j < 32; j++) {
			if(energy[i][j] - energy[i][j+1] - (energy[i-1][j] - energy[i-1][j+1]) > 0)
				fingers[i][j] = '1';
			else
				fingers[i][j] = '0';
		}
	}
}

vector<bitset<32>> FingerExtractor::getQueryFinger() {
	vector<bitset<32>> query_finger;
	for(int i = 0; i < frameNum; i++)
		query_finger.push_back(bitset<32>(fingers[i]));
	return query_finger;
}

int FingerExtractor::getFingerFileId() {
	int slash_idx = wavepath.find_last_of("/");
	string originFile = wavepath.substr(slash_idx + 1);
	return stoi(originFile.substr(0, originFile.find(".")));
}

int FingerExtractor::PrintFingerToFile(const string fingerFile) {
	FILE *fp = fopen(fingerFile.c_str(), "w");
	string sub_finger;
	fprintf(fp, "%s\n", wavepath.c_str());
	for(int i = 0; i < frameNum ; i++)
	{
		sub_finger = "";
		for(int j = 0; j < 32;j ++)
			sub_finger.push_back(fingers[i][j]);
		bitset<32> b(sub_finger);
		fprintf(fp, "%lu\n", b.to_ulong());
	}
	fclose(fp);
	return 0;
}
