#include "..\..\src\cqp.h"
#include "..\..\src\Path.h"	
#include <vector>
#include <io.h>
#include <algorithm>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <string>

#define FillTime 600

#define STOP 10

#define MAX_ORDER_SIZE JPath["MAX_ORDER_SIZE"]
#define MIN_ORDER_SIZE JPath["MIN_ORDER_SIZE"]
#define SEPMARK string(JPath["SEPMARK"])

using namespace std;

void DetectFolder(string Path);	//检测并创建文件夹

int RecieveMessage(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char *msg);	//处理接收到的消息

int cmp(const string& x, const string& y);

void GetFiles(string Path, vector<string>& Files, string Exd, bool If_Detect_Subdir);

int GetEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char* msg);

int DetectEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char* msg);

void SearchOrder(const char* msg, string& Order, int64_t* DesQQ);

void SearchOrder(const char* msg, string& Order, string& Contents);

int SetBasicEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, string Order, string& Send);

void ClearEvent(int64_t fromGroup, int64_t fromQQ);