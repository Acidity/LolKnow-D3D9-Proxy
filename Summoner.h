//Copyright 2013 Tyler O'Meara
//Released under the Creative Commons Attribution 3.0 Unported License
//See LICENSE for more details

#pragma once

#include <string>

using namespace std;
class Summoner
{
public:
	Summoner(void);
	~Summoner(void);

	string champion;
	string name;
	int queueGroup;
	string rank;
	string tier;
	double kills;
	double deaths;
	double assists;
	int wins;
	int losses;
	double champKills;
	double champDeaths;
	double champAssists;
	int champPlayed;
};