// opencvudp.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "opencv2/opencv.hpp"
#include <opencv2/dnn/dnn.hpp>

#include <iostream>
#include <string>


#include <WS2tcpip.h>
#include <WINSOCK2.H>

#include <process.h>
#pragma comment(lib,"ws2_32.lib")
using namespace cv;
using namespace std;
using namespace dnn;


int main()
{
	////加载套接字库
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	//创建套接字
	SOCKET sockClient = socket(AF_INET, SOCK_DGRAM, 0);
	SOCKADDR_IN addrServer;
	inet_pton(AF_INET, "192.168.1.103", (PVOID *)(&addrServer.sin_addr.S_un.S_addr));
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(6000);

	char *Buff = new char [3];//UDP发送的char

	String prototxt = "MobileNetSSD_deploy.prototxt";
	String caffemodel = "MobileNetSSD_deploy.caffemodel";

	Net net = readNetFromCaffe(prototxt, caffemodel);

	const char* classNames[] = { "background", "aeroplane", "bicycle", "bird", "boat", "bottle", "bus", "car", "cat", "chair",
		"cow", "diningtable", "dog", "horse", "motorbike", "person", "pottedplant", "sheep", "sofa", "train", "tvmonitor" };

	float detect_thresh = 0.10;
	VideoCapture cap("http://192.168.1.103:8080/?action=stream;dummy=param.mjpg");
	//if (!cap.isOpened()) return -1;
	Mat frame;
	int xLeftBottom ;
	int yLeftBottom ;
	int xRightTop ;
	int yRightTop ;
	while (true)
	{
		cap >> frame;
		if (frame.empty()) break;
		clock_t start_t = clock();
		net.setInput(blobFromImage(frame, 1.0 / 127.5, Size(300, 300), Scalar(127.5, 127.5, 127.5), false, false));
		Mat cvOut = net.forward();
		cout << "Cost time: " << clock() - start_t << endl;
		Mat detectionMat(cvOut.size[2], cvOut.size[3], CV_32F, cvOut.ptr<float>());
		for (int i = 0; i < detectionMat.rows; i++)
		{
			int obj_class = detectionMat.at<float>(i, 1);
			float confidence = detectionMat.at<float>(i, 2);

			if (confidence > detect_thresh)
			{
				size_t objectClass = (size_t)(detectionMat.at<float>(i, 1));

				 xLeftBottom = static_cast<int>(detectionMat.at<float>(i, 3) * frame.cols);
				 yLeftBottom = static_cast<int>(detectionMat.at<float>(i, 4) * frame.rows);
				 xRightTop = static_cast<int>(detectionMat.at<float>(i, 5) * frame.cols);
				 yRightTop = static_cast<int>(detectionMat.at<float>(i, 6) * frame.rows);

				ostringstream ss;
				int tmpI = 100 * confidence;
				ss << tmpI;
				//ss << confidence;
				String conf(ss.str());

				Rect object((int)xLeftBottom, (int)yLeftBottom,
					(int)(xRightTop - xLeftBottom),
					(int)(yRightTop - yLeftBottom));
				if (classNames[objectClass] == "cat")
				{
					rectangle(frame, object, Scalar(0, 0, 255), 2);
					//String label = String(classNames[objectClass]) + ": " + conf;
					//String label = String(classNames[objectClass]);
					String label = String(classNames[objectClass]) + ": " + conf + "%";
					//putText(image, label, Point(xLeftBottom, yLeftBottom - 10), 3, 1.0, Scalar(0, 0, 255), 2);
					putText(frame, label, Point(xLeftBottom, yLeftBottom + 30), 3, 1.0, Scalar(0, 0, 255), 2);
					
					Buff[0] = 'C';//这是给UDP的buff
					Buff[1] = 'A';
					Buff[2] = 'T';
					
					printf("%c	%c	%c\n", Buff[0], Buff[1], Buff[2]);
					//要发送的字符串
					char *charBuff = (char *)Buff;//把数组变成char数组
													 //UDP发送char数组
					sendto(sockClient, charBuff, strlen(charBuff) + 1, 0, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
				}
			}
		}
		imshow("test", frame);
		if (cv::waitKey(1) > 1) break;
	}
    return 0;
}

