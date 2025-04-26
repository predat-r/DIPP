#include "RabbitMqConnectionHandler.h"
#include <unistd.h>       // For close(), socket functions
#include <cstring>        // For memset, memcpy
#include <opencv2/opencv.hpp>
#include <sys/socket.h>   // For socket(), connect()
#include <netinet/in.h>   // For sockaddr_in
#include <netdb.h>        // For gethostbyname()
#include <stdexcept>      // For std::runtime_error
#include <iostream>       // For std::cout, std::cerr


using namespace std;
using namespace AMQP;
using namespace cv;

// Constructor for the RabbitMqConnectionHandler class
// This constructor initializes the handler with a provided socket file descriptor (fd)
// The socket file descriptor is used for communication over the network.
RabbitMqConnectionHandler::RabbitMqConnectionHandler(int fd) : _fd(fd) {}

// This method is called whenever the AMQP library has data that needs to be sent to RabbitMQ
// It takes a pointer to an AMQP connection, a pointer to the data, and the size of the data
// The method sends this data over the network socket (_fd) to RabbitMQ
void RabbitMqConnectionHandler::onData(AMQP::Connection *connection, const char *data, size_t size) {
    // Try to send the data over the socket using the send() system call
    // The send function returns the number of bytes sent or a negative value in case of an error
    ssize_t sent = ::send(_fd, data, size, 0);  // _fd represents the socket for raw communication

    // If the send function returns a negative value, it means there was an error
    // The error message is then printed to the standard error stream
    if (sent < 0) {
        std::cerr << "Send error\n";
    }
}

// This method is called when the connection to RabbitMQ is ready
// It is triggered once the AMQP library has successfully connected to the RabbitMQ broker
// At this point, you can start declaring queues, publishing messages, etc.
void RabbitMqConnectionHandler::onReady(AMQP::Connection *connection) {
    // Print a message to indicate that the connection to RabbitMQ is ready for operations
    std::cout << "Connection ready!" << std::endl;

    // Create a new AMQP channel using the provided connection
    // Channels are used for interacting with RabbitMQ and performing operations like declaring queues or publishing messages
    AMQP::Channel channel(connection);

    // Declare a queue named "images" on RabbitMQ. This queue will store messages until they are consumed
    // Declaring the queue ensures it exists before publishing any messages to it
    channel.declareQueue("images");

    // Publish a simple message to the "images" queue. In this case, it's a string message.
    // You can replace the string with more complex data, such as image data or binary information
    channel.publish("", "images", "Hello world from raw socket!");

    // Print a message to indicate that the message has been successfully sent to the queue
    std::cout << "Sent message." << std::endl;
}

// This method is called when an error occurs during the AMQP connection process or any operation
// For example, it might be triggered if the connection is lost or if there is a protocol issue
// The error message is passed as a parameter to this method
void RabbitMqConnectionHandler::onError(AMQP::Connection *connection, const char *message) {
    // Output the error message received from the AMQP connection to the standard error stream
    // This helps diagnose what went wrong during the connection or messaging process
    std::cerr << "Error: " << message << std::endl;
}

// This method is called when the connection to RabbitMQ is closed
// It could be triggered by the server closing the connection, or if the client requests a connection close
// This method is useful for cleanup tasks or for notifying the user about the closure
void RabbitMqConnectionHandler::onClosed(AMQP::Connection *connection) {
    // Print a message indicating that the connection to RabbitMQ has been closed
    // This could be the result of an error or normal operation (e.g., graceful shutdown)
    std::cout << "Connection closed." << std::endl;
}

// This method sends an OpenCV image through RabbitMQ using the specified channel and queue
// It takes an AMQP channel, queue name, OpenCV image matrix, and format string (e.g., "jpg", "png")
// The method converts the image to a byte buffer and publishes it to the specified queue
void RabbitMqConnectionHandler::sendImage(Channel &channel, const string &queueName, const Mat &image, string format)
{
    // Convert the OpenCV Mat to a byte vector using the specified format
    vector<uchar> buffer;
    imencode("." + format, image, buffer); // Encode the image to the specified format (e.g., .jpg, .png)
    
    // Convert uchar* to const char* for compatibility with AMQP::Channel::publish
    const char* charData = reinterpret_cast<const char*>(buffer.data());
    
    // Publishing the binary data to the specified queue
    // The empty string "" represents the exchange (default exchange is used)
    channel.publish("", queueName, charData, buffer.size());
    
    // Print information about the sent image for debugging purposes
    std::cout << "Image sent: " << buffer.size() << " bytes, "
              << image.cols << "x" << image.rows << std::endl;
}

// This method receives and decodes image data from a RabbitMQ message
// It takes a string containing the message data and its size
// The method returns an OpenCV Mat object containing the decoded image
Mat RabbitMqConnectionHandler::receiveImage(const string& messageData, size_t messageSize)
{
    // Convert the received message data to a vector of bytes
    // This prepares the data for decoding by OpenCV
    vector<uchar> buffer(messageData.begin(), messageData.end());
    
    // Decode the buffer back to an OpenCV Mat using OpenCV's imdecode function
    // IMREAD_COLOR specifies that the image should be loaded in color if possible
    Mat image = imdecode(buffer, IMREAD_COLOR);
    
    // Check if the image was successfully decoded
    if(image.empty()) {
        throw runtime_error("Failed to decode received image data");
    }
    
    // Print information about the received image for debugging purposes
    std::cout << "Image received: " << messageSize << " bytes, "
              << image.cols << "x" << image.rows << std::endl;
    
    // Return the decoded image
    return image;
}

// This method establishes a connection to a RabbitMQ server
// It handles the entire connection setup process including socket creation and AMQP protocol initialization
// Parameters:
//   host: The hostname or IP address of the RabbitMQ server
//   port: The port number the RabbitMQ server is listening on
//   username: RabbitMQ server login username
//   password: RabbitMQ server login password
//   vhost: Virtual host to connect to (default is "/")
// Returns a pair containing the socket file descriptor and the created AMQP connection object
std::pair<int, AMQP::Connection*> RabbitMqConnectionHandler::establishConnection(
    const std::string& host, 
    uint16_t port, 
    const std::string& username, 
    const std::string& password,
    const std::string& vhost)
{
    // Create a socket for TCP/IP communication
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create socket");
    }
    
    // Set up the server address structure
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    // Convert hostname to IP address
    struct hostent *server = gethostbyname(host.c_str());
    if (server == nullptr) {
        close(sockfd);
        throw std::runtime_error("Failed to resolve host: " + host);
    }
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    
    // Connect to the RabbitMQ server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        throw std::runtime_error("Failed to connect to RabbitMQ server");
    }
    
    // Create a new connection handler with the socket file descriptor
    RabbitMqConnectionHandler *handler = new RabbitMqConnectionHandler(sockfd);
    
    // Create the AMQP connection object with login credentials
    AMQP::Login login(username, password);
    AMQP::Connection *connection = new AMQP::Connection(handler, login, vhost);
    
    std::cout << "Attempting connection to RabbitMQ server at " << host << ":" << port << std::endl;
    
    // Return the socket file descriptor and connection object
    return std::make_pair(sockfd, connection);
}