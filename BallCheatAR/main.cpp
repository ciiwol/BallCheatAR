#include "setting.h"
#include "button.h"
#include <Windows.h>
#include <iostream>
#include <opencv2\imgproc.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

using namespace cv; 
using namespace std;


//�籸�� ��ġ �������� �Ϸ�ƴ���
bool poolSet = false;
int poolSetNumber = 0;
//�籸�� ������
Point poolPos[2];

//��ü��
Button btnResetPoolPos;
Button chkLine;
Button btnPoolColor;

//���� �����쿡 �׷��� ĵ����
Mat3b canvas;

//ī�޶� �������� ������ ����
Mat srcImg;
Mat procImg;
Mat gryImg;
Mat thdImg;
Mat outImg;

Mat img_lbl, stats, centroids;

//ȭ�� �� ���콺 ��ġ
int mX, mY;

//���� �׸���?
bool drawLine = false;

//�Ű�������
int cParam1 = 80, cParam2 = 10;
int cTh1 = 80, cTh2 = 20;

int poolRGB_R_Min = 20, poolRGB_G_Min = 70, poolRGB_B_Min = 60;
int poolRGB_R_Max = 120, poolRGB_G_Max = 180, poolRGB_B_Max = 180;

//������
double radiusMultiply = 2;

//�籸�� ����
Scalar poolColor;
int poolRange;
bool poolColorSet = false;

//ȭ�鿡 ���콺 ������ ���� ���
void callBackFunc(int event, int x, int y, int flags, void* userdata)
{
	mX = x;
	mY = y;

	//�籸�� ���¹�ư
	if (btnResetPoolPos.isInPos(x, y))
	{
		if (poolSet)	//�籸�밡 �����Ǿ��ִ� ��쿡�� ���¹�ư�� ������
		{
			if (event == EVENT_LBUTTONUP)
			{
				cout << "reset Pool Position" << endl;
				for (int i = 0; i < 2; i++)
					poolPos[i] = Point(-1, -1);
				poolSet = false;
				btnResetPoolPos.setText("Pool Position Resetting(LT)");
			}
		}
		return;
	}

	//���� ���� üũ��ư
	if (chkLine.isInPos(x, y))
	{
		if (event == EVENT_LBUTTONUP)
		{
			drawLine = !drawLine;
			chkLine.setText(drawLine?"Line O":"Line X");
		}
	}

	//�籸�� ��ġ ����
	if (!poolSet)
	{
		btnPoolColor.setColor(srcImg.at<Scalar>(y, x));
		//Ŭ���ϸ� �� ���� �籸�� ���������� �����Ѵ�.
		if (event == EVENT_LBUTTONDOWN)
		{
			if (x >= 0 && x < IMG_W && y >= 0 && y < IMG_H)
			{
				poolPos[poolSetNumber].x = x;
				poolPos[poolSetNumber].y = y;
				poolSetNumber += 1;
				btnResetPoolPos.setText("Pool Position Resetting(RD)");
				if (poolSetNumber == 2)
				{
					poolSetNumber = 0;
					poolSet = true;
					btnResetPoolPos.setText("Reset Pool Pos");
				}
			}
		}
	}

	//�籸�� ���� �������۹�ư
	if (btnPoolColor.isInPos(x, y))
	{
		if (event == EVENT_LBUTTONUP)
		{
			poolColorSet = !poolColorSet;
			btnPoolColor.setText(poolColorSet ? "Set Pool Color" : "Setting..");
		}
	}

	//�籸�� ���� ����
	if (!poolColorSet)
	{
		//Ŭ���ϸ� �� ���� �籸�� �������� ����
		if (event == EVENT_LBUTTONDOWN)
		{

		}
	}

}

//������ ���ϴ°�
Point* isIntersection(Point p1, Point p2, Point p3, Point p4) {
	// Store the values for fast access and easy
	// equations-to-code conversion
	float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
	float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;

	float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	// If d is zero, there is no intersection
	if (d == 0) return NULL;

	// Get the x and y
	float pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
	float x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
	float y = (pre * (y3 - y4) - (y1 - y2) * post) / d;

	// Check if the x and y coordinates are within both lines
	if (x < min(x1, x2) || x > max(x1, x2) ||
		x < min(x3, x4) || x > max(x3, x4)) return NULL;
	if (y < min(y1, y2) || y > max(y1, y2) ||
		y < min(y3, y4) || y > max(y3, y4)) return NULL;

	// Return the point of intersection
	Point* ret = new Point();
	ret->x = x;
	ret->y = y;
	return ret;
}

//����? �� �� ���ױ׸��°Ű���
void morphOps(Mat &thresh) {

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle
	Mat erodeElement = getStructuringElement(MORPH_RECT, Size(2, 2));
	//dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement(MORPH_RECT, Size(2, 2));

	erode(thresh, thresh, erodeElement);
	erode(thresh, thresh, erodeElement);

	dilate(thresh, thresh, dilateElement);
	dilate(thresh, thresh, dilateElement);
}

int main()
{
	//cerr << cv::getBuildInformation();

	//�籸�� ������ �ʱ�ȭ
	for each (Point p in poolPos)
	{
		p.x = -1;
		p.y = -1;
	}

	//ī�޶� ����
	VideoCapture capture(0);

	//������ ����
	namedWindow("Main");
	namedWindow("Canny");
	namedWindow("Threshold");
	namedWindow("Setting");
	namedWindow("Display", CV_WINDOW_FULLSCREEN);

	resizeWindow("Main", IMG_W, IMG_H + PANEL_H);
	resizeWindow("Setting", 400, 600);

	//Ʈ���� ����
	createTrackbar("cParam1", "Setting", &cParam1, 255);
	createTrackbar("cParam2", "Setting", &cParam2, 255);
	createTrackbar("cCanny1", "Setting", &cTh1, 255);
	createTrackbar("cCanny2", "Setting", &cTh2, 255);

	createTrackbar("pool_range", "Setting", &poolRange, 50);
	/*
	createTrackbar("poolR_m", "Setting", &poolRGB_R_Min, 255);
	createTrackbar("poolG_m", "Setting", &poolRGB_G_Min, 255);
	createTrackbar("poolB_m", "Setting", &poolRGB_B_Min, 255);
	createTrackbar("poolR_M", "Setting", &poolRGB_R_Max, 255);
	createTrackbar("poolG_M", "Setting", &poolRGB_G_Max, 255);
	createTrackbar("poolB_M", "Setting", &poolRGB_B_Max, 255);
	*/

	//���콺 �ݹ��Լ� ����
	setMouseCallback("Main", callBackFunc);

	//��ü�� ����
	try
	{
		canvas = Mat3b(IMG_H+PANEL_H, IMG_W, Vec3b(0, 0, 0));

		//��ư
		btnResetPoolPos = Button(canvas, 0, IMG_H, 260, PANEL_H, "Pool Position Resetting(LT)", Scalar(200, 200, 200));
		chkLine = Button(canvas, IMG_W - 80, IMG_H, 80, PANEL_H, "Line X", Scalar(230, 230, 230));
		btnPoolColor = Button(canvas, 280, IMG_H, 150, PANEL_H, "Reset pool color", poolColor);
	}
	catch (Exception e)
	{
		cout << "GUI���� : " << e.msg << endl;
	}
	 
	//FPSǥ�� ���ڿ� �ʱ�ȭ
	char strBuf[STRBUFFER] = { 0, };


	//ĸ�� ����(������ �����ͼ� srcImg�� ��´�)
	while (capture.read(srcImg))
	{
		try
		{
			resize(srcImg, srcImg, Size(IMG_W, IMG_H));
			//����� �׸� �̹���
			outImg = Mat(Size(srcImg.cols, srcImg.rows), srcImg.type(), Scalar(0, 0, 0));

			//�籸�� ��ġ�� ������ ���� �� �����̸�
			if (poolSet && poolColorSet)
			{
				//�����ϰ�
				/*Mat sharpen_kernel = (Mat_<char>(3, 3) << 0, -1, 0,
														-1, 5, -1,
														0, -1, 0);
				filter2D(srcImg, srcImg, srcImg.depth(), sharpen_kernel);*/

				inRange(srcImg, Scalar(poolColor[0] - poolRange, poolColor[1]- poolRange, poolColor[2] - poolRange),
								Scalar(poolColor[0] + poolRange, poolColor[1] + poolRange, poolColor[2] + poolRange), thdImg);
				//inRange(srcImg, Scalar(poolRGB_B_Min, poolRGB_G_Min, poolRGB_R_Min), Scalar(poolRGB_B_Max, poolRGB_G_Max, poolRGB_R_Max), thdImg);
				imshow("Threshold", thdImg);

				//�̹��� �׷��̽����� ��ȯ
				//cvtColor(thdImg, procImg, COLOR_BGR2GRAY, 0);

				//�̹��� ����þȺ� ����
				GaussianBlur(thdImg, procImg, Size(3, 3), 0);
				morphOps(procImg);

				//��(�籸��) ���� (circles�� ��´�)
				vector<Vec3f> circles;				
				HoughCircles(procImg, circles, CV_HOUGH_GRADIENT, 1, DIST_BALL, cParam1, cParam2, MAX_BALL_SIZE, MIN_BALL_SIZE);

				//ĳ�� ��ȯ
				Canny(procImg, procImg, cTh1, cTh2);
				imshow("Canny", procImg);

				//����(ť��) ����(lines�� ��´�)
				vector<Vec2f> lines;
				if(drawLine) HoughLines(procImg, lines, 1, CV_PI / 180, 150);

				//����� �� �� �����ڸ��� �پ��ְų� �����ڸ� �Ѿ ������ �� �����
				////////////////////�� �׸���///////////////////////
				Point pntGuideline1(poolPos[0].x + DIST_BALL / 2.3, poolPos[0].y + DIST_BALL / 2.3);
				Point pntGuideline2(poolPos[1].x - DIST_BALL / 2.3, poolPos[1].y - DIST_BALL / 2.3);
				vector<Vec3f>::const_iterator itc = circles.begin();
				while (itc != circles.end())
				{
					Point curCircleP = Point((*itc)[0], (*itc)[1]);
					//�籸�� �ۿ� �ִ� ���� �����.
					if (!curCircleP.inside(Rect(pntGuideline1, pntGuideline2)))
					{
						circles.erase(itc);
						itc = circles.begin();
						continue;
					}
					//�籸�� ������ ������ ���� �����.
					if (norm(curCircleP - poolPos[0]) < DIST_BALL ||
						norm(curCircleP - poolPos[1]) < DIST_BALL ||
						norm(curCircleP - Point(poolPos[0].x, poolPos[1].y)) < DIST_BALL ||
						norm(curCircleP - Point(poolPos[1].x, poolPos[0].y)) < DIST_BALL)
					{
						circles.erase(itc);
						itc = circles.begin();
						continue;
					}

					//��Ƴ��� ���� �׸���
					circle(outImg, Point((*itc)[0], (*itc)[1]), (*itc)[2] * radiusMultiply, Scalar(255,255,255), 2);
					circle(srcImg, Point((*itc)[0], (*itc)[1]), (*itc)[2], Scalar(255,0,0), 2);

					++itc;
				}

				////////////////////�� �׸���///////////////////////
				if (drawLine) {
					//�籸�� �ٱ��� �ܳ��� ������ �� �����
					vector<Vec2f>::const_iterator itc2 = lines.begin();
					while (itc2 != lines.end())
					{
						double a = cos((*itc2)[1]), b = sin((*itc2)[1]);
						double x0 = a*((*itc2)[0]), y0 = b*((*itc2)[0]);
						Point pt1, pt2;
						pt1.x = round(x0 + IMG_W * (-b));
						pt1.y = round(y0 + IMG_W * (a));
						pt2.x = round(x0 - IMG_W * (-b));
						pt2.y = round(y0 - IMG_W * (a));

						//�簢���� �� ������ ������ ã�´�. ������ �ϳ��� ������ �籸�� ���̹Ƿ� �����.
					   /*if(!isIntersection(pt1, pt2, poolPos[0], Point(poolPos[1].x, poolPos[0].y)) &&
							!isIntersection(pt1, pt2, poolPos[0], Point(poolPos[0].x, poolPos[1].y)) &&
							!isIntersection(pt1, pt2, Point(poolPos[0].x, poolPos[1].y), poolPos[1]) &&
							!isIntersection(pt1, pt2, Point(poolPos[1].x, poolPos[0].y), poolPos[1]))*/
						if (!isIntersection(pt1, pt2, pntGuideline1, Point(pntGuideline2.x, pntGuideline1.y)) &&
							!isIntersection(pt1, pt2, pntGuideline1, Point(pntGuideline1.x, pntGuideline2.y)) &&
							!isIntersection(pt1, pt2, Point(pntGuideline1.x, pntGuideline2.y), pntGuideline2) &&
							!isIntersection(pt1, pt2, Point(pntGuideline2.x, pntGuideline1.y), pntGuideline2))
						{
							lines.erase(itc2);
							itc2 = lines.begin();
							continue;
						}

						++itc2;
					}

					////��Ƴ��� ������ �׸���. 
					/*Point pt1 = Point((*itc2)[0], (*itc2)[1]);
					Point pt2 = Point((*itc2)[2], (*itc2)[3]);*/
					if (lines.size() > 1)
					{
						itc2 = lines.begin();
						double t1 = (*itc2)[1];
						double a1 = cos(t1), b1 = sin(t1);
						double x01 = a1*((*itc2)[0]), y01 = b1*((*itc2)[0]);
						Point pt1, pt2, pt3, pt4, ptS, ptE, *ptTS = NULL, *ptTE = NULL;
						pt1.x = round(x01 + IMG_W * (-b1));
						pt1.y = round(y01 + IMG_W * (a1));
						pt2.x = round(x01 - IMG_W * (-b1));
						pt2.y = round(y01 - IMG_W * (a1));
						line(srcImg, pt1, pt2, Scalar(255, 255, 255));
						itc2 = lines.end() - 1;
						double t2 = (*itc2)[1];
						double a2 = cos(t2), b2 = sin(t2);
						double x02 = a2*((*itc2)[0]), y02 = b2*((*itc2)[0]);
						pt3.x = round(x02 + IMG_W * (-b2));
						pt3.y = round(y02 + IMG_W * (a2));
						pt4.x = round(x02 - IMG_W * (-b2));
						pt4.y = round(y02 - IMG_W * (a2));
						line(srcImg, pt3, pt4, Scalar(255, 255, 255));

						//���� ������ ����(t3)�� ��ǥ(x03, y03)
						double t3 = (t1 + t2) / 2;
						double a3 = cos(t3), b3 = sin(t3);
						double x03 = (x01 + x02) / 2, y03 = (y01 + y02) / 2;
						ptS.x = round(x03 + IMG_W * (-b3));
						ptS.y = round(y03 + IMG_W * (a3));
						ptE.x = round(x03 - IMG_W * (-b3));
						ptE.y = round(y03 - IMG_W * (a3));

						cout << t3 << endl;;

						//�籸�� �ȿ���
						ptTS			    = isIntersection(ptS, ptE, poolPos[0], Point(poolPos[1].x, poolPos[0].y));

						if (!ptTS)	   ptTS = isIntersection(ptS, ptE, poolPos[0], Point(poolPos[0].x, poolPos[1].y));
						else if(!ptTE) ptTE = isIntersection(ptS, ptE, poolPos[0], Point(poolPos[0].x, poolPos[1].y));

						if (!ptTS)		ptTS = isIntersection(ptS, ptE, Point(poolPos[0].x, poolPos[1].y), poolPos[1]);
						else if (!ptTE) ptTE = isIntersection(ptS, ptE, Point(poolPos[0].x, poolPos[1].y), poolPos[1]);

						if (!ptTS)		ptTS = isIntersection(ptS, ptE, Point(poolPos[1].x, poolPos[0].y), poolPos[1]);
						else if (!ptTE) ptTE = isIntersection(ptS, ptE, Point(poolPos[1].x, poolPos[0].y), poolPos[1]);

						if (ptTS && ptTE) {
							line(outImg, *ptTS, *ptTE, Scalar(255, 255, 255), 3);
							line(srcImg, *ptTS, *ptTE, Scalar(0, 0, 255));

							//���� ƨ�� �� ������
							Point ptNE;
							Point ptNS;
							double t4 = -t3;
							double a4 = cos(t4), b4 = sin(t4);

							if (abs(norm(pt1 - pt3)) < abs(norm(pt2 - pt4)))
							{
								ptNS = *ptTS;
								ptNE.x = round(ptTE->x - IMG_W * (-b4));
								ptNE.y = round(ptTE->y - IMG_W * (a3));
							}
							else
							{
								ptNS = *ptTE;
								ptNE.x = round(ptTS->x - IMG_W * (-b4));
								ptNE.y = round(ptTS->y - IMG_W * (a3));
							}

							

							line(outImg, ptNS, ptNE, Scalar(255, 255, 255), 3);
							line(srcImg, ptNS, ptNE, Scalar(0, 0, 255));
						}
					}
				}

				//�籸�� �����ڸ� ���� �׸���
				rectangle(srcImg, Rect(poolPos[0], poolPos[1]), Scalar(255, 255, 255), 2);
				rectangle(srcImg, Rect(pntGuideline1, pntGuideline2), Scalar(0, 255, 255), 2);
				rectangle(outImg, Rect(poolPos[0], poolPos[1]), Scalar(255, 255, 255), 2);
			
			}
			//���� �籸�� ��ġ �������̸�
			else
			{
				//���ڼ�(���̵����)�� �׸���.
				line(srcImg, Point(0, mY), Point(IMG_W, mY), Scalar(100, 100, 100));
				line(srcImg, Point(mX, 0), Point(mX, IMG_H), Scalar(100, 100, 100));
			}

			//�籸�� �������� �׸���. ���� ��������� �ȱ׸���.
			for (int i = 0; i < 2; i++)
			{
				if(poolPos[i].x >= 0 && poolPos[i].y >= 0)
					circle(srcImg, poolPos[i], 3, Scalar(0,0,255));
			}
			

			//����� �̹��� ���
			srcImg.copyTo(canvas(Rect(0, 0, srcImg.cols, srcImg.rows)));

			//���

			imshow("Main", canvas);
			imshow("Display", outImg);
		}
		catch(Exception e)
		{
			cout << "���� : " << e.msg << endl;
		}

		//ESC to escape program
		if (cvWaitKey(33) >= 27) break;
	}
	srcImg.release();
	outImg.release();
	procImg.release();
	thdImg.release();
	capture.release();
	cvDestroyAllWindows();
	
	return 0;
}
