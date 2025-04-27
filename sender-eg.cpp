#include <zmq.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>

int main() {
    // Load image using OpenCV
    cv::Mat image = cv::imread("image.jpg", cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Error loading image!" << std::endl;
        return -1;
    }

    // Convert image to raw byte array
    std::vector<uchar> buffer;
    cv::imencode(".jpg", image, buffer);

    // Create ZeroMQ context and socket
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::push);
    socket.bind("tcp://localhost:5555");  // Bind to port 5555

    // Send the image data
    zmq::message_t message(buffer.size());
    memcpy(message.data(), buffer.data(), buffer.size());
    socket.send(message, zmq::send_flags::none);

    std::cout << "Image sent!" << std::endl;

    return 0;
}