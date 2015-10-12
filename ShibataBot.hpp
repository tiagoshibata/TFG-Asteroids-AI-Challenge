#ifndef _SHIBATA_BOT_HPP_
#define _SHIBATA_BOT_HPP_

#include "BotBase.h"
#include "Control.hpp"

class ShibataBot : public BotBase {
public:
	ShibataBot();
	virtual ~ShibataBot();
	virtual void Process();

private:
	Control::PD pd;
};

#endif
