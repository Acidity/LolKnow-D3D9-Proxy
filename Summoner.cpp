//Copyright 2013 Tyler O'Meara
//Released under the Creative Commons Attribution 3.0 Unported License
//See LICENSE for more details

#include "Summoner.h"

Summoner::Summoner(void)
{
	tier = "Unranked in Solo Queue";
	wins = -1;
	losses = -1;
	kills = -1;
	deaths = -1;
	assists = -1;
	champKills = -1;
	champDeaths = -1;
	champAssists = -1;
	champPlayed = -1;
}

Summoner::~Summoner(void)
{

}