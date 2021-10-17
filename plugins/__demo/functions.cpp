#include "functions.h"
#include <time.h>

#define BanTime JPath["BanTime"][0]

#define BasicWarningMsg JPath["BasicWarningMsg"]

#define OrderList JPath[""]["orderlist"]
#define EventList JPath[""]["eventlist"]
#define WarningMsg JPath[""]["warningmsg"]
#define OrderCount OrderList.size()
#define EventCount EventList.size()

int GetEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char* msg)
{
	string Order = "", Contents = "";
	int64_t DesQQ = 0;
//	SearchOrder(msg, Order, );
	int OrderNo;
	for (OrderNo = 0; OrderNo < OrderCount; OrderNo++)
		if (strcmp(string(OrderList[OrderNo]).c_str(), Order.c_str()) == 0)
		{
			string Path = GroupOptionsPath(fromGroup);
			DetectFolder(Path);
			Path.append(Order).append(JPath["optexp"]);
			fstream Opt(__UTA(Path), ios::in);
			if (!Opt)
				return 0;
			Opt.close();
			break;
		}
	switch (OrderNo)
	{
	default:
		return 0;
		break;
	}
	return 1;
}

int DetectEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char* msg)
{
	string Path = EventPath(fromGroup).append(to_string(fromQQ));
	fstream EventData;
	EventData.open(__UTA(Path), ios::in);
	if (!EventData)
	{
		EventData.close();
		return 0;
	}
	char Event[64] = { 0 };
	EventData.getline(Event, sizeof(Event));
	if (strcmp(Event, "") == 0)
		return 0;
	int No = 0;
	for (; No < EventCount; No++)
		if (0 == strcmp(string(EventList[No]).c_str(), Event))
			break;
	if (No == EventCount)
		return 0;
	string Order = "", Contents = "";
	int64_t DesQQ = 0;
	int Option = atoi(msg);
//	SearchOrder(msg, Order, );
	int clear = 1;
	switch (No)
	{
	default:
		return 0;
	}
	EventData.close();
	if (!clear)
	{
		string Send = JPath["AT_QQ"];
		Send.append(to_string(fromQQ)).append(JPath["CodeEnd"]).append("\n");
		if (strcmp(msg, string(JPath["giveuporder"]).c_str()) == 0)
		{
			Send.append(BasicWarningMsg[4]);
			clear = 1;
		}
		else
			Send.append(BasicWarningMsg[5]).append(JPath["giveuporder"]).append(BasicWarningMsg[6]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	}
	if (clear)
	{
		if (clear == STOP)
			return 1;
		EventData.open(__UTA(Path), ios::out | ios::ate);
		EventData.close();
	}
	return 1;
}

void DetectFolder(string Path)
{
	string Test = Path;
	if (Path[Path.length() - 1] != '\\')
		Test += "\\";
	Test.append(JPath["Exist"]);
	ofstream Data(__UTA(Test));
	if (!Data)
	{
		string Create = NewFolder(Path);
		system(Create.c_str());
	}
	Data.close();
}

int RecieveMessage(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char *msg)
{
	try
	{
		if (DetectEvent(ac, fromGroup, fromQQ, msg))
			return 1;
		if (GetEvent(ac, fromGroup, fromQQ, msg))
			return 1;
		return 0;
	}
	catch (exception e)
	{
		api->OutputLog(e.what());
		return 1;
	}
}

int cmp(const string& x, const string& y)
{
	if (x.size() > y.size())
		return 0;
	else if (x.size() < y.size())
		return 1;
	else
	{
		int re = strcmp(y.c_str(), x.c_str());
		return (re + 1) / 2;
	}
}

void GetFiles(string Path, vector<string>& Files, string Exd, bool If_Detect_Subdir)
{
	long Find = 0;
	struct _finddata_t  FileInfo;
	string PathName, ExdName = "*.";
	PathName = Path;
	if (Path[Path.length() - 1] != '\\')
		PathName += "\\";
	if (strcmp(Exd.c_str(), "") != 0)
		ExdName += Exd;
	else
		ExdName += "*";
	if ((Find = _findfirst(PathName.append(ExdName).c_str(), &FileInfo)) != -1)
	{
		do
		{
			if ((FileInfo.attrib & _A_SUBDIR) && If_Detect_Subdir)
			{
				if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
					GetFiles(PathName.append("\\").append(FileInfo.name), Files, Exd, If_Detect_Subdir);
			}
			else
				if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
					Files.push_back(__ATU(FileInfo.name));
		} while (_findnext(Find, &FileInfo) == 0);
	}
	_findclose(Find);
}

void ClearEvent(int64_t fromGroup, int64_t fromQQ)
{
	string Path = EventPath(fromGroup).append(to_string(fromQQ));
	ofstream Event(Path);
	Event.close();
}

void SearchOrder(const char* msg, string& Order, int64_t* DesQQ)
{
	string mesg = msg;
	int pos = mesg.find(string("[@"));
	if (pos > MAX_ORDER_SIZE)
		return;
	if ((pos == -1) && (mesg.length() > MAX_ORDER_SIZE))
		return;
	unsigned int opos = mesg.find_first_of(SEPMARK);
	if (opos)
		Order = mesg.substr(0, opos);
	if (pos != -1)
	{
		pos += string("[@").length();
		*DesQQ = _atoi64(mesg.substr(pos, mesg.find_first_of(string("]"), pos) - pos).c_str());
	}
}

void SearchOrder(const char* msg, string& Order, string& Contents)
{
	string mesg = msg;
	unsigned int pos = mesg.find_first_of(SEPMARK);
	if (pos > MAX_ORDER_SIZE)
	{
		if (mesg.length() < MAX_ORDER_SIZE)
			Order.assign(msg);
		else
			Contents.assign(msg);
		return;
	}
	Order = mesg.substr(0, pos);
	pos = mesg.find_first_not_of(" ", pos);
	Contents.assign((string)msg, pos);
}

int SetBasicEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, string Order, string& Send)
{
	string Path = EventPath(fromGroup);
	DetectFolder(Path);
	Path.append(to_string(fromQQ));
	Send = JPath["AT_QQ"];
	Send.append(to_string(fromQQ)).append(JPath["CodeEnd"]).append("\n");
	ofstream Data(__UTA(Path));
	if (!Data)
	{
		Data.close();
		Send.append(BasicWarningMsg[0]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		return 0;
	}
	Data << Order;
	Data.close();
	return 1;
}
