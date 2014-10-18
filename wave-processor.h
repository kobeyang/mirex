#ifndef __MY_UTIL_H001__
#define __MY_UTIL_H001__

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>	// for "FILE *"
#include <vector>

#define BUFFER_SIZE_IN_SAMPLES_DS	8192	// 8k, for down sampling op. 
#define SamplesVectorQuerySize 25220 // there is 25219 sampling points in a 5 seconds query wave

// WAV file header
typedef struct {
	char			root_chunk_id[4];	/* 'RIFF' */
	unsigned int	root_chunk_data_size;	/* length of root chunk */
	char			riff_type_id[4];		/* 'WAVE' */
	char			fmt_chunk_id[4];		/* 'fmt ' */
	unsigned int	fmt_chunk_data_size;	/* length of sub_chunk, always 16 bytes */
	unsigned short	compression_code;		/* always 1 = PCM-Code */
	unsigned short	num_of_channels;		/* 1 = Mono, 2 = Stereo */
	unsigned int	sample_rate;	/* Sample rate */
	unsigned int	byte_p_sec;	/* average bytes per sec */
	unsigned short	byte_p_sample;	/* Bytes per sample */
	unsigned short	bit_p_sample;	/* bits per sample, 8, 12, 16 */
} waveheader_t;

class WaveProcessor {
private:
	waveheader_t m_header;	// 基本文件头
	unsigned int m_unifiedSamplingRate;	// 统一采样率（也即欠采样后的采样率）
	short m_newlyMadeSamples[(sizeof(short) + 4) * BUFFER_SIZE_IN_SAMPLES_DS];
	std::vector<short> samplesVector;
	unsigned int m_newlyMadeSamplesNumber;
	double coefficient[16];
	FILE *m_pfWaveR;

public:
	WaveProcessor(int unifiedSamplingRate);
	int OpenWaveFile(const char *psfn_waveR);
	int MakeTargetSamplesData();
	std::vector<short> GetSamplesVector();
	void CloseWaveFile();
	void Clear();
	virtual ~WaveProcessor();
};

#endif	// __MY_UTIL_H001__


