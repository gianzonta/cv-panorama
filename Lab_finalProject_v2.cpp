// Lab_FinalProject.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"
#include "panoramic_utils.h"
#include "mask_tool.h"
#include "blend_tool.h"

using namespace cv;

double MASK_PERC = 0.6; 
double SAMPLE_SELECT = 0.4; 
int PATCH_SIZE = 31;

bool TRIM_EDGES = true;


void showImgTest(const cv::Mat& img) {
    cv::namedWindow("Test", 1);
    cv::imshow("Test", img);
    cv::waitKey(0);
}

void customRANSAC(std::vector<Point2f> ptR, std::vector<Point2f> ptL, int& avgDeltaX, int& avgDeltaY) {

    int deltaX = ptR[0].x - ptL[0].x;
    int deltaY = ptR[0].y - ptL[0].y;
    int maxCount = 1;
    avgDeltaX = 0;
    avgDeltaY = 0;

    for (int n = 0; n < 500; n++) {
        int index = rand() % ptR.size();

        int dX = ptR[index].x - ptL[index].x;
        int dY = ptR[index].y - ptL[index].y;
        int inCount = 1;
        // check all the points to see the number of positive cases
        int avgDeltaXn = dX;
        int avgDeltaYn = dY;

        for (int m = 0; m < ptR.size(); m++) {
            int dXn = ptR[m].x - ptL[m].x;
            int dYn = ptR[m].y - ptL[m].y;

            if (abs(dX - dXn) + abs(dY - dYn) < 10) { // required a smaller treshold on Y axis
                inCount++;
                avgDeltaXn += dXn; // summing compatible points in X
                avgDeltaYn += dYn; // summing compatible points in Y
            }
        }

        if (inCount > maxCount) {
            maxCount = inCount;
            deltaX = dX;
            deltaY = dY;
            avgDeltaX = avgDeltaXn;
            avgDeltaY = avgDeltaYn;
        }

    }

    avgDeltaX = cvRound((float)avgDeltaX / (float)maxCount);
    avgDeltaY = cvRound((float)avgDeltaY / (float)maxCount);

    std::cout << "CUSTOM RANSAC " << "delta X: " << deltaX << ", delta Y: " << deltaY << ", with maxCount: " << maxCount - 1 << std::endl;

    return;

}

void trimBlack(Mat& input) {

    int startY = 0;
    int endY = input.rows - 1;

    bool exit = false;
    int y = cvRound(((float)input.rows) / 2.0f);
    while (!exit) {
        int pixelCount = 0;

        for (int x = 0; x < input.cols; x++) {
            Vec3b pixelVal = input.at<cv::Vec3b>(y, x);
            if (pixelVal == Vec3b(0, 0, 0)) { // check for black pixels
                pixelCount++;
            }
        }

        if (pixelCount > 5 || y < 0) {
            startY = y;
            exit = true;
        }

        y--;

    }

    exit = false;
    y = cvRound(((float)input.rows) / 2.0f); // starts from the middle of the image, good choice in normal conditions
    while (!exit) {
        int pixelCount = 0;

        for (int x = 0; x < input.cols; x++) {
            Vec3b pixelVal = input.at<cv::Vec3b>(y, x);
            if (pixelVal == Vec3b(0, 0, 0)) { // check for black pixels
                pixelCount++;
            }
        }

        if (pixelCount > 10 || y >= input.rows - 1) {
            endY = y;
            exit = true;
        }

        y++;

    }

    Rect roi;
    roi.x = 0;
    roi.y = startY;
    roi.width = input.cols;
    roi.height = endY - startY;

    input = input(roi);


}

void paramSetting(std::string seqName) {
    if (seqName == "lab") {
        MASK_PERC = 0.5;
        SAMPLE_SELECT = 0.3;
        PATCH_SIZE = 10;
    }

    if (seqName == "kitchen") {
        MASK_PERC = 0.8;
        SAMPLE_SELECT = 0.3;
        PATCH_SIZE = 31;
    }

    if (seqName == "asiago") {
        MASK_PERC = 0.5;
        SAMPLE_SELECT = 0.4;
        PATCH_SIZE = 31;

    }

    return;
}

int main() //LAB_4 chosen
{
    std::cout << "---PANOBUILDER V2---" << std::endl;
    std::cout << "This tool builds a panorama starting from a sequence of images taken by rotating a tripod head" << std::endl;

    std::vector<cv::String> fn;
    std::string inputFolder;
    bool exit = false;

    // user inputs
    while (!exit) {
        std::cout << "Please provide the name of the image sequence folder" << std::endl;

        std::cin >> inputFolder;

        try {
            cv::glob("./" + inputFolder + "/*.*", fn, false);
            exit = true;
        }
        catch (Exception e) {
            std::cout << "ERROR: Wrong image sequence folder" << std::endl;
        }

    }

    

    std::cout << "Please provide the angle of view of the camera (lab-kitchen: 33, asiago: 41)" << std::endl;
    int ANGLE_OF_VIEW = 0;
    std::cin >> ANGLE_OF_VIEW;

    std::cout << "Do you want to trim the black borders? [y/n] + enter" << std::endl;
    std::string resp;
    std::cin >> resp;
    if (resp == "n") { TRIM_EDGES = false; }
    std::cout << "Now loading chosen image folder..." << std::endl;

    paramSetting(inputFolder);

    // FOV/2
    // 33 lab-kitchen
    // 41 - asiago

    //std::vector<cv::Mat> gImages;
    std::vector<cv::Mat> gpImages;
    std::vector<cv::Mat> cpImages;

    size_t count = fn.size(); //number of png files in images folder
    int status = 0;

    for (size_t i = 0; i < count; i++) {
        status = cvRound((float)i / (float)count * 100);
        std::cout << "In progress: " << status << "% \r";
        cv::Mat gpImg = cv::imread(fn[i]);
        gpImg = PanoramicUtils::cylindricalProj(gpImg, ANGLE_OF_VIEW); // projection + grayscale
        gpImages.push_back(gpImg);

        cv::Mat cpImg = cv::imread(fn[i]);
        cpImg = PanoramicUtils::colorCylindricalProj(cpImg, ANGLE_OF_VIEW); // projection + grayscale
        cpImages.push_back(cpImg);
    }


    int imgCols = gpImages[1].cols;
    int imgRows = gpImages[1].rows;

    std::cout << "Loading completed - The images have size " << imgCols << " x " << imgRows << std::endl;


    // ORB pointer created
    Ptr<ORB> orbDescr;

    Mat outImg = gpImages[0].clone(); // the first image of the sequence is the root of the output
    Mat cOutImg = cpImages[0].clone(); // color equivalent

    std::vector<int> deltaXVec;
    std::vector<int> deltaYVec;
    std::vector<int> newHOfstVec;

    for (size_t i = 1; i < count; i++) {

        Mat newImg = gpImages[i];
        Mat cNewImg = cpImages[i];

        std::cout << "Adding image #" << i << " out of " << count - 1 << std::endl;

        // mask creation for both sides

        Mat maskR, maskL;
        MaskTool::createMaskR(outImg, maskR, MASK_PERC, imgCols); //the mask on the right side has always the same width, indipendent from img size (growing)
        MaskTool::createMaskL(newImg, maskL, MASK_PERC);

        // detection of keypoints and descriptor computation

        std::vector<KeyPoint> keyPL, keyPR;
        Mat descR, descL;

        orbDescr = ORB::create(700,1.2F,8,10,0,2,ORB::HARRIS_SCORE, PATCH_SIZE); // closer to borders - distance was 10

        orbDescr->detectAndCompute(outImg, maskR, keyPR, descR); //compute descriptors for the RIGTH part of the first img (the prev. output)
        orbDescr->detectAndCompute(newImg, maskL, keyPL, descL);  //compute descriptors for the LEFT part of the second img  

        Ptr<BFMatcher> matcher = BFMatcher::create(NORM_HAMMING);
        std::vector<DMatch> matches;
        matcher->match(descR, descL, matches);

        int nMFound = matches.size();

        std::sort(matches.begin(), matches.end()); //order by hamming distance (smaller is better)
        matches.erase(matches.begin() + matches.size() * SAMPLE_SELECT, matches.end()); //takes only the first SAMPLE_SELECT*100% matches, supposed the best ones ofter ordering - 

        std::cout << "Matched considered: " << matches.size() << ", out of " << nMFound << " matches found" << std::endl;

        std::vector<Point2f> ptR; // location of the points in the right part of the first img
        std::vector<Point2f> ptL; // location of the points in the left part of the second img

        for (size_t j = 0; j < matches.size(); j++) {
            ptR.push_back(keyPR[matches[j].queryIdx].pt);
            ptL.push_back(keyPL[matches[j].trainIdx].pt);
        }

        //ESTIMATE TRANSLATION (everage of best matches, stored as points in ptR and ptL)

        int deltaX;
        int deltaY;

        customRANSAC(ptR, ptL, deltaX, deltaY);

        std::cout << "CUSTOM RANSAC done, avg deltaX: " << deltaX << ", avg deltaY: " << deltaY << std::endl;

        //compute new stitched image boundaries

        int newWidth = newImg.cols + deltaX;

        if (newWidth < outImg.cols) {
            newWidth = outImg.cols;
        }

        int newHOfst = 0;
        int newHeight = outImg.rows;
        if (deltaY > 0) {
            newHOfst = deltaY;
            newHeight += newHOfst;
        }
        else {
            newHeight += -deltaY + 1;
        }

        std::cout << "New width: " << newWidth << ", new Height " << newHeight << std::endl;

        Size newSize = Size(newWidth, newHeight);
        Mat newCont = Mat::zeros(newSize, CV_8U);
        newImg.copyTo(newCont(Rect(deltaX, newHOfst - deltaY, newImg.cols, newImg.rows))); // new image placed in the correct position

        // also the previous out image must be fitted and shifted in the new container (to have the same size as src)
        Mat outCont = Mat::zeros(newSize, CV_8U);
        outImg.copyTo(outCont(Rect(0, newHOfst, outImg.cols, outImg.rows)));

        Mat gMask;
        MaskTool::hGradientMask(outCont, gMask, deltaX, outImg.cols); // computing the gradient of the new img superimposition

        // now newCont must be superimposed over outCont using its gradient;

        Mat blendOut = Mat::zeros(outCont.size(), outCont.type());
        BlendTool::blend(newCont, outCont, gMask, blendOut);

        outImg = blendOut.clone(); //basis of next iteration (grayscale)
                                   
        // application to color image

        Mat cNewCont = Mat::zeros(newSize, CV_8UC3);
        cNewImg.copyTo(cNewCont(Rect(deltaX, newHOfst - deltaY, cNewImg.cols, cNewImg.rows))); // new image placed in the correct position

        Mat cOutCont = Mat::zeros(newSize, CV_8UC3);
        cOutImg.copyTo(cOutCont(Rect(0, newHOfst, cOutImg.cols, cOutImg.rows)));

        Mat cBlendOut = Mat::zeros(cOutCont.size(), cOutCont.type());

        BlendTool::blendColor(cNewCont, cOutCont, gMask, cBlendOut);

        cOutImg = cBlendOut.clone();

        //showImgTest(cOutImg);

        std::cout << "------------" << std::endl;

    }

    if (TRIM_EDGES) {
        trimBlack(cOutImg);
    }

    imwrite("out/" + inputFolder + "_out.jpg", cOutImg); //output file shown

    std::cout << "Computation ended - output file now updated \n";

}