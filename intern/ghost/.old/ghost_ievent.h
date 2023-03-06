#pragma once

#include "ghost_types.h"

class GHOST_IntWindow;

class GHOST_IntEvent {
public:
	virtual ~GHOST_IntEvent ( ) = default;

	/**
	 * Get the identifier describing the event type, this can be any of 
	 * the GHOST_TEventType enums.
	 */
	virtual GHOST_TEventType GetType ( ) const = 0;
	
	/**
	 * Get the data associated with the event, this is highly depended on 
	 * the type of the event.
	 */
	virtual GHOST_TEventDataPtr GetData ( ) const = 0;
	
	/**
	 * Get the time stamp this event was generated.
	 */
	virtual uint64_t GetTime ( ) const = 0;

	/**
	 * If this event is a window event we can retrieve the associated window 
	 * of the event using this function, if this event has no window associated 
	 * this will return NULL.
	 */
	virtual GHOST_IntWindow *GetWindow ( ) const = 0;
};
