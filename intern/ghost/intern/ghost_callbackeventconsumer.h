#pragma once

#include "ghost/ghost_c-api.h"
#include "ghost/ghost_ieventconsumer.h"

class GHOST_CallbackEventConsumer : public GHOST_IEventConsumer {
public:
	/**
	 * Constructor.
	 * \param eventCallback: The call-back routine invoked.
	 * \param userData: The data passed back through the call-back routine.
	 */
	GHOST_CallbackEventConsumer ( GHOST_EventCallbackProcPtr callback , GHOST_TUserDataPtr user );
	
	/**
	 * Destructor.
	 */
	~GHOST_CallbackEventConsumer ( void ) {
	}

	/**
	 * This method is called by an event producer when an event is available.
	 * \param event: The event that can be handled or ignored.
	 * \return Indication as to whether the event was handled.
	 */
	bool processEvent ( GHOST_IEvent *event );
protected:
	/** The call-back routine invoked. */
	GHOST_EventCallbackProcPtr m_eventCallback;
	/** The data passed back through the call-back routine. */
	GHOST_TUserDataPtr m_userData;

#ifdef WITH_CXX_GUARDEDALLOC
	MEM_CXX_CLASS_ALLOC_FUNCS ( "GHOST:GHOST_CallbackEventConsumer" )
#endif
};
