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
        handler.establishConnection("tcp://127.0.0.1:5555");

        const string imgExtension = handler.recvMsg();
        cv::Mat img = handler.recvImage();

        if (img.empty())
        {
            std::cerr << "Error decoding image!" << std::endl;
            return -1;
        }

        // Convert to 32-bit float for processing to preserve precision
        Mat processedFloat;
        img.convertTo(processedFloat, CV_32F, 1.0 / 255.0);

        // Working in Lab color space to separate luminance from color
        Mat labImage;
        cvtColor(processedFloat, labImage, COLOR_BGR2Lab);

        // Splitting channels
        vector<Mat> labChannels;
        split(labImage, labChannels);

        // Enhancing only the L channel
        Mat &lChannel = labChannels[0];

        // Subtle contrast adjustment on L channel (luminance only)
        double minVal, maxVal;
        minMaxLoc(lChannel, &minVal, &maxVal);
        lChannel = (lChannel - minVal) * (1.0 / (maxVal - minVal)) * 100.0;

        // Subtle unsharp mask for the L channel
        Mat blurred;
        GaussianBlur(lChannel, blurred, Size(0, 0), 2.0);
        lChannel = lChannel + 0.5 * (lChannel - blurred);

        // Merging channels back
        Mat enhancedLab;
        merge(labChannels, enhancedLab);

        // Converting back to BGR
        Mat enhanced;
        cvtColor(enhancedLab, enhanced, COLOR_Lab2BGR);

        // Converting back to original bit depth
        Mat processed;
        enhanced.convertTo(processed, img.type(), 255.0);

        // Quality check: ensuring values are in valid range
        cv::normalize(processed, processed, 0, 255, NORM_MINMAX);

        // Handler for sending images to next step in pipe
        CommunicationHandler handler2(zmq::socket_type::push, 1);
        handler2.establishConnection("tcp://*:5556");
        handler2.sendMsg(imgExtension);
        handler2.sendImage(processed);

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