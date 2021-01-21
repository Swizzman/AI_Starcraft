#include "ExampleAIModule.h"
#include <iostream>

using namespace BWAPI;
using namespace Filter;

Error ExampleAIModule::createBuilding(UnitType type, Unit unit)
{
	BWAPI::Error error;

	if (Broodwar->self()->minerals() >= type.mineralPrice() && Broodwar->self()->gas() >= type.gasPrice())
	{
		static int lastChecked = 0;
		if (lastChecked + 350 < Broodwar->getFrameCount())
		{
			if (type != UnitTypes::Terran_Supply_Depot || Broodwar->self()->incompleteUnitCount(type) == 0)
			{
				Unit builder = unit->getClosestUnit(GetType == type.whatBuilds().first &&
					(IsIdle || IsGatheringMinerals) && IsOwned);

				if (builder)
				{
					TilePosition buildPos = Broodwar->getBuildLocation(type, builder->getTilePosition());

					if (buildPos)
					{
						stopTraining = true;
						drawRectangleAt(buildPos, type);

						if (builder->build(type, buildPos))
						{
							lastChecked = Broodwar->getFrameCount();
						}
					}
				}
			}
		}
	}
	else
	{
		if (Broodwar->self()->minerals() < type.mineralPrice())
		{
			error = Errors::Insufficient_Minerals;
		}
		else
		{
			error = Errors::Insufficient_Gas;
		}
	}

	return error;
}

void ExampleAIModule::printErrorAt(BWAPI::Error error, BWAPI::Position pos)
{
	Broodwar->registerEvent([pos, error](Game*) { Broodwar->drawTextMap(pos, "%c%s", Text::White, error.c_str()); },   // action
		nullptr,    // condition
		Broodwar->getLatencyFrames());  // frames to run
}

void ExampleAIModule::drawRectangleAt(BWAPI::TilePosition buildPos, BWAPI::UnitType type)
{
	Broodwar->registerEvent([buildPos, type](Game*)
	{
		Broodwar->drawBoxMap(Position(buildPos),
			Position(buildPos + type.tileSize()),
			Colors::Blue);
	},
		nullptr,  // condition
		type.buildTime() + 100);  // frames to run
}

void ExampleAIModule::initializeVariables()
{
	this->nrOfDiscovered = 0;
	this->nrOfWorkers = 0;
	this->nrOfBarracks = 0;
	this->nrOfMarines = 0;
	this->nrOfRefineries = 0;
	this->nrOfAcademies = 0;
	this->nrOfMedics = 0;
	this->nrOfUpgrades = 0;
	this->totalArmySize = 0;
	this->nrOfTech = 0;
	this->nrOfFactories = 0;
	this->nrOfSupplyDepots = 0;
	this->upgrades[0] = UpgradeTypes::U_238_Shells;
	this->upgrades[1] = UpgradeTypes::Caduceus_Reactor;
	this->tech[0] = TechTypes::Stim_Packs;
	this->tech[1] = TechTypes::Restoration;
	this->tech[2] = TechTypes::Optical_Flare;
	this->stopTraining = false;
	this->attackMode = false;
	this->chokePoint = getClosestChokePoint();
	this->siegeModeResearched = false;
	this->stealthedEnemy = false;
	//this->gasWorkers = Unitset::none;

	for (auto& startLoc : Broodwar->getStartLocations())
	{
		if (startLoc != Broodwar->self()->getStartLocation())
		{
			enemyStart = startLoc;
		}
	}
}

BWAPI::Position ExampleAIModule::getClosestChokePoint()
{
	double distance = 99999;

	TilePosition startLocTile = Broodwar->self()->getStartLocation();
	Position startLocPos = { startLocTile.x * 32, startLocTile.y * 32 };
	Region chokePoint;

	for (auto regions : Broodwar->getAllRegions())
	{
		if (regions->getDefensePriority() == 2)
		{
			if (startLocPos.getApproxDistance(regions->getCenter()) < distance)
			{
				distance = startLocPos.getApproxDistance(regions->getCenter());
				if (distance > 300 && regions->getID() != 162)
				{
					chokePoint = regions;
				}
			}
		}
	}

	Broodwar->registerEvent([chokePoint](Game*)
	{
		Broodwar->drawBoxMap(Position(chokePoint->getBoundsLeft(), chokePoint->getBoundsTop()),
			Position(chokePoint->getBoundsRight(), chokePoint->getBoundsBottom()),
			Colors::Red);
	},
		nullptr,  // condition
		850);  // frames to run

	return chokePoint->getCenter();
}

void ExampleAIModule::onStart()
{
	//Here is where you program start
	initializeVariables();

	//setLocalSpeed(0) is sets speed to your framerate, higher value == slower gamespeed
	Broodwar->setLocalSpeed(22);

	Broodwar->sendText("Welcome to Boten Anna!");
	// Print the map name.

	// BWAPI returns std::string when retrieving a string, don't forget to add .c_str() when printing!
	Broodwar << "The map is " << Broodwar->mapName().c_str() << "!" << std::endl;

	// Enable the UserInput flag, which allows us to control the bot and type messages.
	Broodwar->enableFlag(Flag::UserInput);

	// Uncomment the following line and the bot will know about everything through the fog of war (cheat).
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	// Set the command optimization level so that common commands can be grouped
	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(2);

	// Check if this is a replay
	if (Broodwar->isReplay())
	{
		// Announce the players in the replay
		Broodwar << "The following players are in this replay:" << std::endl;

		// Iterate all the players in the game using a std:: iterator
		Playerset players = Broodwar->getPlayers();
		for (auto p : players)
		{
			// Only print the player if they are not an observer
			if (!p->isObserver())
				Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
		}

	}
	else // if this is not a replay
	{
		// Retrieve you and your enemy's races. enemy() will just return the first enemy.
		// If you wish to deal with multiple enemies then you must use enemies().
		if (Broodwar->enemy()) // First make sure there is an enemy
			Broodwar << "The matchup is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;
	}

}

void ExampleAIModule::onEnd(bool isWinner)
{
	// Called when the game ends
	if (isWinner)
	{
		// Log your win here!
	}
}

void ExampleAIModule::onFrame()
{
	// Called once every game frame, you AI logic is written here. Remember that you can add .h and .cpp files and include them in the project to make it more readable

	//I recommend if your AI starts to lag to do some operation each n:th frame. 
	//As an example finding enemies and build locations is computeheavy operations and are not needed to be run every frame

	// Display the game frame rate as text in the upper left area of the screen
	Broodwar->drawTextScreen(200, 0, "FPS: %d", Broodwar->getFPS());
	Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());
	// Return if the game is a replay or is paused
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self())
		return;

	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
		return;

	if (totalArmySize > ATTACK_SIZE && !attackMode)
	{
		attackMode = true;
		Broodwar << "Attacking!" << std::endl;
	}
	else if (totalArmySize < RETREAT_SIZE && attackMode)
	{
		attackMode = false;
	}

	// Iterate through all the units that we own
	for (auto& u : Broodwar->self()->getUnits())
	{
		// Ignore the unit if it no longer exists
		// Make sure to include this block when handling any Unit pointer!
		if (!u->exists())
			continue;

		// Ignore the unit if it has one of the following status ailments
		if (u->isLockedDown() || u->isMaelstrommed() || u->isStasised())
			continue;

		// Ignore the unit if it is in one of the following states
		if (u->isLoaded() || !u->isPowered() || u->isStuck())
			continue;

		// Ignore the unit if it is incomplete or busy constructing
		if (!u->isCompleted() || u->isConstructing())
			continue;
		// Finally make the unit do some stuff!
		// If the unit is a worker unit
		switch (u->getType())
		{
		case UnitTypes::Terran_Siege_Tank_Tank_Mode:
		{
			for (auto& enemy : enemies)
			{
				if (u->canSiege() && !u->isSieged() && !enemy->isFlying() && 
					u->getDistance(enemy) <= (WeaponTypes::Arclite_Shock_Cannon.maxRange() - 85) &&
					u->getDistance(enemy) >= WeaponTypes::Arclite_Shock_Cannon.minRange())
				{
					u->siege();
					break;
				}
			}
			if (attackMode && u->isIdle())
			{
				u->attack(Position(enemyStart));
			}
			else if (!attackMode && u->isIdle() && u->getPosition().getDistance(chokePoint) > CHOKEPOINTDISTANCE)
			{
				u->move(chokePoint);
			}
			break;
		}
		case UnitTypes::Terran_Siege_Tank_Siege_Mode:
		{
			bool inRange = false;
			for (auto& enemy : enemies)
			{
				if (u->isInWeaponRange(enemy))
				{
					inRange = true;
					u->attack(enemy);
					break;
				}
			}
			if (u->isSieged() && u->canUnsiege() && !inRange)
			{
				u->unsiege();
			}
			break;
		}
		case UnitTypes::Terran_SCV:
		{
			if (gasWorkers.getClosestUnit(GetType == UnitTypes::Terran_Refinery))
			{
				for (auto& gasGuy : gasWorkers)
				{
					if (!gasGuy->isGatheringGas())
					{
						gasGuy->gather(gasGuy->getClosestUnit(IsRefinery));
					}
				}
			}

			// if our worker is idle
			if (u->isIdle() || (u->isGatheringGas() && !gasWorkers.contains(u)))
			{
				// Order workers carrying a resource to return them to the center,
				// otherwise find a mineral patch to harvest.

				if (u->isCarryingGas() || u->isCarryingMinerals())
				{
					u->returnCargo();
				}
				else if (!u->getPowerUp())  // The worker cannot harvest anything if it
				{                             // is carrying a powerup such as a flag
				  // Harvest from the nearest mineral patch or gas refinery
					if (!u->gather(u->getClosestUnit(IsMineralField)))
					{
						// If the call fails, then print the last error message
						Broodwar << Broodwar->getLastError() << std::endl;
					}
				} // closure: has no powerup
			} // closure: if idle
			break;
		}
		case UnitTypes::Terran_Marine:
		{
			if (attackMode)
			{
				if (u->isIdle())
				{
					u->attack(Position(enemyStart));
				}
				else if (u->isAttacking() && u->getHitPoints() > (UnitTypes::Terran_Marine.maxHitPoints() - 10))
				{
					u->useTech(TechTypes::Stim_Packs, u);
				}
			}
			else
			{
				if (u->isIdle() && u->getPosition().getDistance(chokePoint) > CHOKEPOINTDISTANCE)
				{
					u->move(chokePoint);
				}
			}
			break;
		}
		case UnitTypes::Terran_Medic:
		{
			if (attackMode)
			{
				if (u->isIdle() && u->getDistance(Position(enemyStart)) > 300)
				{
					u->attack(Position(enemyStart));
				}
				else if (u->getDistance(Position(enemyStart)) < 300)
				{
					for(auto& marine : marines)
					{
						if (marine->getHitPoints() < UnitTypes::Terran_Marine.maxHitPoints() && 
							!marine->isBeingHealed() && u->getEnergy() > 5)
						{
							u->attack(marine);
						}
					}
				}
			}
			else
			{
				if (u->isIdle() && u->getPosition().getDistance(chokePoint) > CHOKEPOINTDISTANCE)
				{
					u->move(chokePoint);
				}
			}
			break;
		}
		case UnitTypes::Terran_Command_Center:
		{
			Error lastErr;
			// Pushing out a refinery once we have a barrack
			if (this->nrOfBarracks > 0 && this->nrOfRefineries < this->MAX_REFINERY && this->nrOfSupplyDepots > 1)
			{
				// REFINERY
				lastErr = createBuilding(UnitTypes::Terran_Refinery, u);
				if (lastErr != Errors::None)
				{
					printErrorAt(lastErr, u->getPosition());
				}
			}
			if (this->nrOfBarracks == 2 && u->canBuildAddon()  && this->nrOfAcademies > 0)
			{
				if (Broodwar->self()->gas() > UnitTypes::Terran_Comsat_Station.gasPrice() &&
					Broodwar->self()->minerals() > UnitTypes::Terran_Comsat_Station.mineralPrice())
				{
					if (!u->buildAddon(UnitTypes::Terran_Comsat_Station))
					{
						lastErr = Errors::Insufficient_Space;
						printErrorAt(lastErr, u->getPosition());
					}
				}
			}
			if (this->nrOfBarracks == 2 && this->nrOfAcademies < this->MAX_ACADEMIES)
			{
				// ACADEMY
				lastErr = createBuilding(UnitTypes::Terran_Academy, u);
				if (lastErr != Errors::None)
				{
					printErrorAt(lastErr, u->getPosition());
				}
			}
			if (this->nrOfBarracks == 2 && this->nrOfFactories < this->MAX_FACTORIES)
			{
				// FACTORY
				lastErr = createBuilding(UnitTypes::Terran_Factory, u);
				if (lastErr != Errors::None)
				{
					printErrorAt(lastErr, u->getPosition());
				}
			}
			if (this->nrOfBarracks < this->MAX_BARRACKS)
			{
				// BARRACK
				lastErr = createBuilding(UnitTypes::Terran_Barracks, u);
				if (lastErr != Errors::None)
				{
					printErrorAt(lastErr, u->getPosition());
				}
			}
			// Only train when we are not trying to build 
			// to avoid using up resources before we placed the building
			if (this->nrOfWorkers < this->MAX_WORKERS && u->isIdle() && !stopTraining)
			{
				u->train(u->getType().getRace().getWorker());
			}

			// Order the depot to construct more workers! But only when it is idle.
				// If that fails, draw the error at the location so that you can visibly see what went wrong!
				// However, drawing the error once will only appear for a single frame
				// so create an event that keeps it on the screen for some frames

			// Retrieve the supply provider type in the case that we have run out of supplies

			if ((Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() <= 8 && this->nrOfBarracks > 0))
			{
				// SUPPLY DEPOT
				lastErr = createBuilding(u->getType().getRace().getSupplyProvider(), u);
				if (lastErr != Errors::None)
				{
					printErrorAt(lastErr, u->getPosition());
				}
			}
			break;
		}
		case UnitTypes::Terran_Factory:
		{
			if (u->isIdle())
			{
				if (u->canBuildAddon(UnitTypes::Terran_Machine_Shop) &&
					Broodwar->self()->minerals() >= UnitTypes::Terran_Machine_Shop.mineralPrice() &&
					Broodwar->self()->gas() >= UnitTypes::Terran_Machine_Shop.gasPrice() && !stopTraining)
				{
					u->buildAddon(UnitTypes::Terran_Machine_Shop);
				}
				if (this->nrOfSiegeTanks < this->MAX_SIEGE_TANKS &&
					Broodwar->self()->minerals() >= UnitTypes::Terran_Siege_Tank_Tank_Mode.mineralPrice() &&
					Broodwar->self()->gas() >= UnitTypes::Terran_Siege_Tank_Tank_Mode.gasPrice() &&
					!this->stopTraining && this->siegeModeResearched)
				{
					if (u->train(UnitTypes::Terran_Siege_Tank_Tank_Mode))
					{
						this->totalArmySize++;
						this->nrOfSiegeTanks++; //Exception since siege tanks can switch modes
					}
				}
			}
			break;
		}
		case UnitTypes::Terran_Machine_Shop:
		{
			if (Broodwar->self()->isResearchAvailable(TechTypes::Tank_Siege_Mode) &&
				Broodwar->self()->minerals() >= TechTypes::Tank_Siege_Mode.mineralPrice() &&
				Broodwar->self()->gas() >= TechTypes::Tank_Siege_Mode.gasPrice() && !stopTraining)
			{
				u->research(TechTypes::Tank_Siege_Mode);
				siegeModeResearched = true;
			}
			break;
		}
		case UnitTypes::Terran_Barracks:
		{
			if (this->nrOfMarines < this->MAX_MARINES)
			{
				if (Broodwar->self()->minerals() >= UnitTypes::Terran_Marine.mineralPrice())
				{
					if (u->isIdle() && !stopTraining)
					{
						u->train(UnitTypes::Terran_Marine);
					}
				}
			}
			if (this->nrOfMedics < this->MAX_MEDICS && this->nrOfAcademies > 0)
			{
				if (Broodwar->self()->minerals() >= UnitTypes::Terran_Medic.mineralPrice())
				{
					if (u->isIdle() && !stopTraining)
					{
						u->train(UnitTypes::Terran_Medic);
					}
				}
			}
			break;
		}
		case UnitTypes::Terran_Academy:
		{
			if (u->isIdle() && !stopTraining && this->nrOfSiegeTanks > 0)
			{
				if (this->nrOfUpgrades < this->MAX_UPGRADES &&
					Broodwar->self()->minerals() >= upgrades[nrOfUpgrades].mineralPrice() &&
					Broodwar->self()->gas() >= upgrades[nrOfUpgrades].gasPrice())
				{
					u->upgrade(upgrades[nrOfUpgrades++]);
				}
				else if (this->nrOfUpgrades >= 2 && this->nrOfTech < MAX_TECH &&
					Broodwar->self()->minerals() >= tech[nrOfTech].mineralPrice() &&
					Broodwar->self()->gas() >= tech[nrOfTech].gasPrice())
				{
					u->research(tech[nrOfTech++]);
				}
			}
			break;
		}
		case UnitTypes::Terran_Comsat_Station:
		{
			if (stealthedEnemy)
			{
				stealthedEnemy = false;
				if (u->getEnergy() >= TechTypes::Scanner_Sweep.energyCost())
				{
					u->useTech(TechTypes::Scanner_Sweep, chokePoint);
				}
			}
			break;
		}
		default:
			break;
		}
	} // closure: unit iterator
}

void ExampleAIModule::onSendText(std::string text)
{
	// Send the text to the game if it is not being processed.
	Broodwar->sendText("%s", text.c_str());
	// Make sure to use %s and pass the text as a parameter,
	// otherwise you may run into problems when you use the %(percent) character!
}

void ExampleAIModule::onReceiveText(BWAPI::Player player, std::string text)
{
	// Parse the received text
	Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void ExampleAIModule::onPlayerLeft(BWAPI::Player player)
{
	// Interact verbally with the other players in the game by
	// announcing that the other player has left.
	Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void ExampleAIModule::onNukeDetect(BWAPI::Position target)
{
	// Check if the target is a valid position
	if (target)
	{
		// if so, print the location of the nuclear strike target
		Broodwar << "Nuclear Launch Detected at " << target << std::endl;
	}
	else
	{
		// Otherwise, ask other players where the nuke is!
		Broodwar->sendText("Where's the nuke?");
	}
	// You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
}

void ExampleAIModule::onUnitDiscover(BWAPI::Unit unit)
{
	if (unit->getPlayer() != Broodwar->self())
	{
		if (unit->getType() != UnitTypes::Resource_Vespene_Geyser && 
			unit->getType() != UnitTypes::Resource_Mineral_Field &&
			unit->getType() != UnitTypes::Unknown)
		{
			enemies.insert(unit);
		}
	}
}

void ExampleAIModule::onUnitEvade(BWAPI::Unit unit)
{
	if (unit->getPlayer() != Broodwar->self())
	{
		if (unit->getType() != UnitTypes::Resource_Vespene_Geyser &&
			unit->getType() != UnitTypes::Resource_Mineral_Field &&
			unit->getType() != UnitTypes::Unknown)
		{
			enemies.erase(unit);
		}
	}
}

void ExampleAIModule::onUnitShow(BWAPI::Unit unit)
{
}

void ExampleAIModule::onUnitHide(BWAPI::Unit unit)
{
}

void ExampleAIModule::onUnitCreate(BWAPI::Unit unit)
{
	if (Broodwar->isReplay())
	{
		// if we are in a replay, then we will print out the build order of the structures
		if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
		{
			int seconds = Broodwar->getFrameCount() / 24;
			int minutes = seconds / 60;
			seconds %= 60;
			Broodwar->sendText("%.2d:%.2d: %s creates a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
		}
	}
	else
	{
		if (unit->getPlayer() == Broodwar->self())
		{
			if (unit->getType().isBuilding() && 
				unit->getType() != UnitTypes::Terran_Machine_Shop &&
				unit->getType() != UnitTypes::Terran_Comsat_Station)
			{
				stopTraining = false;
			}
			switch (unit->getType())
			{
			case UnitTypes::Terran_Barracks:
				nrOfBarracks++;
				Broodwar << "Creating: " << unit->getType() << std::endl;
				break;
			case UnitTypes::Terran_Refinery:
				Broodwar << "Creating: " << unit->getType() << std::endl;
				break;
			case UnitTypes::Terran_Supply_Depot:
				this->nrOfSupplyDepots++;
				Broodwar << "Creating: " << unit->getType() << std::endl;
				break;
			case UnitTypes::Terran_Academy:
				this->nrOfAcademies++;
				Broodwar << "Creating: " << unit->getType() << std::endl;
				break;
			case UnitTypes::Terran_Factory:
				this->nrOfFactories++;
				Broodwar << "Creating: " << unit->getType() << std::endl;
				break;
			case UnitTypes::Terran_Machine_Shop:
				Broodwar << "Creating Addon: " << unit->getType() << std::endl;
				break;
			}
		}
	}
}

void ExampleAIModule::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getPlayer() == Broodwar->self())
	{
		switch (unit->getType())
		{
		case UnitTypes::Terran_SCV:
			this->nrOfWorkers--;
			if (gasWorkers.contains(unit))
			{
				gasWorkers.erase(unit);
			}
			Broodwar << "Active " << unit->getType() << ": " << nrOfWorkers << std::endl;
			break;
		case UnitTypes::Terran_Barracks:
			this->nrOfBarracks--;
			Broodwar << "Destroyed " << unit->getType() << "!" << std::endl;
			break;
		case UnitTypes::Terran_Academy:
			this->nrOfAcademies--;
			Broodwar << "Destroyed " << unit->getType() << "!" << std::endl;
			if (unit->isUpgrading())
			{
				this->nrOfUpgrades--;
			}
			else if (unit->isResearching())
			{
				this->nrOfTech--;
			}
			break;
		case UnitTypes::Terran_Marine:
			this->nrOfMarines--;
			this->totalArmySize--;
			this->marines.erase(unit);
			if (this->enemies.empty())
			{
				stealthedEnemy = true;
			}
			Broodwar << "Active " << unit->getType() << ": " << nrOfMarines << std::endl;
			break;
		case UnitTypes::Terran_Refinery:
			this->nrOfRefineries--;
			Broodwar << "Destroyed " << unit->getType() << "!" << std::endl;
			break;
		case UnitTypes::Terran_Medic:
			this->nrOfMedics--;
			this->totalArmySize--;
			if (this->enemies.empty())
			{
				stealthedEnemy = true;
			}
			Broodwar << "Active " << unit->getType() << ": " << nrOfMedics << std::endl;
			break;
		case UnitTypes::Terran_Factory:
			this->nrOfFactories--;
			Broodwar << "Destroyed " << unit->getType() << "!" << std::endl;
			break;
		case UnitTypes::Terran_Machine_Shop:
			Broodwar << "Destroyed " << unit->getType() << "!" << std::endl;
			break;
		case UnitTypes::Terran_Supply_Depot:
			this->nrOfSupplyDepots--;
			Broodwar << "Destroyed " << unit->getType() << "!" << std::endl;
			break;
		case UnitTypes::Terran_Siege_Tank_Tank_Mode:
			this->nrOfSiegeTanks--;
			this->totalArmySize--;
			if (this->enemies.empty())
			{
				stealthedEnemy = true;
			}
			Broodwar << "Destroyed " << unit->getType() << "!" << std::endl;
			break;
		case UnitTypes::Terran_Siege_Tank_Siege_Mode:
			this->nrOfSiegeTanks--;
			this->totalArmySize--;
			Broodwar << "Destroyed " << unit->getType() << "!" << std::endl;
			break;
		}
	}
}

void ExampleAIModule::onUnitMorph(BWAPI::Unit unit)
{
	if (Broodwar->isReplay())
	{
		// if we are in a replay, then we will print out the build order of the structures
		if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
		{
			int seconds = Broodwar->getFrameCount() / 24;
			int minutes = seconds / 60;
			seconds %= 60;
			Broodwar->sendText("%.2d:%.2d: %s morphs a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
		}
	}
	else
	{
		if (unit->getPlayer() == Broodwar->self())
		{
			switch (unit->getType())
			{
			case UnitTypes::Terran_Refinery:
				this->stopTraining = false;
				break;
			}
		}
	}
}

void ExampleAIModule::onUnitRenegade(BWAPI::Unit unit)
{
}

void ExampleAIModule::onSaveGame(std::string gameName)
{
	Broodwar << "The game was saved to \"" << gameName << "\"" << std::endl;
}

void ExampleAIModule::onUnitComplete(BWAPI::Unit unit)
{
	if (unit->getPlayer() == Broodwar->self())
	{
		switch (unit->getType())
		{
		case UnitTypes::Terran_SCV:
			this->nrOfWorkers++;
			if (gasWorkers.size() < 3)
			{
				gasWorkers.insert(unit);
			}
			Broodwar << "Active " << unit->getType() << ": " << nrOfWorkers << std::endl;
			break;
		case UnitTypes::Terran_Marine:
			this->nrOfMarines++;
			this->totalArmySize++;
			this->marines.insert(unit);
			Broodwar << "Active " << unit->getType() << ": " << nrOfMarines << std::endl;
			break;
		case UnitTypes::Terran_Refinery:
			this->nrOfRefineries++;
			Broodwar << "Finished " << unit->getType() << "!" << std::endl;
			break;
		case UnitTypes::Terran_Barracks:
			Broodwar << "Finished " << unit->getType() << "!" << std::endl;
			break;
		case UnitTypes::Terran_Academy:
			Broodwar << "Finished " << unit->getType() << "!" << std::endl;
			break;
		case UnitTypes::Terran_Factory:
			Broodwar << "Finished " << unit->getType() << "!" << std::endl;
			break;
		case UnitTypes::Terran_Supply_Depot:
			Broodwar << "Finished " << unit->getType() << "!" << std::endl;
			break;
		case UnitTypes::Terran_Medic:
			this->nrOfMedics++;
			this->totalArmySize++;
			Broodwar << "Active " << unit->getType() << ": " << nrOfMedics << std::endl;
			break;
		case UnitTypes::Terran_Siege_Tank_Tank_Mode:
			break;
		case UnitTypes::Terran_Machine_Shop:
			Broodwar << "Finished " << unit->getType() << "!" << std::endl;
			break;
		}
	}
}
