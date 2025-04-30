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
    // Setting up connection handler to receive processed and watermarked images
    CommunicationHandler handler(zmq::socket_type::pull, 1);
    handler.establishConnection("tcp://127.0.0.1:5558");

    // Getting the image format from the previous stage
    const string imageExtension = handler.recvMsg();

    while (true)
    {
        // Creating an array to store the 5 different sized images
        cv::Mat img[5];
     
        // Receiving all 5 images from the watermarker
        for (int i = 0; i < 5; i++)
        {
            img[i] = handler.recvImage(10000000);
            std::cout << "recieved an image" << std::endl;
            
            // Breaking if received an empty image
            if (img[i].empty())
            {
                break;
            }
        }

        // Setting up compression parameters based on image format
        vector<int> compression_params;
        if (imageExtension == ".jpg" || imageExtension == ".jpeg")
        {
            // Using JPEG compression with 60% quality for JPG files
            compression_params.push_back(IMWRITE_JPEG_QUALITY);
            compression_params.push_back(60);
        }
        else if (imageExtension == ".png")
        {
            // Using PNG compression level 6 for PNG files
            compression_params.push_back(IMWRITE_PNG_COMPRESSION);
            compression_params.push_back(6);
        }

        // Saving each image with its corresponding size suffix
        for (int i = 0; i < 5; i++)
        {
            // Creating unique filenames for each size variant
            string filename = "../output_" + to_string(i) + imageExtension;
            imwrite(filename, img[i], compression_params);
        }
    }

    // Waiting for any pending operations to complete
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Cleaning up the ZMQ connection
    handler.close();
    return 0;
}