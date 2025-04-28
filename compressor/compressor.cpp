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

    cv::Mat img[5];
    for (int i = 0; i < 5; i++)
    {
        img[i] = handler.recvImage();
        if (img[i].empty())
        {
            cout << "Failed to load image " << i << endl;
            return -1;
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

    cout << "Compression done." << endl;

    return 0;
}