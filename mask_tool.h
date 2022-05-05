#ifndef MASK_TOOL
#define MASK_TOOL

#include <memory>
#include <iostream>

#include "opencv2/opencv.hpp"


using namespace cv;

class MaskTool
{

public:
    static
    void createMaskL(const Mat& inImg, Mat& maskL, double MASK_PERC) {
        maskL = Mat::zeros(inImg.rows, inImg.cols, CV_8U); // matrix all zeros, same size of an image of the sequence
        int maskWidth = floor(MASK_PERC * (float)inImg.cols);
        maskL(Rect(0, 0, maskWidth, inImg.rows / 2)) = Scalar(200);
    } // in asiago: cvRound( inImg.rows/2)

    static
    void createMaskR(const Mat& inImg, Mat& maskR, double MASK_PERC,int srcImgCols) {
        int width = floor(MASK_PERC * (float)srcImgCols);
        maskR = Mat::zeros(inImg.rows, inImg.cols, CV_8U); // matrix all zeros, same size of an image of the sequence
        maskR(Rect(inImg.cols - width - 1, 0, width, inImg.rows / 2)) = Scalar(200);

    }
    static
    void hGradientMask(Mat& inImg, Mat& mask, int gStart, int gStop) {
        int diff = gStop - gStart;

        gStart = gStart + cvRound(0.45 * diff);
        gStop = gStop - cvRound(0.45 * diff);
        
        mask = Mat::zeros(inImg.rows, inImg.cols, CV_8UC1);
        mask(Rect(gStop, 0, inImg.cols - gStop, inImg.rows)) = Scalar(255); //full white 

        if (gStop > gStart) {
            double step = 255 / ((double)gStop - (double)gStart);
            for (int l = 0; l < 255; l++) {
                mask.col(gStart + l).setTo(l * step);
            }
        }
        else {
            for (int l = 0; l <= (gStop - gStart); l++) {
                mask.col(gStart + l).setTo(l);
            }
        }

    }

};

#endif // MASK_TOOL