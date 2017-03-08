#include "SimCom.h"


#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "ServiceLayer.h"
#include "PhysicalLayer.h"

#define M_ERR 0.1f

float x_now = 0;
float y_now = 0;
float angle_now = 0;

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
  printf("Callback 1:");
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

int SimComMain()
{
  char c;
  char s[200];
  SIMCOM_LENGTH_TYPE length;

  sl_config(0, callback0);
  sl_config(1, callback1);

  if(!sl_init()) {
    printf("Unable to open the serial port\n");
    return -1;
  }


  int count = 0;
  int step = 0;


  while(1) {
    ph_send_intr();
    sl_receive_intr();
    usleep(100);
    if(count < 4000) {
      count++;
    } else {
      float x[] = {0, 3, 0, 0, 3, 0, 0, 3, 0, 0, 3, 0, 0, 3, 0};
      float y[] = {2.5, 0, 0, 2.5, 0, 0, 2.5, 0, 0, 2.5, 0, 0, 2.5, 0, 0};
      float angle[] = {0, -2.266, 0, 0, -2.266, 0, 0, -2.266, 0, 0, -2.266, 0, 0, -2.266, 0};
      move_to_point(x[step], y[step], angle[step]);
      printf("moving to %f, %f, %f\n", x[step], y[step], angle[step]);
      if(step < 14\
        && fabsf(x[step] - x_now) < M_ERR\
        && fabsf(y[step] - y_now) < M_ERR\
        && fabsf(angle[step] - angle_now) < M_ERR) {
        step++;
        usleep(500000);
      }
      count = 0;
    }
  }

  return 0;
}

