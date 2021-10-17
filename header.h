#ifndef HEADER_H
#define HEADER_H

#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<cstring>
#include<cstdlib>
#include<cstdio>
#include<ctime>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<functional>
#include<fstream>
#include<vector>
#include<unistd.h>
#include<cstring>
#include<sys/types.h>
#include<sys/stat.h>
#include<malloc.h>
#include<thread>
#include<random>
#include<sstream>
#include<pthread.h>
#include<fcntl.h>
#include<signal.h>
#include<sys/prctl.h>
#include<vector>
#include<math.h>
#include<netdb.h>

#define PORT 1234
#define RTT 15                  // RTT 15 ms 資料傳遞來回時間
#define THRESHOLD 65536         // threshold 64 Kbytes
#define MSS 1024                // MSS 1 Kbyte 封包資料的大小
#define BUFFER_SIZE 524288      // the receiver’s buffer size 512 Kbytes
#define LOCAL_IP "127.0.0.1"    

using namespace std;

typedef struct{
	unsigned short srcport;    //來源端
	unsigned short desport;    //目的端
	unsigned int seq;   
	unsigned int ack;
	unsigned int fin;
    unsigned int request;
	unsigned short checksum;
	int datalen;
    int checkbit[2];	//0:SYN	1:ACK
    char data[MSS];
}packet;

#endif
