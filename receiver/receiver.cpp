#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv; 

int main()
{
    Mat image = imread("../image.jpg");         
    
    if (image.empty()) {
        cout << "Failed to load image!";
        return -1;
    }      
    cout <<"image loaded";
}
