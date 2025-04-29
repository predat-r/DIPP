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
    // Handler for receiving images
    CommunicationHandler handler(zmq::socket_type::pull, 1);
    handler.establishConnection("tcp://127.0.0.1:5558");
    const string imageExtension = handler.recvMsg();
    while (true)
    {
        cv::Mat img[5];
     
        for (int i = 0; i < 5; i++)
        {
            img[i] = handler.recvImage(10000000);
            std::cout << "recieved an image" << std::endl;
            if (img[i].empty())
            {
                break;
            }
        }

        vector<int> compression_params;
        if (imageExtension == ".jpg" || imageExtension == ".jpeg")
        {
            compression_params.push_back(IMWRITE_JPEG_QUALITY);
            compression_params.push_back(60);
        }
        else if (imageExtension == ".png")
        {
            compression_params.push_back(IMWRITE_PNG_COMPRESSION);
            compression_params.push_back(6);
        }

        // Save all images
        for (int i = 0; i < 5; i++)
        {
            string filename = "../output_" + to_string(i) + imageExtension;
            imwrite(filename, img[i], compression_params);
        }
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    handler.close();
    return 0;
}