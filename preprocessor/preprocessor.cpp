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
        // Setting up the input connection to receive images from the previous stage
        CommunicationHandler handler(zmq::socket_type::pull, 1);
        handler.establishConnection("tcp://127.0.0.1:5555");
        
        // Setting up the output connection to send processed images to the next stage
        CommunicationHandler handler2(zmq::socket_type::push, 1);
        handler2.establishConnection("tcp://*:5556");

        while (true)
        {
            // Receiving the image format information
            const string imgExtension = handler.recvMsg();
            
            // Receiving and checking the image data
            cv::Mat img = handler.recvImage(10000000);
            cout<<"received an image"<<endl;

            if (img.empty()) break;

            // Converting image to floating point for more precise calculations
            Mat processedFloat;
            img.convertTo(processedFloat, CV_32F, 1.0 / 255.0);

            // Converting to Lab color space to separate luminance from chrominance
            Mat labImage;
            cvtColor(processedFloat, labImage, COLOR_BGR2Lab);

            // Extracting individual channels for selective processing
            vector<Mat> labChannels;
            split(labImage, labChannels);

            // Processing only the luminance channel for enhanced contrast
            Mat &lChannel = labChannels[0];
            
            // Normalizing the luminance channel to full range
            double minVal, maxVal;
            minMaxLoc(lChannel, &minVal, &maxVal);
            lChannel = (lChannel - minVal) * (1.0 / (maxVal - minVal)) * 100.0;

            // Applying unsharp masking for edge enhancement
            Mat blurred;
            GaussianBlur(lChannel, blurred, Size(0, 0), 2.0);
            lChannel = lChannel + 0.5 * (lChannel - blurred);

            // Recombining the channels after processing
            Mat enhancedLab;
            merge(labChannels, enhancedLab);

            // Converting back to BGR color space
            Mat enhanced;
            cvtColor(enhancedLab, enhanced, COLOR_Lab2BGR);

            // Converting back to 8-bit depth
            Mat processed;
            enhanced.convertTo(processed, img.type(), 255.0);

            // Ensuring pixel values are within valid range
            cv::normalize(processed, processed, 0, 255, NORM_MINMAX);

            // Sending the processed image to the next stage
            handler2.sendMsg(imgExtension);
            handler2.sendImage(processed);
        }

        // Cleanup and resource release
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