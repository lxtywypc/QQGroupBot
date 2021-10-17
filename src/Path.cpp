#include "Path.h"
#include <iostream>
#include <fstream>
#include <string>
//#include "sdk/api/api.h"

using namespace std;

//using Json = nlohmann::json;

nlohmann::json JPath;

bool ReadPathFromJson(string Pluginpath)
{
	try
	{
		JPath["_PluginPath"] = Pluginpath;
		ifstream Pathfile(__UTA(Pluginpath.append("..\\..\\..\\plugin\\path.json")));
		if (!Pathfile)
		{
			JPath["error_msg"] = string("file open failed");
			return false;
		}
		JPath = nlohmann::json::parse(Pathfile);
		Pathfile.close();
		return true;
	}
	catch (exception e)
	{
		JPath["error_msg"] = e.what();
		return false;
	}
}
