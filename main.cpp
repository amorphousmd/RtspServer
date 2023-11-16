#include "RtspServer.hpp"
#include <boost/thread.hpp>

// Function to generate a black canvas with the current time appended
cv::Mat generate_frame() {
    // Create a black canvas
    cv::Mat frame(288, 384, CV_8UC3, cv::Scalar(0, 0, 0));

    // Get the current time
    auto now = std::chrono::system_clock::now();
    auto time_point = std::chrono::system_clock::to_time_t(now);

    // Convert the time to a string
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_point), "%Y-%m-%d %H:%M:%S");

    // Put the time on the canvas
    cv::putText(frame, ss.str(), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);

    return frame;
}

int main() {
    // Create an instance of RtspServer
    RtspServer* server_handler = new RtspServer();

    // Start the RTSP server in a separate thread
    boost::thread* rtsp_server_thread_handler = new boost::thread(&RtspServer::start_server, server_handler);

    // Main loop to generate frames and feed them into the pipeline
    while (true) {
        // Generate a new frame (replace this with your actual frame generation logic)
        cv::Mat new_frame = generate_frame();

        // Feed the new frame into the pipeline
        server_handler->feed_frame(&new_frame);

        // Sleep for a short duration (adjust as needed)
        usleep(10000);  // Sleep for 10 milliseconds
    }

    // Stop the RTSP server (this won't be reached in this example)
    server_handler->stop_server();

    // Wait for the RTSP server thread to finish
    rtsp_server_thread_handler->join();

    // Cleanup
    delete server_handler;
    delete rtsp_server_thread_handler;

    return 0;
}