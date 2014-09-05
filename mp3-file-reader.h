#pragma once
#include <string>
#include <vector>
#include <mpg123.h>

class MpegFileReader {
public:
    MpegFileReader(std::string filepath, float samplerate, int channels,
            int blocksize, int stepsize);
    ~MpegFileReader();

    bool initialize();
    size_t read(float *buf, size_t count);
		float** nextBlock();
		long getSampleRate();

protected:
    /* mpeg file handle */
    mpg123_handle *m_handle;
    std::string m_filepath;
    long m_samplerate;
    long m_audio_samplerate;
    int m_channels;
    int m_audio_channels;
    float *m_filebuf;
    float **m_plugbuf;
    int m_blocksize;
    int m_stepsize;
    int m_remain;
    int m_overlap;
    int m_currentstep;
};
