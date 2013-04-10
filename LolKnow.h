#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Summoner.h"
#include <sstream>

class LolKnow
{
public:
	LolKnow(void);
	~LolKnow(void);
	static void retrieveDataFromMain();
	static void LolKnow::handleInputData(string);
	static std::vector<Summoner> teamOne;
	static std::vector<Summoner> teamTwo;
	static bool completedDataTransfer;
	static bool fontCreated;
	static bool checkIfDataAvailable();
	static void timerCheckForData(long);
	static bool hasCreatedThread;

private:
	static int currentTeam;
	static int lineNumber;
};