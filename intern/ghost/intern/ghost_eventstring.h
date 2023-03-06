#pragma once

#include "ghost_event.h"

class GHOST_EventString : public GHOST_Event {
   public:
	/**
	 * Constructor.
	 * \param msec: The time this event was generated.
	 * \param type: The type of this event.
	 * \param window: The generating window (or NULL if system event).
	 * \param data_ptr: Pointer to the (un-formatted) data associated with the
	 * event.
	 */
	GHOST_EventString(uint64_t msec,
					  GHOST_TEventType type,
					  GHOST_IWindow *window,
					  GHOST_TEventDataPtr data_ptr)
		: GHOST_Event(msec, type, window)
	{
		m_data = data_ptr;
	}

	~GHOST_EventString()
	{
		if (m_data)
			free(m_data);
	}
};
