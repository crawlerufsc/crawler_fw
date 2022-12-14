// gst-launch-1.0 videotestsrc ! videoconvert ! autovideosink

#include <gst/gst.h>
#include <gst/app/app.h>
#include <glib.h>
#include <glib/gtypes.h>
#include <iostream>
#include <sstream>
#include <string>

class TestVideoSource
{
private:
    GstElement *pipeline;
    GstBus *bus;

public:
    TestVideoSource(const char *pipeline)
    {
        buildPipeline(pipeline);
    }
   ~TestVideoSource()
    {
        destroyPipeline();
    }
private:
 

    static gboolean busCallback(GstBus *bus, GstMessage *message, gpointer data)
    {
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
            g_print("EOS\n");
            break;
        }
        return true; // notify again
    }

    void buildPipeline(const char *pipelineStr)
    {
        gst_init(NULL, NULL);

        // GstElement *source = gst_element_factory_make("videotestsrc", "source");
        // GstElement *sink = gst_element_factory_make("autovideoconvert", "sink");
        // GstElement *pipeline = gst_pipeline_new("test-pipeline");
        // gst_bin_add_many(GST_BIN(pipeline), source, sink, NULL);

        // g_object_set(source, "pattern", 18, NULL);

        GError *error = nullptr;

        pipeline = gst_parse_launch(pipelineStr, &error);

        if (error)
        {
            g_print("could not construct pipeline: %s\n", error->message);
            g_error_free(error);
            throw std::invalid_argument("pipelineConfig is invalid");
        }

        GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);

        if (ret == GST_STATE_CHANGE_FAILURE)
            throw std::runtime_error("the pipeline could not be created");

        bus = gst_element_get_bus(pipeline);
        pipeline = pipeline;
        gst_bus_add_watch(bus, TestVideoSource::busCallback, this);
    }

    void destroyPipeline()
    {
        gst_object_unref(bus);
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
    }
};
