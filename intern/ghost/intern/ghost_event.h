#pragma once

#include "ghost/ghost_ievent.h"

class GHOST_Event : public GHOST_IEvent {
   public:
	/**
	 * Constructor.
	 * \param msec: The time this event was generated.
	 * \param type: The type of this event.
	 * \param window: The generating window (or NULL if system event).
	 */
	GHOST_Event(uint64_t msec, GHOST_TEventType type, GHOST_IWindow *window)
		: m_type(type), m_time(msec), m_window(window), m_data(nullptr)
	{
	}

	/**
	 * Returns the event type.
	 * \return The event type.
	 */
	GHOST_TEventType getType()
	{
		return m_type;
	}

	/**
	 * Returns the time this event was generated.
	 * \return The event generation time.
	 */
	uint64_t getTime()
	{
		return m_time;
	}

	/**
	 * Returns the window this event was generated on,
	 * or NULL if it is a 'system' event.
	 * \return The generating window.
	 */
	GHOST_IWindow *getWindow()
	{
		return m_window;
	}

	/**
	 * Returns the event data.
	 * \return The event data.
	 */
	GHOST_TEventDataPtr getData()
	{
		return m_data;
	}

   protected:
	/** Type of this event. */
	GHOST_TEventType m_type;
	/** The time this event was generated. */
	uint64_t m_time;
	/** Pointer to the generating window. */
	GHOST_IWindow *m_window;
	/** Pointer to the event data. */
	GHOST_TEventDataPtr m_data;
};
