#include <stdio.h>
#include <string>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "../include/dataset_stream_reader.h"
#include "../utils/image_utils.h"

void DataSetStreamReader::delayFrameRate()
{
    if (frameRate <= 0)
        return;

    // fprintf (stdout, "sleeping for %d miliseconds\n",(1000/frameRate));
    std::this_thread::sleep_for(std::chrono::milliseconds(1000 / frameRate));
}

DataSetStreamReader::DataSetStreamReader(uint32_t width, uint32_t height, uint32_t frameRate)
{
    this->input = new std::vector<std::string>();
    this->width = width;
    this->height = height;
    this->pos = 0;
    this->repeat_for = 1;
    this->repeat_for_pos = 0;
    this->frameRate = frameRate;
}

DataSetStreamReader::~DataSetStreamReader()
{
    delete input;
}

DataSetStreamReader *DataSetStreamReader::addSource(std::string path)
{
    if (FileUtils::fileExists(path))
        input->push_back(path);

    return this;
}

uint32_t DataSetStreamReader::GetWidth()
{
    return width;
}

uint32_t DataSetStreamReader::GetHeight()
{
    return height;
}
uint32_t DataSetStreamReader::GetFrameRate()
{
    return frameRate;
}
DataSetStreamReader *DataSetStreamReader::repeatFrame(uint32_t times)
{
    this->repeat_for = times;
    return this;
}

bool DataSetStreamReader::initialize()
{
    return true;
}
Frame<unsigned char> * DataSetStreamReader::onRequestNextFrame()
{
    if (pos >= input->size())
    {
        pos = 0;
    }

    std::string filename = input->at(pos);

    this->repeat_for_pos++;

    if (this->repeat_for_pos >= this->repeat_for)
    {
        this->repeat_for_pos = 0;
        this->pos++;
    }

    delayFrameRate();

    FileData *p = FileUtils::readFile(filename);
    return ImageUtils::decodeImageToOccupancyGrid(p->data, p->length);
}


void DataSetStreamReader::onTerminate()
{
}
