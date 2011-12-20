// Head pose
#include "cvhp.h"

// OpenCV
#include <opencv/cv.h>
#include <opencv/highgui.h>

// Std. lib
#include <stdio.h>

int main(int argc, char** argv) {
  CvHeadPose* headPose;
  CvCapture* camera;
  CvHeadPoseOptions options = cvHeadPoseDefaultOptions;
  IplImage* frame;
  char c;
  int isHeadPoseOn = 0;
  
  // Initialize window and camera capture
  cvNamedWindow("3D HeadPose Demo", CV_WINDOW_AUTOSIZE);
  camera = cvCaptureFromCAM(0);
  if (!camera) {
    perror("Cannot create camera capture\n");
    return 1;
  }

  // Initialize CvHeadPose
  options.focalLength = 602;
  headPose = cvhpInit(&options);
  if (!headPose) {
    perror("Cannot initialize HeadPose");
    return 1;
  }

  // Print info
  printf("C - Start capture\n");

  while (1) {
    frame = cvQueryFrame(camera);

    cvShowImage("3D HeadPose Demo", frame);

    if (isHeadPoseOn) {
      cvhpFind(frame, headPose);
      if (headPose->status == TRACKING) {
        printf("Translation vector:\n\t%f\t%f\t%f\n",
               headPose->translationVector[0],
               headPose->translationVector[1],
               headPose->translationVector[2]);

        printf("Rotation matrix:\n");
        printf("\t%f\t%f\t%f\n", headPose->rotationMatrix[0],
               headPose->rotationMatrix[1], headPose->rotationMatrix[2]);
        printf("\t%f\t%f\t%f\n", headPose->rotationMatrix[3],
               headPose->rotationMatrix[4], headPose->rotationMatrix[5]);
        printf("\t%f\t%f\t%f\n\n", headPose->rotationMatrix[6],
               headPose->rotationMatrix[7], headPose->rotationMatrix[8]);
      }
    }

    c = cvWaitKey(33);
    if (c == 'c') {
      headPose->status = NONE;
      isHeadPoseOn = 1;
    } else if (c == 'q') {
      cvhpDestroy(headPose);
      exit(0);
    }
  }

  return 0;
}
