#include "CommunicationHandler.hpp"
#include <iostream>
#include <chrono>
#include <thread>

// Constructor: Sets up ZMQ context and socket
CommunicationHandler::CommunicationHandler(zmq::socket_type socket_type, int context_size)
{
    try
    {
        // Stored socket type for connection management
        this->socket_type = socket_type;

        // Created new ZMQ context
        this->context = zmq::context_t(context_size);

        // Created socket from context and type
        this->socket = zmq::socket_t(this->context, this->socket_type);

        std::cout << "Socket initialized with type: " << static_cast<int>(this->socket_type) << std::endl;
    }
    catch (const zmq::error_t &e)
    {
        // Logged error and propagated exception
        std::cerr << "ZMQ Error during constructor: " << e.what() << std::endl;
        throw;
    }
}

// Sets up connection based on socket type
void CommunicationHandler::establishConnection(const std::string &address)
{
    try
    {
        if (this->socket_type == zmq::socket_type::pull)
        {
            // Connected PULL socket to specified address
            std::cout << "Connects PULL socket to: " << address << std::endl;
            socket.connect(address);
        }
        else if (this->socket_type == zmq::socket_type::push)
        {
            // Bound PUSH socket to specified address
            std::cout << "Binds PUSH socket to: " << address << std::endl;
            socket.bind(address);
        }
        else
        {
            throw std::runtime_error("Unsupported socket type");
        }

        // Added delay to ensure connection setup
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    catch (const zmq::error_t &e)
    {
        // Logged error and propagated exception
        std::cerr << "ZMQ Error: " << e.what() << std::endl;
        throw;
    }
}

// Encodes and sends image through ZMQ socket
void CommunicationHandler::sendImage(const cv::Mat &img)
{
    // Set JPEG quality parameters
    std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, 100};
    std::vector<uchar> buffer;

    // Encoded image to JPEG format
    cv::imencode(".jpg", img, buffer, compression_params);

    // Created and sent ZMQ message
    zmq::message_t message(buffer.size());
    memcpy(message.data(), buffer.data(), buffer.size());
    socket.send(message, zmq::send_flags::none);
}

// Receives and decodes image from ZMQ socket
cv::Mat CommunicationHandler::recvImage(int timeout_ms)
{
    // Set receive timeout
    socket.set(zmq::sockopt::rcvtimeo, timeout_ms);

    zmq::message_t message;
    try
    {
        auto result = socket.recv(message, zmq::recv_flags::none);
        if (!result.has_value())
        {
            std::cerr << "recvImage timed out after " << timeout_ms << "ms\n";
            return cv::Mat();
        }
    }
    catch (const zmq::error_t &e)
    {
        std::cerr << "ZMQ Error in recvImage: " << e.what() << std::endl;
        return cv::Mat();
    }

    std::vector<uchar> buffer(
        static_cast<uchar *>(message.data()),
        static_cast<uchar *>(message.data()) + message.size());
    return cv::imdecode(buffer, cv::IMREAD_COLOR);
}
void CommunicationHandler::sendMsg(const std::string &msg)
{
    zmq::message_t message(msg.size());
    memcpy(message.data(), msg.data(), msg.size());
    socket.send(message, zmq::send_flags::none);
}
std::string CommunicationHandler::recvMsg()
{
    zmq::message_t message;
    socket.recv(message, zmq::recv_flags::none);
    std::string msg(static_cast<char *>(message.data()), message.size());
    return msg;
}
void CommunicationHandler::close()
{
    this->socket.close();
}