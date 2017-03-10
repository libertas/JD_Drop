#include "SimCom.h"


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
	double xsum = 0;
	double ysum = 0;
	for(int i = 0; i < src.rows; i++) {
		for(int j = 0; j < src.cols; j++) {
			if(!src.at<uchar>(i, j)) {
				xsum += j;
				ysum += i;
			}
		}
	}
	center.x = (int)(xsum / src.rows / src.cols);
	center.y = (int)(ysum / src.rows / src.cols);
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
  printf("\tCallback 1:");
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

#define CHK_ERR 0.2f
#define CHECK_POINT_NUM 1
bool check_pos(float *ox, float *oy)
{
	bool near_check_point = false;
	
	float check_points[CHECK_POINT_NUM ][2] = {{0, 0}};
	int check_point_i;
	
	for(check_point_i = 0; check_point_i < CHECK_POINT_NUM; check_point_i++) {
		if(fabsf(x_now - check_points[check_point_i][0]) < CHK_ERR\
	        && fabsf(x_now - check_points[check_point_i][1]) < CHK_ERR)  {
				near_check_point = true;
		}
	}
	
	if(!near_check_point) {
		return false;
	}
	
	float x_ratio = 20.0f;
	float y_ratio = -20.0f;
	float x_err, y_err;
	
	do {
		 CvPoint cent;
		 
		 grayLock.lock();
		 
		 gravityCenter(gray, cent);
		 int gray_x = gray.cols;
		 int gray_y = gray.rows;
		 
		 grayLock.unlock();
		 
		 x_err = (((float)(cent.x)  - gray_x / 2) / gray_x);
		 y_err = (((float)(cent.y)  - gray_y / 2) / gray_y);
		 
		 move_to_point(x_now + x_ratio * x_err, y_now + y_ratio * y_err, 0.0f);
		 printf("moving to %f, %f, %f\n", x_now + x_ratio * x_err, y_now + y_ratio * y_err, 0.0f);
		 
		 printf("cx=%d, cy=%d\n", cent.x, cent.y);
		 printf("x_err=%f, y_err=%f\n", x_err, y_err);
		 usleep(200000);
     } while(1);//(fabsf(x_err ) > 5 || fabsf(y_err) > 5);
	 
	 *ox = x_now - check_points[check_point_i][0];
	 *oy = y_now - check_points[check_point_i][1];
	 
	 printf("ox=%f, oy=%f\n", *ox, *oy);
	
	return near_check_point;
}

void SimComDaemonSend()
{
	while(running) {
		ph_send_intr();
		usleep(100);
	}
}

void SimComDaemonReceive()
{
	while(running) {
		ph_receive_intr();
		sl_receive_intr();
		usleep(100);
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
  char c;
  char s[200];

  int count = 0;
  int step = 0;

  float offset_x = 0;
  float offset_y = 0;

  while(running) {
	  usleep(400000);
    
    {
      float x[] = {0, 3, 0, 0, 3, 0, 0, 3, 0, 0, 3, 0, 0, 3, 0};
      float y[] = {2.5, 0, 0, 2.5, 0, 0, 2.5, 0, 0, 2.5, 0, 0, 2.5, 0, 0};
      float angle[] = {0, -2.266, 0, 0, -2.266, 0, 0, -2.266, 0, 0, -2.266, 0, 0, -2.266, 0};
      
check_pos(&offset_x, &offset_y) ;
      
      move_to_point(x[step], y[step], angle[step]);
      printf("moving to %f, %f, %f\n", x[step] + offset_x, y[step] + offset_y, angle[step]);
      if(step < 14\
        && fabsf(x[step] + offset_x - x_now) < M_ERR\
        && fabsf(y[step] + offset_y - y_now) < M_ERR\
        && fabsf(angle[step] - angle_now) < M_ERR) {

        check_pos(&offset_x, &offset_y) ;
        usleep(500000);
        step++;
      }
    }
  }

  return 0;
}

