#include <opencv2/opencv.hpp>
#include <iostream>
using namespace cv;
using namespace std;

int main()
{
    Mat img = imread("../image.jpg");
    if (img.empty())
    {
        cout << "Failed to load image" << endl;
        return -1;
    }

    vector<int> compression_params;
    string filename = "../image.jpg";
    
    // Setting image compression Parameters based on image type
    if (filename.find(".jpg") != string::npos || filename.find(".jpeg") != string::npos)
    {
        compression_params.push_back(IMWRITE_JPEG_QUALITY);
        compression_params.push_back(60); // JPEG quality 0-100
    }
    else if (filename.find(".png") != string::npos)
    {
        compression_params.push_back(IMWRITE_PNG_COMPRESSION);
        compression_params.push_back(6); // PNG compression level 0-9
    }

    // Saving compressed image
    imwrite("../output.jpg", img, compression_params);

    cout << "Compression done." << endl;

    return 0;
}