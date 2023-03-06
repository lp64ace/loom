#pragma once

#include "ghost_ievent.h"

class GHOST_IntEventConsumer {
public:
	virtual ~GHOST_IntEventConsumer ( ) = default;

	/**
	 * This method is called by the system when it has events to dispatch.
	 * \param evt The event to dispatch.
	 * \return If the event was handled by the consumer, the return value is \c True, otherwise 
	 * the return value is \c False.
	 */
	virtual bool ProcessEvent ( GHOST_IntEvent *evt ) = 0;
};
