#include <iostream>
#include <memory>
#include <vector>
#include <bitset>
#include "fft.h"
#include "wave-processor.h"

extern const int NumBinsInFftWinM; // 2048
extern const int NumSamplesPerFrameM; //1850
const double timeInterval = 0.0116;
const double frameInterval = 0.37;
const int sampleRate = 5000;

class FingerExtractor{
public:
	FingerExtractor();
	bool CalcFingerprint(const string waveFilePath);
	bool PrintFingerToFile(const string& filename, const string& filepath);
	std::vector<std::bitset<32>> getQueryFinger();
	//int getFingerFileId();

private:
	void _calc_freq_bind();
	int _select_bind(double point_freq);
	string wavepath;
	int frameNum; //��¼���������Ƶһ���ж���֡

	std::vector<short> all_time_data; // for query
	std::vector<std::vector<double>> energy; //����E[n,m]����ʾ��n֡�ĵ�m��Ƶ��������ֵ 33
	std::vector<std::string> fingers; //���յ�ָ�ƽ�� 32

	std::shared_ptr<WaveProcessor> wp;

	bool _Energying(int all_time_data_size);
	bool _Fingerprinting();
};
