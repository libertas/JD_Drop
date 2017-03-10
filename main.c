#include <mutex>
#include <opencv2/opencv.hpp>
#include <thread>
#include <time.h>
#include <unistd.h>

using namespace cv;
using namespace std;

bool running = true;

mutex frameLock;

Mat camFrame;
VideoCapture inputVideo(0);


std::mutex grayLock;

cv::Mat gray;

int wl_x_max, wl_y_max;


vector<Point> maxContour(vector<vector<Point> > contours)
{
  vector<Point> contour;
  double area;
  double maxArea = 0;
  for(auto i: contours) {
    area = contourArea(i);
    if(area > maxArea) {
      maxArea = area;
      contour = i;
    }
  }
  return contour;
}


Mat imgProcessing(Mat img)
{
 Mat tmp;

  cvtColor(img, tmp, CV_BGR2GRAY);
  threshold(tmp, tmp, 200, 255, THRESH_BINARY);

  return tmp;

}

void camera_thr()
{
  while(running) {
    frameLock.lock();
    inputVideo >> camFrame;
    frameLock.unlock();
    wl_x_max = camFrame.cols;
    wl_y_max = camFrame.rows;
    usleep(20000);
  }
}


int SimComMain();

void simcom_thr()
{
  SimComMain();
}

void SimComDaemon();

int main()
{
  Mat img;
  char c;

  thread ct(camera_thr);
  thread st(simcom_thr);
  thread sd(SimComDaemon);

  clock_t thistime;
  clock_t lasttime = clock();

  while(1) {
    frameLock.lock();
    img = imgProcessing(camFrame);
    frameLock.unlock();

    grayLock.lock();
    gray = img;
    grayLock.unlock();

    // imshow("Frame", img);

    thistime = clock();
    // cout<<"T="<<double(thistime - lasttime)/1000000<<endl;
    lasttime = thistime;

    c = waitKey(10);
    if(c == 27) {
      break;
    }
  }

  running = false;
  ct.join();
  st.join();
  sd.join();

  return 0;
}

