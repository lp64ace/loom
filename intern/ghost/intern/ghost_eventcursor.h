#pragma once

#include "ghost_event.h"

class GHOST_EventCursor : public GHOST_Event {
   public:
	/**
	 * Constructor.
	 * \param msec: The time this event was generated.
	 * \param type: The type of this event.
	 * \param x: The x-coordinate of the location the cursor was at the time of
	 * the event. \param y: The y-coordinate of the location the cursor was at
	 * the time of the event. \param tablet: The tablet data associated with
	 * this event.
	 */
	GHOST_EventCursor(uint64_t msec,
					  GHOST_TEventType type,
					  GHOST_IWindow *window,
					  int32_t x,
					  int32_t y)
		: GHOST_Event(msec, type, window), m_cursorEventData({x, y})
	{
		m_data = &m_cursorEventData;
	}

   protected:
	/** The x,y-coordinates of the cursor position. */
	GHOST_TEventCursorData m_cursorEventData;
};
