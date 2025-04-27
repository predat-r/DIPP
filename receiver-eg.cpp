#include <zmq.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include "CommunicationHandler.hpp"

int main()
{
    try {
        CommunicationHandler handler(zmq::socket_type::pull, 1);
        
        std::cout << "Starting receiver..." << std::endl;
        handler.establishConnection("tcp://127.0.0.1:5555");
        
        std::cout << "Waiting for image..." << std::endl;
        cv::Mat img = handler.recvImage();

        if (img.empty()) {
            std::cerr << "Error decoding image!" << std::endl;
            return -1;
        }

        cv::imshow("Received Image", img);
        cv::waitKey(0);

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}