#ifndef COMMUNICATION_HANDLER_HPP
#define COMMUNICATION_HANDLER_HPP

#include <zmq.hpp>
#include <string>
#include <opencv2/opencv.hpp>

class CommunicationHandler
{
public:
    zmq::socket_t socket;
    zmq::context_t context;
    zmq::socket_type socket_type;

    // Constructor initializes context and socket
    CommunicationHandler(zmq::socket_type socket_type, int context_size);

    // Method to establish a connection and bind the socket
    void establishConnection(const std::string &address);

    void sendImage(const cv::Mat &img);

    cv::Mat recvImage();
    void sendMsg(const std::string &msg);
    std::string recvMsg();

    void close();
};

#endif // COMMUNICATION_HANDLER_HPP
