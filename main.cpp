#include <iostream>
#include <fstream>

#include "bot_interface.h"
#include "ShibataBot.hpp"

int main() {
	GameState gamestate = GameState();
	ShibataBot bot = ShibataBot();

	bot.gamestate = &gamestate;
	bot.myShip = gamestate.myShip;

	while (bot.myShip != NULL) {
		gamestate.Update();
		bot.Update();
		gamestate.WriteData();
	}

	return 0;
}
