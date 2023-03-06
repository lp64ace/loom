#pragma once

#include "ghost/ghost_ieventconsumer.h"

class GHOST_EventPrinter : public GHOST_IEventConsumer {
public:
	/**
	 * Prints all the events received to std out.
	 * \param event: The event that can be handled or not.
	 * \return Indication as to whether the event was handled.
	 */
	bool processEvent ( GHOST_IEvent *event );
protected:
	/**
	 * Converts GHOST key code to a readable string.
	 * \param key: The GHOST key code to convert.
	 * \param str: The GHOST key code converted to a readable string.
	 */
	void getKeyString ( GHOST_TKey key , char str [ 32 ] ) const;
};
