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

        // Handler for sending images ( start point of the pipe)
        CommunicationHandler handler(zmq::socket_type::push, 1);
        handler.establishConnection("tcp://*:5555");

        std::filesystem::path imagePath = "../image.jpg";
        cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);
        std::string imgExtension = imagePath.extension().string();
        handler.sendMsg(imgExtension);

        if (image.empty())
        {
            std::cerr << "Error loading image!" << std::endl;
            return -1;
        }

        handler.sendImage(image);

        std::this_thread::sleep_for(std::chrono::seconds(1));
        handler.close();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}