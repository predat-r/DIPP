#include <opencv2/opencv.hpp>
#include <iostream>
#include "CommunicationHandler.hpp"
#include <zmq.hpp>
#include <thread>
#include <chrono>

int main()
{
    try
    {
        // Creating connection to receive processed images from resizer
        CommunicationHandler handler(zmq::socket_type::pull, 1);
        handler.establishConnection("tcp://127.0.0.1:5557");

        // Reading image format from the message queue
        std::string imageExtension = handler.recvMsg();

        // Creating connection to send watermarked images to compressor
        CommunicationHandler handler2(zmq::socket_type::push, 1);
        handler2.establishConnection("tcp://*:5558");

        while (true)
        {
            // Collecting 5 different sized images from the resizer
            std::vector<cv::Mat> images;
            for (int i = 0; i < 5; i++)
            {
                cv::Mat img = handler.recvImage(10000000);
                std::cout << std::endl
                          << "Received image " << i + 1 << " of 5" << std::endl;
                if (img.empty())
                {
                    break;
                }
                images.push_back(img);
            }

            // Loading the watermark image with alpha channel
            cv::Mat watermark = cv::imread("../watermark.png", cv::IMREAD_UNCHANGED);
            if (watermark.empty())
            {
                break;
            }

            // Processing each image in the received batch
            for (int i = 0; i < images.size(); i++)
            {
                // Calculating watermark size based on image dimensions
                double maxWatermarkRatio = 0.25;
                cv::Mat resizedWatermark = watermark.clone();
                if (watermark.cols > images[i].cols * maxWatermarkRatio ||
                    watermark.rows > images[i].rows * maxWatermarkRatio)
                {
                    // Scaling watermark to maintain aspect ratio
                    double scale = std::min(
                        (images[i].cols * maxWatermarkRatio) / watermark.cols,
                        (images[i].rows * maxWatermarkRatio) / watermark.rows);
                    cv::resize(watermark, resizedWatermark, cv::Size(), scale, scale);
                }

                // Calculating bottom-right position with padding
                int x = images[i].cols - resizedWatermark.cols - 10;
                int y = images[i].rows - resizedWatermark.rows - 10;
                cv::Rect roi(x, y, resizedWatermark.cols, resizedWatermark.rows);

                // Applying alpha blending for transparent watermark
                if (resizedWatermark.channels() == 4)
                {
                    // Separating color and alpha channels
                    std::vector<cv::Mat> channels;
                    cv::split(resizedWatermark, channels);

                    // Creating color image from BGR channels
                    cv::Mat colorImg;
                    cv::merge(std::vector<cv::Mat>{channels[0], channels[1], channels[2]}, colorImg);
                    cv::Mat alpha = channels[3];

                    // Getting region of interest for watermark placement
                    cv::Mat imgRoi = images[i](roi);

                    // Blending watermark with original image using alpha channel
                    for (int r = 0; r < colorImg.rows; r++)
                    {
                        for (int c = 0; c < colorImg.cols; c++)
                        {
                            float alphaValue = alpha.at<uchar>(r, c) / 255.0f;
                            if (alphaValue > 0)
                            {
                                // Applying alpha blending formula for each channel
                                for (int ch = 0; ch < 3; ch++)
                                {
                                    imgRoi.at<cv::Vec3b>(r, c)[ch] =
                                        alphaValue * colorImg.at<cv::Vec3b>(r, c)[ch] +
                                        (1.0f - alphaValue) * imgRoi.at<cv::Vec3b>(r, c)[ch];
                                }
                            }
                        }
                    }
                }
                else
                {
                    // Copying opaque watermark directly onto image
                    resizedWatermark.copyTo(images[i](roi));
                }
            }

            // Sending processed images to next stage
            std::cout << "Sending watermarked images to compressor" << std::endl;
            handler2.sendMsg(imageExtension);
            for (int i = 0; i < images.size(); i++)
            {
                handler2.sendImage(images[i]);
            }
        }

        // Allowing time for message transmission
        std::this_thread::sleep_for(std::chrono::seconds(1));

        // Cleaning up ZMQ connections
        handler.close();
        handler2.close();
        return 0;
    }
    catch (const std::exception &e)
    {
        // Logging any errors that occurred during execution
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}