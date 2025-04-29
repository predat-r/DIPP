#include "CommunicationHandler.hpp"
#include <iostream>
#include <chrono>
#include <thread>
CommunicationHandler::CommunicationHandler(zmq::socket_type socket_type, int context_size)
{
    try
    {
        this->socket_type = socket_type;

        // Initialize context
        this->context = zmq::context_t(context_size);

        // Initialize socket using the context and socket_type
        this->socket = zmq::socket_t(this->context, this->socket_type);

        std::cout << "Socket initialized with type: " << static_cast<int>(this->socket_type) << std::endl;
    }
    catch (const zmq::error_t &e)
    {
        std::cerr << "ZMQ Error during constructor: " << e.what() << std::endl;
        throw; // propagate
    }
}

void CommunicationHandler::establishConnection(const std::string &address)
{
    try
    {
        if (this->socket_type == zmq::socket_type::pull)
        {
            std::cout << "Connecting PULL socket to: " << address << std::endl;
            socket.connect(address);
        }
        else if (this->socket_type == zmq::socket_type::push)
        {
            std::cout << "Binding PUSH socket to: " << address << std::endl;
            socket.bind(address);
        }
        else
        {
            throw std::runtime_error("Unsupported socket type");
        }

        // Add small delay after binding/connecting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    catch (const zmq::error_t &e)
    {
        std::cerr << "ZMQ Error: " << e.what() << std::endl;
        throw;
    }
}

void CommunicationHandler::sendImage(const cv::Mat &img)
{
    std::vector<int> compression_params = {cv::IMWRITE_JPEG_QUALITY, 100};
    std::vector<uchar> buffer;
    cv::imencode(".jpg", img, buffer, compression_params);
    zmq::message_t message(buffer.size());
    memcpy(message.data(), buffer.data(), buffer.size());
    socket.send(message, zmq::send_flags::none);
}

cv::Mat CommunicationHandler::recvImage(int timeout_ms)
{
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