#include <zmq.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int main() {
    // Create ZeroMQ context and socket
    zmq::context_t context(1);
    zmq::socket_t socket(context, zmq::socket_type::pull);
    socket.connect("tcp://localhost:5555");  // Connect to sender

    // Receive the image data
    zmq::message_t message;
    socket.recv(message, zmq::recv_flags::none);

    // Convert received data to byte vector
    std::vector<uchar> buffer(static_cast<uchar*>(message.data()), static_cast<uchar*>(message.data()) + message.size());

    // Decode the byte array to an OpenCV image
    cv::Mat img = cv::imdecode(buffer, cv::IMREAD_COLOR);
    if (img.empty()) {
        std::cerr << "Error decoding image!" << std::endl;
        return -1;
    }

    // Show the received image
    cv::imshow("Received Image", img);
    cv::waitKey(0);

    return 0;
}