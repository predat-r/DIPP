#include <RabbitMqConnectionHandler.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <amqpcpp.h>
#include <unistd.h> // For close() function

using namespace std;
using namespace cv;
using namespace AMQP;

int main()
{
    try
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0); 
        if (sockfd < 0)
        {
            std::cerr << "Failed to create socket" << std::endl;
            return -1;
        }

        RabbitMqConnectionHandler handlerInstance(sockfd);

        AMQP::Connection* connection = handlerInstance.establishConnection("localhost", 5672, "guest", "guest", "/", sockfd);

        Channel channel(connection);

        channel.declareQueue("images");

        Mat image = imread("../image.jpg");
        if (image.empty())
        {
            cerr << "Failed to load image!" << endl;
            delete connection;
            close(sockfd);
            return -1;
        }
        cout << "Image loaded" << endl;

        // Create a handler instance to access non-static methods
        RabbitMqConnectionHandler handler(sockfd);

        // Send the image
        handler.sendImage(channel, "images", image, "jpg");
        cout << "Image sent" << endl;

        // Wait a moment to ensure message is sent before closing
        sleep(1);

        // Clean up
        delete connection;
        close(sockfd);

        return 0;
    }
    catch (const exception &e)
    {
        cerr << "Error: " << e.what() << endl;
        return -1;
    }
}