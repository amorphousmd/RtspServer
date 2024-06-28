#ifndef __RTSPSERVER_H__
#define __RTSPSERVER_H__

#include <cstdlib> // For rand function
#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <opencv2/opencv.hpp>

class RtspServer
{
private:
    GstRTSPServer* server;
    GMainLoop* loop;
    GstElement* appsrc;
    GstRTSPMediaFactory* factory;

    typedef struct {
        cv::Mat* frame_pointer;
        GstClockTime timestamp;
    } MyContext;

    static void need_data(GstElement* appsrc, guint unused, MyContext* ctx);
    static void media_configure(GstRTSPMediaFactory* factory, GstRTSPMedia* media, gpointer user_data);

public:
    RtspServer(/* args */);
    ~RtspServer();

    cv::Mat* frame_pointer;
    void start_server();
    void stop_server();
    void feed_frame(cv::Mat* new_frame);
};



#endif // __RTSPSERVER_H__W