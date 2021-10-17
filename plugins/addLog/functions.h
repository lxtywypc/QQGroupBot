#include "../../src/cqp.h"
#include "../../src/Path.h"	
#include <vector>
#include <io.h>
#include <algorithm>
#include <signal.h>
#include <iostream>
#include <fstream>
#include "../../src/base64.h"
#include <string>

//#define PathCount (sizeof(KeyPath)/64)
//#define DelKeyCount (sizeof(DelKeyPath)/64)
#define FillTime 600

#define STOP 10

#define MAX_ORDER_SIZE JPath["MAX_ORDER_SIZE"]
#define MIN_ORDER_SIZE JPath["MIN_ORDER_SIZE"]
#define SEPMARK string(JPath["SEPMARK"])

using namespace std;

//static const char KeyPath[][64] ={};

//static const char DelKeyPath[][64] ={};

void DetectFolder(string Path);	//检测并创建文件夹

void AddLog(int64_t msgId, int64_t fromGroup, string contents);	//聊天记录

int RecieveMessage(int64_t ac, int64_t msgId, int64_t random, int64_t req, int64_t fromGroup, int64_t fromQQ, const char *msg, const char *fromAnonymous);	//处理接收到的消息

int DetectBanwords(int64_t ac, int64_t msgId, int64_t fromGroup, int64_t fromQQ, const char *msg);	//检测敏感词

void OptionBanWordsData(int64_t ac, int64_t fromGroup, int64_t fromQQ, string Path, int Pos, char *Order, string mesg);	//添加或删除敏感词

void ShowBanWordsList(int64_t ac, int64_t fromGroup, int64_t fromQQ);	//显示敏感词列表

/*********Syscall**********/

int GetSysCall(int64_t ac, string msg, int& No, int64_t& Num, string& Content); //分离各部

int Syscall(int64_t ac, int64_t fromQQ, int64_t fromGroup, const char* msg, int64_t msgId, bool Ifgroup);	//检测命令

void SendResult(int64_t ac, int64_t fromGroup, int res, string text);	//消息反馈

void SetOptions(int64_t ac, int64_t Obj, int64_t fromGroup, const char* options, bool IfOn);	//修改功能列表

void DetectFolder(string Path);

int cmp(const string& x, const string& y);

void GetFiles(string Path, vector<string>& Files, string Exd, bool If_Detect_Subdir);

void Clear(int64_t ac, int64_t Obj, int64_t fromGroup, const char* DelCount, int64_t beginId);	//清屏

void AnalisisCall(int64_t ac, string& Call);	//检测图片和音频

void cmd(int64_t ac, int64_t fromGroup, const char* Call);

void ReadFile(int64_t ac, int64_t fromGroup, const char* Call);

void UserAPI(int64_t ac, int64_t fromGroup, string Contents);

/*****************Undo***************/

int GetEvent(int64_t ac, int64_t msgId, int64_t fromGroup, int64_t fromQQ, const char* msg);

int DetectEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char* msg);

void SearchOrder(const char* msg, string& Order, int64_t* DesQQ);

void SetOptionMsgEvent(int64_t ac, int64_t msgId, int64_t fromGroup, int64_t fromQQ, string Order, bool If_Look);

int SetBasicEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, string Order, string& Send);

int OptionMsg(int64_t ac, int64_t fromGroup, int64_t fromQQ, char* BeginPos, const char* msg, bool If_Look);
