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

    // Define popular sizes (width x height)
    vector<pair<int, int>> sizes = {
        {1920, 1080},   // Full HD
        {1280, 720},    // HD
        {800, 600},     // SVGA
        {640, 480},     // VGA
        {320, 240}      // QVGA
    };

    for (auto [w, h] : sizes)
    {
        Mat resized;
        resize(img, resized, Size(w, h));

        string output_filename = "../output_" + to_string(w) + "x" + to_string(h) + ".jpg";
        imwrite(output_filename, resized);
    }

    cout << "Resizing done." << endl;

    return 0;
}