#ifndef BLEND_TOOL
#define BLEND_TOOL

#include <memory>
#include <iostream>


#include "opencv2/opencv.hpp"


using namespace cv;

class BlendTool
{

public:
    static
        void blend(Mat& newImg, Mat& background, Mat& alpha, Mat& outImage) { // works with any number of channels per image

            // Convert Mat to float data type
            newImg.convertTo(newImg, CV_32FC3);
            background.convertTo(background, CV_32FC3);

            // Normalize the alpha mask to keep intensity between 0 and 1
            alpha.convertTo(alpha, CV_32FC3, 1.0 / 255); // 

            // Multiply the foreground with the alpha matte
            multiply(alpha, newImg, newImg);

            // Multiply the background with ( 1 - alpha )
            multiply(Scalar::all(1.0) - alpha, background, background);

            // Add the masked foreground and background.
            add(newImg, background, outImage);

            //outImage = outImage / 255;

            outImage.convertTo(outImage, CV_8UC3);

    }

    static
        void blendColor(Mat& foreground, Mat& background, Mat& alpha, Mat& outImage) { // works with any number of channels per image


            std::vector<cv::Mat> array{ alpha, alpha, alpha };
            cv::Mat alpha_multichannel;
            cv::merge(array, alpha_multichannel);

            alpha_multichannel = alpha_multichannel * 255; // fixing range before conversion
            alpha_multichannel.convertTo(alpha_multichannel, CV_8UC3);

            outImage = foreground.mul(alpha_multichannel, 1. / 255) + background.mul(cv::Scalar(255, 255, 255) - alpha_multichannel, 1. / 255);

    }
    
};

#endif // BLEND_TOOL
