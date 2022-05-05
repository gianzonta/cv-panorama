#ifndef LAB4__PANORAMIC__UTILS__H
#define LAB4__PANORAMIC__UTILS__H

#include <memory>
#include <iostream>

//#include <opencv2/opencv.hpp>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
//#include <opencv2/xfeatures2d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
//#include <opencv2/stitching.hpp>

class PanoramicUtils
{
public:
  static
  cv::Mat cylindricalProj(
      const cv::Mat& image,
      const double angle)
  {
    cv::Mat tmp,result;
    cv::cvtColor(image, tmp, cv::COLOR_BGR2GRAY);
    result = tmp.clone();


    double alpha(angle / 180 * CV_PI);
    double d((image.cols / 2.0) / tan(alpha));
    double r(d/cos(alpha));
    double d_by_r(d / r);
    int half_height_image(image.rows / 2);
    int half_width_image(image.cols / 2);

    for (int x = - half_width_image + 1,
         x_end = half_width_image; x < x_end; ++x)
    {
      for(int y = - half_height_image + 1,
          y_end = half_height_image; y < y_end; ++y)
      {
        double x1(d * tan(x / r));
        double y1(y * d_by_r / cos(x / r));

        if (x1 < half_width_image &&
            x1 > - half_width_image + 1 &&
            y1 < half_height_image &&
            y1 > -half_height_image + 1)
        {
          result.at<uchar>(y + half_height_image, x + half_width_image)
              = tmp.at<uchar>(round(y1 + half_height_image),
                              round(x1 + half_width_image));
        }
      }
    }

    return result;
  }

  static
      cv::Mat colorCylindricalProj(
          const cv::Mat& image,
          const double angle)
  {
      cv::Mat tmp[3];
      cv::Mat resultBGR[3];
      
      //cv::cvtColor(image, tmp, cv::COLOR_BGR2GRAY);
      //result = tmp.clone();

      split(image, tmp);
      resultBGR[0] = tmp[0].clone();
      resultBGR[1] = tmp[1].clone();
      resultBGR[2] = tmp[2].clone();


      // BLUE

      double alpha(angle / 180 * CV_PI);
      double d((image.cols / 2.0) / tan(alpha));
      double r(d / cos(alpha));
      double d_by_r(d / r);
      int half_height_image(image.rows / 2);
      int half_width_image(image.cols / 2);

      for (int x = -half_width_image + 1,
          x_end = half_width_image; x < x_end; ++x)
      {
          for (int y = -half_height_image + 1,
              y_end = half_height_image; y < y_end; ++y)
          {
              double x1(d * tan(x / r));
              double y1(y * d_by_r / cos(x / r));

              if (x1 < half_width_image &&
                  x1 > -half_width_image + 1 &&
                  y1 < half_height_image &&
                  y1 > -half_height_image + 1)
              {
                  resultBGR[0].at<uchar>(y + half_height_image, x + half_width_image)
                      = tmp[0].at<uchar>(round(y1 + half_height_image),
                          round(x1 + half_width_image));

                  resultBGR[1].at<uchar>(y + half_height_image, x + half_width_image)
                      = tmp[1].at<uchar>(round(y1 + half_height_image),
                          round(x1 + half_width_image));

                  resultBGR[2].at<uchar>(y + half_height_image, x + half_width_image)
                      = tmp[2].at<uchar>(round(y1 + half_height_image),
                          round(x1 + half_width_image));


              }
          }
      }
      //cv::Mat result;

      std::vector<cv::Mat> array_to_merge;

      array_to_merge.push_back(resultBGR[0]);
      array_to_merge.push_back(resultBGR[1]);
      array_to_merge.push_back(resultBGR[2]);

      cv::Mat result;

      cv::merge(array_to_merge, result);

      return result;
  }

};

#endif // LAB4__PANORAMIC__UTILS__H
