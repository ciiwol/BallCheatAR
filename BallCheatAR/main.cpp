/*
BallCheatAR
Projection mapping project.
Detect balls and cue shaft, draw some effects and expected path using OpenCV
jk012345@gmail.com
Jujin Kim, Jaehyun Sim, Yeongjin Lee.
*/

#include <Windows.h>
#include <iostream>
#include <opencv2\imgproc.hpp>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>

#include "setting.h"
#include "button.h"
#include "functions.h"
//#include "main.h"

using namespace cv; 
using namespace std;

#pragma region Button Objs
Button btnResetPoolPos;	//(��ư) �籸�� ��ġ ����
Button chkLine;			//(��۹�ư) �� �Բ�
Button btnPoolColor;	//(��ư) �籸�� ���� ����
Button btnCuePointColor;	//(��ư) ť�� ����Ʈ�� ���� ����
Button btnCueStickColor;	//(��ư) ť�� ������ ���� ����
#pragma endregion

//���� �����쿡 �׷��� ĵ����
Mat3b canvas;

#pragma region Mat
Mat srcImg;	//ī�޶� �̹���
Mat hsvImg;	//HSV��ȯ �̹���
Mat procImg;	//���μ��̿�
Mat gryImg;	//�׷��̽����� �̹���
Mat thdImg;	//����ȭ �̹���
Mat thdWhiteImg;	//���Ž�� ����ȭ�̹���
Mat proc_cuePImg;	//ť��-����Ʈ��
Mat proc_cueSImg;	//ť��-������
Mat outImg;	//���������� ��� �̹���
Mat img_lbl, stats, centroids;

Mat logoImg;	//�ΰ�
#pragma endregion

//ȭ�� �� ���콺 ��ġ
int mX, mY;

//���� �׸���?
bool drawLine = false;

//�Ű�������
int cParam1 = 80, cParam2 = 9, cParam2W = 15;
int cTh1 = 80, cTh2 = 20;

//������
double radiusMultiply = 2;

//Perspective Transform Parameter
int perspectiveParameterTop = 0;
int perspectiveParameterBottom = 0;

//White ball position
Point WhiteBallPos;
int whiteBallLostCount = 0;

//Variables of pool position
#pragma region �籸�� ��ġ ������
bool poolSet = false;	//�籸�� ��ġ �������� �Ϸ�ƴ���
int poolSetNumber = 0;	//0�̸� ���� ���� �� ��, 1�̸� �� �� ����. 2�̸� ���� �Ϸ�
Point poolPos[2];		//�籸�� ������. 
float poolWidth = 0, poolHeight = 0;	//�籸�� ����/����. (poolPos ���)
int roiRange = 20;		//�籸�� ��ġ�κ��� roiRange�� ��ŭ Ȯ���ؼ� ROI(Region of Interest)�� ����
Point poolPosROI[2];	//�籸�� ��ġ���� ����(roiRange) Ȯ���Ѱ�. ROI�� �ȴ�.
float poolROIWidth = 0, poolROIHeight = 0;	//ROI ����/����
#pragma endregion

//Color of pool&cue shaft
#pragma region �籸��/ť�� ����
//�籸�� ����
Scalar poolColor = Scalar(255, 255, 255);
int poolRangeH = Range_H_INIT, poolRangeS = Range_S_INIT, poolRangeV = Range_V_INIT;
bool poolColorSet = true;

//ť�� ����
Scalar cuePntColor = Scalar(255, 255, 255);
int cuePntCRangeH = CUE_H_INIT, cuePntCRangeS = CUE_S_INIT, cuePntCRangeV = CUE_V_INIT;
bool cuePntSet = true;
Scalar cueStkColor = Scalar(255, 255, 255);
int cueStkCRangeH = CUE_H_INIT, cueStkCRangeS = CUE_S_INIT, cueStkCRangeV = CUE_V_INIT;
bool cueStkSet = true;
#pragma endregion

//Mouse callback function
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
					poolWidth = abs(poolPos[1].x - poolPos[0].x);
					poolHeight = abs(poolPos[1].y - poolPos[0].y);

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
			poolColorSet = false;
			btnPoolColor.setText("Setting..");
		}
	}
	//�籸�� ���� ����
	if (!poolColorSet)
	{
		try {
			btnPoolColor.setColor(srcImg.at<Vec3b>(y, x));
		}
		catch (Exception e) { cout << "�籸�������" << e.msg << endl; }
		//Ŭ���ϸ� �� ���� �籸�� �������� ����
		if (event == EVENT_LBUTTONDOWN)
		{
			poolColor = Scalar(hsvImg.at<Vec3b>(y, x));
			poolColorSet = true;
			btnPoolColor.setText("Set Pool Color");
		}
	}

	//ť ����Ʈ���� �������۹�ư
	if (btnCuePointColor.isInPos(x, y))
	{
		if (event == EVENT_LBUTTONUP)
		{
			cuePntSet = false;
			btnCuePointColor.setText("...");
		}
	}
	//ť ����Ʈ���� ����
	if (!cuePntSet)
	{
		try {
			btnCuePointColor.setColor(srcImg.at<Vec3b>(y, x));
		}
		catch (Exception e) { cout << "ť����Ʈ������" << e.msg << endl; }
		//Ŭ���ϸ� �� ���� ť ����Ʈ�������� ����
		if (event == EVENT_LBUTTONDOWN)
		{
			cuePntColor = Scalar(hsvImg.at<Vec3b>(y, x));
			cuePntSet = true;
			btnCuePointColor.setText("C_P_C");
		}
	}

	//ť ������� �������۹�ư
	if (btnCueStickColor.isInPos(x, y))
	{
		if (event == EVENT_LBUTTONUP)
		{
			cueStkSet = false;
			btnCueStickColor.setText("...");
		}
	}
	//ť ������� ����
	if (!cueStkSet)
	{
		try {
			btnCueStickColor.setColor(srcImg.at<Vec3b>(y, x));
		}
		catch (Exception e) { cout << "ť���������" << e.msg << endl; }
		//Ŭ���ϸ� �� ���� ť ����������� ����
		if (event == EVENT_LBUTTONDOWN)
		{
			cueStkColor = Scalar(hsvImg.at<Vec3b>(y, x));
			cueStkSet = true;
			btnCueStickColor.setText("C_S_C");
		}
	}
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

	//Generate Windows
	#pragma region Generate Windows
	namedWindow("Main");
	namedWindow("Cue");
	namedWindow("Threshold");
	namedWindow("Threshold_W");
	namedWindow("Setting");
	namedWindow("Setting HSV Range");
	namedWindow("Display", CV_WINDOW_FREERATIO);

	resizeWindow("Main", IMG_W, IMG_H + PANEL_H);
	resizeWindow("Setting", 400, 300);
	resizeWindow("Setting HSV Range", 500, 500);
	#pragma endregion

	//Generate Trackbars
	#pragma region  Generate Trackbars
	createTrackbar("RoI Range", "Setting", &roiRange, 50);

	createTrackbar("cParam1", "Setting", &cParam1, 255);
	createTrackbar("cParam2", "Setting", &cParam2, 255);
	createTrackbar("cParam2W", "Setting", &cParam2W, 255);
	createTrackbar("output Perspective Top", "Setting", &perspectiveParameterTop, 50);
	createTrackbar("output Perspective Bottom", "Setting", &perspectiveParameterBottom, 50);

	createTrackbar("pool_range_H", "Setting HSV Range", &poolRangeH, 100);
	createTrackbar("pool_range_S", "Setting HSV Range", &poolRangeS, 100);
	createTrackbar("pool_range_V", "Setting HSV Range", &poolRangeV, 100);

	createTrackbar("cue_p_range_H", "Setting HSV Range", &cuePntCRangeH, 100);
	createTrackbar("cue_p_range_S", "Setting HSV Range", &cuePntCRangeS, 100);
	createTrackbar("cue_p_range_V", "Setting HSV Range", &cuePntCRangeV, 100);

	createTrackbar("cue_s_range_H", "Setting HSV Range", &cueStkCRangeH, 100);
	createTrackbar("cue_s_range_S", "Setting HSV Range", &cueStkCRangeS, 100);
	createTrackbar("cue_s_range_V", "Setting HSV Range", &cueStkCRangeV, 100);
	#pragma endregion
	
	//Mouse Callback method bind.
	setMouseCallback("Main", callBackFunc);

	//Assign Objects.
	try
	{
		canvas = Mat3b(IMG_H+PANEL_H, IMG_W, Vec3b(0, 0, 0));

		//��ư
		btnResetPoolPos = Button(canvas, 0, IMG_H, 260, PANEL_H, "Pool Position Resetting(LT)", Scalar(200, 200, 200));
		chkLine = Button(canvas, IMG_W - 80, IMG_H, 80, PANEL_H, "Line X", Scalar(230, 230, 230));
		btnPoolColor = Button(canvas, 280, IMG_H, 150, PANEL_H, "Reset pool color", poolColor);
		btnCuePointColor = Button(canvas, 450, IMG_H, 50, PANEL_H, "C_P_C", cuePntColor);
		btnCueStickColor = Button(canvas, 500, IMG_H, 50, PANEL_H, "C_S_C", cueStkColor);
	}
	catch (Exception e)	{cout << "GUI���� : " << e.msg << endl;}
	 
	//FPSǥ�� ���ڿ� �ʱ�ȭ
	char strBuf[STRBUFFER] = { 0, };

	//get logo img
	logoImg = imread("ballcheatLogo.png", IMREAD_UNCHANGED);
	resize(logoImg, logoImg, Size(300, 300));

	//Capture loop (to srcImg)
	while (capture.read(srcImg))
	{
		try
		{
			resize(srcImg, srcImg, Size(IMG_W, IMG_H));
			cvtColor(srcImg, hsvImg, COLOR_BGR2HSV);

			//�籸�� ��ġ�� ������ ���� �� �����̸�
			if (poolSet && poolColorSet && cuePntSet && cueStkSet)
			{
				//Region of Interest setting
				#pragma region RoI ����
				if (poolPos[0].x - roiRange < 0) roiRange = poolPos[0].x;
				if (poolPos[0].y - roiRange < 0) roiRange = poolPos[0].y;
				if (poolPos[1].x + roiRange >= srcImg.cols) roiRange = srcImg.cols - poolPos[1].x;
				if (poolPos[1].y + roiRange >= srcImg.rows) roiRange = srcImg.rows - poolPos[1].y;

				poolPosROI[0].x = poolPos[0].x - roiRange;
				poolPosROI[0].y = poolPos[0].y - roiRange;
				poolPosROI[1].x = poolPos[1].x + roiRange;
				poolPosROI[1].y = poolPos[1].y + roiRange;
				poolROIWidth = abs(poolPosROI[1].x - poolPosROI[0].x);
				poolROIHeight = abs(poolPosROI[1].y - poolPosROI[0].y);
				#pragma endregion
			
				procImg = hsvImg(Rect(poolPosROI[0], poolPosROI[1]));
				outImg = Mat(Size(procImg.cols, procImg.rows), CV_8U, Scalar(0, 0, 0));
				//�����ϰ�
				/*Mat sharpen_kernel = (Mat_<char>(3, 3) << 0, -1, 0,
														-1, 5, -1,
														0, -1, 0);
				filter2D(srcImg, srcImg, srcImg.depth(), sharpen_kernel);*/
				
				//�̹��� �׷��̽����� ��ȯ
				/*cvtColor(srcImg, gryImg, COLOR_BGR2GRAY, 0);
				threshold(gryImg, thdImg, 0, 255, THRESH_BINARY_INV + THRESH_OTSU);*/

				inRange(procImg, Scalar(poolColor[0] - poolRangeH, poolColor[1] - poolRangeS, poolColor[2] - poolRangeV),
								 Scalar(poolColor[0] + poolRangeH, poolColor[1] + poolRangeS, poolColor[2] + poolRangeV), thdImg);
				inRange(procImg, Scalar(0, 0, 140), Scalar(179, 70, 255), thdWhiteImg);
				thdImg = ~thdImg;

				//#pragma region �ν� ROI ����
				//Point wbPosFull(WhiteBallPos.x + poolPosROI[0].x, WhiteBallPos.y + poolPosROI[0].y);
				//Rect whiteBallRect(wbPosFull.x - whiteBallROIRange, wbPosFull.y - whiteBallROIRange, whiteBallROIRange * 2, whiteBallROIRange * 2);
				//if (wbPosFull.x - whiteBallROIRange < 0) whiteBallRect.x = wbPosFull.x;
				//if (wbPosFull.y - whiteBallROIRange < 0) whiteBallRect.y = wbPosFull.y;
				//if (wbPosFull.x + whiteBallROIRange >= hsvImg.cols) whiteBallRect.width = hsvImg.cols - wbPosFull.x;
				//if (wbPosFull.y + whiteBallROIRange >= hsvImg.rows) whiteBallRect.height = hsvImg.rows - wbPosFull.y;
				//rectangle(srcImg, whiteBallRect, Scalar(0, 255, 255));
				//#pragma endregion 

				inRange(procImg, Scalar(cuePntColor[0] - cuePntCRangeH, cuePntColor[1] - cuePntCRangeS, cuePntColor[2] - cuePntCRangeV),
								 Scalar(cuePntColor[0] + cuePntCRangeH, cuePntColor[1] + cuePntCRangeS, cuePntColor[2] + cuePntCRangeV), proc_cuePImg);
				inRange(procImg, Scalar(cueStkColor[0] - cueStkCRangeH, cueStkColor[1] - cueStkCRangeS, cueStkColor[2] - cueStkCRangeV),
								 Scalar(cueStkColor[0] + cueStkCRangeH, cueStkColor[1] + cueStkCRangeS, cueStkColor[2] + cueStkCRangeV), proc_cueSImg);
				

				//morph -> ����� ���ְ� ������ ���ְ�
				morphOpCl(thdImg, 5, 5);
				//erode(thdImg, thdImg, getStructuringElement(MORPH_ELLIPSE, Size(20, 20))); <- ħ�Ŀ����� ����غ��� �����Ű���.
				morphOpCl(thdWhiteImg, 3, 3);
				morphOpCl(proc_cuePImg, 5, 5);
				morphOpCl(proc_cueSImg, 5, 5);
				
				GaussianBlur(thdImg, thdImg, Size(3, 3), 2, 2);

				imshow("Threshold", thdImg);

				//�̹��� ����þȺ� ����
				GaussianBlur(thdWhiteImg, thdWhiteImg, Size(3, 3), 2,2);
				
				imshow("Threshold_W", thdWhiteImg);

				//Hough Circles ��(�籸��) ���� (circles�� ��´�)
				vector<Vec3f> circles;				
				HoughCircles(thdImg, circles, CV_HOUGH_GRADIENT, 1, DIST_BALL, cParam1, cParam2, MAX_BALL_SIZE, 1);
				vector<Vec3f> circlesWhite;
				HoughCircles(thdWhiteImg, circlesWhite, CV_HOUGH_GRADIENT, 1, DIST_BALL, cParam1, cParam2W , MAX_BALL_SIZE, 1);
				
				//get position and direction of cue shaft (use labelling)
				#pragma region ť�� �߽��� ����(�󺧸�)
				int numOfLables, max, idx;
				Point pPoint, pStick;

				numOfLables = connectedComponentsWithStats(proc_cuePImg, img_lbl, stats, centroids, 8, CV_32S);
				max = -1; idx = 0;
				for (int i = 1; i < numOfLables; i++)
				{
					int area = stats.at<int>(i, CC_STAT_AREA);
					if (max < area)
					{
						max = area;
						idx = i;
					}
				}
				pPoint = Point(centroids.at<double>(idx, 0), centroids.at<double>(idx, 1));
				circle(srcImg, Point(pPoint.x + poolPosROI[0].x, pPoint.y + poolPosROI[0].y), 3, Scalar(255, 0, 255), 3);

				numOfLables = connectedComponentsWithStats(proc_cueSImg, img_lbl, stats, centroids, 8, CV_32S);
				max = -1; idx = 0;
				for (int i = 1; i < numOfLables; i++)
				{
					int area = stats.at<int>(i, CC_STAT_AREA);
					if (max < area)
					{
						max = area;
						idx = i;
					}
				}
				pStick = Point(centroids.at<double>(idx, 0), centroids.at<double>(idx, 1));
				circle(srcImg, Point(pStick.x + poolPosROI[0].x, pStick.y + poolPosROI[0].y), 3, Scalar(255, 255, 0), 3);
				#pragma endregion
				proc_cueSImg.copyTo(proc_cuePImg, proc_cueSImg);
				imshow("Cue", proc_cuePImg);

				//�� �׸���
				if (drawLine) {
	
					float ang = calcAngleFromPoints(pPoint, pStick, true);

					Point pStart = pPoint, *pEnd = NULL;
					
					for (int i = 0; i < NUMBER_OF_LINE; i++)
					{
						//get end point of current line
						if (!pEnd && pStart.y != roiRange)
							pEnd = calcEndOfLinePoint(pStart, ang, Point(roiRange, roiRange), Point(poolWidth + roiRange, roiRange));	//Top
						if (!pEnd && pStart.x != poolWidth + roiRange)
							pEnd = calcEndOfLinePoint(pStart, ang, Point(poolWidth + roiRange, roiRange), Point(poolWidth + roiRange, poolHeight + roiRange));	//Right
						if (!pEnd && pStart.y != poolHeight + roiRange)
							pEnd = calcEndOfLinePoint(pStart, ang, Point(roiRange, poolHeight + roiRange), Point(poolWidth + roiRange, poolHeight + roiRange));	//Bottom
						if (!pEnd && pStart.x != roiRange)
							pEnd = calcEndOfLinePoint(pStart, ang, Point(roiRange, roiRange), Point(roiRange, poolHeight + roiRange));	//Left

						if (!pEnd)
							break;
						arrowedLine(srcImg, poolPosROI[0] + pStart, poolPosROI[0] + *pEnd, Scalar(0, 0, 255));
						line(outImg, pStart, *pEnd, Scalar(255, 255, 255));

						//assign end point to start point for calculate expected next line
						pStart = *pEnd;

						//revert angle
						if(pStart.x == roiRange || pStart.x == roiRange + poolWidth)
							ang = 3.14159262 - ang;	//if startpoint is in left or right edge
						else if(pStart.y == roiRange || pStart.y == roiRange + poolHeight)
							ang = ang * -1;			//if startoint is in top or bottom edge

						//rest end point
						pEnd = NULL;
						
					}

				}

				//�� �׸���
				#pragma region Draw Circles (all balls)
				Point pntGuideline1(roiRange + DIST_BALL / 2.3, roiRange + DIST_BALL / 2.3);
				Point pntGuideline2(roiRange + poolWidth - DIST_BALL / 2.3, roiRange + poolHeight - DIST_BALL / 2.3);
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
					if (norm(curCircleP - Point(roiRange, roiRange)) < DIST_BALL ||
						norm(curCircleP - Point(roiRange + poolWidth, roiRange + poolHeight)) < DIST_BALL ||
						norm(curCircleP - Point(roiRange, roiRange + poolHeight)) < DIST_BALL ||
						norm(curCircleP - Point(roiRange + poolWidth, roiRange)) < DIST_BALL)
					{
						circles.erase(itc);
						itc = circles.begin();
						continue;
					}

					//��Ƴ��� ���� �׸���
					circle(outImg, Point((*itc)[0], (*itc)[1]), (*itc)[2] * radiusMultiply, Scalar(255,255,255), 2);
					circle(srcImg, Point((*itc)[0]+poolPosROI[0].x, (*itc)[1]+poolPosROI[0].y), (*itc)[2], Scalar(255,0,0), 2);

					++itc;
				}
				#pragma endregion

				#pragma region Draw White Circle
				//���
				vector<Vec3f>::const_iterator itcW = circlesWhite.begin();
				float maxRadius = 0;
				while (itcW != circlesWhite.end())
				{
					Point curCircleP = Point((*itcW)[0], (*itcW)[1]);
					//�籸�� �������� ����Ȱ� ����
					if (norm(curCircleP - Point(roiRange, roiRange)) < DIST_BALL ||
						norm(curCircleP - Point(roiRange + poolWidth, roiRange + poolHeight)) < DIST_BALL ||
						norm(curCircleP - Point(roiRange, roiRange + poolHeight)) < DIST_BALL ||
						norm(curCircleP - Point(roiRange + poolWidth, roiRange)) < DIST_BALL)
					{
						circlesWhite.erase(itcW);
						itcW = circlesWhite.begin();
						continue;
					}

					//�ִ� �������� �����Ѵ�
					if((*itcW)[2] > maxRadius)
						maxRadius = (*itcW)[2];
					else
					{
						//�ִ���������� ������ �����.
						circlesWhite.erase(itcW);
						itcW = circlesWhite.begin();
						continue;
					}

					++itcW;
				}
				//�� ���� �ϳ��� �׸���.
				if (circlesWhite.size() > 0)
				{	
					whiteBallLostCount = 0;
					WhiteBallPos = Point(circlesWhite[0][0], circlesWhite[0][1]);
				} 
				else whiteBallLostCount += 1;
				
				if (whiteBallLostCount < WHITE_BALL_TRACK_LIMIT)
				{
					circle(outImg, Point(WhiteBallPos.x, WhiteBallPos.y), MAX_BALL_SIZE + 20, Scalar(255, 255, 255), 15);
					circle(srcImg, Point(WhiteBallPos.x + poolPosROI[0].x, WhiteBallPos.y + poolPosROI[0].y), MAX_BALL_SIZE + 5, Scalar(0, 255, 0), 2);
				}
				else cout << "White Ball Lost" << endl;
				#pragma endregion

				//Draw edge of pool (refer to poolPos and poolPosROI)
				rectangle(srcImg, Rect(poolPosROI[0], poolPosROI[1]), Scalar(0, 0, 255), 1);
				rectangle(srcImg, Rect(poolPos[0], poolPos[1]), Scalar(255, 255, 255), 2);
				rectangle(srcImg, Rect(pntGuideline1 + poolPosROI[0], pntGuideline2 + poolPosROI[0]), Scalar(0, 255, 255), 1);
				rectangle(outImg, Rect(roiRange,roiRange, poolWidth, poolHeight), Scalar(255, 255, 255), 2);

				//Output perspective transform (warp image)
				#pragma region perspectiveTransform ��ȯ
				Point2f p1[4], p2[4];
				p1[0] = Point2f(0, 0);
				p1[1] = Point2f(outImg.cols, 0);
				p1[2] = Point2f(outImg.cols, outImg.rows-1);
				p1[3] = Point2f(0, outImg.rows-1);

				p2[0] = Point2f(perspectiveParameterTop, 0);
				p2[1] = Point2f(outImg.cols-1 - perspectiveParameterTop, 0);
				p2[2] = Point2f(outImg.cols-1 - perspectiveParameterBottom, outImg.rows-1);
				p2[3] = Point2f(perspectiveParameterBottom, outImg.rows-1);
				Mat m = getPerspectiveTransform(p1, p2);
				warpPerspective(outImg, outImg, m, Size(outImg.cols, outImg.rows));
				#pragma endregion
			}
			//���� �籸�� ��ġ �������̸�
			else if((poolColorSet || cuePntSet || cueStkSet) && !poolSet)
			{
				//���ڼ�(���̵����)�� �׸���.
				line(srcImg, Point(0, mY), Point(IMG_W, mY), Scalar(100, 100, 100));
				line(srcImg, Point(mX, 0), Point(mX, IMG_H), Scalar(100, 100, 100));

				//�ΰ� ����̹����� �׸���.
				#pragma region Logo Output
				outImg = Mat(Size(srcImg.cols, srcImg.rows), CV_32S , Scalar(0, 0, 0));
				//outImg = Mat(srcImg.rows, srcImg.cols, CV_32S);
				//cvtColor(outImg, outImg, CV_BGR2BGRA);
				Mat logoRoi = outImg(Rect(outImg.cols / 2 - logoImg.cols / 2, outImg.rows / 2 - logoImg.rows / 2, logoImg.cols, logoImg.rows));
				int nr = logoImg.rows, nc = logoImg.cols;

				for (int i = 0; i < nr; i++)
				{
					Vec4b *logoData = logoImg.ptr<Vec4b>(i);
					Vec4b *roiData = logoRoi.ptr<Vec4b>(i);

					for (int j = 0; j < nc; j++)
					{
						if (logoData[j][3] > 0) {
							double bgAlpha = (255 - logoData[j][3]) / 255.0;
							double fgAlpha = (logoData[j][3]) / 255.0;

							roiData[j][0] = roiData[j][0] * bgAlpha + logoData[j][0] * fgAlpha;
							roiData[j][1] = roiData[j][1] * bgAlpha + logoData[j][1] * fgAlpha;
							roiData[j][2] = roiData[j][2] * bgAlpha + logoData[j][2] * fgAlpha;
						}
					}
				}
				#pragma endregion
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

		//press any key to escape program
		if (cvWaitKey(33) >= 27) break;
	}
	srcImg.release();
	outImg.release();
	gryImg.release();
	procImg.release();
	thdImg.release();
	logoImg.release();
	thdWhiteImg.release();
	capture.release();
	cvDestroyAllWindows();
	
	return 0;
}
