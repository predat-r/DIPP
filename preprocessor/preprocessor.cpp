#include <opencv2/opencv.hpp>
#include <iostream>
using namespace cv;
using namespace std;

int main()
{
    // Loading Image
    Mat img = imread("../image.jpg");
    if (img.empty())
    {
        cout << "Failed to load image" << endl;
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


    return 0;
}