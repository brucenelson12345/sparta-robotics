/***********************************************************************************************
Sparta Robotics
Editor: Bruce Nelson
Prototype Code REV1_A
November 6 2017

PURPOSE: 
    The main purpose of this program to demostrate the tracking of an object 
within a designated bounding box. For this demo, a predetermined bounding box can 
be given based off the object or can be allocated by the user from the offset of 
the first frame. The bounding box will display the name of the object being tracked,
in this case it is the refueling port. The demo loads in a video file where the camera
is being moved around at a variety of distances and views to demostrate the tracking 
of an object.
    The program below is written in C++ with the OpenCV module. The tracker implemented
below is based off the Kernelized Correlation Filter (KCF). The tracked uses KFC features
that is based of applying a circulant matrix to increase speed. A bounding box is then 
formed on the KCF tracked object and will be tracked until tracker can no longer track it.
Another track, GOTURN, was suggested because it uses Convolutional Neural Networks (CNN). 
This would provide a faster detection speed of the object versus KCF, however this method
was not implemented due to problems of compatibility with later versions of OpenCV.

Whats Needed:
*Makefile
*capture.cpp
*refuel_port.mp4

No command arguements are needed. The file can be compiled with the makefile and ran without
any additional arguements. If using the live video implementation, adjust capture for selected
video device.
***********************************************************************************************/

// Standard Header Files for OpenCV standard and tracker API functions an
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <iostream>
 
using namespace cv;
using namespace std;

// For connecting to a video device
// Value dependent on video Dev number of the camera to be used 
#define DEV 1
 

// Convert FPS from integer to ostream to be display on frame
#define SSTR( x ) static_cast< std::ostringstream & >( \
( std::ostringstream() << std::dec << x ) ).str()
 

/*************************************__CirclesTransform__************************************/
// Displays circles on screen
void CirclesTransform(Mat & frame)
{
    Mat img_gray, img_blur;

    // Converts image type from BGR to grayscale
    cvtColor(frame, img_gray, COLOR_BGR2GRAY);
    // Blurs the image to find edges easier
    GaussianBlur(img_gray, img_blur, Size(5,5), 2, 2);

    vector<Vec3f> circles;
    // Find the circles
    HoughCircles(img_gray, circles,CV_HOUGH_GRADIENT, 1, img_gray.rows/8, 80, 55, 0, 0);

    // Creates the cricles
    for(size_t i = 0; i < circles.size(); i++)
    {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        circle(frame, center, 3, Scalar(0,225,0), -1, 8, 0);
        circle(frame, center,radius, Scalar(0,0,255), 3, 8, 0);
    }
}
/************************************__END_CirclesTransform__**********************************/


/******************************************__MAIN__********************************************/
int main(int argc, char **argv)
{
    // Creates the smart pointer of the <Tracker> type to store the selected tracker used
    Ptr<Tracker> tracker;
    // Creates the KCF tracker and appoints to the tracker
    tracker = TrackerKCF::create();

    // Declares a type for capturing video
    VideoCapture cap;

    // Opens video from within project directory
    // Comment for live video
    cap.open("refuel_port.mp4");

    // Opens live video from selected Dev device
    // Uncomment for live video
    // cap.open(DEV);

    // Checks if a video or capturing device was opened
    // If unsuccessfull exit program
    if(!cap.isOpened())
    {
        cout << "ERROR! No video was found" << endl;
        return 1;  
    }
     
    // Declare frame to segement each video frame
    Mat frame;
    // Reads the first frame
    bool chk = cap.read(frame);
     
    // Default bounding box selected in first frame
    // Set for "refuel_port.mp4"
    Rect2d bbox(760, 400, 270, 200);

    // User selected bounding box
    // Uncomment to select bounding box in first frame
    // bbox = selectROI(frame, false);

    // Creates the initial bounding box of the selected object
    rectangle(frame, bbox, Scalar( 255, 0, 0 ), 2, 1 );
    // Displays the first frame
    imshow("Port_Tracker", frame); 
    // Initializes the bounding box and video frame to the tracker
    tracker->init(frame, bbox);

    // For user to confirm selected tracked object
    waitKey(0);

    // Check to exit program
    char quitKey; 
     
    // Loops through all frames in video. If live video, will loop until user quits
    while(cap.read(frame))
    {     
        // Checks time to begin timing FPS
        double timer = (double)getTickCount();
        
        // Updates the tracker on frame-by-frame basis that updates the bounding box
        bool chk = tracker->update(frame, bbox);
         
        // Retreives the FPS per frame
        float fps = getTickFrequency() / ((double)getTickCount() - timer);

        // Checks for circle features
        CirclesTransform(frame);
         
        // Checks if tracked object is within the frame
        // IF so display the bounding box and that the port is identified
        // ELSE display that tracking did not occur within the frame
        if (chk){
            // Constructs bounding box around the tracked object
            rectangle(frame, bbox, Scalar( 0, 255, 0 ), 2, 1 );
            // Tracked object is found
            putText(frame, "Refueling Port", Point(bbox.x,bbox.y), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255,0,0),2);
        }
        else{
            // Tracked object is not found
            putText(frame, "Tracking failure detected", Point(200,80), FONT_HERSHEY_SIMPLEX, 1.00, Scalar(0,0,255),2);
        }
        
        // Displays program's purpose
        putText(frame, "PORT TRACKER", Point(100,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255,170,50),2);
        // Displays the FPS
        putText(frame, "FPS : " + SSTR(int(fps)), Point(200,50), FONT_HERSHEY_SIMPLEX, 1.00, Scalar(255,170,50), 2);
 
        // Shows frame to screen
        imshow("Port_Tracker", frame);
         
        // Stores the value pressed on the active window
        // If the key pressed is 'q' or 'ESC', then exit loop
        int quitKey = waitKey(1);
        if(quitKey == 27 || quitKey == 'q')
        {
            break;
        }
 
    }

    // Releases the vidoe capturing device
    cap.release();
    // Deconstructs all windows allowing memory to be freed manually
    destroyAllWindows();

    return(0);
}
/**************************************__END_MAIN__********************************************/