#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "opencv2/opencv.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include<pthread.h>
#include <netinet/in.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>  
#include <netdb.h> 
#include <errno.h> 
using namespace cv;
using namespace std;

int main()
{
	Mat frame;
	VideoCapture cap("http://192.168.1.103:8080/?action=stream");
	char filename[256];
	int i;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(6000);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int sock;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	bind(sock, (struct sockaddr *)&addr, sizeof(addr));

	char buff[64];
	struct sockaddr_in clientAddr;
	int n;
	unsigned int len = sizeof(clientAddr);



	while (1)
	{
		n = recvfrom(sock, buff, 64, 0, (struct sockaddr*)&clientAddr, &len);
		if (n > 0)
		{
			char *tmpi = (char*)buff;
			printf("%s %u says: %c %c %c\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), tmpi[0], tmpi[1], tmpi[2]);
			i++;
			cap >> frame;
			sprintf(filename, "/mnt/ud/detec/%06d.jpg", i);
			imwrite(filename, frame);
		}
	}

	return 0;
}

