#ifndef __DIRECT_PROCESS_PIPELINE_H
#define __DIRECT_PROCESS_PIPELINE_H

#include <queue>
#include <mutex>
#include <thread>
#include <iostream>
#include "frame.h"

template <typename T>
class DirectProcessPipeline
{
private:
    std::mutex *queueFrameProcess;
    bool loop_run;
    T *frame;

    void (*onProcess)(DirectProcessPipeline<T> *, T *);
    std::thread *requestFrameThread;
    std::thread *processThread;

    void requestFrameThr()
    {
        while (this->loop_run)
        {
            T *f = this->onRequestNextFrame();
            this->queueFrameProcess->lock();
            replaceFrame(f);
            this->queueFrameProcess->unlock();
            //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void replaceFrame(T *f)
    {
        if (this->frame != nullptr)
        {
            delete this->frame;
        }
        this->frame = f;
        //std::cout << "new frame on " << reinterpret_cast<void *>(this->frame) << std::endl;
    }

    void processThr()
    {
        if (this->onProcess == nullptr)
        {
            return;
        }

        while (this->loop_run)
        {

            T *procFrame = nullptr;
            this->queueFrameProcess->lock();
            if (this->frame != nullptr)
            {
                procFrame = this->frame;
                this->frame = nullptr;
            }
            this->queueFrameProcess->unlock();

            if (procFrame != nullptr)
            {
                //std::cout << "processing frame " << reinterpret_cast<void *>(procFrame) << std::endl;
                this->onProcess(this, procFrame);
                delete procFrame;
            }
        }
    }

    // std::cout << "new frame "<< reinterpret_cast<void *>(this->frame) << std::endl;
protected:
    virtual bool initialize() = 0;
    virtual T *onRequestNextFrame() = 0;
    virtual void onTerminate() = 0;

    virtual bool isConnected()
    {
        return loop_run;
    }

public:
    DirectProcessPipeline()
    {
        this->queueFrameProcess = new std::mutex();

        loop_run = false;
        frame = NULL;
    }
    ~DirectProcessPipeline()
    {
        delete this->queueFrameProcess;

        if (this->requestFrameThread != nullptr)
            delete this->requestFrameThread;

        if (this->processThread != nullptr)
            delete this->processThread;
    }

    DirectProcessPipeline<T> *withOnProcessCallback(void (*onProcess)(DirectProcessPipeline<T> *, T *))
    {
        this->onProcess = onProcess;
        return this;
    }

    void run(bool block = true)
    {
        loop_run = initialize();
        requestFrameThread = new std::thread(&DirectProcessPipeline::requestFrameThr, this);
        processThread = new std::thread(&DirectProcessPipeline::processThr, this);
        if (block)
            wait();
    }

    void wait()
    {
        if (requestFrameThread != nullptr)
            requestFrameThread->join();

        if (processThread != nullptr)
            processThread->join();

        requestFrameThread = nullptr;
        processThread = nullptr;
    }

    void terminate()
    {
        loop_run = false;
    }
};

#endif