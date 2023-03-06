#include "ghost_callbackeventconsumer.h"

#include "ghost/ghost_c-api.h"

GHOST_CallbackEventConsumer::GHOST_CallbackEventConsumer ( GHOST_EventCallbackProcPtr callback , GHOST_TUserDataPtr user ) {
	this->m_eventCallback = callback;
	this->m_userData = user;
}

bool GHOST_CallbackEventConsumer::processEvent ( GHOST_IEvent *event ) {
	return this->m_eventCallback ( ( GHOST_EventHandle ) event , this->m_userData );
}
