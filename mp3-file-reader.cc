#include <iostream>
#include <cstring>
#include "mp3-file-reader.h"

MpegFileReader::MpegFileReader(std::string filepath, float samplerate,
        int channels, int blocksize, int stepsize) {
    m_filepath   = filepath;
    m_samplerate = samplerate;
    m_channels   = channels;
    m_blocksize  = blocksize;
    m_stepsize   = stepsize;
    m_filebuf    = NULL;
    m_plugbuf    = NULL;
    m_remain = m_overlap = m_currentstep = 0;
}

MpegFileReader::~MpegFileReader() {
    mpg123_close(m_handle);
    mpg123_delete(m_handle);
    mpg123_exit();

    if(m_filebuf)
        delete m_filebuf;
    if(m_plugbuf) {
        for(int c = 0; c < m_audio_channels; c++)
            delete m_plugbuf[c];
        delete m_plugbuf;
    }
}

bool MpegFileReader::initialize() {
    // create mpg123 handle
    int err = mpg123_init();
    if(err != MPG123_OK || (m_handle = mpg123_new(NULL, &err)) == NULL)
        return false;

    // set some param
    mpg123_param(m_handle, MPG123_ADD_FLAGS, MPG123_FORCE_FLOAT, 0);

    // open mpeg file
    if(mpg123_open(m_handle, m_filepath.c_str()) != MPG123_OK)
        return false;
    int encoding, channels;
    long rate;
    if(mpg123_getformat(m_handle, &rate, &channels, &encoding) != MPG123_OK)
        return false;

    // set format
    mpg123_format_none(m_handle);
    mpg123_format(m_handle, rate, channels, encoding);
    //mpeg_outblock();

    // allocate buffers
    m_audio_samplerate = rate;
    m_audio_channels = channels;
    m_filebuf = new float[m_blocksize * m_audio_channels];
    m_plugbuf = new float*[m_audio_channels];
    for(int c = 0; c < m_audio_channels; c++)
        m_plugbuf[c] = new float[m_blocksize + 2];

    int tmp = (m_blocksize / m_stepsize) - 1;
    m_remain = tmp > 1 ? tmp : 1;
    m_overlap = m_blocksize - m_stepsize;

    //std::cerr << "  samplerate : " << m_audio_samplerate << std::endl;
    //std::cerr << "  channels   : " << m_audio_channels << std::endl;

    return true;
}

long MpegFileReader::getSampleRate() {
	return m_audio_samplerate;
}

size_t MpegFileReader::read(float *buf, size_t count) {
    size_t done = 0;
    unsigned char * cbuf = (unsigned char *)(void *)buf;
    int ret = mpg123_read(m_handle, cbuf,
            count * sizeof(float) * m_audio_channels, &done);
    if(ret != MPG123_OK)    return 0;
    return done / sizeof(float) / m_audio_channels;
    //return done;
}

float ** MpegFileReader::nextBlock() {
    // failed or complete reading
    if(m_filebuf == NULL || m_remain <= 0)   return NULL;

    // read from the audio file to m_filebuf
    int count;
    if(m_blocksize == m_stepsize || m_currentstep == 0) {
        count = read(m_filebuf, m_blocksize);
        if(count < 0)   return NULL;
        if(count != m_blocksize)    m_remain--;
    } else {
        memmove(m_filebuf, m_filebuf + (m_stepsize * m_audio_channels),
                m_overlap * m_audio_channels * sizeof(float));
        count = read(m_filebuf + (m_overlap * m_audio_channels), m_stepsize);
        if(count < 0)   return NULL;
        if(count != m_stepsize)   m_remain--;
        count += m_overlap;
    }

    // copy data from m_filebuf to m_plugbuf
    for(int c = 0; c < m_audio_channels; c++) {
        int j = 0;
        while(j < count) {
            m_plugbuf[c][j] = m_filebuf[j * m_audio_channels + c];
            j++;
        }
        // not enough data then fill 0.0
        while(j < m_blocksize) {
            m_plugbuf[c][j] = 0.0f;
            j++;
        }
    }

    // do some converts when m_channels is 1
    // current we just get the mean of two channels
    // TODO let the user to choose one: only use one or use the mean
    if(m_channels == 1 && m_audio_channels == 2) {
        for(int j = 0; j < m_blocksize; j++) {
            m_plugbuf[0][j] = (m_plugbuf[0][j] + m_plugbuf[1][j]) / 2.0;
        }
    }

    m_currentstep ++;
    return m_plugbuf;
}

