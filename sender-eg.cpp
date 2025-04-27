#include <zmq.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include "CommunicationHandler.hpp"
#include <chrono>
#include <thread>

int main()
{
    try
    {
        CommunicationHandler handler(zmq::socket_type::push, 1);

        handler.establishConnection("tcp://*:5555");

        cv::Mat image = cv::imread("image.jpg", cv::IMREAD_COLOR);
        if (image.empty())
        {
            std::cerr << "Error loading image!" << std::endl;
            return -1;
        }

        std::cout << "Sending image..." << std::endl;
        handler.sendImage(image);
        std::cout << "Image sent!" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}