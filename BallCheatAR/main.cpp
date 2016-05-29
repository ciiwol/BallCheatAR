#include "stdafx.h"
#include "setting.h"
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
Rect btnResetPoolPos;


//���� �����쿡 �׷��� ĵ����
Mat3b canvas;

//ī�޶� �������� ������ ����
Mat srcImg;
Mat procImg;
Mat outImg;

//ȭ�� �� ���콺 ��ġ
int mX, mY;

//ȭ�鿡 ���콺 ������ ���� ���
void callBackFunc(int event, int x, int y, int flags, void* userdata)
{
	mX = x;
	mY = y;

	//�籸�� ���¹�ư
	if (btnResetPoolPos.contains(Point(x, y)))
	{
		if (poolSet)	//�籸�밡 �����Ǿ��ִ� ��쿡�� ���¹�ư�� ������
		{
			if (event == EVENT_LBUTTONDOWN)
			{

			}
			if (event == EVENT_LBUTTONUP)
			{
				cout << "reset Pool Position" << endl;
				for (int i = 0; i < 2; i++)
					poolPos[i] = Point(-1, -1);
				poolSet = false;
				canvas(btnResetPoolPos) = Vec3b(200, 200, 200);
				putText(canvas(btnResetPoolPos), "Pool Position Resetting(LT)",
					Point(btnResetPoolPos.width*0.02, btnResetPoolPos.height*0.7),
					FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
			}
		}
	}
	
	else
	{
		
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
				canvas(btnResetPoolPos) = Vec3b(200, 200, 200);
				putText(canvas(btnResetPoolPos), "Pool Position Resetting(RD)",
					Point(btnResetPoolPos.width*0.02, btnResetPoolPos.height*0.7),
					FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
				if (poolSetNumber == 2)
				{
					poolSetNumber = 0;
					poolSet = true;
					canvas(btnResetPoolPos) = Vec3b(200, 200, 200);
					putText(canvas(btnResetPoolPos), format("Reset Pool Pos", poolSetNumber + 1),
						Point(btnResetPoolPos.width*0.1, btnResetPoolPos.height*0.7),
						FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));
				}
			}
		}
	}

}

int main()
{
	cerr << cv::getBuildInformation();

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
	namedWindow("Display", CV_WINDOW_FULLSCREEN);

	resizeWindow("Main", IMG_W, IMG_H + PANEL_H);

	//���콺 �ݹ��Լ� ����
	setMouseCallback("Main", callBackFunc);

	//��ü�� ����
	try
	{
		canvas = Mat3b(IMG_H+PANEL_H, IMG_W, Vec3b(0, 0, 0));

		//��ư
		btnResetPoolPos = Rect(0, IMG_H, 230, PANEL_H);
		canvas(btnResetPoolPos) = Vec3b(200, 200, 200);
		putText(canvas(btnResetPoolPos), "Pool Position Resetting(1)",
			Point(btnResetPoolPos.width*0.02, btnResetPoolPos.height*0.7),
			FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 0));

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
			//����� �׸� �̹���
			outImg = Mat(Size(srcImg.cols, srcImg.rows), srcImg.type(), Scalar(0, 0, 0));

			//�籸�� ��ġ�� ���� �� �����̸�
			if (poolSet)
			{
				//�����ϰ�
				/*Mat sharpen_kernel = (Mat_<char>(3, 3) << 0, -1, 0,
														-1, 5, -1,
														0, -1, 0);
				filter2D(srcImg, srcImg, srcImg.depth(), sharpen_kernel);*/

				//�̹��� �׷��̽����� ��ȯ
				cvtColor(srcImg, procImg, COLOR_BGR2GRAY, 0);

				//�̹��� ����þȺ� ����
				//GaussianBlur(procImg, procImg, Size(3, 3), 0);

				//�� �迭
				vector<Vec3f> circles;

				//�� ���� (circles�� ��´�)
				HoughCircles(procImg, circles, CV_HOUGH_GRADIENT, 1, DIST_BALL, 60, 10, MAX_BALL_SIZE, MIN_BALL_SIZE);

				//����� �� �� �����ڸ��� �پ��ְų� �����ڸ� �Ѿ ������ �� �����
				//�׸��� �������� �ʴ� ���� 
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
					circle(outImg, Point((*itc)[0], (*itc)[1]), (*itc)[2] * 2, Scalar(255,255,255), 2);
					circle(srcImg, Point((*itc)[0], (*itc)[1]), (*itc)[2], Scalar(255,0,0), 2);

					++itc;
				}

				//�籸�� �����ڸ� ���� �׸���
				rectangle(srcImg, Rect(poolPos[0], poolPos[1]), Scalar(255, 255, 255), 2);
				rectangle(srcImg, Rect(pntGuideline1, pntGuideline2), Scalar(0, 255, 255), 2);
				rectangle(outImg, Rect(poolPos[0], poolPos[1]), Scalar(255, 255, 255), 2);
				/*for (int i = 0; i < 4; i++)
				{
					line(srcImg, poolPos[i], poolPos[i + 1 >= 4 ? 0 : i + 1], Scalar(0, 255, 0), 2);
					line(outImg, poolPos[i], poolPos[i + 1 >= 4 ? 0 : i + 1], Scalar(255,255,255), 2);
				}*/
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
	capture.release();
	cvDestroyAllWindows();
	
	return 0;
}

