/*
 * Test file: clock_t test
 * Ovewrite src/cvhp.c with this file, make, run.
 */

#include "cvhp.h"
#include <time.h>

typedef struct {
  int num;
  double time;
} test;

test haarTest;
test cornerTest;
test trackTest;
test poseTest;
test statusNone;
test statusTracking;
double start;
double statusStart;

CvHeadPose* cvhpInit(CvHeadPoseOptions* options) {
  CvHeadPose* headPose = malloc(sizeof(CvHeadPose));

  // Fallback to default options
  if (!options) {
    options = &cvHeadPoseDefaultOptions;
  }
  
  headPose->status = NONE;
  headPose->cornerCount = 0;
  headPose->maxCorners = 100;
  headPose->previousCorners = NULL;
  headPose->corners = NULL;  
  headPose->translationVector = NULL;
  headPose->rotationMatrix = NULL;

  headPose->haarCascade = (CvHaarClassifierCascade*) cvLoad(options->haarCascadeSrc, 0, 0, 0);
  if (!headPose->haarCascade) {
    return NULL;
  }

  headPose->storage = cvCreateMemStorage(0);
  headPose->previousFrame = NULL;
  headPose->focalLength = options->focalLength;
  
  return headPose;
}

void cvhpDestroy(CvHeadPose* headPose) {
  free(headPose->previousCorners);
  free(headPose->corners);
  free(headPose->modelPoints);
  cvReleasePOSITObject(&headPose->positObject);
  free(headPose->translationVector);
  free(headPose->rotationMatrix);
  cvReleaseMemStorage(&headPose->storage);
  cvReleaseImage(&headPose->previousFrame);
  free(headPose);

  printf("_cvhpGetFace:\t\t%.2fms (%d)\n", haarTest.time / haarTest.num, haarTest.num);
  printf("_cvhpFindCorners:\t%.2fms (%d)\n", cornerTest.time / cornerTest.num, cornerTest.num);  
  printf("_cvhpTrack:\t\t%.2fms (%d)\n", trackTest.time / trackTest.num, trackTest.num); 
  printf("POSIT:\t\t\t%.2fms (%d)\n", poseTest.time / poseTest.num, poseTest.num);
  printf("Status NONE:\t\t%.2fms (%d)\n", statusNone.time / statusNone.num, statusNone.num);
  printf("Status TRACKING:\t%.2fms (%d)\n", statusTracking.time / statusTracking.num, statusTracking.num);
}

void cvhpFind(IplImage* frame, CvHeadPose* headPose) {
  CvRect* face;
  int i;

  // Head not found
  if (headPose->status == NONE) {

    statusStart =  (double) clock();
    
    // Get face
    cvClearMemStorage(headPose->storage);
    start = (double) clock(); // start test
    face = _cvhpGetFace(frame, headPose);
    haarTest.num += 1;
    haarTest.time += ((double) clock() - start) / (CLOCKS_PER_SEC) * 1000; // Write results
    if (!face) {
      return; 
    }

    // Initialize corners
    free(headPose->corners);
    headPose->corners = (CvPoint2D32f*) malloc(headPose->maxCorners * sizeof(CvPoint2D32f));
    headPose->cornerCount = headPose->maxCorners;
    
    // Find corners
    cvSetImageROI(frame, *face);
    start = (double) clock(); // start test
    _cvhpFindCorners(frame, headPose->corners, &headPose->cornerCount);
    cornerTest.num += 1;
    cornerTest.time += ((double) clock() - start) / (CLOCKS_PER_SEC) * 1000; // Write results
    cvResetImageROI(frame);
    if (headPose->cornerCount < 4) {
      return;
    }

    // Map face points to model
    free(headPose->modelPoints);
    headPose->modelPoints = (CvPoint3D32f*) malloc(headPose->cornerCount * sizeof(CvPoint3D32f));
    for (i = 0; i < headPose->cornerCount; i += 1) {
      headPose->modelPoints[i] = _cvhpModel(headPose->corners[i].x  / face->width,
                                            headPose->corners[i].y  / face->height);
      
    }
        
    // Traslate corners from face coordinated to image coordinates
    for (i = 0; i < headPose->cornerCount; i += 1) {
      headPose->corners[i].x += face->x;
      headPose->corners[i].y += face->y;
    }
    
    // Change status
    headPose->status = KEYFRAME;

    statusNone.num += 1;
    statusNone.time += ((double) clock() - statusStart) / (CLOCKS_PER_SEC) * 1000; // Write results

  } else {

    statusStart = (double) clock();
    
    // Replace previousCorners with corners
    free(headPose->previousCorners);
    headPose->previousCorners = headPose->corners;
    headPose->corners = (CvPoint2D32f *) malloc(headPose->maxCorners * sizeof(CvPoint2D32f));
    
    // Track face features
    start = (double) clock(); // start test
    _cvhpTrack(frame, headPose);
    trackTest.num += 1;
    trackTest.time += ((double) clock() - start) / (CLOCKS_PER_SEC) * 1000; // Write results
    if (headPose->cornerCount < 4) {
      headPose->status = NONE;
      return;
    }

    // Create POSIT object
    cvReleasePOSITObject(&headPose->positObject);
    headPose->positObject = cvCreatePOSITObject(headPose->modelPoints, headPose->cornerCount);

    // Allocate translation vector and rotation matrix
    free(headPose->translationVector);
    free(headPose->rotationMatrix);
    headPose->translationVector = (float*) malloc(3 * sizeof(float));
    headPose->rotationMatrix = (float *) malloc(9 * sizeof(float));
    
    // POSIT    
    start = (double) clock(); // start test
    cvPOSIT(headPose->positObject, headPose->corners, headPose->focalLength,
            cvTermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 100, 1.0e-4f),
            headPose->rotationMatrix, headPose->translationVector);    
    poseTest.num += 1;
    poseTest.time += ((double) clock() - start) / (CLOCKS_PER_SEC) * 1000; // Write results

    // Change status
    headPose->status = TRACKING;

    statusTracking.num += 1;
    statusTracking.time += ((double) clock() - statusStart) / (CLOCKS_PER_SEC) * 1000; // Write results    
  }
  
  // Copy frame to praviousFrame
  if (!headPose->previousFrame) {
    headPose->previousFrame = cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, 3);
  }
  cvCopy(frame, headPose->previousFrame, NULL);

}

CvRect* _cvhpGetFace(const IplImage* frame, CvHeadPose* headPose) {
  CvSeq* faces;
  CvRect* face;
  CvPoint2D32f clipProportions = {.x = 0.3, .y = 0.2 };
  CvPoint clipRegion;
  int i;
  int area;
  int tmpArea;
  CvRect* tmpFace;

  // Detect faces
  faces = cvHaarDetectObjects(frame, headPose->haarCascade,
                      headPose->storage, 1.1, 3, CV_HAAR_DO_CANNY_PRUNING,
                              cvSize(10, 10), cvSize(200, 200));
  if (faces->total == 0) {
    return NULL;
  }

  // Get face with biggest area
  area = 0;
  for (i = 0; i < faces->total; i += 1) {
    tmpFace = (CvRect*) cvGetSeqElem(faces, i);
    tmpArea = tmpFace->x * tmpFace->y;
    if (tmpArea > area) {
      area = tmpArea;
      face = tmpFace;
    }
  }

  // Apply face padding
  clipRegion.x = (int) (clipProportions.x * face->width);
  clipRegion.y = (int) (clipProportions.y * face->height);
  face->x += clipRegion.x;
  face->y += clipRegion.y;
  face->width -= 2 * clipRegion.x;
  face->height -= 2 * clipRegion.y;

  return face;
}

void _cvhpFindCorners(IplImage* frame, CvPoint2D32f* corners, int* cornerCount) {
  CvRect roi = cvGetImageROI(frame);
  IplImage* faceG = cvCreateImage(cvSize(roi.width, roi.height), IPL_DEPTH_8U, 1);
  IplImage* eigImage = cvCreateImage(cvSize(roi.width, roi.height), IPL_DEPTH_32F, 1);
  IplImage* tmpImage = cvCreateImage(cvSize(roi.width, roi.height), IPL_DEPTH_32F, 1);
  
  cvCvtColor(frame, faceG, CV_RGB2GRAY);
  cvGoodFeaturesToTrack(faceG, eigImage, tmpImage, corners, cornerCount,
                        0.01, 10, NULL, 3, 0, 0.04);

  cvReleaseImage(&faceG);
  cvReleaseImage(&eigImage);
  cvReleaseImage(&tmpImage);
}

CvPoint3D32f _cvhpModel(float x, float y) {
  return cvPoint3D32f(x - 0.5, -y + 0.5, 0.5 * sin(PI *x));
}

void _cvhpTrack(IplImage* frame, CvHeadPose* headPose) {
  char status[headPose->cornerCount];
  IplImage* prev = cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, 1);
  IplImage* curr = cvCreateImage(cvSize(frame->width, frame->height), IPL_DEPTH_8U, 1);
  CvPoint2D32f* tmpCorners = (CvPoint2D32f*) malloc(headPose->cornerCount * sizeof(CvPoint2D32f));
  int i, j, count;

  // Good features to track
  cvCvtColor(headPose->previousFrame, prev, CV_RGB2GRAY);
  cvCvtColor(frame, curr, CV_RGB2GRAY);
  cvCalcOpticalFlowPyrLK(prev, curr, NULL, NULL, headPose->previousCorners, tmpCorners, headPose->cornerCount,
                         cvSize(10, 10), 3, status, NULL,
                         cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3), 0);
  cvReleaseImage(&prev);
  cvReleaseImage(&curr);
  
  // Count found corners
  count = 0;
  for (i = 0; i < headPose->cornerCount; i += 1) {
    if (status[i] == 1) {
      count += 1;
    }
  }

  // Replace headPose->corners and headPose->modelPoints
  for (i = 0, j = 0; i < headPose->cornerCount; i += 1) {
    if (status[i] == 1) {
      headPose->corners[j] = tmpCorners[i];
      headPose->modelPoints[j] = headPose->modelPoints[i];
      j += 1;
    }
  }
  headPose->cornerCount = count;

  free(tmpCorners);
}
