#include "cqp.h"
#include "sdk/third_party/json.hpp"

#define __LogDataPath(targid,ifgroup) string(JPath["RobotRoot"]).append(ifgroup?JPath["GroupData"]:JPath["PrivateData"]).append(to_string(targid)).append("\\").append(JPath["Log"])
#define __ImageDataPath string(JPath["RobotRoot"]).append(JPath["Bin"]).append(JPath["ImageData"])
#define __RecDataPath string(JPath["RobotRoot"]).append(JPath["Bin"]).append(JPath["RecData"])

using namespace std;

using Json = nlohmann::json;


int64_t CQ_sendPrivateMsg(int64_t AuthCode, int64_t QQID, const char* msg)
{
	string _Res = api->SendFriendMessage(AuthCode, QQID, msg);
	api->OutputLog(_Res);
	Json Res = Json::parse(_Res);
	return Res["retcode"];
}


int64_t CQ_sendGroupMsg(int64_t AuthCode, int64_t groupid, const char* msg)
{
	string _Res = api->SendGroupMessage(AuthCode, groupid, msg);
	api->OutputLog(_Res);
	Json Res = Json::parse(_Res);
	return Res["retcode"];
}


int64_t CQ_sendDiscussMsg(int64_t AuthCode, int64_t discussid, const char* msg)
{
	return 0;
}


int64_t CQ_deleteMsg(int64_t AuthCode, int64_t msgid, int64_t target, bool ifgroup)
{
	int64_t random;
	int32_t req;
	string Path = __LogDataPath(target, ifgroup);
	Path.append(to_string(msgid));
	ifstream Log(__UTA(Path));
	if (!Log)
		return 0;
	Log >> random;
	Log >> req;
	Log.close();
	return ifgroup ? api->Undo_Group(AuthCode, target, random, req) :
		api->Undo_Private(AuthCode, target, random, req, msgid);
}


int64_t CQ_sendLike(int64_t AuthCode, int64_t QQID)
{
	api->OutputLog(api->QQLike(AuthCode, QQID));
	return 0;
}


int64_t CQ_setGroupKick(int64_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL rejectaddrequest)
{
	return api->RemoveGroupMember(AuthCode, groupid, QQID, rejectaddrequest);;
}


int64_t CQ_setGroupBan(int64_t AuthCode, int64_t groupid, int64_t QQID, int64_t duration)
{
	return api->ShutUpGroupMember(AuthCode, groupid, QQID, duration) ? duration : -1;
}


int64_t CQ_setGroupAdmin(int64_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL setadmin)
{
	return api->SetAdministrator(AuthCode, groupid, QQID, setadmin);
}


int64_t CQ_setGroupWholeBan(int64_t AuthCode, int64_t groupid, CQBOOL enableban)
{
	return api->ShutUpAll(AuthCode, groupid, enableban);
}


int64_t CQ_setGroupAnonymousBan(int64_t AuthCode, int64_t groupid, const char* anomymous, int64_t duration)
{
	return 0;
}


int64_t CQ_setGroupAnonymous(int64_t AuthCode, int64_t groupid, CQBOOL enableanomymous)
{
	return api->GroupPermission_Anonymous(AuthCode, groupid, enableanomymous);
}


int64_t CQ_setGroupCard(int64_t AuthCode, int64_t groupid, int64_t QQID, const char* newcard)
{
	api->OutputLog(api->SetGroupNickname(AuthCode, groupid, QQID, newcard));
	return 0; 
}


int64_t CQ_setGroupLeave(int64_t AuthCode, int64_t groupid, CQBOOL isdismiss)
{
	return isdismiss ? api->DissolveGroup(AuthCode, groupid) : api->QuitGroup(AuthCode, groupid);
}


int64_t CQ_setGroupSpecialTitle(int64_t AuthCode, int64_t groupid, int64_t QQID, const char* newspecialtitle, int64_t duration)
{
	return api->SetExclusiveTitle(AuthCode, groupid, QQID, newspecialtitle);
}


int64_t CQ_setDiscussLeave(int64_t AuthCode, int64_t discussid)
{
	return 0;
}


int64_t CQ_setFriendAddRequest(int64_t AuthCode, const char* responseflag, int32_t responseoperation, const char* remark)
{
	return 0;
}


int64_t CQ_setGroupAddRequestV2(int64_t AuthCode, const char* responseflag, int32_t requesttype, int32_t responseoperation, const char* reason)
{
	return 0;
}


GroupMemberInformation CQ_getGroupMemberInfoV2(int64_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL nocache)
{
	try
	{
		vector<GroupMemberInformation> Info;
		api->GetGroupMemberList(AuthCode, groupid, Info);
		for (int i = 0; i < Info.size(); i++)
			if (QQID == _atoi64(Info[i].QQNumber.c_str()))
				return Info[i];
	}
	catch (exception e)
	{
		api->OutputLog(e.what());
	}
}


FriendInformation CQ_getStrangerInfo(int64_t AuthCode, int64_t QQID, CQBOOL nocache)
{
	FriendInformation Info;
	api->GetFriendInformation(AuthCode, QQID, Info);
	return Info;
}


int64_t CQ_addLog(int64_t AuthCode, int32_t priority, const char* category, const char* content)
{
	return 0;
}


const char* CQ_getCookies(int64_t AuthCode)
{
	return "";
}


int64_t CQ_getCsrfToken(int64_t AuthCode)
{
	return 0;
}


int64_t CQ_getLoginQQ(int64_t AuthCode)
{
	return 0;
}


const char* CQ_getLoginNick(int64_t AuthCode)
{
	return "";
}


const char* CQ_getAppDirectory(int64_t AuthCode)
{
	return api->GetPluginDataDirectory().c_str();
}


int64_t CQ_setFatal(int64_t AuthCode, const char* errorinfo)
{
	return 0;
}


string CQ_getCQCode(int64_t AuthCode, string file, const bool CQType)
{
	try
	{
		string Res;
		//api->OutputLog(_file);
		if (-1 == file.find(":"))
			file.insert(0, CQType ? __ImageDataPath : __RecDataPath);
		FILE* sour = fopen(__UTA(file).c_str(), "rb");
		if (!sour)
		{
			Res = string((!CQType) ? "音频" : "图片").append("文件打开失败").append(" path:").append(file);
			api->OutputLog(Res);
			return Res;
		}
		fseek(sour, 0, SEEK_END);
		unsigned long fsize = ftell(sour);
		uint8_t* buffer = new uint8_t[fsize + 1];
		try
		{
			memset(buffer, 0, fsize + 1);
			rewind(sour);
			fread_s(buffer, fsize, sizeof(uint8_t), fsize, sour);
			Res = (!CQType) ? api->UploadGroupAudio(AuthCode, JPath["TempGroup"], buffer, fsize) :
				api->UploadGroupImage(AuthCode, JPath["TempGroup"], buffer, fsize, CQType - 1);
		}
		catch (exception e)
		{
			api->OutputLog(e.what());
		}
		fclose(sour);
		delete[] buffer;
		return Res;
	}
	catch (exception e)
	{
		api->OutputLog(e.what());
		return "GET CODE ERROR";
	}
}

/*
vector<GroupMemberInformation> CQ_getGroupMemberList(int64_t AuthCode, int64_t groupid)
{
	vector<GroupMemberInformation> Info;
	api->GetGroupMemberList(AuthCode, groupid, Info);
	return Info;
}


vector<GroupInformation> CQ_getGroupList(int64_t AuthCode)
{
	vector<GroupInformation> Info;
	api->GetGroupList(AuthCode, Info);
	return Info;
}
*/


int getPermission(int64_t AuthCode, int64_t groupid, int64_t QQID)
{
	string adlist = JPath["Adlist"][to_string(groupid)].is_null() ?
		(JPath["Adlist"][to_string(groupid)] = api->GetAdministratorList(AuthCode, groupid)) :
		string(JPath["Adlist"][to_string(groupid)]);
	int perm = adlist.find(to_string(QQID));
	return ((-1 == perm) ? 1 : ((!perm) ? 3 : 2));
}

int64_t HexstrToDecnum(string Hex)
{
	int64_t Num = 0;
	for (int i = 0; i < Hex.length(); i++)
		if ((Hex[i] >= '0') && (Hex[i] <= '9'))
			Num = Num * 16 + Hex[i] - '0';
		else if ((Hex[i] >= 'a') && (Hex[i] <= 'f'))
			Num = Num * 16 + (Hex[i] - 'a') + 10;
		else if ((Hex[i] >= 'A') && (Hex[i] <= 'F'))
			Num = Num * 16 + (Hex[i] - 'A') + 10;
		else
			return 0;
	return Num;
}

string retransCode(string content)
{
	string u_begin = "\\u00";
	unsigned long int pos = content.find(u_begin, 0);
	while (pos != -1)
	{
		content.replace(pos, u_begin.length() + 2, 1, HexstrToDecnum(content.substr(pos + u_begin.length(), 2)));
		pos = content.find(u_begin, pos + 1);
	}
	return content;
}