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
        CommunicationHandler handler(zmq::socket_type::pull, 1);

        std::cout << "Starting receiver..." << std::endl;
        handler.establishConnection("tcp://127.0.0.1:5555");

        std::cout << "Waiting for image..." << std::endl;
        cv::Mat img = handler.recvImage();


        cout << "Recieved image"<<endl;

        if (img.empty())
        {
            std::cerr << "Error decoding image!" << std::endl;
            return -1;
        }

        // Splitting channels
        vector<Mat> channels;
        split(img, channels);

        // Histogram Equalization per channel
        for (int i = 0; i < channels.size(); i++)
        {
            equalizeHist(channels[i], channels[i]);
        }

        // Merging back
        Mat equalized;
        merge(channels, equalized);

        // Sharpening Kernel
        Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0,
                      -1, 4.5, -1,
                      0, -1, 0);
        Mat sharpened;
        filter2D(equalized, sharpened, equalized.depth(), kernel);

        imwrite("../output.jpg", sharpened);
        handler.close();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}