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
        // Setting up the input connection to receive processed images from the preprocessor
        CommunicationHandler handler(zmq::socket_type::pull, 1);
        handler.establishConnection("tcp://127.0.0.1:5556");

        // Setting up the output connection to send resized images to the watermarker
        CommunicationHandler handler2(zmq::socket_type::push, 1);
        handler2.establishConnection("tcp://*:5557");

        while (true)
        {
            // Receiving the image format information from the previous stage
            const string imageExtension = handler.recvMsg();
            
            // Receiving and verifying the image data
            cv::Mat img = handler.recvImage(10000000);
            cout << "recveived an image" << endl;

            // Breaking the loop if receiving empty image
            if (img.empty())
            {
                break;
            }

            // Defining standard resolution sizes for output images
            vector<pair<int, int>> sizes = {
                {1920, 1080}, // Full HD resolution
                {1280, 720},  // HD resolution
                {800, 600},   // SVGA resolution
                {640, 480},   // VGA resolution
                {320, 240}    // QVGA resolution
            };

            // Sending the image extension first so receivers know the format
            handler2.sendMsg(imageExtension);

            // Creating and sending different sized versions of the image
            for (auto [w, h] : sizes)
            {
                cout<<std::endl<<"sending resized image"<<std::endl;
                
                // Creating a new matrix for the resized image
                Mat resized;
                
                // Resizing the image using cubic interpolation for better quality
                resize(img, resized, Size(w, h), 0, 0, INTER_CUBIC);
                
                // Sending the resized image to the next stage
                handler2.sendImage(resized);
            }
        }

        // Waiting briefly to ensure all messages are sent
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Cleaning up ZMQ connections
        handler.close();
        handler2.close();
        return 0;
    }
    catch (const std::exception &e)
    {
        // Handling any exceptions that occurred during execution
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}