#ifndef _DATA_STREAM_READER_H
#define _DATA_STREAM_READER_H

#include "file_utils.h"
#include "frame.h"
#include "direct_process_pipeline.h"

class DataSetStreamReader : public DirectProcessPipeline<Frame<unsigned char>>
{
private:
    std::vector<std::string> *input;
    uint32_t width;
    uint32_t height;
    uint32_t frameRate;
    uint32_t repeat_for;
    uint32_t repeat_for_pos;
    int pos;

    void delayFrameRate();

public:
    DataSetStreamReader(uint32_t width, uint32_t height, uint32_t frameRate);
    ~DataSetStreamReader();

    DataSetStreamReader *addSource(std::string path);
    uint32_t GetWidth();
    uint32_t GetHeight();
    uint32_t GetFrameRate();
    DataSetStreamReader *repeatFrame(uint32_t times);

    bool initialize() override;
    Frame<unsigned char> * onRequestNextFrame() override;
    void onTerminate() override;
};

#endif