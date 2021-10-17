#include "functions.h"
#include "../../src/HttpPost.h"
#include "openssl/hmac.h"
#include <time.h>

#define BanTime JPath["BanTime"][0]

#define NO_AUTH 0
#define AUTH_OK 1

#define PornTh JPath["porn_th"]
#define AddAndDetectWarningMsg JPath["DetectBanWords"]["warningmsg"]
#define DetectOrder JPath["DetectBanWords"]["orderlist"]

#define EventPath(groupid) _InfoDataPath(groupid,true).append(JPath["Event"])
#define GroupOptionsPath(groupid) _InfoDataPath(groupid,true).append(JPath["GroupOptList"])

#define BasicWarningMsg JPath["BasicWarningMsg"]

/***********AddLog&DetectBanWords***********/

int GetAuth(int64_t ac,int64_t fromGroup,int64_t fromQQ)
{
	try
	{
		int SubPerm = getPermission(ac, fromGroup, fromQQ);
		int MyPerm = getPermission(ac, fromGroup, ac);
		//if (Host == fromQQ)	return NO_AUTH;
		if (1 == MyPerm)
			return NO_AUTH;
		if (MyPerm <= SubPerm)
			return NO_AUTH;
	}
	catch (exception e)
	{
		api->OutputLog(string("addlog--GetAuth:").append(e.what()));
		return NO_AUTH;
	}
	return AUTH_OK;
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

void AddLog(int64_t msgId, int64_t fromGroup, string contents = "")
{
	string Path = LogDataPath(fromGroup, true);
	DetectFolder(Path);
	Path.append(to_string(msgId));
	ofstream Log(__UTA(Path));
	if (!Log)
		return;
	if (contents.length())
		Log << contents;
	Log.close();
}

/*
void GetMemInfo(int32_t ac, int64_t fromGroup, int64_t fromQQ)
{
	CQ_Type_GroupMember MemInfo;
	CQTool::GetGroupMemberInfo(ac, fromGroup, fromQQ, MemInfo);
	string encodeStr = CQ_getGroupMemberInfoV2(ac, fromGroup, fromQQ, false);
	string decodeStr = base64_decode(encodeStr);
	ofstream MemData("E:\\Robot Data\\MemData");
	MemData << "GroupID:" << (int)MemInfo.GroupID << endl;
	MemData << "QQID:" << (int)MemInfo.QQID << endl;
	MemData << "Name:" << MemInfo.username << endl;
	MemData << "GroupName:" << MemInfo.nick << endl;
	MemData << "Sex(0-Male,1-Female):" << MemInfo.sex << endl;
	MemData << "Age:" << MemInfo.age << endl;
	MemData << "Area:" << MemInfo.area << endl;
	MemData << "JointTime:" << MemInfo.jointime << endl;
	MemData << "LastSentTime:" << MemInfo.lastsent << endl;
	MemData << "GivenName:" << MemInfo.level_name << endl;
	MemData << "Permission(1-Normal,2-Admin,3-Host):" << MemInfo.permission << endl;
	MemData << "GetNick:" << MemInfo.GetName() << endl;
	MemData << "DecodeStr:" << decodeStr << endl;
	MemData.close();
}
*/

string GetAnonymousName(const string &fromAnonymous)
{
	const string AnonyDecode = base64_decode(fromAnonymous);
	unsigned int Size = (AnonyDecode[8] << 8) | AnonyDecode[9];
	return AnonyDecode.substr(10, Size);
}

int _AddLog(int64_t ac, int64_t msgId, int64_t random, int32_t req, int64_t fromGroup, int64_t fromQQ, const char *msg, string fromAnonymous)
{
	try
	{
		string contents = to_string(random);
		contents.append("\n").append(to_string(req)).append("\n").append(to_string(fromQQ)).append("\n");
		time_t timer = time(NULL);
		tm TM;
		localtime_s(&TM, &timer);
		contents.append(to_string(TM.tm_year + 1900)).append("-")
			.append(to_string(TM.tm_mon + 1)).append("-")
			.append(to_string(TM.tm_mday)).append(" ")
			.append(to_string(TM.tm_hour)).append(":")
			.append(to_string(TM.tm_min)).append(":")
			.append(to_string(TM.tm_sec)).append("\n");
		contents.append(msg);
		AddLog(msgId, fromGroup, contents);
	}
	catch (exception e)
	{
		api->OutputLog(string("addlog--_AddLog:").append(e.what()));
		return 0;
	}
	if (ac == fromQQ)
		return 1;
	return fromAnonymous.length();
}

int RecieveMessage(int64_t ac, int64_t msgId, int64_t random, int64_t req, int64_t fromGroup, int64_t fromQQ, const char *msg,const char *fromAnonymous)
{
	try
	{
		if (_AddLog(ac, msgId, random, req, fromGroup, fromQQ, msg, fromAnonymous))
			return 1;
		if (DetectBanwords(ac, msgId, fromGroup, fromQQ, msg))
			return 1;
		if (Syscall(ac, fromQQ, fromGroup, msg, msgId, true))
			return 1;
		if (DetectEvent(ac, fromGroup, fromQQ, msg))
			return 1;
		if (GetEvent(ac, msgId, fromGroup, fromQQ, msg))
			return 1;
		return 0;
	}
	catch (exception e)
	{
		api->OutputLog(e.what());
		return 1;
	}
}

void ShowBanWordsList(int64_t ac, int64_t fromGroup, int64_t fromQQ)
{
	try
	{
		string Send = JPath["AT_QQ"];
		fstream BanWordsData;
		string Path = _InfoDataPath(fromGroup, true);
		Path.append(JPath["BanWordsData"]);
		Send.append(to_string(fromQQ)).append(JPath["CodeEnd"]).append("\n");
		BanWordsData.open(__UTA(Path), ios::in);
		if (!BanWordsData)
			Send.append(AddAndDetectWarningMsg[0]);
		else
		{
			string BanWords;
			for (; !BanWordsData.eof();)
			{
				char Temp[128] = { 0 };
				BanWordsData.getline(Temp, sizeof(Temp));
				if (strcmp(Temp, "") == 0)
					continue;
				BanWords += Temp;
				BanWords += "\n";
			}
			BanWordsData.close();
			Send.append(BanWords);
		}
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	}
	catch (exception e)
	{
		api->OutputLog(string("addlog--ShowBanWordsList:").append(e.what()));
	}
}

int DetectBanwords(int64_t ac, int64_t msgId, int64_t fromGroup, int64_t fromQQ, const char *msg)
{
	try
	{
		string Path = _InfoDataPath(fromGroup, true);
		string Opt = GroupOptionsPath(fromGroup).append(JPath["DetectBanWords"]["corefile"]).append(JPath["optexp"]);
		fstream opt(__UTA(Opt), ios::in);
		if (!opt)
			return 0;
		opt.close();
		string mesg = msg;
		fstream BanWordsData;
		Path.append(JPath["BanWordsData"]);
		int Option = -1;
		char Order[16] = { 0 };
		if ((Option = mesg.find(DetectOrder[0])) == 0)
			strcpy_s(Order, string(DetectOrder[0]).c_str());
		else if ((Option = mesg.find(DetectOrder[1])) == 0)
			strcpy_s(Order, string(DetectOrder[1]).c_str());
		else if ((Option = mesg.find(DetectOrder[2])) == 0)
		{
			ShowBanWordsList(ac, fromGroup, fromQQ);
			return 1;
		}
		if (Option == 0)
		{
			OptionBanWordsData(ac, fromGroup, fromQQ, Path, Option + strlen(Order), Order, mesg);
			return 1;
		}
		if (GetAuth(ac, fromGroup, fromQQ) == NO_AUTH)
			return 0;
		BanWordsData.open(__UTA(Path), ios::in);
		if (!BanWordsData)
			return 0;
		char BanWords[128] = { 0 };
		for (; !BanWordsData.eof();)
		{
			memset(BanWords, 0, sizeof(BanWords));
			BanWordsData.getline(BanWords, sizeof(BanWords), '\n');
			if (strlen(BanWords) == 0)
				continue;
			if (mesg.find(BanWords) != -1)
			{
				BanWordsData.close();
				string Send = JPath["AT_QQ"];
				Send.append(to_string(fromQQ)).append(JPath["CodeEnd"]).append("\n").append(AddAndDetectWarningMsg[1]);
				CQ_deleteMsg(ac, msgId, fromGroup, true);
				CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
				CQ_setGroupBan(ac, fromGroup, fromQQ, BanTime);
				return 1;
			}
		}
		BanWordsData.close();
	}
	catch (exception e)
	{
		api->OutputLog(string("addlog--DetectBanWords:").append(e.what()));
	}
	return 0;
}

void OptionBanWordsData(int64_t ac, int64_t fromGroup, int64_t fromQQ, string Path, int Pos, char *Order, string mesg)
{
	try
	{
		int SubPerm = getPermission(ac, fromGroup, fromQQ);
		string Send = JPath["AT_QQ"];
		Send.append(to_string(fromQQ)).append(JPath["CodeEnd"]).append("\n");
		if ((SubPerm <= 1) && fromQQ != JPath["Host"])	/**********/
		{
			Send.append(BasicWarningMsg[1]);
			CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
			return;
		}
		fstream BanWordsData;
		if (strcmp(Order, string(DetectOrder[0]).c_str()) == 0)
		{
			BanWordsData.open(__UTA(Path), ios::out | ios::app);
			if (!BanWordsData)
			{
				BanWordsData.close();
				string New = NewFolder(_InfoDataPath(fromGroup, true));
				system(New.c_str());
				BanWordsData.open(__UTA(Path), ios::out | ios::app);
			}
			string KeyWord;
			for (; mesg[Pos] == ' '; Pos++);
			for (; Pos < mesg.length(); Pos++)
				KeyWord.push_back(mesg[Pos]);
			BanWordsData << KeyWord << endl;
			BanWordsData.close();
			Send.append(AddAndDetectWarningMsg[2]).append(KeyWord).append(AddAndDetectWarningMsg[3]);
		}
		else if (strcmp(Order, string(DetectOrder[1]).c_str()) == 0)
		{
			int Count = 0;
			int KeyCount = 0;
			string KeyWord;
			string List = "";
			for (; mesg[Pos] == ' '; Pos++);
			for (; Pos < mesg.length(); Pos++)
				KeyWord.push_back(mesg[Pos]);
			BanWordsData.open(__UTA(Path), ios::in);
			for (; !BanWordsData.eof();)
			{
				char Temp[128] = { 0 };
				BanWordsData.getline(Temp, sizeof(Temp), '\n');
				if ((strcmp(Temp, KeyWord.c_str()) == 0))
				{
					Count++;
					continue;
				}
				if (!strlen(Temp))
					continue;
				List.append(Temp).append("\n");
				KeyCount++;
			}
			BanWordsData.close();
			BanWordsData.open(__UTA(Path), ios::out | ios::ate);
			BanWordsData << List;
			BanWordsData.close();
			if (!KeyCount)
			{
				string DEL = DelFile(Path);
				system(DEL.c_str());
			}
			if (Count)
				Send.append(AddAndDetectWarningMsg[2]).append(KeyWord).append(AddAndDetectWarningMsg[4]);
			else
				Send.append(AddAndDetectWarningMsg[5]).append(KeyWord).append(AddAndDetectWarningMsg[6]);
		}
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	}
	catch (exception e)
	{
		api->OutputLog(string("addlog--OptionBanWordsData:").append(e.what()));
	}
}

bool DetectPorn(string path, int &porn)
{
	try
	{
		FILE* Img = fopen(__UTA(path).c_str(), "rb");
		if (!Img)
			return true;
		fseek(Img, 0, SEEK_END);
		unsigned long Size = ftell(Img);
		rewind(Img);
		char* buffer = new char[Size * sizeof(char) + 1];
		fread_s(buffer, Size * sizeof(char), sizeof(char), Size, Img);
		fclose(Img);
		string Imgencode = base64_encode(buffer, Size);
		delete[] buffer;
		string Origin = "";
		unsigned long t = time(NULL);
		srand(t);
		unsigned int r = rand();
		unsigned int e = t + JPath["LiveTime"];
		Origin.append("u=").append(JPath["YoutuQQ"])
			.append("&a=").append(JPath["YoutuAppID"])
			.append("&k=").append(JPath["YoutuSecretID"])
			.append("&e=").append(to_string(e))
			.append("&t=").append(to_string(t))
			.append("&r=").append(to_string(r))
			.append("&f=");
		unsigned char HASH[1024] = { 0 };
		unsigned int len = 0;
		HMAC(EVP_sha1(), string(JPath["YoutuSecretKey"]).c_str(), ((string)JPath["YoutuSecretKey"]).length(), (const unsigned char*)Origin.c_str(), Origin.length(), HASH, &len);
		string sign = base64_encode(((string)(char*)HASH).append(Origin).c_str(), len + Origin.length());
		string Get;
		Json Post, Root;
		Post["app_id"] = JPath["YoutuAppID"];
		Post["image"] = Imgencode;
		string p = Post;
		if (GetURL(JPath["YoutuPornServer"], "", receive_data, &Get, 20, PostHTTPHeaders,
			"Content-Type: text/json", ((string)"Authorization:").append(sign).c_str(), "", p.c_str()))
			return true;
		Root = Json::parse(Get);
		if (Root.is_null())
			return true;
		if (Root["errorcode"])
			return true;
		for (int i = 0; i < Root["tags"].size(); i++)
			if (!strcmp(string(Root["tags"][i]["tag_name"]).c_str(), "porn"))
			{
				if ((porn = Root["tags"][i]["tag_confidence"]) >= PornTh)
					return true;
				else
					return false;
			}
	}
	catch (exception e)
	{
		api->OutputLog(string("addlog--DetectPorn:").append(e.what()));
		return false;
	}
}


/*****************SystemCall***************/

int re = 0;

/*static char syscall[][16] =
{ "sendg","sendp" ,"off","on","cmd",
"clear","read" ,"api" };*/

#define syscall JPath["Syscall"]["syscall"]
#define funcname JPath["Syscall"]["funcname"]
#define CallCount syscall.size()
#define FuncCount funcname.size()
#define sysmark string(JPath["Syscall"]["mark"])
#define allmark string(JPath["Syscall"]["allmark"])
#define privatemark string(JPath["Syscall"]["privatemark"])
#define SyscallMsg JPath["Syscall"]["warningmsg"]

/*
@@sgm/spm:发送群聊/私聊消息
@Param
int64_t gid uid
string content content
bool anony **
@@dgm/dpm:撤回群聊/私聊消息
@Param
int64_t gid uid
int64_t random random
int32_t req req
int32_t ** time
@@upi/ugi/upa/uga:上传私聊/群聊图片/音乐
@Param
int64_t uid gid uid gid
string path path path path
bool flash flash ** **
@@sgn:设置群名片
@Param
int64_t gid
int64_t uid
string nickname
@@rgm:删除群成员
@Param
int64_t gid
int64_t uid
@@ban/ban_a:禁言/全群禁言
@Param
int64_t gid gid
int64_t uid **
int32_t time **
bool ** ban_all
@@ugf:上传群文件
@Param
int64_t gid
string path
@@like:点赞
@Param
int64_t uid
@@gda:获取图片下载地址
@string text
*/
/*static char funcname[][16] =
{ "sgm","spm","dgm","dpm","upi",
"ugi","upa","uga","sgn","rgm",
"ban","ban_a","ugf","like"
};*/

int GetSysCall(int64_t ac, string msg, int& No, int64_t& Num, string& Content)
{
	string call;
	int pos = -1;
	pos = msg.find(sysmark);
	if (!pos)
		call.assign(msg.substr(pos + sysmark.length(), msg.find_first_of(" ") - sysmark.length()));
	else
		return 0;
	for (No = 0; No < CallCount; No++)
		if (!strcmp(string(syscall[No]).c_str(), call.c_str()))
			break;
	if (No == CallCount)
		return 0;
	pos = msg.find('<') + 1;
	if (pos)
	{
		string cNum = msg.substr(pos, msg.find_first_of(">", pos) - pos);
		if (!(Num = _atoi64(cNum.c_str())))
			if (!strcmp(allmark.c_str(), cNum.c_str()))
				Num = -1;
	}
	pos = msg.find('{') + 1;
	if (!pos)
		return 0;
	Content.assign(msg, pos);
	return 1;
}

int Syscall(int64_t ac, int64_t fromQQ, int64_t fromGroup, const char* msg, int64_t msgId, bool Ifgroup)
{
	int No = -1;
	int64_t Obj = fromGroup;
	string Call = "";
	if (!GetSysCall(ac, msg, No, Obj, Call))
		return 0;
	if (Ifgroup)
		if (getPermission(ac, fromGroup, fromQQ) < 2)
			return 1;
	AnalisisCall(ac, Call);
	switch (No)
	{
	case 0:
	{
		re = CQ_sendGroupMsg(ac, Obj, Call.c_str());
		if (re)
			SendResult(ac, fromGroup, re, string(SyscallMsg[0]));
		else
			SendResult(ac, fromGroup, re, string(SyscallMsg[1]));
		break;
	}
	case 1:
	{
		re = CQ_sendPrivateMsg(ac, Obj, Call.c_str());
		if (re)
			SendResult(ac, fromGroup, re, string(SyscallMsg[0]));
		else
			SendResult(ac, fromGroup, re, string(SyscallMsg[1]));
		break;
	}
	case 2:
	case 3:
		SetOptions(ac, Obj, fromGroup, Call.c_str(), No - 2);
		break;
	case 4:
	{
		if (fromQQ != JPath["Host"])
			return 0;
		cmd(ac, fromGroup, Call.c_str());
		break;
	}
	case 5:
		Clear(ac, Obj, fromGroup, Call.c_str(), msgId);
		break;
	case 6:
		ReadFile(ac, fromGroup, Call.c_str());
		break;
	case 7:
		if (fromQQ != JPath["Host"])
			return 0;
		UserAPI(ac, fromGroup, Call);
		break;
	default:
		return 0;
		break;
	}
	return 1;
}

void cmd(int64_t ac, int64_t fromGroup, const char* Call)
{
	FILE* fc = _popen(Call, "r");
	char buffer[1024] = { 0 };
	string Res = "Res:\n";
	int rs = -1, total = 0;
	while (rs)
	{
		Res.append(buffer);
		memset(buffer, 0, 1024);
		total += (rs = fread_s(buffer, 1024, sizeof(char), 1023, fc));
	}
	Res.insert(4, to_string(total));
	_pclose(fc);
	fromGroup ? CQ_sendGroupMsg(ac, fromGroup, Res.c_str()) : CQ_sendPrivateMsg(ac, JPath["Host"], Res.c_str());
}

void SendResult(int64_t ac, int64_t fromGroup, int res, string text)
{
	char Res[16] = { 0 };
	string Send = "Res:";
	itoa(res, Res, 10);
	Send.append(Res).append("\n").append(text);
	if (fromGroup)
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	else
		CQ_sendPrivateMsg(ac, JPath["Host"], Send.c_str());
}

/*
void AddLog(int64_t msgId, int64_t fromGroup)
{
	string Path = LogDataPath(fromGroup, true);
	DetectFolder(Path);
	Path.append(to_string(msgId));
	ofstream Log(Path);
	if (!Log)
		return;
	Log.close();
}
*/

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
					Files.push_back(FileInfo.name);
		} while (_findnext(Find, &FileInfo) == 0);
	}
	_findclose(Find);
}

void SetOptions(int64_t ac, int64_t Obj, int64_t fromGroup, const char* options, bool IfOn)
{
	string opts = options;
	int NotAll = strcmp(options, allmark.c_str());
	if (!NotAll)
		opts = "*";
	string sour = OptData;
	sour.append(opts.append(JPath["optexp"]));
	fstream List(__UTA(sour), ios::in);
	if ((!List) && NotAll)
		return SendResult(ac, fromGroup, 0, SyscallMsg[2]);
	List.close();
	string dest = _InfoDataPath(fromGroup, true);
	dest.append(JPath["GroupOptList"]);
	if (IfOn)
	{
		DetectFolder(__UTA(dest));
		syscopy(sour, dest);
		List.open(__UTA(dest.append(opts)).c_str(), ios::in);
		if ((!List) && NotAll)
			SendResult(ac, fromGroup, Obj, SyscallMsg[3]);
		else
		{
			List.close();
			SendResult(ac, fromGroup, Obj, SyscallMsg[4]);
		}
		return;
	}
	else
	{
		string DEL = DelFile(dest.append(opts));
		system(DEL.c_str());
		SendResult(ac, fromGroup, Obj, SyscallMsg[5]);
		return;
	}
}

void Clear(int64_t ac, int64_t Obj, int64_t fromGroup, const char* DelCount, int64_t beginId)
{
	string Temp = DelCount;
	int ifp = Temp.find(privatemark) + 1;
	int Count = atoi(ifp ? Temp.substr(0, ifp - 1).c_str() : DelCount);
	if (!Count)
		return SendResult(ac, fromGroup, 0, SyscallMsg[6]);
	string Path = LogDataPath(fromGroup, true);
	vector<string> Files;
	GetFiles(Path, Files, "", false);
	if (Count >= Files.size())
		Count = Files.size() - 1;
	sort(Files.begin(), Files.end(), cmp);
	int Pos = Files.size() - 2;
	for (; Pos >= 0; Pos--)
		if (beginId >= atoi(Files[Pos].c_str()))
			break;
	for (int i = Count; i > 0; i--)
		CQ_deleteMsg(ac, _atoi64(Files[Pos--].c_str()), Obj, !ifp);
	SendResult(ac, fromGroup, Count, SyscallMsg[7]);
}

void AnalisisCall(int64_t ac, int64_t fromGroup, string& Call)
{
	string path;
	if (Call.find(JPath["Symbol_rec"]) + 1)
	{
		string REC = JPath["Symbol_rec"];
		path = Call.assign(Call, Call.find(REC) + REC.length());
		return (void)(Call = CQ_getCQCode(ac, path.c_str(), CQ_REC));
	}
	int Offset = 0;
	int Endset = 0;
	string IMG = JPath["Symbol_img"];
	while (Offset = Call.find(IMG, Offset) + 1)
	{
		Call.erase(Offset - 1, IMG.length());
		if (Endset = Call.find(IMG, Offset) + 1)
			Call.erase(Endset - 1, IMG.length());
		else
			Endset = Call.length() + 1;
		path.assign(Call.substr(Offset - 1, Endset - Offset));
		Call.erase(Offset - 1, Endset - Offset).insert(Offset - 1, CQ_getCQCode(ac, path.c_str(), CQ_IMG));
	}
}

void ReadFile(int64_t ac, int64_t fromGroup, const char* Filename)
{
	CQ_sendGroupMsg(ac, fromGroup, Filename);
	FILE* F = fopen(__UTA(Filename).c_str(), "r");
	fseek(F, 0, SEEK_END);
	unsigned long Fsize = ftell(F);
	rewind(F);
	char* buffer = new char[Fsize + 1];
	fread_s(buffer, (size_t)Fsize + 1, sizeof(char), (size_t)Fsize, F);
	fclose(F);
	CQ_sendGroupMsg(ac, fromGroup, buffer);
	delete[] buffer;
}

void UserAPI(int64_t ac, int64_t fromGroup, string Contents)
{
	Json JParam;
	JParam["uid"] = fromGroup;
	JParam["gid"] = fromGroup;
	unsigned int pos = Contents.find(" ");
	string func = Contents.substr(0, pos);
	string Param = Contents.substr(pos + 1);
	CQ_sendGroupMsg(ac, fromGroup, Param.c_str());
	try
	{
		JParam = Json::parse(Param);
	}
	catch (...)
	{
		CQ_sendGroupMsg(ac, fromGroup, string(SyscallMsg[8]).c_str());
		return;
	}
	CQ_sendGroupMsg(ac, fromGroup, string(SyscallMsg[9]).c_str());
	int count = 0;
	for (; count < FuncCount; count++)
		if (!strcmp(func.c_str(), string(funcname[count]).c_str()))
			break;
	if (FuncCount == count)
	{
		CQ_sendGroupMsg(ac, fromGroup, string(SyscallMsg[10]).c_str());
		return;
	}
	string Res;
	try
	{
		switch (count)
		{
		case 0:
		case 1:
		{
			int64_t Rand = 0;
			int32_t Req = 0;
			Res = (0 == count ?
				api->SendGroupMessage(ac, JParam["gid"], JParam["content"], JParam["anony"]) :
				api->SendFriendMessage(ac, JParam["uid"], JParam["content"]));
			Res.append("\nrandom:").append(to_string(Rand)).append("\nreq:").append(to_string(Req));
			break;
		}
		case 2:
		case 3:
		{
			Res = (2 == count ?
				api->Undo_Group(ac, JParam["gid"], JParam["random"], JParam["req"]) :
				api->Undo_Private(ac, JParam["uid"], JParam["random"], JParam["req"], JParam["time"]));
			break;
		}
		case 4:
		case 5:
		case 6:
		case 7:
		{
			FILE* File = fopen(__UTA(JParam["path"]).c_str(), "rb");
			if (!File)
				Res = string(BasicWarningMsg[3]);
			else
			{
				fseek(File, 0, SEEK_END);
				unsigned long fsize = ftell(File);
				uint8_t* f_buff = new uint8_t[fsize + 1];
				memset(f_buff, 0, fsize + 1);
				rewind(File);
				fread_s(f_buff, fsize, sizeof(uint8_t), fsize, File);
				fclose(File);
				Res = (count % 2 == 0 ?
					(count == 4 ?
						api->UploadFriendImage(ac, JParam["uid"], f_buff, fsize, JParam["flash"]) :
						api->UploadFriendAudio(ac, JParam["uid"], f_buff, fsize)) :
					(count == 5 ?
						api->UploadGroupImage(ac, JParam["gid"], f_buff, fsize, JParam["flash"]) :
						api->UploadGroupAudio(ac, JParam["gid"], f_buff, fsize)));
				delete[] f_buff;
			}
			break;
		}
		case 8:
		{
			Res = api->SetGroupNickname(ac, JParam["gid"], JParam["uid"], JParam["nickname"]);
			break;
		}
		case 9:
		{
			Res = api->RemoveGroupMember(ac, JParam["gid"], JParam["uid"], false);
			break;
		}
		case 10:
		case 11:
		{
			Res = 10 == count ?
				api->ShutUpGroupMember(ac, JParam["gid"], JParam["uid"], JParam["time"]) :
				api->ShutUpAll(ac, JParam["gid"], JParam["ban_all"]);
			break;
		}
		case 12:
		{
			Res = api->UploadGroupFile(ac, JParam["gid"], JParam["path"]);
			break;
		}
		case 13:
		{
			Res = api->QQLike(ac, JParam["uid"]);
			break;
		}
		case 14:
			Res = api->GetImageDownloadLink(JParam["text"], ac, fromGroup);
			break;
		default:
			return;
		}
	}
	catch (exception e)
	{
		ofstream File(__UTA(string(JPath["RobotRoot"]).append(JPath["ErrorFile"])).c_str());
		File << e.what() << endl;
		File.close();
		return;
	}
	CQ_sendGroupMsg(ac, fromGroup, Res.c_str());
}


/*************Undo************/

#define EventList JPath["Undo"]["eventlist"]
#define OrderList JPath["Undo"]["orderlist"]
#define OrderCount OrderList.size()
#define EventCount EventList.size()
#define UndoMsg JPath["Undo"]["warningmsg"]

int GetEvent(int64_t ac, int64_t msgId, int64_t fromGroup, int64_t fromQQ, const char* msg)
{
	string Order = "";
	int64_t DesQQ = 0;
	SearchOrder(msg, Order, &DesQQ);
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
	case 0:
	case 1:
		SetOptionMsgEvent(ac, msgId, fromGroup, fromQQ, Order, OrderNo);
		break;
	default:
		return 0;
		break;
	}
	return 1;
}

int DetectEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char* msg)
{
	string Path = EventPath(fromGroup);
	Path.append(to_string(fromQQ));
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
	string Order = "";
	int64_t DesQQ = 0;
	int Option = atoi(msg);
	SearchOrder(msg, Order, &DesQQ);
	int clear = 1;
	switch (No)
	{
	case 0:
	case 1:
	{
		char BeginPos[24] = { 0 };
		EventData.getline(BeginPos, sizeof(BeginPos));
		clear = OptionMsg(ac, fromGroup, fromQQ, BeginPos, msg, No);
		break;
	}
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

void SetOptionMsgEvent(int64_t ac, int64_t msgId, int64_t fromGroup, int64_t fromQQ, string Order, bool If_Look)
{
	int SubPerm = getPermission(ac, fromGroup, fromQQ);
	string Send = JPath["AT_QQ"];
	Send.append(to_string(fromQQ)).append(JPath["CodeEnd"]).append("\n");
	if ((SubPerm < 2) && (fromQQ != JPath["Host"]))	/**********/
	{
		Send.append(BasicWarningMsg[1]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		return;
	}
	if (!If_Look)
	{
		int MyPerm = getPermission(ac, fromGroup, ac);
		if (MyPerm < 2)
		{
			Send.append(BasicWarningMsg[2]);
			CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
			return;
		}
	}
	string Data = Order;
	Data.append("\n").append(to_string(msgId));
	if (!SetBasicEvent(ac, fromGroup, fromQQ, Data, Send))
		return;
	Send.append(UndoMsg[3]);
	CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
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

int OptionMsg(int64_t ac, int64_t fromGroup, int64_t fromQQ, char* BeginPos, const char* msg, bool If_Look)
{
	int Count = atoi(msg);
	if (!Count)
		return 0;
	string Send = JPath["AT_QQ"];
	Send.append(to_string(fromQQ)).append(JPath["CodeEnd"]).append("\n");
	string Path = LogDataPath(fromGroup, true);
	vector<string> Files;
	GetFiles(Path, Files, "", false);
	if (Count >= Files.size())
		Send.append(UndoMsg[0]);
	else
	{
		sort(Files.begin(), Files.end(), cmp);
		int Pos = Files.size() - 2;
		for (; Pos >= 0; Pos--)
			if (strcmp(BeginPos, Files[Pos].c_str()) == 0)
				break;
		Pos -= Count;
		if (If_Look)
		{
			Path += Files[Pos];
			ifstream Data(__UTA(Path));
			if (!Data)
			{
				Send.append(UndoMsg[1]);
				CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
			}
			else
			{
				int64_t random, QQNum;
				int32_t req;
				Data >> random;
				Data >> req;
				Data >> QQNum;
				string name = api->GetGroupNickname(ac, fromGroup, QQNum);
				Send.append(name.length() ? name : api->GetNameForce(ac, QQNum)).append("\n");
				for (; !Data.eof();)
					Send += Data.get();
			}
			Send[Send.length() - 1] = '\0';
			Data.close();
		}
		else
		{
			int64_t msgId = _atoi64(Files[Pos].c_str());
			CQ_deleteMsg(ac, msgId, fromGroup, true);
			Send.append(UndoMsg[2]);
		}
	}
	CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	return 1;
}