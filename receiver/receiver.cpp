#include <zmq.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include "CommunicationHandler.hpp"
#include <chrono>
#include <thread>
#include <filesystem>

int main()
{
    try
    {
        // Creating a handler for sending images, which is acting as the start point of the pipeline
        CommunicationHandler handler(zmq::socket_type::push, 1);
        
        // Binding to all interfaces on port 5555 to allow incoming connections
        handler.establishConnection("tcp://*:5555");

        // Reading the image path and extracting its extension for format handling
        std::filesystem::path imagePath = "../image.jpg";
        cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
        std::string imgExtension = imagePath.extension().string();
        
        // Sending the image extension first so receivers know the format
        handler.sendMsg(imgExtension);

        // Checking if image loading was successful
        if (image.empty())
        {
            std::cerr << "Error loading image!" << std::endl;
            return -1;
        }

        // Sending the loaded image through the ZMQ socket
        handler.sendImage(image);

        // Waiting briefly to ensure message transmission is complete
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Cleaning up the ZMQ connection
        handler.close();
        return 0;
    }
    catch (const std::exception &e)
    {
        // Handling any exceptions that occurred during execution
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}