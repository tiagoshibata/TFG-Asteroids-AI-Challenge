#include <iostream>
#include <fstream>
#include <fenv.h>

#include "bot_interface.h"
#include "ShibataBot.hpp"

int main() {
	feenableexcept(FE_DIVBYZERO | FE_INVALID | FE_OVERFLOW);
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
