#include "SimCom.h"


#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>

#include "ServiceLayer.h"
#include "PhysicalLayer.h"


#include <opencv2/opencv.hpp>
#include <mutex>
#include <thread>

using namespace std;
using namespace cv;


extern bool running;

extern mutex grayLock;
extern Mat gray;



#define M_ERR 0.1f

float x_now = 0;
float y_now = 0;
float angle_now = 0;


int gravityCenter(Mat src, CvPoint &center)
{
	assert(src.channels() == 1);
	int count = 0;
	double xsum = 0;
	double ysum = 0;
	for(int i = 0; i < src.rows; i++) {
		for(int j = 0; j < src.cols; j++) {
			if(src.at<uchar>(i, j) != 0) {
				xsum += j;
				ysum += i;
				count++;
			}
		}
	}
	
	center.x = cvRound(xsum / count);
	center.y = cvRound(ysum / count);
	
	return 0;
}


void callback0(char from, char to, char* data, SIMCOM_LENGTH_TYPE length)
{
  float *pf;
  char ldata[12];
  if(length == 12) {
    for(int i = 0; i < 12; i++) {
      ldata[i] = data[i];
    }
    pf = (float*)ldata;
    printf("x=%f\ty=%f\tangle=%f\n", pf[0], pf[1], pf[2]);
    x_now = pf[0];
    y_now = pf[1];
    angle_now = pf[2];
  } else {
    printf("length=%d\n", length);
  }
}

void callback1(char from, char to, char* data, SIMCOM_LENGTH_TYPE length)
{
  printf("\tCallback 1 from port %d:", (unsigned char)from);
  for(SIMCOM_LENGTH_TYPE i = 0; i < length; i++) {
    putchar(data[i]);
  }
}

void move_to_point(float x, float y, float angle) {
  float tmp[3];
  tmp[0] = x;
  tmp[1] = y;
  tmp[2] = angle;

  char *pi;
  pi = (char*)tmp;
  sl_send(0, 0, pi, 12);
}

void SimComDaemonSend()
{
	while(running) {
		ph_send_intr();
		usleep(10);
	}
}

void SimComDaemonReceive()
{
	while(running) {
		ph_receive_intr();
		sl_receive_intr();
		usleep(10);
	}
}

void SimComDaemon()
{
	  sl_config(0, callback0);
	  sl_config(1, callback1);
	
	  if(!sl_init()) {
	    printf("Unable to open the serial port\n");
	    return;
	  } else {
	    printf("Serial port opened\n");
	  }
  
	thread sds(SimComDaemonSend);
	thread sdr(SimComDaemonReceive);
	
	sds.join();
	sdr.join();
}

int SimComMain()
{
  CvPoint cent;
  uint16_t xy[2];
  char *pc = (char*)xy;
  if(running) {
	  //grayLock.lock();
	  
	  gravityCenter(gray, cent);
	  
	  //grayLock.unlock();
	  
	  xy[0] = cent.x;
	  xy[1] = cent.y;
	  sl_send(2, 2, pc, 4);
	  
	  //usleep(50000);
  }
	
  //char c;

  //int count = 0;
  //int step = 0;

  //float offset_x = 0;
  //float offset_y = 0;

  //while(running) {
	  //usleep(400000);
	  
      //float x[] = {0, 3, 0, 0, 3, 0, 0, 3, 0, 0, 3, 0, 0, 3, 0};
      //float y[] = {2.5, 0, 0, 2.5, 0, 0, 2.5, 0, 0, 2.5, 0, 0, 2.5, 0, 0};
      //float angle[] = {0, -2.266, 0, 0, -2.266, 0, 0, -2.266, 0, 0, -2.266, 0, 0, -2.266, 0};
      
      //move_to_point(x[step], y[step], angle[step]);
      //printf("moving to %f, %f, %f\n", x[step] + offset_x, y[step] + offset_y, angle[step]);
      //if(step < 14\
        //&& fabsf(x[step] + offset_x - x_now) < M_ERR\
        //&& fabsf(y[step] + offset_y - y_now) < M_ERR\
        //&& fabsf(angle[step] - angle_now) < M_ERR) {
        //usleep(500000);
        //step++;
      //}
  //}

  return 0;
}

