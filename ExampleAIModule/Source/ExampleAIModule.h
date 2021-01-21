#pragma once
#include <BWAPI.h>
#include <BWTA.h>
#define CHOKEPOINTDISTANCE 150
// Remember not to use "Broodwar" in any global class constructor!

class ExampleAIModule : public BWAPI::AIModule
{
private:
	static const int MAX_BARRACKS = 2;
	static const int MAX_WORKERS = 14;
	static const int MAX_MARINES = 15;
	static const int MAX_REFINERY = 1;
	static const int MAX_ACADEMIES = 1;
	static const int MAX_MEDICS = 7;
	static const int MAX_UPGRADES = 2;
	static const int MAX_TECH = 3;
	static const int MAX_FACTORIES = 1;
	static const int MAX_SIEGE_TANKS = 10;
	static const int ATTACK_SIZE = 30;
	static const int RETREAT_SIZE = 10;
	int nrOfWorkers;
	int nrOfBarracks;
	int nrOfMarines;
	int nrOfRefineries;
	int nrOfAcademies;
	int nrOfMedics;
	int nrOfUpgrades;
	int nrOfTech;
	int nrOfFactories;
	int nrOfSiegeTanks;
	int nrOfSupplyDepots;
	int totalArmySize;
	BWAPI::UpgradeType upgrades[MAX_UPGRADES];
	BWAPI::TechType tech[MAX_TECH];
	bool stopTraining;
	BWAPI::Position chokePoint;
	BWAPI::TilePosition enemyStart;
	bool attackMode;
	int nrOfDiscovered;
	bool siegeModeResearched;
	BWAPI::Unitset enemies;
	BWAPI::Unitset marines;
	BWAPI::Unitset gasWorkers;
	bool stealthedEnemy;

	BWAPI::Error createBuilding(BWAPI::UnitType type, BWAPI::Unit unit);
	void printErrorAt(BWAPI::Error error, BWAPI::Position pos);
	void drawRectangleAt(BWAPI::TilePosition buildPos, BWAPI::UnitType type);
	void initializeVariables();
	BWAPI::Position getClosestChokePoint();

public:
	// Virtual functions for callbacks, leave these as they are.
	virtual void onStart();
	virtual void onEnd(bool isWinner);
	virtual void onFrame();
	virtual void onSendText(std::string text);
	virtual void onReceiveText(BWAPI::Player player, std::string text);
	virtual void onPlayerLeft(BWAPI::Player player);
	virtual void onNukeDetect(BWAPI::Position target);
	virtual void onUnitDiscover(BWAPI::Unit unit);
	virtual void onUnitEvade(BWAPI::Unit unit);
	virtual void onUnitShow(BWAPI::Unit unit);
	virtual void onUnitHide(BWAPI::Unit unit);
	virtual void onUnitCreate(BWAPI::Unit unit);
	virtual void onUnitDestroy(BWAPI::Unit unit);
	virtual void onUnitMorph(BWAPI::Unit unit);
	virtual void onUnitRenegade(BWAPI::Unit unit);
	virtual void onSaveGame(std::string gameName);
	virtual void onUnitComplete(BWAPI::Unit unit);
	// Everything below this line is safe to modify.

};
