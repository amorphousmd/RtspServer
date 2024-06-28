#include "RtspServer.hpp"

RtspServer::RtspServer()
{
    cv::Mat pic1 = cv::imread("images/1.jpg");
    frame_pointer = &pic1;
    gst_init(NULL, NULL);
    loop = g_main_loop_new(NULL, FALSE);
    server = gst_rtsp_server_new();
    GstRTSPMountPoints* mounts = gst_rtsp_server_get_mount_points(server);

    factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(factory,
        "( appsrc name=mysrc ! videoconvert ! video/x-raw,format=BGRx ! queue ! nvvidconv ! video/x-raw(memory:NVMM), format=(string)NV12, width=640,height=480 ! nvv4l2h264enc bitrate=8000000 maxperf-enable=true insert-sps-pps=true ! rtph264pay name=pay0 pt=96 config-interval=1 )");

    g_signal_connect(factory, "media-configure", G_CALLBACK(media_configure), this);
    gst_rtsp_mount_points_add_factory(mounts, "/test", factory);
    g_object_unref(mounts);
    gst_rtsp_server_attach(server, NULL);
}

RtspServer::~RtspServer()
{
    g_main_loop_unref(loop);
}

void RtspServer::start_server()
{
    g_print("stream ready at rtsp://127.0.0.1:8554/test\n");
    g_main_loop_run(loop);
}

void RtspServer::stop_server()
{
    g_main_loop_quit(loop);
}

void RtspServer::feed_frame(cv::Mat* new_frame)
{
    frame_pointer = new_frame;
}
void RtspServer::need_data(GstElement* appsrc, guint unused, MyContext* ctx)
{
    GstBuffer* buffer;
    guint size;
    GstMapInfo map;
    GstFlowReturn ret;

    // Define image properties
    int width = 384;
    int height = 288;
    int channels = 3;

    size = width * height * channels;

    buffer = gst_buffer_new_allocate(NULL, size, NULL);

    // Map the buffer for writing
    gst_buffer_map(buffer, &map, GST_MAP_WRITE);

    // Generate a random image number between 1 and 5
    // int randomImageNumber = rand() % 5 + 1;

    // Construct the image filename
    // std::string filename = "images/" + std::to_string(randomImageNumber) + ".jpg";

    // // Load the image from disk
    // cv::Mat frame = cv::imread(filename);
    cv::Mat frame = *(ctx->frame_pointer);

    if (frame.empty()) {
        // Handle the case where the image couldn't be loaded
        // g_print("Failed to load the image from disk: %s\n", filename.c_str());
        g_print("Failed to load the image from disk: %s\n");
        gst_buffer_unref(buffer);
        return;
    }

    // Resize the image to the desired height and width
    cv::resize(frame, frame, cv::Size(width, height));

    // Copy the frame data to the GStreamer buffer
    std::memcpy(map.data, frame.data, size);

    // Unmap the buffer
    gst_buffer_unmap(buffer, &map);

    // Set timestamp and duration
    GST_BUFFER_PTS(buffer) = ctx->timestamp;
    GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30);
    ctx->timestamp += GST_BUFFER_DURATION(buffer);

    // Push the buffer to appsrc
    g_signal_emit_by_name(appsrc, "push-buffer", buffer, &ret);

    // Unref the buffer
    gst_buffer_unref(buffer);
}

void RtspServer::media_configure(GstRTSPMediaFactory* factory, GstRTSPMedia* media, gpointer user_data)
{
    RtspServer* self = static_cast<RtspServer*>(user_data);
    GstElement *element, *appsrc;
    MyContext *ctx;

    /* get the element used for providing the streams of the media */
    element = gst_rtsp_media_get_element(media);

    /* get our appsrc, we named it 'mysrc' with the name property */
    appsrc = gst_bin_get_by_name_recurse_up(GST_BIN(element), "mysrc");

    /* this instructs appsrc that we will be dealing with timed buffer */
    gst_util_set_object_arg(G_OBJECT(appsrc), "format", "time");
    /* configure the caps of the video */
    g_object_set(G_OBJECT(appsrc), "caps",
                gst_caps_new_simple("video/x-raw",
                                    "format", G_TYPE_STRING, "BGR",
                                    "width", G_TYPE_INT, 384,
                                    "height", G_TYPE_INT, 288,
                                    "framerate", GST_TYPE_FRACTION, 0, 1, NULL), NULL);

    ctx = g_new0(MyContext, 1);
    ctx->frame_pointer = self->frame_pointer; // Adjust the size as needed
    ctx->timestamp = 0;
    /* make sure the data are freed when the media is gone */
    g_object_set_data_full(G_OBJECT(media), "my-extra-data", ctx,
                            (GDestroyNotify)g_free);

    /* install the callback that will be called when a buffer is needed */
    g_signal_connect(appsrc, "need-data", (GCallback)need_data, ctx);
    gst_object_unref(appsrc);
    gst_object_unref(element);
}
