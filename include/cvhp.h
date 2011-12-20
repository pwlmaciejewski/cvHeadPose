#include <opencv/cv.h>
#include <math.h>

#define PI 3.141592

/* Options structure */
typedef struct {
  char* haarCascadeSrc;
  int focalLength;
} CvHeadPoseOptions;

// Default options
CvHeadPoseOptions cvHeadPoseDefaultOptions = {
  .haarCascadeSrc = "data/haarcascade_frontalface_alt2.xml",
  .focalLength = 600
};

/* Head pose status enum */
enum CvHeadPoseStatus { NONE, KEYFRAME, TRACKING };

/* CvHeadPose */
typedef struct {
  enum CvHeadPoseStatus status;
  CvPoint2D32f* previousCorners;
  CvPoint2D32f* corners;
  int cornerCount;
  int maxCorners;
  CvPoint3D32f* modelPoints;
  CvPOSITObject* positObject;
  float* translationVector;
  float* rotationMatrix;
  CvHaarClassifierCascade* haarCascade;
  CvMemStorage* storage;
  IplImage* previousFrame;
  int focalLength;
} CvHeadPose;

/* cvHeadPoseInit */
CvHeadPose* cvhpInit(CvHeadPoseOptions*);

/* cvHeadPoseDestroy */
void cvhpDestroy(CvHeadPose*);
  
/* cvHeadPose */
void cvhpFind(IplImage*, CvHeadPose*);

/* _cvHeadPoseGetFace */
CvRect* _cvhpGetFace(const IplImage*, CvHeadPose*);

/* _cvHeadPoseModel */
CvPoint3D32f _cvhpModel(float, float);

/* _cvHeadPoseFindCorners */
void _cvhpFindCorners(IplImage*, CvPoint2D32f*, int*);

/* _cvHeadPoseTrack */
void _cvhpTrack(IplImage*, CvHeadPose* headPose);

