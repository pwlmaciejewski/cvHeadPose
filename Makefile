# Compiler options
CC=gcc
CFLAGS=-Iinclude/
INSTALL_PATH=/usr/local

all: cvhp

cvhp: 
	$(CC) $(CFLAGS) -fPIC -c src/cvhp.c -o lib/cvhp.o 
	$(CC) $(CFLAGS) -shared -o lib/libcvhp.so lib/cvhp.o -lopencv_core -lopencv_objdetect -lopencv_video

samples: sample3DHeadPose sample2DHeadPose

sample3DHeadPose: cvhp
	$(CC) $(CFLAGS) samples/3DHeadPose/3DHeadPose.c -o samples/3DHeadPose/3DHeadPose -Llib -lcvhp -lGL -lGLU -lglut -lopencv_core -lopencv_highgui -lopencv_imgproc	

sample2DHeadPose: cvhp
	$(CC) $(CFLAGS) samples/2DHeadPose/2DHeadPose.c -o samples/2DHeadPose/2DHeadPose -Llib -lcvhp -lopencv_core -lopencv_highgui -lopencv_imgproc

install:
	cp lib/* $(INSTALL_PATH)/lib/
	ldconfig

clear: clearCvhp clearSamples	

clearCvhp:
	rm -f lib/*.o
	rm -f lib/*.so*

clearSamples:
	rm -f samples/3DHeadPose/3DHeadPose
	rm -f samples/2DHeadPose/2DHeadPose

