#include <opencv2/opencv.hpp>
#include <iostream>
#include <amqpcpp.h>
#include <unistd.h>
#include <ctime>
#include <sstream>
#include <fstream>

using namespace std;
using namespace cv;
using namespace AMQP;

int main()
{
    try
    {

        Mat image = imread("../image.jpg");
        if (image.empty())
        {
            cerr << "Failed to load image!" << endl;
            return -1;
        }
        cout << "Image loaded" << endl;
        return 0;
    }
    catch (Exception e)
    {
        cout << "Error" << e.what() << endl;
    }
}