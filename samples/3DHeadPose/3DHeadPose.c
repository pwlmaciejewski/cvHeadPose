// Head pose
#include "cvhp.h"

// OpenCV
#include <opencv/cv.h>
#include <opencv/highgui.h>

// OpenGL
#include <GL/glut.h>
#include <GL/glu.h>

// Std. lib
#include <stdio.h>
#include <math.h>

// Const
#define FOCAL_LENGTH 604

// Global variables
int frameWidth;
int frameHeight;
CvHeadPose* headPose;
CvCapture* camera;
int isHeadPoseOn = 0;

void videoLoop() {
  IplImage* frame;
  char c;
  int i;

  frame = cvQueryFrame(camera);

  if (isHeadPoseOn == 1) {
    cvhpFind(frame, headPose);
    if (headPose->status == TRACKING ) {
      for (i = 0; i < headPose->cornerCount; i += 1) {
        cvCircle(frame, cvPoint(headPose->corners[i].x, headPose->corners[i].y),
                 2, CV_RGB(255, 0, 0), -1, CV_AA, 0);
      }
    }
  }
  
  cvShowImage("3D HeadPose Demo", frame);

  // Check use input
  c = cvWaitKey(33);
  if (c == 'c') {
    headPose->status = NONE;
    isHeadPoseOn = 1;
  } else if (c == 'w') {
    isHeadPoseOn = 0;
  }

  glutPostRedisplay();   
}

void renderAxis(float size)
{
  CvPoint3D32f faceCenter = { 0, 0, 0 };

  glLineWidth(1);
  glBegin(GL_LINES);
    glColor3f(0.5, 0.5, 0.5);
    glVertex3f(faceCenter.x - size, faceCenter.y, faceCenter.z);
    glVertex3f(faceCenter.x + size, faceCenter.y, faceCenter.z);
  glEnd();

  glBegin(GL_LINES);
    glColor3f(0.5, 0.5, 0.5);
    glVertex3f(faceCenter.x, faceCenter.y - size, faceCenter.z);
    glVertex3f(faceCenter.x, faceCenter.y + size, faceCenter.z);
  glEnd();

  glLineWidth(3);
  glBegin(GL_LINES);
    glColor3f(1, 0.0, 0.0);
    glVertex3f(faceCenter.x, faceCenter.y, faceCenter.z);
    glVertex3f(faceCenter.x, faceCenter.y, faceCenter.z + size);
  glEnd();
}

void draw() {
  int i, j;
  IplImage* frame;
  char c;
  float pose[16];

  videoLoop();
  
  // Initialize OpenGL camera
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Load projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(0, 1, -1, 0, 1, 10);

  // Construct model pose matrix
  for (i = 0; i < 16; i += 1) {
    pose[i] = 0;
  }
  if (headPose->status == TRACKING) {
    for (i = 0; i < 3; i += 1) {
      for (j = 0; j < 3; j += 1) {
        pose[i + j*4] = headPose->rotationMatrix[j + i*3];
      }
    }
    pose[12] = headPose->translationVector[0];
    pose[13] = headPose->translationVector[1];
    pose[14] = headPose->translationVector[2];
    pose[15] = 1.0;
  }
  
  // Draw head pose
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();  
  gluLookAt(0,0,0, 0,0,+1, 0,-2,0);
  glMultMatrixf(pose);
  renderAxis(1);
  
  glutSwapBuffers();
}

GLvoid reshape(GLsizei width, GLsizei height) {
  if (height == 0) {
    height = 1;
  }

  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
}

int main(int argc, char** argv) {
  CvHeadPoseOptions options = cvHeadPoseDefaultOptions;
  
  // Initialize window and camera capture
  cvNamedWindow("3D HeadPose Demo", CV_WINDOW_AUTOSIZE);
  camera = cvCaptureFromFile("out3.mpg");
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

  // Get frame properties
  frameWidth = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_WIDTH);
  frameHeight = cvGetCaptureProperty(camera, CV_CAP_PROP_FRAME_HEIGHT);
  
  // Glut initialization
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(frameWidth, frameHeight);
  glutInit(&argc, argv);
  glutCreateWindow("3D Head Pose Demo");
  glutDisplayFunc(draw);
  glutReshapeFunc(reshape);
  
  // Main loop
  glutMainLoop();

  // Clean up
  cvDestroyWindow("3D HeadPose Demo");
  cvReleaseCapture(&camera);
  cvhpDestroy(headPose);
  
  return 0;
}
