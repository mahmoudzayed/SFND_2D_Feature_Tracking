/* INCLUDES FOR THIS PROJECT */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <limits>
#include <queue>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include "dataStructures.h"
#include "matching2D.hpp"

/* MAIN PROGRAM */
int main(int argc, const char *argv[])
{

  /* INIT VARIABLES AND DATA STRUCTURES */

  // data location
  std::string dataPath = "../";

  // camera
  std::string imgBasePath = dataPath + "images/";
  std::string imgPrefix = "KITTI/2011_09_26/image_00/data/000000"; // left camera, color
  std::string imgFileType = ".png";
  int imgStartIndex = 0; // first file index to load (assumes Lidar and camera names have identical naming convention)
  int imgEndIndex = 9;   // last file index to load
  int imgFillWidth = 4;  // no. of digits which make up the file index (e.g. img-0001.png)

  // misc
  int dataBufferSize = 2;       // no. of images which are held in memory (ring buffer) at the same time


  std::vector<std::string> detectorTypelist{"SHITOMASI", "HARRIS", "FAST", "BRISK", "ORB", "AKAZE", "SIFT"};
  std::vector<std::string> descriptorTypelist{"BRISK", "BRIEF", "ORB", "FREAK", "AKAZE", "SIFT"};

  // configuration for different combination of detector and descriptors
  std::string detectorType = "AKAZE";        // SHITOMASI, HARRIS, FAST, BRISK, ORB, AKAZE, and SIFT
  std::string descriptorType = "AKAZE";      // BRISK, BRIEF, ORB, FREAK, AKAZE, SIFT
  std::string matcherType = "MAT_BF";     // MAT_BF, MAT_FLANN
  std::string matcherdescriptorType = "DES_BINARY"; // DES_BINARY, DES_HOG
  std::string selectorType = "SEL_KNN";       // SEL_NN, SEL_KNN
  bool bFocusOnVehicle = true; // only keep keypoints on the preceding vehicle

  // to allow to plot the keypoints on output stream
  bool bVis_detector = false; 
  bool bVis_matcher = true;
  bool bVis_onVehicle = false;
  std::vector<DataFrame> dataBuffer; // list of data frames which are held in memory at the same time

  /* MAIN LOOP OVER ALL IMAGES */
  for (size_t imgIndex = 0; imgIndex <= imgEndIndex - imgStartIndex; imgIndex++)
  {
    /* LOAD IMAGE INTO BUFFER */

    // assemble filenames for current index
    std::ostringstream imgNumber;
    imgNumber << std::setfill('0') << std::setw(imgFillWidth) << imgStartIndex + imgIndex;
    std::string imgFullFilename = imgBasePath + imgPrefix + imgNumber.str() + imgFileType;

    // load image from file and convert to grayscale
    cv::Mat img, imgGray;
    img = cv::imread(imgFullFilename);
    cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

    //// STUDENT ASSIGNMENT
    // TASK MP.1 -> replace the following code with ring buffer of size dataBufferSize

    // push image into data frame buffer
    DataFrame frame;
    frame.cameraImg = imgGray;
    
    // if the buffer is full we remove the first element
    if(dataBuffer.size() >= dataBufferSize)
    {
      dataBuffer.erase(dataBuffer.begin());
      dataBuffer.push_back(frame);
    }
    else
    {
      dataBuffer.push_back(frame);
    }

    //// EOF STUDENT ASSIGNMENT
    std::cout << "#1 : LOAD IMAGE INTO BUFFER done" << std::endl;

    /* DETECT IMAGE KEYPOINTS */

    // extract 2D keypoints from current image
    std::vector<cv::KeyPoint> keypoints; // create empty feature list for current image

    /// STUDENT ASSIGNMENT
    //// TASK MP.2 -> add the following keypoint detectors in file matching2D.cpp and enable string-based selection based on detectorType
    //// -> HARRIS, FAST, BRISK, ORB, AKAZE, SIFT

    if (detectorType.compare("SHITOMASI") == 0)
    {
      detKeypointsShiTomasi(keypoints, imgGray, bVis_detector);
    }
    else
    {
      if(detectorType.compare("HARRIS") == 0)
      {
        detKeypointsHarris(keypoints, imgGray, bVis_detector);
      }
      else
      {
        detKeypointsModern(keypoints, imgGray, detectorType, bVis_detector);
      }
      //...
    }
    //// EOF STUDENT ASSIGNMENT

    //// STUDENT ASSIGNMENT
    //// TASK MP.3 -> only keep keypoints on the preceding vehicle

    // only keep keypoints on the preceding vehicle
    cv::Rect vehicleRect(535, 180, 180, 150);
    if (bFocusOnVehicle)
    {
      bool inbox;
      std::vector<cv::KeyPoint> inboxkeypoints;
      for (auto it = keypoints.begin(); it < keypoints.end(); ++it)
      {
        bool inbox = vehicleRect.contains((*it).pt);
        if(inbox)
        {
          inboxkeypoints.push_back(*it);
        }
      }
      keypoints = inboxkeypoints;
      
      if(bVis_onVehicle)
      {
        std::string windowName = "on vehicle Detection Results";
        cv::namedWindow(windowName, 5);
        cv::Mat visImage = img.clone();
        cv::drawKeypoints(img, keypoints, visImage, cv::Scalar::all(-1), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
        cv::imshow(windowName, visImage);
        cv::waitKey(0);
      }
    }

    //// EOF STUDENT ASSIGNMENT

    // optional : limit number of keypoints (helpful for debugging and learning)
    bool bLimitKpts = false;
    if (bLimitKpts)
    {
      int maxKeypoints = 50;

      if (detectorType.compare("SHITOMASI") == 0)
      { // there is no response info, so keep the first 50 as they are sorted in descending quality order
        keypoints.erase(keypoints.begin() + maxKeypoints, keypoints.end());
      }
      cv::KeyPointsFilter::retainBest(keypoints, maxKeypoints);
      std::cout << " NOTE: Keypoints have been limited!" << std::endl;
    }

    // push keypoints and descriptor for current frame to end of data buffer
    (dataBuffer.end() - 1)->keypoints = keypoints;
    std::cout << "#2 : DETECT KEYPOINTS done" << std::endl;

    /* EXTRACT KEYPOINT DESCRIPTORS */

    //// STUDENT ASSIGNMENT
    //// TASK MP.4 -> add the following descriptors in file matching2D.cpp and enable string-based selection based on descriptorType
    //// -> BRIEF, ORB, FREAK, AKAZE, SIFT

    cv::Mat descriptors;
    descKeypoints((dataBuffer.end() - 1)->keypoints, (dataBuffer.end() - 1)->cameraImg, descriptors, descriptorType);
    //// EOF STUDENT ASSIGNMENT

    // push descriptors for current frame to end of data buffer
    (dataBuffer.end() - 1)->descriptors = descriptors;

    std::cout << "#3 : EXTRACT DESCRIPTORS done" << std::endl;

    if (dataBuffer.size() > 1) // wait until at least two images have been processed
    {

      /* MATCH KEYPOINT DESCRIPTORS */
      std::vector<cv::DMatch> matches;

      //// STUDENT ASSIGNMENT
      //// TASK MP.5 -> add FLANN matching in file matching2D.cpp
      //// TASK MP.6 -> add KNN match selection and perform descriptor distance ratio filtering with t=0.8 in file matching2D.cpp
      matchDescriptors((dataBuffer.end() - 2)->keypoints, (dataBuffer.end() - 1)->keypoints,
                       (dataBuffer.end() - 2)->descriptors, (dataBuffer.end() - 1)->descriptors,
                       matches, matcherdescriptorType, matcherType, selectorType);

      //// EOF STUDENT ASSIGNMENT

      // store matches in current data frame
      (dataBuffer.end() - 1)->kptMatches = matches;

      //       std::cout << "#4 : MATCH KEYPOINT DESCRIPTORS done" << std::endl;

      // visualize matches between current and previous image

      if (bVis_matcher)
      {
        cv::Mat matchImg = ((dataBuffer.end() - 1)->cameraImg).clone();
        cv::drawMatches((dataBuffer.end() - 2)->cameraImg, (dataBuffer.end() - 2)->keypoints,
                        (dataBuffer.end() - 1)->cameraImg, (dataBuffer.end() - 1)->keypoints,
                        matches, matchImg,
                        cv::Scalar::all(-1), cv::Scalar::all(-1),
                        std::vector<char>(), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

        std::string windowName = "Matching keypoints between two camera images";
        cv::namedWindow(windowName, 7);
        cv::imshow(windowName, matchImg);
        std::cout << "Press key to continue to next image" << std::endl;
        cv::waitKey(0); // wait for key to be pressed
      }
    }
    std::cout<<std::endl;

  } // eof loop over all images

  return 0;
}
