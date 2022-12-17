#include <gtest/gtest.h>
#include <stdlib.h>

#include "../include/dataset_stream_reader.h"
#include "../include/file_utils.h"
#include "../utils/image_utils.h"

int processFrameCallCount = 0;

void onProcessFrame(DirectProcessPipeline<StreamData> *reader, StreamData *frame)
{
    processFrameCallCount++;
    EXPECT_EQ(160, frame->width);
    EXPECT_EQ(200, frame->height);
    EXPECT_TRUE(frame != nullptr);

    FileData *p = FileUtils::readFile(std::string("occupancy_grid_output_test.png"));
    auto decoded = ImageUtils::decodeImageData(p->data, p->length);
    EXPECT_EQ(*(decoded->data), *(frame->data));
}

TEST(DataSetStreamReader, ReadSuccess)
{
    int frameRate = 50;

    DirectProcessPipeline<StreamData> *reader = (new DataSetStreamReader(160, 200, frameRate))
                                                    ->addSource(std::string("occupancy_grid_output_test.png"))
                                                    ->withOnProcessCallback(&onProcessFrame);

    reader->run(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    reader->terminate();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_TRUE(processFrameCallCount >= 1000 / frameRate);
}