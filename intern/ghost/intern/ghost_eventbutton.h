#pragma once

#include "ghost_event.h"

class GHOST_EventButton : public GHOST_Event {
public:
        /**
         * Constructor.
         * \param time: The time this event was generated.
         * \param type: The type of this event.
         * \param window: The window of this event.
         * \param button: The state of the buttons were at the time of the event.
         * \param tablet: The tablet data associated with this event.
         */
        GHOST_EventButton ( uint64_t time ,
                            GHOST_TEventType type ,
                            GHOST_IWindow *window ,
                            GHOST_TButton button )
                : GHOST_Event ( time , type , window ) , m_buttonEventData ( { button } ) {
                m_data = &m_buttonEventData;
        }
protected:
        /** The button event data. */
        GHOST_TEventButtonData m_buttonEventData;
};

