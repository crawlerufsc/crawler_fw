#ifndef _GSTREAM_CLIENT_H
#define _GSTREAM_CLIENT_H
#include <gst/gst.h>
#include <gst/app/app.h>
#include <glib.h>
#include <glib/gtypes.h>
#include "frame.h"

class GstreamClient
{

public:
    GstreamClient();
    ~GstreamClient();
    void initializeStream(const char *pipelineConfig);
    void stop();
    void requestNextFrame();

protected:
    virtual void onTerminate() {}
    virtual void onFrameReceived(Frame<u_char> *frame) = 0;
    void dropCurrentFrame();
    void dropAlwaysUnlessNull();
    
    
private:
    GstElement *pipeline;
    GstBus *bus;
    Frame<u_char> *rawFrame;
    bool dropUnlessNull;

    bool readStreamData(GstAppSink *sink);
    void terminate();
    void initFrame(int width, int height, long length);
    void onFrameReceived();

    static GstFlowReturn new_preroll(GstAppSink *sink, gpointer data);

    static gboolean busCallback(GstBus *bus, GstMessage *message, gpointer data);

    static GstFlowReturn newSample(GstAppSink *appsink, gpointer data);
};

#endif