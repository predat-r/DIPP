#include <opencv2/opencv.hpp>
#include <iostream>
#include "CommunicationHandler.hpp"
#include <zmq.hpp>
#include <chrono>
#include <thread>

using namespace cv;
using namespace std;

int main()
{
    try
    {
        // Handler for receiving images
        CommunicationHandler handler(zmq::socket_type::pull, 1);
        handler.establishConnection("tcp://127.0.0.1:5556");
        const string imageExtension = handler.recvMsg();
        cv::Mat img = handler.recvImage();

        if (img.empty())
        {
            std::cerr << "Error decoding image!" << std::endl;
            return -1;
        }

        vector<pair<int, int>> sizes = {
            {1920, 1080}, // Full HD
            {1280, 720},  // HD
            {800, 600},   // SVGA
            {640, 480},   // VGA
            {320, 240}    // QVGA
        };

        CommunicationHandler handler2(zmq::socket_type::push, 1);
        handler2.establishConnection("tcp://*:5556");
        handler2.sendMsg(imgExtension);

        for (auto [w, h] : sizes)
        {
            Mat resized;
            resize(img, resized, Size(w, h), 0, 0, INTER_CUBIC);
            handler2.sendImage(resized);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
        handler.close();
        handler2.close();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}