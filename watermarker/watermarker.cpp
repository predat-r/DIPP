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
        // Connect and receive images
        CommunicationHandler handler(zmq::socket_type::pull, 1);
        handler.establishConnection("tcp://127.0.0.1:5557");
        std::string imageExtension = handler.recvMsg();

        // Receive 5 images of different sizes
        std::vector<cv::Mat> images;
        for (int i = 0; i < 5; i++)
        {
            cv::Mat img = handler.recvImage();
            if (img.empty())
            {
                std::cerr << "Failed to receive image " << i << std::endl;
                return -1;
            }
            images.push_back(img);
            std::cout << "added an image in watermarker";
        }

        // Load watermark
        cv::Mat watermark = cv::imread("../watermark.png", cv::IMREAD_UNCHANGED);
        if (watermark.empty())
        {
            std::cerr << "Failed to load watermark" << std::endl;
            return -1;
        }

        // Process each image
        for (int i = 0; i < images.size(); i++)
        {
            // Resize watermark based on current image size
            double maxWatermarkRatio = 0.25;
            cv::Mat resizedWatermark = watermark.clone();
            if (watermark.cols > images[i].cols * maxWatermarkRatio ||
                watermark.rows > images[i].rows * maxWatermarkRatio)
            {
                double scale = std::min(
                    (images[i].cols * maxWatermarkRatio) / watermark.cols,
                    (images[i].rows * maxWatermarkRatio) / watermark.rows);
                cv::resize(watermark, resizedWatermark, cv::Size(), scale, scale);
            }

            // Position watermark
            int x = images[i].cols - resizedWatermark.cols - 10;
            int y = images[i].rows - resizedWatermark.rows - 10;
            cv::Rect roi(x, y, resizedWatermark.cols, resizedWatermark.rows);

            // Apply watermark
            if (resizedWatermark.channels() == 4)
            {
                std::vector<cv::Mat> channels;
                cv::split(resizedWatermark, channels);

                cv::Mat colorImg;
                cv::merge(std::vector<cv::Mat>{channels[0], channels[1], channels[2]}, colorImg);
                cv::Mat alpha = channels[3];

                cv::Mat imgRoi = images[i](roi);

                for (int r = 0; r < colorImg.rows; r++)
                {
                    for (int c = 0; c < colorImg.cols; c++)
                    {
                        float alphaValue = alpha.at<uchar>(r, c) / 255.0f;
                        if (alphaValue > 0)
                        {
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
                resizedWatermark.copyTo(images[i](roi));
            }


                }
        CommunicationHandler handler2(zmq::socket_type::push, 1);
        handler2.establishConnection("tcp://*:5558");
        handler2.sendMsg(imageExtension);
        for (int i = 0; i < 5; i++)
        {
            handler2.sendImage(images[i]);
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