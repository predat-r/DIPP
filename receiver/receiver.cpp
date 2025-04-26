#include "RabbitMqConnectionHandler.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <amqpcpp.h>

using namespace std;
using namespace cv; 
using namespace  AMQP;

int main()
{  
    RabbitMqConnectionHandler Handler(1);
    Mat image = imread("../image.jpg");         
    
    if (image.empty()) {
        cout << "Failed to load image!";
        return -1;
    }      
    cout <<"image loaded";
}
