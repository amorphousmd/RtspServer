#include <cstdlib> // For rand function

#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <opencv2/opencv.hpp>


typedef struct
{
  cv::Mat frame;
  GstClockTime timestamp;
} MyContext;

/* called when we need to give data to appsrc */
void need_data(GstElement* appsrc, guint unused, MyContext* ctx)
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
    int randomImageNumber = rand() % 5 + 1;

    // Construct the image filename
    std::string filename = "images/" + std::to_string(randomImageNumber) + ".jpg";

    // Load the image from disk
    cv::Mat frame = cv::imread(filename);

    if (frame.empty()) {
        // Handle the case where the image couldn't be loaded
        g_print("Failed to load the image from disk: %s\n", filename.c_str());
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

/* called when a new media pipeline is constructed. We can query the
 * pipeline and configure our appsrc */
void media_configure(GstRTSPMediaFactory *factory, GstRTSPMedia *media,
                            gpointer user_data)
{
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
  ctx->frame = cv::Mat(288, 384, CV_8UC3); // Adjust the size as needed
  ctx->timestamp = 0;
  /* make sure the data are freed when the media is gone */
  g_object_set_data_full(G_OBJECT(media), "my-extra-data", ctx,
                         (GDestroyNotify)g_free);

  /* install the callback that will be called when a buffer is needed */
  g_signal_connect(appsrc, "need-data", (GCallback)need_data, ctx);
  gst_object_unref(appsrc);
  gst_object_unref(element);
}

int main(int argc, char *argv[])
{
  GMainLoop *loop;
  GstRTSPServer *server;
  GstRTSPMountPoints *mounts;
  GstRTSPMediaFactory *factory;

  gst_init(&argc, &argv);

  loop = g_main_loop_new(NULL, FALSE);

  /* create a server instance */
  server = gst_rtsp_server_new();

  /* get the mount points for this server, every server has a default object
   * that be used to map uri mount points to media factories */
  mounts = gst_rtsp_server_get_mount_points(server);

  /* make a media factory for a test stream. The default media factory can use
   * gst-launch syntax to create pipelines.
   * any launch line works as long as it contains elements named pay%d. Each
   * element with pay%d names will be a stream */
  factory = gst_rtsp_media_factory_new();
  gst_rtsp_media_factory_set_launch(factory,
                                    "( appsrc name=mysrc ! videoconvert ! video/x-raw,format=BGRx ! queue ! nvvidconv ! video/x-raw(memory:NVMM), format=(string)NV12, width=640,height=480 ! nvv4l2h264enc bitrate=8000000 maxperf-enable=true insert-sps-pps=true ! rtph264pay name=pay0 pt=96 config-interval=1 )");

  /* notify when our media is ready, This is called whenever someone asks for
   * the media and a new pipeline with our appsrc is created */
  g_signal_connect(factory, "media-configure", (GCallback)media_configure,
                   NULL);

  /* attach the test factory to the /test url */
  gst_rtsp_mount_points_add_factory(mounts, "/test", factory);

  /* don't need the ref to the mounts anymore */
  g_object_unref(mounts);

  /* attach the server to the default maincontext */
  gst_rtsp_server_attach(server, NULL);

  /* start serving */
  g_print("stream ready at rtsp://127.0.0.1:8554/test\n");
  g_main_loop_run(loop);

  return 0;
}

