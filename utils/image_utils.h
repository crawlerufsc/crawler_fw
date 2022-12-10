#ifndef _IMAGE_UTILS_H
#define _IMAGE_UTILS_H

#include <opencv2/opencv.hpp>
#include <vector>
#include "../include/frame.h"

class ImageUtils
{
public:
    static cv::Mat *convertToOpenCVMatrix(Frame<unsigned char> *data)
    {
        return new cv::Mat(data->height, data->width, data->imgType, data->data);
    }

    static Frame<unsigned char> *convertOpenCVMatrixToFrame(cv::Mat  *mat)
    {
        return new Frame<unsigned char>(mat->cols, mat->rows, mat->data, mat->type());
    }

    static Frame<unsigned char> *decodeImageData(char *buffer, int length)
    {
        std::vector<char> imgv(buffer, buffer + length);
        cv::Mat img = cv::imdecode(cv::Mat(imgv), 1);       
        return convertOpenCVMatrixToFrame(&img);
    }

    static Frame<unsigned char> *decodeImageToOccupancyGrid(char *buffer, int length)
    {
        std::vector<char> imgv(buffer, buffer + length);
        cv::Mat img = cv::imdecode(cv::Mat(imgv), 1);

        cv::cvtColor(img, img, cv::COLOR_RGB2GRAY);
        cv::threshold(img, img, 0, 1, cv::THRESH_BINARY);

        // return new Frame<unsigned char>(img.rows, img.cols, img.data, img.type());

        Frame<unsigned char> * f = Frame<unsigned char>::newEmptyFrame(img.cols, img.rows);
        
        for (int i = 0; i < img.rows * img.cols; i++)
            f->data[i] = img.data[i];
        
        f->imgType = img.type();

        return f;
    }
};

#endif