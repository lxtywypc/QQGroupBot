#pragma once
#include "sdk/third_party/json.hpp"
#include "sdk/sdk.h"

extern nlohmann::json JPath;

/*
#define LikeDataPath "E:\\Robot Data\\Like Data\\" 
#define CardDataPath "E:\\Robot Data\\Card Data\\"
#define GroupDataPath "E:\\Robot Data\\Group Data\\"
#define SpGroupDataPath "E:\\Robot Data\\Group Data\\MemAccount\\"
#define TransDataPath "E:\\Robot Data\\Translate Data\\"
#define ImageDataPath "E:\\Robot Data\\Bin\\data\\image\\"
#define CardImagePath "E:\\Robot Data\\Card Image\\"
#define DLImagePath "E:\\Robot Data\\Bin\\data\\image\\DownloadImage\\"
#define WebDLPath "E:\\Robot Data\\WebDownload\\"
#define MakeGifData "E:\\Robot Data\\MakeGif\\"
#define RobotBin "E:\\Robot Data\\Bin\\bin\\"
#define OptData "E:\\Robot Data\\Options List\\"
#define TimerDataPath "E:\\Robot Data\\Timer\\"

#define AT_QQ "[CQ:at,qq="	//@ĳ��ԱCQ�룬���Ŀ��QQ���мǲ�ĩβ����
#define IMAGE "[CQ:image,file="	//����ͼƬCQ�룬����ļ������мǲ�ĩβ����

//��������̶��ļ�·��
#define CardSetDataPath "E:\\Robot Data\\Card Data\\SetData"
#define TransInfoPath "E:\\Robot Data\\Translate Data\\TranslateInfo"
#define TimeDataPath "E:\\Robot Data\\Timer"
#define CQIMGPath "E:\\Robot Data\\bin\\data\\image\\*.cqimg"
#define ClassRatePath "E:\\Robot Data\\Card Data\\ClassRate"
#define TimenamedImg "E:\\Robot Data\\Bin\\data\\image\\DownloadImage\\*.img.*"
*/

#define __UTA(str) WideCharToANSI(UTF8ToWideChar(str))
#define __ATU(str) WideCharToUTF8(ANSIToWideChar(str))

#define _InfoDataPath(targid,ifgroup) string(JPath["RobotRoot"]).append(ifgroup?JPath["GroupData"]:JPath["PrivateData"]).append(to_string(targid)).append("\\")
#define LogDataPath(targid,ifgroup) _InfoDataPath(targid,ifgroup).append(JPath["Log"])
#define OptData string(JPath["RobotRoot"]).append(JPath["OptData"])
#define EventPath(groupid) _InfoDataPath(groupid,true).append(JPath["Event"])
#define GroupOptionsPath(groupid) _InfoDataPath(groupid,true).append(JPath["GroupOptList"])

#define AT_QQ(qqid) string(JPath["AT_QQ"]).append(to_string(qqid)).append(JPath["CodeEnd"]).append("\n")

#define NewFolder(path) string("md \"").append(__UTA(path)).append("\"")	//�����ļ���������
#define DelFile(path) string("del /F /S /Q \"").append(__UTA(path)).append("\"")	//ɾ���ļ������У����·�����мǲ�ĩβ����
#define execall(root) ((string)("cd /D ")).append(__UTA(root)).append(" &")	//����Ŀ¼��root�Ա����.exe
#define syscopy(sour,dest) system(((string)("copy /Y \"")).append(__UTA(sour)).append("\" \"").append(__UTA(dest)).append("\"").c_str())	//��sour���Ƶ�dest

bool ReadPathFromJson(std::string);