#include "ExampleAIModule.h"
#include <iostream>

using namespace BWAPI;
using namespace Filter;

void ExampleAIModule::onStart()
{
	this->nrOfWorkers = 0;
	this->nrOfBarracks = 0;
	this->nrOfMarines = 0;
	this->nrOfRefineries = 0;
	//Here is where you program start

	//setLocalSpeed(0) is sets speed to your framerate, higher value == slower gamespeed
	Broodwar->setLocalSpeed(22);



	Broodwar->sendText("Welcome to BTH_AI!");
	// Print the map name.

	// BWAPI returns std::string when retrieving a string, don't forget to add .c_str() when printing!
	Broodwar << "The map is " << Broodwar->mapName() << "!" << std::endl;

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
		if (u->getType().isWorker())
		{
			// if our worker is idle
			if (u->isIdle())
			{
				// Order workers carrying a resource to return them to the center,
				// otherwise find a mineral patch to harvest.

				if (!u->getClosestUnit(GetType == UnitTypes::Terran_Barracks))
				{
					UnitType barrack(UnitTypes::Terran_Barracks);
					if (Broodwar->self()->minerals() >= barrack.mineralPrice())
					{
						Unit barrackBuilder = u->getClosestUnit(GetType == barrack.whatBuilds().first &&
							(IsIdle || IsGatheringMinerals) &&
							IsOwned);

						if (barrackBuilder)
						{
							TilePosition buildPos = Broodwar->getBuildLocation(barrack, barrackBuilder->getTilePosition());

							if (buildPos)
							{
								barrackBuilder->build(barrack, buildPos);
							}
						}
					}
				}

				if (u->isCarryingGas() || u->isCarryingMinerals())
				{
					u->returnCargo();
				}
				else if (!u->getPowerUp())  // The worker cannot harvest anything if it
				{                             // is carrying a powerup such as a flag
				  // Harvest from the nearest mineral patch or gas refinery
					if (!u->gather(u->getClosestUnit(IsMineralField || IsRefinery)))
					{
						// If the call fails, then print the last error message
						Broodwar << Broodwar->getLastError() << std::endl;
					}

				} // closure: has no powerup
			} // closure: if idle
		}
		else if (u->getType().isResourceDepot()) // A resource depot is a Command Center, Nexus, or Hatchery
		{
			// Pushing out a refinery once we have a barrack
			if (this->nrOfBarracks > 0 && this->nrOfRefineries < this->MAX_REFINERY)
			{
				if (Broodwar->self()->minerals() >= UnitTypes::Terran_Refinery.mineralPrice())
				{
					static int lastChecked = 0;
					if (lastChecked + 400 < Broodwar->getFrameCount())
					{
						lastChecked = Broodwar->getFrameCount();

						Unit refineryBuilder = u->getClosestUnit(GetType == UnitTypes::Terran_Refinery.whatBuilds().first &&
							(IsIdle || IsGatheringMinerals) && IsOwned);

						if (refineryBuilder)
						{
							TilePosition buildPos = Broodwar->getBuildLocation(UnitTypes::Terran_Refinery, refineryBuilder->getTilePosition());

							if (buildPos)
							{
								refineryBuilder->build(UnitTypes::Terran_Refinery, buildPos);
							}
						}
					}
				}
			}
			if (this->nrOfBarracks < this->MAX_BARRACKS)
			{
				// If we are supply blocked and haven't tried constructing more recently
				if (Broodwar->self()->minerals() >= UnitTypes::Terran_Barracks.mineralPrice())
				{
					static int lastChecked = 0;
					if (lastChecked + 400 < Broodwar->getFrameCount())
					{
						lastChecked = Broodwar->getFrameCount();
						Unit barrackBuilder = u->getClosestUnit(GetType == UnitTypes::Terran_Barracks.whatBuilds().first &&
							(IsIdle || IsGatheringMinerals) && IsOwned);

						if (barrackBuilder)
						{
							TilePosition buildPos = Broodwar->getBuildLocation(UnitTypes::Terran_Barracks, barrackBuilder->getTilePosition());

							if (buildPos)
							{
								barrackBuilder->build(UnitTypes::Terran_Barracks, buildPos);
							}
						}
					}
				}
			}

			if (this->nrOfWorkers < this->MAX_WORKERS && u->isIdle())
			{
				u->train(u->getType().getRace().getWorker());
			}

			// Order the depot to construct more workers! But only when it is idle.
				// If that fails, draw the error at the location so that you can visibly see what went wrong!
				// However, drawing the error once will only appear for a single frame
				// so create an event that keeps it on the screen for some frames
			Error lastErr = Broodwar->getLastError();

			// Retrieve the supply provider type in the case that we have run out of supplies

			if ((Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() <= 8 && this->nrOfBarracks > 0) ||
				(Broodwar->self()->supplyTotal() - Broodwar->self()->supplyUsed() <= 4 && this->nrOfBarracks == 0))
			{
				Position pos = u->getPosition();
				static int lastChecked = 0;
				Broodwar->registerEvent([pos, lastErr](Game*) { Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
					nullptr,    // condition
					Broodwar->getLatencyFrames());  // frames to run

				if (Broodwar->self()->minerals() >= UnitTypes::Terran_Supply_Depot.mineralPrice())
				{
					UnitType supplyProviderType = u->getType().getRace().getSupplyProvider();
					// If we are supply blocked and haven't tried constructing more recently
					if (lastChecked + 200 < Broodwar->getFrameCount() &&
						Broodwar->self()->incompleteUnitCount(supplyProviderType) == 0)
					{
						lastChecked = Broodwar->getFrameCount();

						// Retrieve a unit that is capable of constructing the supply needed
						Unit supplyBuilder = u->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
							(IsIdle || IsGatheringMinerals) &&
							IsOwned);
						// If a unit was found
						if (supplyBuilder)
						{
							if (supplyProviderType.isBuilding())
							{
								TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, supplyBuilder->getTilePosition());
								if (targetBuildLocation)
								{
									// Register an event that draws the target build location
									Broodwar->registerEvent([targetBuildLocation, supplyProviderType](Game*)
									{
										Broodwar->drawBoxMap(Position(targetBuildLocation),
											Position(targetBuildLocation + supplyProviderType.tileSize()),
											Colors::Blue);
									},
										nullptr,  // condition
										supplyProviderType.buildTime() + 100);  // frames to run

								// Order the builder to construct the supply structure
									supplyBuilder->build(supplyProviderType, targetBuildLocation);
								}
							}
							else
							{
								// Train the supply provider (Overlord) if the provider is not a structure
								supplyBuilder->train(supplyProviderType);
							}
						}
					}
				}
			}

		}
		else if (u->getType() == UnitTypes::Terran_Barracks && this->nrOfMarines < this->MAX_MARINES)
		{
			if (Broodwar->self()->minerals() >= UnitTypes::Terran_Marine.mineralPrice())
			{
				if (u->isIdle())
				{
					u->train(UnitTypes::Terran_Marine);
				}
			}
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

}

void ExampleAIModule::onUnitEvade(BWAPI::Unit unit)
{
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
		switch (unit->getType())
		{
		case UnitTypes::Terran_SCV:
			nrOfWorkers++;
			Broodwar << "Active SCV: " << nrOfWorkers << std::endl;
			break;
		case UnitTypes::Terran_Barracks:
			nrOfBarracks++;
			Broodwar << "Active Barracks: " << nrOfBarracks << std::endl;
			break;
		case UnitTypes::Terran_Marine:
			nrOfMarines++;
			Broodwar << "Active Marines: " << nrOfMarines << std::endl;
			break;
		case UnitTypes::Terran_Refinery:
			nrOfRefineries++;
			Broodwar << "Active Refineries: " << nrOfRefineries << std::endl;
			break;
		}
	}
}

void ExampleAIModule::onUnitDestroy(BWAPI::Unit unit)
{
	switch (unit->getType())
	{
	case UnitTypes::Terran_SCV:
		nrOfWorkers--;
		Broodwar << "Active SCV: " << nrOfWorkers << std::endl;
		break;
	case UnitTypes::Terran_Barracks:
		nrOfBarracks--;
		Broodwar << "Active Barracks: " << nrOfBarracks << std::endl;
		break;
	case UnitTypes::Terran_Marine:
		nrOfMarines--;
		Broodwar << "Active Marines: " << nrOfMarines << std::endl;
		break;
	case UnitTypes::Terran_Refinery:
		nrOfRefineries--;
		Broodwar << "Active Refineries: " << nrOfRefineries << std::endl;
		break;
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
}
