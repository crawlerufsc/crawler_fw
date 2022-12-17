#include "../include/gstream_client.h"

bool GstreamClient::gstreamInitialized = false;

GstreamClient::GstreamClient()
{
    if (!GstreamClient::gstreamInitialized)
    {
        gst_init(NULL, NULL);
        GstreamClient::gstreamInitialized = true;
    }

    this->dropUnlessNull = false;
    this->rawFrame = nullptr;
    this->bus = nullptr;
    this->pipeline = nullptr;
}

GstreamClient::~GstreamClient()
{
    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);

    if (bus != nullptr)
        gst_object_unref(bus);

    gst_object_unref(pipeline);

    if (this->rawFrame != nullptr)
    {
        delete this->rawFrame;
        this->rawFrame = nullptr;
    }
}

void GstreamClient::initializeStream(const char *pipelineConfig)
{
    printf ("initializing stream receive for %s\n", pipelineConfig);
    
    GError *error = nullptr;
    pipeline = gst_parse_launch(pipelineConfig, &error);

    if (error)
    {
        g_print("could not construct pipeline: %s\n", error->message);
        g_error_free(error);
        throw std::invalid_argument("pipelineConfig is invalid");
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, &GstreamClient::busCallback, this);

    GstElement *sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");
    if (sink == nullptr)
    {
        throw std::invalid_argument("pipelineConfig must have a sink element named sink");
    }

    gst_app_sink_set_emit_signals((GstAppSink *)sink, true);
    gst_app_sink_set_drop((GstAppSink *)sink, true);
    gst_app_sink_set_max_buffers((GstAppSink *)sink, 1);

    GstAppSinkCallbacks callbacks = {nullptr, GstreamClient::new_preroll, GstreamClient::newSample};
    gst_app_sink_set_callbacks(GST_APP_SINK(sink), &callbacks, this, nullptr);

    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
}

void GstreamClient::initializeNonInterableStream(const char *pipelineConfig)
{
    GError *error = nullptr;
    pipeline = gst_parse_launch(pipelineConfig, &error);

    if (error)
    {
        g_print("could not construct pipeline: %s\n", error->message);
        g_error_free(error);
        throw std::invalid_argument("pipelineConfig is invalid");
    }

    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_bus_add_watch(bus, &GstreamClient::busCallback, this);

    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
}

GstFlowReturn GstreamClient::new_preroll(GstAppSink *sink, gpointer data)
{
    return GST_FLOW_OK;
}

gboolean GstreamClient::busCallback(GstBus *bus, GstMessage *message, gpointer data)
{
    GstreamClient *client = (GstreamClient *)data;

    switch (GST_MESSAGE_TYPE(message))
    {
    case GST_MESSAGE_ERROR:
    {
        GError *err;
        gchar *debug;
        gst_message_parse_error(message, &err, &debug);
        g_print("Error: %s\n", err->message);
        g_error_free(err);
        g_free(debug);
        break;
    }
    case GST_MESSAGE_EOS:
        client->onTerminate();
        break;
    }
    return true; // notify again
}

void GstreamClient::initFrame(int width, int height, long length)
{
    if (rawFrame == nullptr)
    {
        u_char *data = (u_char *)malloc(sizeof(u_char) * (length + 1));
        rawFrame = new StreamData(width, height, data, length);
        return;
    }

    if (rawFrame->width != width || rawFrame->height != height || rawFrame->len != length)
    {
        delete rawFrame;
        initFrame(width, height, length);
    }
}

bool GstreamClient::readStreamData(GstAppSink *sink)
{
    if (sink == nullptr)
        return false;

    GstSample *sample = gst_app_sink_pull_sample(sink);
    if (sample == nullptr)
        return false;

    GstBuffer *buffer = gst_sample_get_buffer(sample);
    if (buffer == nullptr)
        return false;

    GstCaps *caps = gst_sample_get_caps(sample);
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    StreamData *result = nullptr;
    GstMapInfo map;

    if (!dropUnlessNull || rawFrame == nullptr)
    {
        if (gst_buffer_map(buffer, &map, GST_MAP_READ))
        {
            int width = g_value_get_int(gst_structure_get_value(structure, "width"));
            int height = g_value_get_int(gst_structure_get_value(structure, "height"));

            // printf ("detected size: %dx%d\n", width, height);

            if (map.data != nullptr)
            {
                initFrame(width, height, map.size);
                memcpy(rawFrame->data, map.data, sizeof(u_char) * map.size);
                gst_buffer_unmap(buffer, &map);
            }
        }
    }
    // else {
    //     printf ("frame drop\n");
    // }

    gst_sample_unref(sample);
    return true;
}

void GstreamClient::onFrameReceived()
{
    this->onFrameReceived(rawFrame);
}

GstFlowReturn GstreamClient::newSample(GstAppSink *appsink, gpointer data)
{

    GstreamClient *client = (GstreamClient *)data;
    // printf ("newSample: response client addr: %p\n", client);
    if (client->readStreamData(appsink))
        client->onFrameReceived();
    return GST_FLOW_OK;
}

void GstreamClient::stop()
{
    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_NULL);
}
void GstreamClient::restart()
{
    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);
}

void GstreamClient::dropCurrentFrame()
{
    this->rawFrame = nullptr;
}

void GstreamClient::dropAlwaysUnlessNull()
{
    this->dropUnlessNull = true;
}

void GstreamClient::requestNextFrame()
{
    GstMessage *message = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                                     (GstMessageType)(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    busCallback(this->bus, message, this);
}