#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <vector>
#include "wave-processor.h"
#include <iostream>

typedef struct {  
	char			data_chunk_id[4];	/* 'data' */
	unsigned int	data_chunk_size;	/* length of data */
} waveheader_ext_t;

using namespace std;

WaveProcessor::WaveProcessor(int unifiedSamplingRate)
{
	m_unifiedSamplingRate = unifiedSamplingRate;
	m_pfWaveR = NULL;
	memset(m_newlyMadeSamples, 0, (sizeof(short) + 4) * BUFFER_SIZE_IN_SAMPLES_DS);
	coefficient[0] = 0.003638091;
	coefficient[1] = 0.008158223;
	coefficient[2] = 0.020889028;
	coefficient[3] = 0.043770138;
	coefficient[4] = 0.074889944;
	coefficient[5] = 0.108669917;
	coefficient[6] = 0.137467074;
	coefficient[7] = 0.154050524;
	coefficient[8] = 0.154050524;
	coefficient[9] = 0.137467074;
	coefficient[10] = 0.108669917;
	coefficient[11] = 0.074889944;
	coefficient[12] = 0.043770138;
	coefficient[13] = 0.020889028;
	coefficient[14] = 0.008158223;
	coefficient[15] = 0.003638091;
}

WaveProcessor::~WaveProcessor()
{
	if (m_pfWaveR)
		fclose(m_pfWaveR);
	if(m_newlyMadeSamples);
		//free(m_newlyMadeSamples);
}

void WaveProcessor::CloseWaveFile()
{
	if (m_pfWaveR) {
		fclose(m_pfWaveR);
		m_pfWaveR = NULL;
	}
	if(m_newlyMadeSamples);
	{
		//free(m_newlyMadeSamples);
		//m_newlyMadeSamples = NULL;
	}
}

void WaveProcessor::Clear()
{
	m_pfWaveR = NULL;
	memset(m_newlyMadeSamples, 0, (sizeof(short) + 4) * BUFFER_SIZE_IN_SAMPLES_DS);
	samplesVector.clear();
}

// ���ļ�ͷ��������ļ��Ƿ�Ϸ�
// ���������������Ϣ����ָ��ͻ����С���ֽ�����
// 
int WaveProcessor::OpenWaveFile(const char *psfn_waveR)
{
	m_pfWaveR = NULL;

	FILE *pfWave = fopen(psfn_waveR, "rb");	// read from a binary file
	if (pfWave == NULL) {
		printf("Can't open the wf file %s!\n", psfn_waveR);
		return -10;
	}

	// �õ��ļ����ȣ�������
	fseek(pfWave, 0, SEEK_END);	// ָ���Ƶ��ļ�β
	int llen_data;
	llen_data = ftell(pfWave);
	fseek(pfWave, 0, SEEK_SET);	// ָ���ػ��ļ�ͷ
	//

	fread(&m_header, sizeof(waveheader_t), 1, pfWave);

	if ( strncmp(m_header.root_chunk_id, "RIFF", 4) != 0 || strncmp(m_header.riff_type_id, "WAVE", 4) != 0) {
		// not a wave file
		fclose(pfWave);
		printf("Not a wave file, abort !\n");
		return -20;
	}
	

	if (m_header.fmt_chunk_data_size > 16) {
		fseek(pfWave, m_header.fmt_chunk_data_size-16, SEEK_CUR);
	}

	waveheader_ext_t header_ext;	// ��չ�ļ�ͷ
	fread(&header_ext, sizeof(waveheader_ext_t), 1, pfWave);
	int lLenWaveHeader = sizeof(waveheader_t)+m_header.fmt_chunk_data_size-16+sizeof(waveheader_ext_t);

	////////////////////////////////////////////////////////////////////////////////////////////////////

	llen_data -= lLenWaveHeader;

	//	printf("\nwave header length : %u\n", lLenWaveHeader);
	//	printf("wave data length : %u\n\n", llen_data);

	//	char			root_chunk_id[4];		// 'RIFF'
	//	unsigned long	root_chunk_data_size;	// length of root chunk
	//	printf("root_chunk_data_size : %u\n", m_header.root_chunk_data_size);
	//	char			riff_type_id[4];		// 'WAVE'
	//	char			fmt_chunk_id[4];		// 'fmt '
	//	unsigned long	fmt_chunk_data_size;	// length of sub_chunk, always 16 bytes
	//	printf("fmt_chunk_data_size(16) : %u\n", m_header.fmt_chunk_data_size);
	//	unsigned short	compression_code;		// always 1 = PCM-Code
	//	printf("compression_code(1) : %d\n", m_header.compression_code);

	//	unsigned short	num_of_channels;		// 1 = Mono, 2 = Stereo
	//	printf("num_of_channels(1 = Mono, 2 = Stereo) : %d\n", m_header.num_of_channels);

	//	unsigned long	sample_rate;			// Sample rate
	//	printf("sample_rate : %u\n", m_header.sample_rate);

	//	unsigned long	byte_p_sec;				// average bytes per sec
	//	printf("byte_p_sec(average bytes per sec) : %u\n", m_header.byte_p_sec);

	//	unsigned short	byte_p_sample;			// Bytes per sample, including the sample's data for all channels!
	//	printf("byte_p_sample : %d\n", m_header.byte_p_sample);
	//	unsigned short	bit_p_sample;			// bits per sample, 8, 12, 16
	//	printf("bit_p_sample(8, 12, or 16) : %d\n", m_header.bit_p_sample);

	//	char			data_chunk_id[4];		// 'data'
	//	unsigned long	data_chunk_size;		// length of data
	//	printf("data_chunk_size : %u\n", header_ext.data_chunk_size);
	if (llen_data != header_ext.data_chunk_size) {
		//printf("llen_data = %u, not equal to data_chunk_size !\n", (unsigned long)llen_data);
		//		assert(0);
	}

	if (m_header.compression_code != 1) {
		fclose(pfWave);
		printf("Not in the reqired compression code, abort !\n");
		return -30;
	}
	if (m_header.bit_p_sample != 16) {
		fclose(pfWave);
		printf("Bits per sample is not 16, abort !\n");
		return -40;
	}
	if (m_header.sample_rate < m_unifiedSamplingRate) {
		fclose(pfWave);
		printf("Sampling rate is less than required, abort !\n");
		return -50;
	}
	if (m_header.num_of_channels > 2) {
		fclose(pfWave);
		printf("More than 2 channels, abort !\n");
		return -60;
	}

	// number of original samples in the audio file !!!
	//	m_lNumSamplesInFile = m_header_ext.data_chunk_size/m_header.byte_p_sample;
	// number of selected samples
	//	m_lNumSamplesInFile /= DOWN_SAMPLING_RATE;

	// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	m_pfWaveR = pfWave;
	fseek(m_pfWaveR, lLenWaveHeader, SEEK_SET);	// ���ļ�ָ���Ƶ� Waveform ���ݿ�ʼ��

	return 0;	// OK
}

// ��ͳһ����������Ƿ���������������������������ֵ���ϲ���һ��������һ������ֵ��
int WaveProcessor::MakeTargetSamplesData()
{
	assert(m_header.bit_p_sample == 16 || m_header.bit_p_sample == 8);
	// "m_header.sample_rate" : ԭʼ����Ƶ��
	// ԭʼ�����ʱ�����ڻ���� "m_unifiedSamplingRate"���˼�Ŀ������ʣ�������
	if (m_header.sample_rate < m_unifiedSamplingRate) {
		printf("m_header.sample_rate(%u) < %d !\n", m_header.sample_rate, m_unifiedSamplingRate);
		return -1;
	}

	//short *m_newlyMadeSamples = NULL;
	unsigned char *szOrginalSampsBuffer = NULL;
	// buffer to hold newly made down-sampling samples, 16 bits per sample!
	/* in order to accelerate
	m_newlyMadeSamples = (short *)malloc(sizeof(short)*BUFFER_SIZE_IN_SAMPLES_DS+
		(m_header.byte_p_sample)*BUFFER_SIZE_IN_SAMPLES_DS);
	*/

	if (m_newlyMadeSamples == NULL) {
		printf("No memory !\n");
		return -2;
	}
	szOrginalSampsBuffer = (unsigned char *)(m_newlyMadeSamples+BUFFER_SIZE_IN_SAMPLES_DS);

	// ////////////////////////////////////////////////////////////////////////////////////////

#ifdef __NORMALIZE_AUDIO_SAMPLES
	maxAbs = 0;
#endif

	int xxx;	// OK

	// ע�⣺Ƿ�����źź�ԭ�����źŶ�Ӧ��ʱ����һ���ģ�����

	// ��ԭ�������桱�е�һ��λ������Ĳ�����ȫ���±�
	int iOriginalSampStartG = 0;
	// �������ɵĵ�һ��Ƿ�����㣨����Ƿ�������桱�е�һ���㣩��ȫ���±�
	int iTargetSampStartG = 0;
	// total number of down-sampling samples made 
	unsigned int nNumNewlyMadeSampsTotal = 0;

	// �������źţ�������
	int& sr = xxx;
	sr = m_unifiedSamplingRate;
	//_write(fhw, &sr, sizeof(int));	// ������
	//_write(fhw, &iTargetSampStartG, sizeof(unsigned long));	// ��ʼ�����±�
	//_write(fhw, &nNumNewlyMadeSampsTotal, sizeof(unsigned long));	// Ŀ���������

	xxx = 0;
	samplesVector.clear();
	//memset(samplesVector, 0, sizeof(short) * SamplesVectorSize);
	//����Ƕ�������ȡ����ʡ����ƽ��
	if(m_header.num_of_channels > 1)
	{
		if(m_header.bit_p_sample == 16)
		{
			// ÿ��ԭʼ���������ļ�����һ����ԭ�������ݣ�������һ��Ƿ������
			while ( !feof(m_pfWaveR) ) {
				int nNumNewlyMadeSamps = 0;	// ���������ӱ��ζ�ȡ��ԭʼ�ź����ݣ����ɵ�Ƿ���������
				//��һ���ӵ�ǰ������Ƶ�����ļ���һ�����ݣ�������
				size_t uNumOrigSampsRead;
				uNumOrigSampsRead = fread(szOrginalSampsBuffer, 
					m_header.byte_p_sample,	// һ����������������һ������ʱ�����������������
					BUFFER_SIZE_IN_SAMPLES_DS,	// Ҫ��ȡ�Ĳ�������
					m_pfWaveR);
				if (uNumOrigSampsRead == 0) {// no content read
					break;
				}

				if (iTargetSampStartG == 0) {
					// ���������Ƿ��������һ��ԭʼ��������Ҫֱ�ӿ����ġ�Ҳ����˵��Ƿ������ŵĵ�һ�����������ԭ�źŵĵ�һ��
					// �����㣡
					assert(nNumNewlyMadeSamps == 0);
					// ��һ������ֵ�հ�
					// ȡ����������ƽ��
					int lval;	// �� "long" ��֤������̲����
					lval = *((short *)szOrginalSampsBuffer);
					lval += *(((short *)szOrginalSampsBuffer)+1);
					lval /= 2;
					m_newlyMadeSamples[nNumNewlyMadeSamps] = lval;
					nNumNewlyMadeSamps++;
				}

				//���������ɶ�Ӧ���²�����������
				while (1) {
					// �����ǽ�Ƿ�������Ӧ��ĳ��ԭ�����㣨ֻ��������㣩�����������ڵ�ԭ�����㣨���������������㣩֮�䣡����

					// ������Ƿ�������Ӧ��ԭʼ�������ȫ���±꣨�����ϲ�һ����������
					int iOriginalSampG;
					int iTargetSampG;
					// ����Ƿ�������±꣬������
					iTargetSampG = iTargetSampStartG+nNumNewlyMadeSamps;
					// �������Ӧ��ԭ�������±�
					iOriginalSampG = (double)iTargetSampG*m_header.sample_rate/m_unifiedSamplingRate+0.5;
					if (iOriginalSampG > iOriginalSampStartG+uNumOrigSampsRead-1) {
						// ��δ������Ŀ��ԭ�����㡱��������
						iOriginalSampStartG += uNumOrigSampsRead;
						break;
					}
					// Ŀ��ԭ�������ڻ����е�λ��
					const unsigned char *puchar = szOrginalSampsBuffer+
						(m_header.byte_p_sample)*(iOriginalSampG-iOriginalSampStartG);
					// clrc_002
					//#ifdef __COMBINE_L_R_CHANNELS
					// ȡ����������ƽ��
					int lval = 0;
					int total = 0;
					//�������15�������㣬��ȡ֮ǰ�������ƽ��
					if(iOriginalSampG - iOriginalSampStartG < 15)
					{
						int sampleNum = iOriginalSampG - iOriginalSampStartG;
						for(int i = sampleNum; i >= 0; i--)
						{
							lval = *(((short *)puchar) - i);	// �������ĵ�һ������
							lval += *(((short *)puchar) - i +1);	// �������ĵڶ�������
							lval /= 2;
							total += lval;
						}
						m_newlyMadeSamples[nNumNewlyMadeSamps] = total/(sampleNum + 1);

					}
					//��������FIR�˲���ʹ��16��������
					else
					{
						for(int i = 15; i >= 0; i--)
						{
							lval = *(((short *)puchar) - i);	// �������ĵ�һ������
							lval += *(((short *)puchar) - i +1);	// �������ĵڶ�������
							lval /= 2;
							total += lval * coefficient[i];
						}
						m_newlyMadeSamples[nNumNewlyMadeSamps] = total;
					}
					nNumNewlyMadeSamps++;	// ����Ƿ������������

				}	// end of "while (1) {"
				for(int i = 0; i < nNumNewlyMadeSamps;i++)
					samplesVector.push_back(m_newlyMadeSamples[i]);
				//���ģ�׼���������ļ�������һ��ԭ�������ݣ�������
				iTargetSampStartG += nNumNewlyMadeSamps;
				nNumNewlyMadeSampsTotal += nNumNewlyMadeSamps;
			}	// end of "while ( !feof(m_pfWaveR) ) {"
		}
		else if(m_header.bit_p_sample == 8)
		{
			// ÿ��ԭʼ���������ļ�����һ����ԭ�������ݣ�������һ��Ƿ������
			while ( !feof(m_pfWaveR) ) {
				int nNumNewlyMadeSamps = 0;	// ���������ӱ��ζ�ȡ��ԭʼ�ź����ݣ����ɵ�Ƿ���������
				//��һ���ӵ�ǰ������Ƶ�����ļ���һ�����ݣ�������
				size_t uNumOrigSampsRead;
				uNumOrigSampsRead = fread(szOrginalSampsBuffer, 
					m_header.byte_p_sample,	// һ����������������һ������ʱ�����������������
					BUFFER_SIZE_IN_SAMPLES_DS,	// Ҫ��ȡ�Ĳ�������
					m_pfWaveR);
				if (uNumOrigSampsRead == 0) {// no content read
					break;
				}

				if (iTargetSampStartG == 0) {
					// ���������Ƿ��������һ��ԭʼ��������Ҫֱ�ӿ����ġ�Ҳ����˵��Ƿ�����źŵĵ�һ�����������ԭ�źŵĵ�һ��
					// �����㣡
					assert(nNumNewlyMadeSamps == 0);
					// ��һ������ֵ�հ�
					// ȡ����������ƽ��
					int lval;	// �� "long" ��֤������̲����
					lval = *((char *)szOrginalSampsBuffer);
					lval += *(((char *)szOrginalSampsBuffer)+1);
					lval *= 256;
					lval /= 2;
					m_newlyMadeSamples[nNumNewlyMadeSamps] = lval;
					nNumNewlyMadeSamps++;
				}

				//���������ɶ�Ӧ���²�����������
				while (1) {
					// �����ǽ�Ƿ�������Ӧ��ĳ��ԭ�����㣨ֻ��������㣩�����������ڵ�ԭ�����㣨���������������㣩֮�䣡����

					// ������Ƿ�������Ӧ��ԭʼ�������ȫ���±꣨�����ϲ�һ����������
					int iOriginalSampG;
					int iTargetSampG;
					// ����Ƿ�������±꣬������
					iTargetSampG = iTargetSampStartG+nNumNewlyMadeSamps;
					// �������Ӧ��ԭ�������±�
					iOriginalSampG = (double)iTargetSampG*m_header.sample_rate/m_unifiedSamplingRate+0.5;
					if (iOriginalSampG > iOriginalSampStartG+uNumOrigSampsRead-1) {
						// ��δ������Ŀ��ԭ�����㡱��������
						iOriginalSampStartG += uNumOrigSampsRead;
						break;
					}
					// Ŀ��ԭ�������ڻ����е�λ��
					const unsigned char *puchar = szOrginalSampsBuffer+
						(m_header.byte_p_sample)*(iOriginalSampG-iOriginalSampStartG);
					// clrc_002
					//#ifdef __COMBINE_L_R_CHANNELS
					// ȡ����������ƽ��
					int lval = 0;
					int total = 0;
					//�������15�������㣬��ȡ֮ǰ�������ƽ��
					if(iOriginalSampG - iOriginalSampStartG < 15)
					{
						int sampleNum = iOriginalSampG - iOriginalSampStartG;
						for(int i = sampleNum; i >= 0; i--)
						{
							lval = *(((char *)puchar) - i);	// �������ĵ�һ������
							lval += *(((char *)puchar) - i +1);	// �������ĵڶ�������
							lval *= 256;
							lval /= 2;
							total += lval;
						}
						m_newlyMadeSamples[nNumNewlyMadeSamps] = total/(sampleNum + 1);

					}
					//��������FIR�˲���ʹ��16��������
					else
					{
						for(int i = 15; i >= 0; i--)
						{

							lval = *(((char *)puchar) - i);	// �������ĵ�һ������
							lval += *(((char *)puchar) - i +1);	// �������ĵڶ�������
							lval *= 256;
							lval /= 2;
							total += lval * coefficient[i];
						}
						m_newlyMadeSamples[nNumNewlyMadeSamps] = total;
					}
					nNumNewlyMadeSamps++;	// ����Ƿ������������

				}	// end of "while (1) {"
				for(int i = 0; i < nNumNewlyMadeSamps;i++)
					samplesVector.push_back(m_newlyMadeSamples[i]);
				//���ģ�׼���������ļ�������һ��ԭ�������ݣ�������
				iTargetSampStartG += nNumNewlyMadeSamps;
				nNumNewlyMadeSampsTotal += nNumNewlyMadeSamps;
			}	// end of "while ( !feof(m_pfWaveR) ) {"
		}

	}


	//�������Ļ���ֱ�Ӿ���
	else
	{
		// ÿ��ԭʼ���������ļ�����һ����ԭ�������ݣ�������һ��Ƿ������
		while ( !feof(m_pfWaveR) ) {
			int nNumNewlyMadeSamps = 0;	// ���������ӱ��ζ�ȡ��ԭʼ�ź����ݣ����ɵ�Ƿ���������
			//��һ���ӵ�ǰ������Ƶ�����ļ���һ�����ݣ�������
			size_t uNumOrigSampsRead;
			uNumOrigSampsRead = fread(szOrginalSampsBuffer, 
				m_header.byte_p_sample,	// һ����������������һ������ʱ�����������������
				BUFFER_SIZE_IN_SAMPLES_DS,	// Ҫ��ȡ�Ĳ�������
				m_pfWaveR);
			if (uNumOrigSampsRead == 0) {// no content read
				break;
			}

			if (iTargetSampStartG == 0) {
				// ���������Ƿ��������һ��ԭʼ��������Ҫֱ�ӿ����ġ�Ҳ����˵��Ƿ�����źŵĵ�һ�����������ԭ�źŵĵ�һ��
				// �����㣡
				assert(nNumNewlyMadeSamps == 0);
				// ��һ������ֵ�հ�
				if (m_header.bit_p_sample == 16) {
					m_newlyMadeSamples[nNumNewlyMadeSamps] = *((short *)szOrginalSampsBuffer);
				} else if (m_header.bit_p_sample == 8) {
					m_newlyMadeSamples[nNumNewlyMadeSamps] = *((char *)szOrginalSampsBuffer);
					m_newlyMadeSamples[nNumNewlyMadeSamps] *= 256;
				}
				nNumNewlyMadeSamps++;
			}

			//���������ɶ�Ӧ���²�����������
			while (1) {
				// �����ǽ�Ƿ�������Ӧ��ĳ��ԭ�����㣨ֻ��������㣩�����������ڵ�ԭ�����㣨���������������㣩֮�䣡����

				// ������Ƿ�������Ӧ��ԭʼ�������ȫ���±꣨�����ϲ�һ����������
				int iOriginalSampG;
				int iTargetSampG;
				// ����Ƿ�������±꣬������
				iTargetSampG = iTargetSampStartG+nNumNewlyMadeSamps;
				// �������Ӧ��ԭ�������±�
				iOriginalSampG = (double)iTargetSampG*m_header.sample_rate/m_unifiedSamplingRate+0.5;
				if (iOriginalSampG > iOriginalSampStartG+uNumOrigSampsRead-1) {
					// ��δ������Ŀ��ԭ�����㡱��������
					iOriginalSampStartG += uNumOrigSampsRead;
					break;
				}
				// Ŀ��ԭ�������ڻ����е�λ��
				const unsigned char *puchar = szOrginalSampsBuffer+
					(m_header.byte_p_sample)*(iOriginalSampG-iOriginalSampStartG);
				int lval = 0;
				int total = 0;
				//�������15�������㣬��ȡ֮ǰ�������ƽ��
				if(iOriginalSampG - iOriginalSampStartG < 15)
				{
					int sampleNum = iOriginalSampG - iOriginalSampStartG;
					for(int i = sampleNum; i >= 0; i--)
					{	
						if (m_header.bit_p_sample == 16) {
							lval = *(((short *)puchar) - i);
						} else if (m_header.bit_p_sample == 8) {
							lval = *(((char *)puchar) - i);
							lval *= 256;
						}
						total += lval;
					}
					m_newlyMadeSamples[nNumNewlyMadeSamps] = total/(sampleNum + 1);

				}
				//��������FIR�˲���ʹ��16��������
				else
				{
					for(int i = 15; i >= 0; i--)
					{

						if (m_header.bit_p_sample == 16) {
							lval = *(((short *)puchar) - i);
						} else if (m_header.bit_p_sample == 8) {
							lval = *(((char *)puchar) - i);
							lval *= 256;
						}
						total += lval * coefficient[i];
					}
					m_newlyMadeSamples[nNumNewlyMadeSamps] = total;
				}
				nNumNewlyMadeSamps++;	// ����Ƿ������������

			}	// end of "while (1) {"
			for(int i = 0; i < nNumNewlyMadeSamps;i++)
				samplesVector.push_back(m_newlyMadeSamples[i]);
			//���ģ�׼���������ļ�������һ��ԭ�������ݣ�������
			iTargetSampStartG += nNumNewlyMadeSamps;
			nNumNewlyMadeSampsTotal += nNumNewlyMadeSamps;
		}	// end of "while ( !feof(m_pfWaveR) ) {"
	}
	m_newlyMadeSamplesNumber = nNumNewlyMadeSampsTotal;
	return xxx;	// OK
}


vector<short> WaveProcessor::GetSamplesVector() {
	return samplesVector;
}
