#ifndef _FRAME_H
#define _FRAME_H

#include <memory>
#include <opencv2/opencv.hpp>

template <typename FrameType>
class Frame
{
private:
    cv::Mat *_mat;
    static int convertMatrixToVectorPos(int width, int heightPos, int widthPos)
    {
        return (width * heightPos) + widthPos;
    }

public:
    FrameType *data;
    int width;
    int height;
    int imgType;
    long seq;
    long len;

    void clear()
    {
        memset(data, 0, sizeof(FrameType) * (width * height));
    }

    void print()
    {
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width; j++)
                printf("%d", data[i * width + j]);
            printf("\n");
        }
    }

    static Frame<FrameType> *newEmptyFrame(int width, int height)
    {
        long size = width * height;
        FrameType *data = new FrameType[size];
        Frame<FrameType> *f = new Frame<FrameType>(width, height, data, size, 0);

        f->clear();
        return f;
    }

    Frame(int width, int height, FrameType *data, long len, int imgType = 0)
    {
        this->width = width;
        this->height = height;
        this->data = data;
        this->imgType = imgType;
        this->_mat = nullptr;
        this->len = len;
    }

    ~Frame()
    {
        if (_mat != nullptr)
        {
            //std::cout << "dealloc Frame matrix\n";
            _mat->release();
            
            delete _mat;
        }
        if (data != nullptr)
        {
            //std::cout << "dealloc Frame data on " << reinterpret_cast<void *>(data) << std::endl;
            delete data;
        }
    }

    cv::Mat *getMatrix(int type)
    {
        if (this->_mat == nullptr) {
            //std::cout << "alloc Frame matrix\n";
            this->_mat = new cv::Mat(height, width, type, this->data);
        }
        return this->_mat;
    }

    const FrameType operator()(int h, int w) const
    {
        if (w < 0 || w > width)
            throw std::invalid_argument("invalid index width for Frame data");

        if (h < 0 || h > height)
            throw std::invalid_argument("invalid index height for Frame data");

        int pos = convertMatrixToVectorPos(width, h, w);

        return data[pos];
    }

    FrameType get(int h, int w)
    {
        int pos = convertMatrixToVectorPos(width, h, w);
        return data[pos];
    }

    void set(int h, int w, FrameType value)
    {
        if (w < 0 || w > width)
            throw std::invalid_argument("invalid index width for Frame data");

        if (h < 0 || h > height)
            throw std::invalid_argument("invalid index height for Frame data");

        int pos = convertMatrixToVectorPos(width, h, w);

        data[pos] = value;
    }

    void copyFrom(Frame<FrameType> *target)
    {
        for (int i = 0; i < width * height; i++)
            this->data[i] = target->data[i];
    }
};

typedef Frame<unsigned char> StreamData;

#endif