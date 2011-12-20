# cvHeadPose (libcvhp) #

OpenCV function for 6DOF head pose tracking

## Dependencies

To compile libcvhp:

 * gcc >= 4.4.5
 * OpenCV >= 2.3

To compile samples:

 * OpenGL
 * GLUT (freeglut3 >= 2.6.0)

## Install

CvHeadPose builds as shared object. Just type:

  $ make
  $ sudo make install 


##  Samples

To build samples, you need OpenCV >= 2.3. In addition,
if you want to build 3D Head Pose Demo, you need
OpenGL and libGLUT developement files, and OpenCV
build with GTK support.

To build all samples type:
  $ make samples

You can always build specific sample:
  $ make SAMPLE_NAME
e.g.: 
  $ make 2DHeadPose

You can see available samples in samples/ directory

## GETTING STARTED

 * Optional step: create new CvHeadPoseOptions structure
 * Initialize CvHeadPose*
 * Get frame from camera
 * Run cvhpFind. Pass frame and CvHeadPose as an arguments
 * Destroy CvHeadPose

 More details in samples (e.g. samples/2DHeadPose/2DHeadPose).
