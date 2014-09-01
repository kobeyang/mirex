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

// ¶ÁÎÄ¼þÍ·£¬²¢¼ì²éÎÄ¼þÊÇ·ñºÏ·¨
// ×îºóÁ½¸ö²ÎÊý£ºÏûÏ¢»º´æÖ¸ÕëºÍ»º´æ´óÐ¡£¨×Ö½ÚÊý£©
// 
int WaveProcessor::OpenWaveFile(const char *psfn_waveR)
{
	m_pfWaveR = NULL;

	FILE *pfWave = fopen(psfn_waveR, "rb");	// read from a binary file
	if (pfWave == NULL) {
		printf("Can't open the wf file %s!\n", psfn_waveR);
		return -10;
	}

	// µÃµ½ÎÄ¼þ³¤¶È£¬¡£¡£¡£
	fseek(pfWave, 0, SEEK_END);	// Ö¸ÕëÒÆµ½ÎÄ¼þÎ²
	int llen_data;
	llen_data = ftell(pfWave);
	fseek(pfWave, 0, SEEK_SET);	// Ö¸ÕëÖØ»ØÎÄ¼þÍ·
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

	waveheader_ext_t header_ext;	// À©Õ¹ÎÄ¼þÍ·
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
	fseek(m_pfWaveR, lLenWaveHeader, SEEK_SET);	// ½«ÎÄ¼þÖ¸ÕëÒÆµ½ Waveform Êý¾Ý¿ªÊ¼´¦

	return 0;	// OK
}

// °´Í³Ò»²ÉÑùÂÊ×ö£¨Ç·£©²ÉÑù£¬²¢½«¶àÉùµÀ£¨¶à¸ö²ÉÑùÖµ£©ºÏ²¢³ÉÒ»¸öÉùµÀ£¨Ò»¸ö²ÉÑùÖµ£©
int WaveProcessor::MakeTargetSamplesData()
{
	assert(m_header.bit_p_sample == 16 || m_header.bit_p_sample == 8);
	// "m_header.sample_rate" : Ô­Ê¼²ÉÑùÆµÂÊ
	// Ô­Ê¼²ÉÑùÂÊ±ØÐë¸ßÓÚ»òµÈÓÚ "m_unifiedSamplingRate"£¨´Ë¼´Ä¿±ê²ÉÑùÂÊ£¡£¡£¡£©
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

	// ×¢Òâ£ºÇ·²ÉÑùÐÅºÅºÍÔ­²ÉÑùÐÅºÅ¶ÔÓ¦µÄÊ±³¤ÊÇÒ»ÑùµÄ£¡£¡£¡

	// ¡°Ô­²ÉÑù»º´æ¡±ÖÐµÚÒ»¸öÎ»ÖÃËù´æµÄ²ÉÑùµÄÈ«¾ÖÏÂ±ê
	int iOriginalSampStartG = 0;
	// ±¾ÅúÉú³ÉµÄµÚÒ»¸öÇ·²ÉÑùµã£¨¼´¡°Ç·²ÉÑù»º´æ¡±ÖÐµÚÒ»¸öµã£©µÄÈ«¾ÖÏÂ±ê
	int iTargetSampStartG = 0;
	// total number of down-sampling samples made 
	unsigned int nNumNewlyMadeSampsTotal = 0;

	// ÓÃÍêÕûÐÅºÅ£¬¡£¡£¡£
	int& sr = xxx;
	sr = m_unifiedSamplingRate;
	//_write(fhw, &sr, sizeof(int));	// ²ÉÑùÂÊ
	//_write(fhw, &iTargetSampStartG, sizeof(unsigned long));	// ÆðÊ¼²ÉÑùÏÂ±ê
	//_write(fhw, &nNumNewlyMadeSampsTotal, sizeof(unsigned long));	// Ä¿±ê²ÉÑù¸öÊý

	xxx = 0;
	samplesVector.clear();
	//memset(samplesVector, 0, sizeof(short) * SamplesVectorSize);
	//Èç¹ûÊÇ¶àÉùµÀ£¬È¡¸÷¸öÊ¡µÀµÄÆ½¾ù
	if(m_header.num_of_channels > 1)
	{
		if(m_header.bit_p_sample == 16)
		{
			// Ã¿´ÓÔ­Ê¼²ÉÑùÊý¾ÝÎÄ¼þ¶Á¡°Ò»Åú¡±Ô­²ÉÑùÊý¾Ý£¬¾ÍÉú³ÉÒ»ÅúÇ·²ÉÑù¡£
			while ( !feof(m_pfWaveR) ) {
				int nNumNewlyMadeSamps = 0;	// ±¾Åú£¨¼´´Ó±¾´Î¶ÁÈ¡µÄÔ­Ê¼ÐÅºÅÊý¾Ý£©Éú³ÉµÄÇ·²ÉÑùµã¸öÊý
				//£¨Ò»£©´Óµ±Ç°¸èÇúÒôÆµÊý¾ÝÎÄ¼þ¶ÁÒ»¿éÊý¾Ý£¬¡£¡£¡£
				size_t uNumOrigSampsRead;
				uNumOrigSampsRead = fread(szOrginalSampsBuffer, 
					m_header.byte_p_sample,	// Ò»¸ö¡°²ÉÑù¡±°üº¬ÁËÒ»¸ö²ÉÑùÊ±¼äµãËùÓÐÉùµÀµÄÊý¾Ý
					BUFFER_SIZE_IN_SAMPLES_DS,	// Òª¶ÁÈ¡µÄ²ÉÑù¸öÊý
					m_pfWaveR);
				if (uNumOrigSampsRead == 0) {// no content read
					break;
				}

				if (iTargetSampStartG == 0) {
					// ÎÞÂÛÈçºÎ×öÇ·²ÉÑù£¬µÚÒ»¸öÔ­Ê¼²ÉÑù×ÜÊÇÒªÖ±½Ó¿½±´µÄ¡£Ò²¾ÍÊÇËµ£¬Ç·²ÉÑùÐÅÅµÄµÚÒ»¸öÑù±¾µã¾ÍÊÇÔ­ÐÅºÅµÄµÚÒ»¸ö
					// Ñù±¾µã£¡
					assert(nNumNewlyMadeSamps == 0);
					// µÚÒ»¸ö²ÉÑùÖµÕÕ°á
					// È¡¸÷¸öÉùµÀµÄÆ½¾ù
					int lval;	// ÓÃ "long" ±£Ö¤¼ÆËã¹ý³Ì²»Òç³ö
					lval = *((short *)szOrginalSampsBuffer);
					lval += *(((short *)szOrginalSampsBuffer)+1);
					lval /= 2;
					m_newlyMadeSamples[nNumNewlyMadeSamps] = lval;
					nNumNewlyMadeSamps++;
				}

				//£¨¶þ£©Éú³É¶ÔÓ¦µÄÐÂ²ÉÑù£¬¡£¡£¡£
				while (1) {
					// ºËÐÄÊÇ½«Ç·²ÉÑùµã¶ÔÓ¦µ½Ä³¸öÔ­²ÉÑùµã£¨Ö»ÓÐ×ó²ÉÑùµã£©£¬»òÁ½¸öÏàÁÚµÄÔ­²ÉÑùµã£¨¼´×óÓÒÁ½¸ö²ÉÑùµã£©Ö®¼ä£¡£¡£¡

					// Óë´ýÇóµÄÇ·²ÉÑùµã¶ÔÓ¦µÄÔ­Ê¼²ÉÑùµãµÄÈ«¾ÖÏÂ±ê£¨ÀíÂÛÉÏ²»Ò»¶¨ÊÇÕûÊý£©
					int iOriginalSampG;
					int iTargetSampG;
					// ¸ù¾ÝÇ·²ÉÑùµãÏÂ±ê£¬¡£¡£¡£
					iTargetSampG = iTargetSampStartG+nNumNewlyMadeSamps;
					// ¼ÆËãÆä¶ÔÓ¦µÄÔ­²ÉÑùµãÏÂ±ê
					iOriginalSampG = (double)iTargetSampG*m_header.sample_rate/m_unifiedSamplingRate+0.5;
					if (iOriginalSampG > iOriginalSampStartG+uNumOrigSampsRead-1) {
						// »¹Î´¶Áµ½¡°Ä¿±êÔ­²ÉÑùµã¡±£¬¡£¡£¡£
						iOriginalSampStartG += uNumOrigSampsRead;
						break;
					}
					// Ä¿±êÔ­²ÉÑùµãÔÚ»º´æÖÐµÄÎ»ÖÃ
					const unsigned char *puchar = szOrginalSampsBuffer+
						(m_header.byte_p_sample)*(iOriginalSampG-iOriginalSampStartG);
					// clrc_002
					//#ifdef __COMBINE_L_R_CHANNELS
					// È¡¸÷¸öÉùµÀµÄÆ½¾ù
					int lval = 0;
					int total = 0;
					//Èç¹û²»¹»15¸ö²ÉÑùµã£¬¾ÍÈ¡Ö®Ç°²ÉÑùµãµÄÆ½¾ù
					if(iOriginalSampG - iOriginalSampStartG < 15)
					{
						int sampleNum = iOriginalSampG - iOriginalSampStartG;
						for(int i = sampleNum; i >= 0; i--)
						{
							lval = *(((short *)puchar) - i);	// ±¾²ÉÑùµÄµÚÒ»¸öÉùµÀ
							lval += *(((short *)puchar) - i +1);	// ±¾²ÉÑùµÄµÚ¶þ¸öÉùµÀ
							lval /= 2;
							total += lval;
						}
						m_newlyMadeSamples[nNumNewlyMadeSamps] = total/(sampleNum + 1);

					}
					//·ñÔò£¬ÀûÓÃFIRÂË²¨Æ÷Ê¹ÓÃ16¸ö²ÉÑùµã
					else
					{
						for(int i = 15; i >= 0; i--)
						{
							lval = *(((short *)puchar) - i);	// ±¾²ÉÑùµÄµÚÒ»¸öÉùµÀ
							lval += *(((short *)puchar) - i +1);	// ±¾²ÉÑùµÄµÚ¶þ¸öÉùµÀ
							lval /= 2;
							total += lval * coefficient[i];
						}
						m_newlyMadeSamples[nNumNewlyMadeSamps] = total;
					}
					nNumNewlyMadeSamps++;	// ±¾ÅúÇ·²ÉÑù¸öÊý¼ÆÊý

				}	// end of "while (1) {"
				for(int i = 0; i < nNumNewlyMadeSamps;i++)
					samplesVector.push_back(m_newlyMadeSamples[i]);
				//£¨ËÄ£©×¼±¸´ÓÒôÀÖÎÄ¼þ¶ÁÈëÏÂÒ»ÅúÔ­²ÉÑùÊý¾Ý£¬¡£¡£¡£
				iTargetSampStartG += nNumNewlyMadeSamps;
				nNumNewlyMadeSampsTotal += nNumNewlyMadeSamps;
			}	// end of "while ( !feof(m_pfWaveR) ) {"
		}
		else if(m_header.bit_p_sample == 8)
		{
			// Ã¿´ÓÔ­Ê¼²ÉÑùÊý¾ÝÎÄ¼þ¶Á¡°Ò»Åú¡±Ô­²ÉÑùÊý¾Ý£¬¾ÍÉú³ÉÒ»ÅúÇ·²ÉÑù¡£
			while ( !feof(m_pfWaveR) ) {
				int nNumNewlyMadeSamps = 0;	// ±¾Åú£¨¼´´Ó±¾´Î¶ÁÈ¡µÄÔ­Ê¼ÐÅºÅÊý¾Ý£©Éú³ÉµÄÇ·²ÉÑùµã¸öÊý
				//£¨Ò»£©´Óµ±Ç°¸èÇúÒôÆµÊý¾ÝÎÄ¼þ¶ÁÒ»¿éÊý¾Ý£¬¡£¡£¡£
				size_t uNumOrigSampsRead;
				uNumOrigSampsRead = fread(szOrginalSampsBuffer, 
					m_header.byte_p_sample,	// Ò»¸ö¡°²ÉÑù¡±°üº¬ÁËÒ»¸ö²ÉÑùÊ±¼äµãËùÓÐÉùµÀµÄÊý¾Ý
					BUFFER_SIZE_IN_SAMPLES_DS,	// Òª¶ÁÈ¡µÄ²ÉÑù¸öÊý
					m_pfWaveR);
				if (uNumOrigSampsRead == 0) {// no content read
					break;
				}

				if (iTargetSampStartG == 0) {
					// ÎÞÂÛÈçºÎ×öÇ·²ÉÑù£¬µÚÒ»¸öÔ­Ê¼²ÉÑù×ÜÊÇÒªÖ±½Ó¿½±´µÄ¡£Ò²¾ÍÊÇËµ£¬Ç·²ÉÑùÐÅºÅµÄµÚÒ»¸öÑù±¾µã¾ÍÊÇÔ­ÐÅºÅµÄµÚÒ»¸ö
					// Ñù±¾µã£¡
					assert(nNumNewlyMadeSamps == 0);
					// µÚÒ»¸ö²ÉÑùÖµÕÕ°á
					// È¡¸÷¸öÉùµÀµÄÆ½¾ù
					int lval;	// ÓÃ "long" ±£Ö¤¼ÆËã¹ý³Ì²»Òç³ö
					lval = *((char *)szOrginalSampsBuffer);
					lval += *(((char *)szOrginalSampsBuffer)+1);
					lval *= 256;
					lval /= 2;
					m_newlyMadeSamples[nNumNewlyMadeSamps] = lval;
					nNumNewlyMadeSamps++;
				}

				//£¨¶þ£©Éú³É¶ÔÓ¦µÄÐÂ²ÉÑù£¬¡£¡£¡£
				while (1) {
					// ºËÐÄÊÇ½«Ç·²ÉÑùµã¶ÔÓ¦µ½Ä³¸öÔ­²ÉÑùµã£¨Ö»ÓÐ×ó²ÉÑùµã£©£¬»òÁ½¸öÏàÁÚµÄÔ­²ÉÑùµã£¨¼´×óÓÒÁ½¸ö²ÉÑùµã£©Ö®¼ä£¡£¡£¡

					// Óë´ýÇóµÄÇ·²ÉÑùµã¶ÔÓ¦µÄÔ­Ê¼²ÉÑùµãµÄÈ«¾ÖÏÂ±ê£¨ÀíÂÛÉÏ²»Ò»¶¨ÊÇÕûÊý£©
					int iOriginalSampG;
					int iTargetSampG;
					// ¸ù¾ÝÇ·²ÉÑùµãÏÂ±ê£¬¡£¡£¡£
					iTargetSampG = iTargetSampStartG+nNumNewlyMadeSamps;
					// ¼ÆËãÆä¶ÔÓ¦µÄÔ­²ÉÑùµãÏÂ±ê
					iOriginalSampG = (double)iTargetSampG*m_header.sample_rate/m_unifiedSamplingRate+0.5;
					if (iOriginalSampG > iOriginalSampStartG+uNumOrigSampsRead-1) {
						// »¹Î´¶Áµ½¡°Ä¿±êÔ­²ÉÑùµã¡±£¬¡£¡£¡£
						iOriginalSampStartG += uNumOrigSampsRead;
						break;
					}
					// Ä¿±êÔ­²ÉÑùµãÔÚ»º´æÖÐµÄÎ»ÖÃ
					const unsigned char *puchar = szOrginalSampsBuffer+
						(m_header.byte_p_sample)*(iOriginalSampG-iOriginalSampStartG);
					// clrc_002
					//#ifdef __COMBINE_L_R_CHANNELS
					// È¡¸÷¸öÉùµÀµÄÆ½¾ù
					int lval = 0;
					int total = 0;
					//Èç¹û²»¹»15¸ö²ÉÑùµã£¬¾ÍÈ¡Ö®Ç°²ÉÑùµãµÄÆ½¾ù
					if(iOriginalSampG - iOriginalSampStartG < 15)
					{
						int sampleNum = iOriginalSampG - iOriginalSampStartG;
						for(int i = sampleNum; i >= 0; i--)
						{
							lval = *(((char *)puchar) - i);	// ±¾²ÉÑùµÄµÚÒ»¸öÉùµÀ
							lval += *(((char *)puchar) - i +1);	// ±¾²ÉÑùµÄµÚ¶þ¸öÉùµÀ
							lval *= 256;
							lval /= 2;
							total += lval;
						}
						m_newlyMadeSamples[nNumNewlyMadeSamps] = total/(sampleNum + 1);

					}
					//·ñÔò£¬ÀûÓÃFIRÂË²¨Æ÷Ê¹ÓÃ16¸ö²ÉÑùµã
					else
					{
						for(int i = 15; i >= 0; i--)
						{

							lval = *(((char *)puchar) - i);	// ±¾²ÉÑùµÄµÚÒ»¸öÉùµÀ
							lval += *(((char *)puchar) - i +1);	// ±¾²ÉÑùµÄµÚ¶þ¸öÉùµÀ
							lval *= 256;
							lval /= 2;
							total += lval * coefficient[i];
						}
						m_newlyMadeSamples[nNumNewlyMadeSamps] = total;
					}
					nNumNewlyMadeSamps++;	// ±¾ÅúÇ·²ÉÑù¸öÊý¼ÆÊý

				}	// end of "while (1) {"
				for(int i = 0; i < nNumNewlyMadeSamps;i++)
					samplesVector.push_back(m_newlyMadeSamples[i]);
				//£¨ËÄ£©×¼±¸´ÓÒôÀÖÎÄ¼þ¶ÁÈëÏÂÒ»ÅúÔ­²ÉÑùÊý¾Ý£¬¡£¡£¡£
				iTargetSampStartG += nNumNewlyMadeSamps;
				nNumNewlyMadeSampsTotal += nNumNewlyMadeSamps;
			}	// end of "while ( !feof(m_pfWaveR) ) {"
		}

	}


	//µ¥ÉùµÀµÄ»°£¬Ö±½Ó¾ÍÊÇ
	else
	{
		// Ã¿´ÓÔ­Ê¼²ÉÑùÊý¾ÝÎÄ¼þ¶Á¡°Ò»Åú¡±Ô­²ÉÑùÊý¾Ý£¬¾ÍÉú³ÉÒ»ÅúÇ·²ÉÑù¡£
		while ( !feof(m_pfWaveR) ) {
			int nNumNewlyMadeSamps = 0;	// ±¾Åú£¨¼´´Ó±¾´Î¶ÁÈ¡µÄÔ­Ê¼ÐÅºÅÊý¾Ý£©Éú³ÉµÄÇ·²ÉÑùµã¸öÊý
			//£¨Ò»£©´Óµ±Ç°¸èÇúÒôÆµÊý¾ÝÎÄ¼þ¶ÁÒ»¿éÊý¾Ý£¬¡£¡£¡£
			size_t uNumOrigSampsRead;
			uNumOrigSampsRead = fread(szOrginalSampsBuffer, 
				m_header.byte_p_sample,	// Ò»¸ö¡°²ÉÑù¡±°üº¬ÁËÒ»¸ö²ÉÑùÊ±¼äµãËùÓÐÉùµÀµÄÊý¾Ý
				BUFFER_SIZE_IN_SAMPLES_DS,	// Òª¶ÁÈ¡µÄ²ÉÑù¸öÊý
				m_pfWaveR);
			if (uNumOrigSampsRead == 0) {// no content read
				break;
			}

			if (iTargetSampStartG == 0) {
				// ÎÞÂÛÈçºÎ×öÇ·²ÉÑù£¬µÚÒ»¸öÔ­Ê¼²ÉÑù×ÜÊÇÒªÖ±½Ó¿½±´µÄ¡£Ò²¾ÍÊÇËµ£¬Ç·²ÉÑùÐÅºÅµÄµÚÒ»¸öÑù±¾µã¾ÍÊÇÔ­ÐÅºÅµÄµÚÒ»¸ö
				// Ñù±¾µã£¡
				assert(nNumNewlyMadeSamps == 0);
				// µÚÒ»¸ö²ÉÑùÖµÕÕ°á
				if (m_header.bit_p_sample == 16) {
					m_newlyMadeSamples[nNumNewlyMadeSamps] = *((short *)szOrginalSampsBuffer);
				} else if (m_header.bit_p_sample == 8) {
					m_newlyMadeSamples[nNumNewlyMadeSamps] = *((char *)szOrginalSampsBuffer);
					m_newlyMadeSamples[nNumNewlyMadeSamps] *= 256;
				}
				nNumNewlyMadeSamps++;
			}

			//£¨¶þ£©Éú³É¶ÔÓ¦µÄÐÂ²ÉÑù£¬¡£¡£¡£
			while (1) {
				// ºËÐÄÊÇ½«Ç·²ÉÑùµã¶ÔÓ¦µ½Ä³¸öÔ­²ÉÑùµã£¨Ö»ÓÐ×ó²ÉÑùµã£©£¬»òÁ½¸öÏàÁÚµÄÔ­²ÉÑùµã£¨¼´×óÓÒÁ½¸ö²ÉÑùµã£©Ö®¼ä£¡£¡£¡

				// Óë´ýÇóµÄÇ·²ÉÑùµã¶ÔÓ¦µÄÔ­Ê¼²ÉÑùµãµÄÈ«¾ÖÏÂ±ê£¨ÀíÂÛÉÏ²»Ò»¶¨ÊÇÕûÊý£©
				int iOriginalSampG;
				int iTargetSampG;
				// ¸ù¾ÝÇ·²ÉÑùµãÏÂ±ê£¬¡£¡£¡£
				iTargetSampG = iTargetSampStartG+nNumNewlyMadeSamps;
				// ¼ÆËãÆä¶ÔÓ¦µÄÔ­²ÉÑùµãÏÂ±ê
				iOriginalSampG = (double)iTargetSampG*m_header.sample_rate/m_unifiedSamplingRate+0.5;
				if (iOriginalSampG > iOriginalSampStartG+uNumOrigSampsRead-1) {
					// »¹Î´¶Áµ½¡°Ä¿±êÔ­²ÉÑùµã¡±£¬¡£¡£¡£
					iOriginalSampStartG += uNumOrigSampsRead;
					break;
				}
				// Ä¿±êÔ­²ÉÑùµãÔÚ»º´æÖÐµÄÎ»ÖÃ
				const unsigned char *puchar = szOrginalSampsBuffer+
					(m_header.byte_p_sample)*(iOriginalSampG-iOriginalSampStartG);
				int lval = 0;
				int total = 0;
				//Èç¹û²»¹»15¸ö²ÉÑùµã£¬¾ÍÈ¡Ö®Ç°²ÉÑùµãµÄÆ½¾ù
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
				//·ñÔò£¬ÀûÓÃFIRÂË²¨Æ÷Ê¹ÓÃ16¸ö²ÉÑùµã
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
				nNumNewlyMadeSamps++;	// ±¾ÅúÇ·²ÉÑù¸öÊý¼ÆÊý

			}	// end of "while (1) {"
			for(int i = 0; i < nNumNewlyMadeSamps;i++)
				samplesVector.push_back(m_newlyMadeSamples[i]);
			//£¨ËÄ£©×¼±¸´ÓÒôÀÖÎÄ¼þ¶ÁÈëÏÂÒ»ÅúÔ­²ÉÑùÊý¾Ý£¬¡£¡£¡£
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
