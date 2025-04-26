#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main() {
    
    Mat img = imread("../image.jpg");
    Mat logo = imread("../apple.png", IMREAD_UNCHANGED); 

    if (img.empty() || logo.empty()) {
        cerr << "Failed to load image or logo." << endl;
        return -1;
    }

    // Resizing logo if too big
    if (logo.cols > img.cols / 4 || logo.rows > img.rows / 4) {
        resize(logo, logo, Size(img.cols / 4, img.rows / 4));
    }

    // Splitting logo into BGR and Alpha
    Mat logo_bgr, logo_alpha;
    vector<Mat> channels;
    split(logo, channels);

    if (channels.size() == 4) {
        merge(vector<Mat>{channels[0], channels[1], channels[2]}, logo_bgr);
        logo_alpha = channels[3];
    } else {
        logo_bgr = logo;
        logo_alpha = Mat(logo.size(), CV_8UC1, Scalar(255)); // Opaque if no alpha
    }

    //position: bottom-right
    int x = img.cols - logo.cols - 10;
    int y = img.rows - logo.rows - 10;

    // ROI (Region of Interest) where logo will be placed
    Mat roi = img(Rect(x, y, logo.cols, logo.rows));

    logo_bgr.copyTo(roi, logo_alpha);

    imwrite("../watermarked.jpg", img);

    cout << "Watermark applied successfully." << endl;
    return 0;
}
