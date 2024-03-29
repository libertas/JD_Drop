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


//vector<Point> maxContour(vector<vector<Point> > contours)
//{
  //vector<Point> contour;
  //double area;
  //double maxArea = 0;
  //for(auto i: contours) {
    //area = contourArea(i);
    //if(area > maxArea) {
      //maxArea = area;
      //contour = i;
    //}
  //}
  //return contour;
//}

bool triThreshold(Mat src, Mat &dst, uint8_t num, uint8_t high, uint8_t low)
{
	if(src.channels() == 3 && num >= 0 && num <= 2) {
		Mat tmp(src.rows, src.cols, CV_8U, Scalar(0));

		for(int i = 0; i < src.rows; i++) {
			for(int j = 0; j < src.cols; j++) {
				bool flag = true;
				for(int k = 0; k < 3; k++) {
					if(k == num) {
						if(src.at<Vec3b>(i, j)[k] < high) {
							flag = false;
							break;
						}
					} else {
						if(src.at<Vec3b>(i, j)[k] > low) {
							flag = false;
							break;
						}
					}
				}

				tmp.at<uchar>(i, j) = (uchar)flag * 255;
			}
	}

		dst = tmp;
		return true;
	} else {
		return false;
	}
}


Mat imgProcessing(Mat img)
{
   Mat tmp;

  triThreshold(img, tmp, 2, 180, 100);
  //medianBlur(tmp, tmp, 5);
  GaussianBlur(tmp, tmp, Size(5, 5), 1.5);

  return tmp;

}

void camera_thr()
{
  if(running) {
    frameLock.lock();
    inputVideo >> camFrame;
    frameLock.unlock();
    wl_x_max = camFrame.cols;
    wl_y_max = camFrame.rows;
    //usleep(2000);
  }
}


int SimComMain();

void simcom_thr()
{
  SimComMain();
}


void SimComDaemon();
void SimComDaemonReceive();
void SimComDaemonSend();


int main()
{
  Mat img;
  char c;

  //thread ct(camera_thr);
  //thread st(simcom_thr);
  thread sd(SimComDaemon);

  clock_t lasttime;


  while(1) {
    lasttime = clock();
    camera_thr();

    //frameLock.lock();
    img = imgProcessing(camFrame);
    //frameLock.unlock();

    //grayLock.lock();
    gray = img;
    
    SimComMain();
    imshow("gray", gray);
    moveWindow("gray", 0, 0);
    //grayLock.unlock();

    c = waitKey(1);
    if(c == 27) {
      break;
    }
    cout<<"  T="<<double(clock()-lasttime)/CLOCKS_PER_SEC<<endl;
  }

  running = false;
  //ct.join();
  //st.join();
  sd.join();

  return 0;
}

