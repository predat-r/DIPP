#ifndef COMMUNICATION_HANDLER_HPP
#define COMMUNICATION_HANDLER_HPP

#include <zmq.hpp>
#include <string>
#include <opencv2/opencv.hpp>

/**
 * CommunicationHandler manages ZMQ socket communications for image processing pipeline
 * Handling both sending and receiving of images and messages between pipeline stages
 */
class CommunicationHandler
{
public:
    // Socket for ZMQ communication - must outlive dependent objects
    zmq::socket_t socket;
    
    // Context for ZMQ initialization - must be declared before socket
    zmq::context_t context;
    
    //socket type for connection management
    zmq::socket_type socket_type;

    /**
     * Constructor creating ZMQ context and socket
     * @param socket_type Type of ZMQ socket (PUSH/PULL)
     * @param context_size Size of ZMQ context (typically 1)
     */
    CommunicationHandler(zmq::socket_type socket_type, int context_size);

    /**
     * Establishes connection based on socket type
     * PUSH sockets bind to address, PULL sockets connect to address
     * @param address ZMQ address string (e.g., "tcp://*:5555")
     */
    void establishConnection(const std::string &address);

    /**
     * Sends OpenCV Mat image through ZMQ socket
     * Converts image to bytes before transmission
     * @param img OpenCV Mat containing the image to send
     */
    void sendImage(const cv::Mat &img);

    /**
     * Receives image data and converts back to OpenCV Mat
     * @param timeout_ms Maximum time to wait for data in milliseconds
     * @return OpenCV Mat containing received image
     */
    cv::Mat recvImage(int timeout_ms);

    /**
     * Sends string message through ZMQ socket
     * @param msg String message to send
     */
    void sendMsg(const std::string &msg);

    /**
     * Receives string message from ZMQ socket
     * @return Received string message
     */
    std::string recvMsg();

    /**
     * Closes ZMQ socket 
     */
    void close();
};

#endif // COMMUNICATION_HANDLER_HPP
