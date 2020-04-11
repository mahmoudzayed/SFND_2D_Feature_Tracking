Sensor Fusion Nanodegree - 2D Feature Tracking Project
======================================================

In this document we summarize the work done for the 2D Feature Tracking Project,
specifying how the different points in the rubric are fulfilled.

MP.0 Mid-Term Report
--------------------

```
Provide a Writeup / README that includes all the rubric points and how you addressed each one.
You can submit your writeup as markdown or pdf.
```
MP.1 Data Buffer Optimization
-----------------------------

```
Implement a vector for dataBuffer objects whose size does not exceed a limit (e.g. 2 elements).
This can be achieved by pushing in new elements on one end and removing elements on the other end.
```

This is implemented in `MidTermProject_Camera_Student.cpp` as follows:
```
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
```

MP.2 Keypoint Detection
-----------------------

```
Implement detectors HARRIS, FAST, BRISK, ORB, AKAZE, and SIFT 
and make them selectable by setting a string accordingly.
```
This is implemented in matching2D_Student.cpp in the functions detKeypointsHarris and detKeypointsModern.

The selection is done via a string. All the detectors are based on the OpenCV implementation, default constructed.

MP.3 Keypoint Removal
---------------------

```
Remove all keypoints outside of a pre-defined rectangle and 
only use the keypoints within the rectangle for further processing.
```

This is done by checkig if the rectangle contained the keypoint as shown below:
```
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
}
```
MP.4 Keypoint Descriptors
-------------------------
```
Implement descriptors BRIEF, ORB, FREAK, AKAZE and SIFT and 
make them selectable by setting a string accordingly.
```
This is implemented in matching2D_Student.cpp in the functions descKeypoints.

The selection is done via a string. All the detectors are based on the OpenCV implementation, default constructed.

MP.5 Descriptor Matching
------------------------
```
Implement FLANN matching as well as k-nearest neighbor selection. 
Both methods must be selectable using the respective strings in the main function.
```
This is implemented in matching2D_Student.cpp in the functions matchDescriptors. 
The openCV implementation and k parameter equal 2 was used for this part too. as shown below.
```
if (matcherType.compare("MAT_FLANN") == 0)
{
    if (descSource.type() != CV_32F || descRef.type() != CV_32F)
    { // OpenCV bug workaround : convert binary descriptors to floating point due to a bug in current OpenCV implementation
      descSource.convertTo(descSource, CV_32F);
      descRef.convertTo(descRef, CV_32F);
    }

    matcher = cv::DescriptorMatcher::create(cv::DescriptorMatcher::FLANNBASED);
}
```

MP.6 Descriptor Distance Ratio
------------------------------
```
Use the K-Nearest-Neighbor matching to implement the descriptor distance ratio test, which looks at the ratio of best vs. second-best match to decide whether to keep an associated pair of keypoints.
```
This is implemented in matching2D_Student.cpp in the functions matchDescriptors. 
The openCV implementation and k parameter equal 2 was used for this part too. as shown below.

```
if (selectorType.compare("SEL_KNN") == 0)
{  // k nearest neighbors (k=2)
    std::vector<std::vector<cv::DMatch>> knn_matches;
    int k = 2;
    // implement k-nearest-neighbor matching
    matcher->knnMatch(descSource, descRef, knn_matches, k);
    // filter matches using descriptor distance ratio test
    double minDescDistRatio = 0.8;
    for (auto it = knn_matches.begin(); it != knn_matches.end(); ++it)
    {
      if((*it)[0].distance < (minDescDistRatio*(*it)[1].distance))
      {
        matches.push_back((*it)[0]);
      }
    }
}
```
MP.7 Performance Evaluation 1
-----------------------------
```
Count the number of keypoints on the preceding vehicle for all 10 
images and take note of the distribution of their neighborhood size.
Do this for all the detectors you have implemented.
```
For all detectors, the number of keypoints detected on the preceeding
vehicle has been counted, noted down in the spreadsheet `2D_feature_tracking_performance.csv`.

The exact number will be shown below.
The evalution calculated the average for every frame with different descriptors.

| detector  | Keypoints | Distribution                 | size       |
|-----------|-----------|------------------------------|------------|
| SHITOMASI | 1343      | car top-half + license plate | small      |
| HARRIS    | 173       | roof + backlights            | small      |
| FAST      | 1787      | contour + inside             | small      |
| BRISK     | 2711      | contour + inside             | medium-big |
| ORB       | 500       | headlights + roof            | big        |
| AKAZE     | 1342      | contour                      | medium     |
| SIFT      | 1386      | roof + contour               | small-big  |

MP.8 Performance Evaluation 2
-----------------------------
```
Count the number of matched keypoints for all 10 images 
using all possible combinations of detectors and descriptors. 
In the matching step, the BF approach is used with the descriptor distance ratio set to 0.8.
```
For all detectors, the number of keypoints detected on the preceeding
vehicle has been counted, noted down in the spreadsheet `2D_feature_tracking_performance.csv`.

The exact number will be shown below.
The evalution calculated the average for every frame with different descriptors.

| detector  | BRISK       | BRIEF       | ORB         | FREAK       | SIFT        | AKAZE       |
|-----------|-------------|-------------|-------------|-------------|-------------|-------------|
| SHITOMASI | 85.22222222 | 104.8888889 | 100.8888889 | 85.33333333 | 103         | NAN         |
| HARRIS    | 15.77777778 | 19.22222222 | 18          | 16          | 18.11111111 | NAN         |
| FAST      | 99.88888889 | 122.1111111 | 119         | 97.55555556 | 116.2222222 | NAN         |
| BRISK     | 174.4444444 | 189.3333333 | 168.2222222 | 169.3333333 | 182.8888889 | NAN         |
| ORB       | 83.44444444 | 60.55555556 | 84.77777778 | 46.66666667 | 84.77777778 | NAN         |
| AKAZE     | 140         | NAN         | NAN         | NAN         | NAN         | 139.8888889 |
| SIFT      | 65.77777778 | 78          | NAN         | 65.88888889 | 88.88888889 | NAN         |

MP.9 Performance Evaluation 3
-----------------------------
```
Log the time it takes for keypoint detection and descriptor extraction.
The results must be entered into a spreadsheet and based on this data, 
the TOP3 detector / descriptor combinations must be recommended as the
best choice for our purpose of detecting keypoints on vehicles.
```
A more detailed data is found in the spreadsheet `2D_feature_tracking_performance.csv`.
The total time take by each combination is shown below. 

| detector  | BRISK       | BRIEF       | ORB         | FREAK       | SIFT        | AKAZE       |
|-----------|-------------|-------------|-------------|-------------|-------------|-------------|
| SHITOMASI | 20.39539544 | 18.79758822 | 19.18909011 | 53.12913989 | 28.27333256 | NAN         |
| HARRIS    | 16.34063467 | 15.35435549 | 15.84478283 | 54.07191456 | 30.019741   | NAN         |
| FAST      | 3.897620667 | 2.5808174   | 2.914457556 | 42.12976344 | 21.90067956 | NAN         |
| BRISK     | 45.18466544 | 42.81393189 | 46.48247567 | 83.107434   | 83.70242556 | NAN         |
| ORB       | 9.358418222 | 8.541480122 | 12.31751933 | 47.47291211 | 53.83340889 | NAN         |
| AKAZE     | 140         | NAN         | NAN         | NAN         | NAN         | 130.9719026 |
| SIFT      | 132.6871007 | 127.6092074 | NAN         | 167.0224792 | 188.0691452 | NAN         |

The fastest combination is the FAST + BRIEF and it did a very good performance.
The TOP 3 detector / descriptor combinations is 
1. FAST + ORB 
2. FAST + BRISK
3. FAST + BRIEF

The reason for this choices is based upon the following reasons:
1. The time taken by this combination is a huge advantage considering it need to run in realtime system.
2. The amount of the keypoint matched in the 10 frame
3. The the distribution of their neighborhood size and number of keypoints generated.
