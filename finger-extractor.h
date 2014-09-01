#include <iostream>
#include <vector>
#include <bitset>
#include "fft.h"
#include "wave-processor.h"

extern const int NumBinsInFftWinM; // 2048
extern const int NumSamplesPerFrameM; //1850
const double timeInterval = 0.0232;
const double frameInterval = 0.37;
const int sampleRate = 5000;

class FingerExtractor{
public:
	FingerExtractor();
	void CalcFingerprint(const string waveFilePath);
	int PrintFingerToFile(const string filepath);
	std::vector<std::bitset<32>> getQueryFinger();
	int getFingerFileId();

private:
	void _calc_freq_bind();
	int _select_bind(double point_freq);
	string wavepath;
	int frameNum; //记录被处理的音频一共有多少帧

	std::vector<short> all_time_data; // for query
	std::vector<std::vector<double>> energy; //能量E[n,m]，表示第n帧的第m个频带的能量值 33
	std::vector<std::string> fingers; //最终的指纹结果 32

	WaveProcessor* wp = NULL;

	int _Energying(int all_time_data_size);
	void _Fingerprinting();
};
