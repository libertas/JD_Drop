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

  medianBlur(tmp, tmp, 3);
  threshold(tmp, tmp, 230, 255, THRESH_TOZERO);



  adaptiveThreshold(tmp, tmp, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 45, 5);

  vector<vector<Point> > contours;

  findContours(tmp, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
  vector<Point> contour = maxContour(contours);
  contours.clear();
  contours.push_back(contour);


  Mat dst;
  //dst.create(img.rows, img.cols, 1);
  threshold(tmp, dst, 255, 255, THRESH_TOZERO);
  drawContours(dst, contours, 0, Scalar(255), CV_FILLED, 8, vector<Vec4i>(), 0, Point());

  // for (int i = 0; i < contours.size(); i++ ){
  //   drawContours(tmp, contours, i, Scalar(255), CV_FILLED, 8, vector<Vec4i>(), 0, Point() );
  // }

  return dst;

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


int main()
{
  Mat img;
  char c;

  thread ct(camera_thr);
  thread st(simcom_thr);

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

  return 0;
}

