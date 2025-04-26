#pragma once

// Include AMQP++ library headers for handling AMQP protocol (used for RabbitMQ messaging)
#include <amqpcpp.h>

// Standard library headers for I/O and socket communication
#include <iostream>
#include <sys/socket.h> // For handling socket-based communication
#include <amqpcpp/linux_tcp/tcpchannel.h>
#include <opencv2/core/mat.hpp>

using namespace std;
using namespace AMQP;
using namespace cv;

// RabbitMqConnectionHandler class definition
// This class is responsible for handling the connection lifecycle between an AMQP connection (RabbitMQ)
// and a raw socket (for network communication). It is designed to interface with the AMQP++ library's
// connection events such as data transmission, connection readiness, error handling, and connection closure.
class RabbitMqConnectionHandler : public AMQP::ConnectionHandler
{
public:
    int _fd; // Socket file descriptor, used to send and receive data over the network

    // Constructor that initializes the connection handler with a provided socket file descriptor (fd)
    // fd is the network socket used to send data to RabbitMQ
    RabbitMqConnectionHandler(int fd);

    // Method to handle incoming data from the AMQP connection
    // It is called when there is data to be sent to the RabbitMQ server over the raw socket
    // Parameters:
    // - connection: The active AMQP connection object
    // - data: Pointer to the data to be sent
    // - size: Size of the data in bytes
    void onData(AMQP::Connection *connection, const char *data, size_t size) override;

    // Method called when the AMQP connection is ready for communication
    // This is triggered once the connection to RabbitMQ has been successfully established
    // Here, you can declare queues and publish messages to RabbitMQ
    // Parameters:
    // - connection: The active AMQP connection object
    void onReady(AMQP::Connection *connection) override;

    // Method called in case of an error during the AMQP connection or messaging process
    // This is useful for logging or handling protocol errors that occur during interaction with RabbitMQ
    // Parameters:
    // - connection: The active AMQP connection object
    // - message: The error message received from the connection
    void onError(AMQP::Connection *connection, const char *message) override;

    // Method called when the connection to RabbitMQ is closed
    // This could be triggered by either the server or the client closing the connection
    // It is useful for cleanup tasks and notifying the system about the closure
    // Parameters:
    // - connection: The active AMQP connection object
    void onClosed(AMQP::Connection *connection) override;

    void sendImage(Channel &channel, const string &queueName, const Mat &image, string format);

    static cv::Mat receiveImage(const std::string &messageData, size_t messageSize);

    std::pair<int, AMQP::Connection *> establishConnection(
        const string &,
        uint16_t,
        const string &,
        const string &,
        const string &);
};