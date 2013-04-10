#include "LolKnow.h"
#include "Windows.h"

using namespace std;

vector<Summoner> LolKnow::teamOne;
vector<Summoner> LolKnow::teamTwo;
int LolKnow::currentTeam = 1;
int LolKnow::lineNumber = 0;
bool LolKnow::completedDataTransfer = false;
bool LolKnow::fontCreated = false;
bool LolKnow::hasCreatedThread = false;

LolKnow::LolKnow(void)
{

}


LolKnow::~LolKnow(void)
{

}

void LolKnow::timerCheckForData(long millis)
{
	while(!checkIfDataAvailable())
	{
		Sleep(millis);
	}
}

bool LolKnow::checkIfDataAvailable()
{
	ifstream input;
	input.open("Temp.txt", ios::in); 
	if(input)
	{
		retrieveDataFromMain();
		return true;
	}
	return false;
}

void LolKnow::retrieveDataFromMain()
{
	ifstream input;
	input.open("Temp.txt", ios::in); 
	string line;

	if(input.is_open())
	{
		while(input.good())
		{
			  getline(input,line);
			  handleInputData(line);
		}
		input.close();
	}
	completedDataTransfer = true;
}

void LolKnow::handleInputData(string line)
{
	if(line == "//BEGIN LOLKNOW DATA TRANSFER" || line == "//END LOLKNOW DATA TRANSFER")
	{
		return;
	}
	if(line == "//BEGIN TEAM ONE")
	{
		currentTeam = 1;
		lineNumber = 0;
		return;
	}
	if(line == "//BEGIN TEAM TWO")
	{
		currentTeam = 2;
		lineNumber = 0;
		return;
	}
	if(line == "-----")
	{
		Summoner s;
		if(currentTeam == 1)
		{
			teamOne.push_back(s);
		}
		if(currentTeam == 2)
		{
			teamTwo.push_back(s);
		}
		lineNumber = 1;
		return;
	}
	Summoner* s;
	if(currentTeam == 1)
	{
		if(teamOne.size() > 0)
		s = &(teamOne.at(teamOne.size()-1));
	}
	if(currentTeam == 2)
	{
		if(teamTwo.size() > 0)
		s = &(teamTwo.at(teamTwo.size()-1));
	}
	stringstream ss;
	switch(lineNumber)
	{
		case 1: (*s).champion = line; break;
		case 2: (*s).name = line; break;
		case 3: try { ss << line; ss >> (*s).queueGroup; }catch(...){} break; //TODO Handle exceptions relating to line not holding a number
		case 4: if(line != "") { (*s).tier = line; }; break;
		case 5: if(line != "") { (*s).rank = line; }; break;
		case 6: try { ss << line; ss >> (*s).kills;  }catch(...){} break;//TODO Handle exceptions relating to line not holding a number
		case 7: try { ss << line; ss >> (*s).deaths;  }catch(...){} break;//TODO Handle exceptions relating to line not holding a number
		case 8: try { ss << line; ss >> (*s).assists;  }catch(...){} break;//TODO Handle exceptions relating to line not holding a number
		case 9: try { ss << line; ss >> (*s).wins;  }catch(...){} break;//TODO Handle exceptions relating to line not holding a number
		case 10: try { ss << line; ss >> (*s).losses;  }catch(...){} break;//TODO Handle exceptions relating to line not holding a number
		case 11: try { ss << line; ss >> (*s).champKills;  }catch(...){} break;//TODO Handle exceptions relating to line not holding a number
		case 12: try { ss << line; ss >> (*s).champDeaths;  }catch(...){} break;//TODO Handle exceptions relating to line not holding a number
		case 13: try { ss << line; ss >> (*s).champAssists;  }catch(...){} break;//TODO Handle exceptions relating to line not holding a number
		case 14: try { ss << line; ss >> (*s).champPlayed;   }catch(...){} break;//TODO Handle exceptions relating to line not holding a number
	}
	lineNumber++;
}