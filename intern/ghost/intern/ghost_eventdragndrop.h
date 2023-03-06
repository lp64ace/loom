#pragma once

#include "ghost_event.h"

#include <cassert>

class GHOST_EventDragnDrop : public GHOST_Event {
   public:
	/**
	 * Constructor.
	 * \param time: The time this event was generated.
	 * \param type: The type of this event.
	 * \param dataType: The type of the drop candidate object.
	 * \param window: The window where the event occurred.
	 * \param x: The x-coordinate of the location the cursor was at the time of
	 * the event. \param y: The y-coordinate of the location the cursor was at
	 * the time of the event. \param data: The "content" dropped in the window.
	 */
	GHOST_EventDragnDrop(uint64_t time,
						 GHOST_TEventType type,
						 GHOST_TDragnDropTypes dataType,
						 GHOST_IWindow *window,
						 int x,
						 int y,
						 GHOST_TEventDataPtr data)
		: GHOST_Event(time, type, window)
	{
		m_dragnDropEventData.x = x;
		m_dragnDropEventData.y = y;
		m_dragnDropEventData.dataType = dataType;
		m_dragnDropEventData.data = data;
		m_data = &m_dragnDropEventData;
	}

	~GHOST_EventDragnDrop()
	{
		/* Free the dropped object data. */
		if (m_dragnDropEventData.data == NULL)
			return;

		switch (m_dragnDropEventData.dataType) {
			case GHOST_kDragnDropTypeBitmap:
				// Destroy image data.
				assert(0);
				break;
			case GHOST_kDragnDropTypeFilenames: {
				GHOST_TStringArray *strArray = (GHOST_TStringArray *)
												   m_dragnDropEventData.data;
				int i;

				for (i = 0; i < strArray->count; i++)
					free(strArray->strings[i]);

				free(strArray->strings);
				free(strArray);
			} break;
			case GHOST_kDragnDropTypeString:
				free(m_dragnDropEventData.data);
				break;

			default:
				break;
		}
	}

   protected:
	/** The x,y-coordinates of the cursor position. */
	GHOST_TEventDragnDropData m_dragnDropEventData;
};
