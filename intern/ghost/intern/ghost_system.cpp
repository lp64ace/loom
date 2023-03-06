#include "ghost_system.h"

#include <chrono>
#include <cstdio>

#include "ghost_displaymanager.h"
#include "ghost_eventmanager.h"
#include "ghost_timermanager.h"
#include "ghost_timertask.h"
#include "ghost_windowmanager.h"

GHOST_System::GHOST_System ( )
	: m_nativePixel ( false ) ,
	m_windowFocus ( true ) ,
	m_displayManager ( nullptr ) ,
	m_timerManager ( nullptr ) ,
	m_windowManager ( nullptr ) ,
	m_eventManager ( nullptr ) ,
	m_multitouchGestures ( true ) ,
	m_tabletAPI ( GHOST_kTabletAutomatic ) {
}

GHOST_System::~GHOST_System ( ) {
	exit ( );
}

uint64_t GHOST_System::getMilliSeconds ( ) const {
	return std::chrono::duration_cast< std::chrono::milliseconds >(
		std::chrono::steady_clock::now ( ).time_since_epoch ( ) )
		.count ( );
}

GHOST_ITimerTask *GHOST_System::installTimer ( uint64_t delay ,
					       uint64_t interval ,
					       GHOST_TimerProcPtr timerProc ,
					       GHOST_TUserDataPtr userData ) {
	uint64_t millis = getMilliSeconds ( );
	GHOST_TimerTask *timer = new GHOST_TimerTask ( millis + delay , interval , timerProc , userData );
	if ( timer ) {
		if ( m_timerManager->addTimer ( timer ) == GHOST_kSuccess ) {
			/* Check to see whether we need to fire the timer right away. */
			m_timerManager->fireTimers ( millis );
		} else {
			delete timer;
			timer = nullptr;
		}
	}
	return timer;
}

GHOST_TSuccess GHOST_System::removeTimer ( GHOST_ITimerTask *timerTask ) {
	GHOST_TSuccess success = GHOST_kFailure;
	if ( timerTask ) {
		success = m_timerManager->removeTimer ( ( GHOST_TimerTask * ) timerTask );
	}
	return success;
}

GHOST_TSuccess GHOST_System::disposeWindow ( GHOST_IWindow *window ) {
	GHOST_TSuccess success;

	/*
	 * Remove all pending events for the window.
	 */
	if ( m_windowManager->getWindowFound ( window ) ) {
		m_eventManager->removeWindowEvents ( window );
	}
	if ( window == m_windowManager->getFullScreenWindow ( ) ) {
		success = endFullScreen ( );
	} else {
		if ( m_windowManager->getWindowFound ( window ) ) {
			success = m_windowManager->removeWindow ( window );
			if ( success ) {
				delete window;
			}
		} else {
			success = GHOST_kFailure;
		}
	}
	return success;
}

bool GHOST_System::validWindow ( GHOST_IWindow *window ) {
	return m_windowManager->getWindowFound ( window );
}

GHOST_TSuccess GHOST_System::beginFullScreen ( const GHOST_DisplaySetting &setting ,
					       GHOST_IWindow **window ,
					       const bool stereoVisual ) {
	GHOST_TSuccess success = GHOST_kFailure;
	if ( m_displayManager ) {
		if ( !m_windowManager->getFullScreen ( ) ) {
			m_displayManager->getCurrentDisplaySetting ( GHOST_DisplayManager::kMainDisplay ,
								     m_preFullScreenSetting );

			// GHOST_PRINT("GHOST_System::beginFullScreen(): activating new display settings\n");
			success = m_displayManager->setCurrentDisplaySetting ( GHOST_DisplayManager::kMainDisplay ,
									       setting );
			if ( success == GHOST_kSuccess ) {
				// GHOST_PRINT("GHOST_System::beginFullScreen(): creating full-screen window\n");
				success = createFullScreenWindow ( ( GHOST_Window ** ) window , setting , stereoVisual );
				if ( success == GHOST_kSuccess ) {
					m_windowManager->beginFullScreen ( *window , stereoVisual );
				} else {
					m_displayManager->setCurrentDisplaySetting ( GHOST_DisplayManager::kMainDisplay ,
										     m_preFullScreenSetting );
				}
			}
		}
	}
	if ( success == GHOST_kFailure ) {
	}
	return success;
}


GHOST_TSuccess GHOST_System::updateFullScreen ( const GHOST_DisplaySetting &setting ,
						GHOST_IWindow ** /*window*/ ) {
	GHOST_TSuccess success = GHOST_kFailure;
	if ( m_displayManager ) {
		if ( m_windowManager->getFullScreen ( ) ) {
			success = m_displayManager->setCurrentDisplaySetting ( GHOST_DisplayManager::kMainDisplay ,
									       setting );
		}
	}

	return success;
}

GHOST_TSuccess GHOST_System::endFullScreen ( ) {
	GHOST_TSuccess success = GHOST_kFailure;
	if ( m_windowManager->getFullScreen ( ) ) {
		success = m_windowManager->endFullScreen ( );
		success = m_displayManager->setCurrentDisplaySetting ( GHOST_DisplayManager::kMainDisplay ,
								       m_preFullScreenSetting );
	} else {
		success = GHOST_kFailure;
	}
	return success;
}

bool GHOST_System::getFullScreen ( ) {
	bool fullScreen;
	if ( m_windowManager ) {
		fullScreen = m_windowManager->getFullScreen ( );
	} else {
		fullScreen = false;
	}
	return fullScreen;
}

GHOST_IWindow *GHOST_System::getWindowUnderCursor ( int32_t x , int32_t y ) {
	/* TODO: This solution should follow the order of the activated windows (Z-order).
	 * It is imperfect but usable in most cases. */
	for ( GHOST_IWindow *iwindow : m_windowManager->getWindows ( ) ) {
		if ( iwindow->getState ( ) == GHOST_kWindowStateMinimized ) {
			continue;
		}

		GHOST_Rect bounds;
		iwindow->getClientBounds ( bounds );
		if ( bounds.isInside ( x , y ) ) {
			return iwindow;
		}
	}

	return nullptr;
}

void GHOST_System::dispatchEvents ( ) {
	if ( m_eventManager ) {
		m_eventManager->dispatchEvents ( );
	}

	m_timerManager->fireTimers ( getMilliSeconds ( ) );
}

GHOST_TSuccess GHOST_System::addEventConsumer ( GHOST_IEventConsumer *consumer ) {
	GHOST_TSuccess success;
	if ( m_eventManager ) {
		success = m_eventManager->addConsumer ( consumer );
	} else {
		success = GHOST_kFailure;
	}
	return success;
}

GHOST_TSuccess GHOST_System::removeEventConsumer ( GHOST_IEventConsumer *consumer ) {
	GHOST_TSuccess success;
	if ( m_eventManager ) {
		success = m_eventManager->removeConsumer ( consumer );
	} else {
		success = GHOST_kFailure;
	}
	return success;
}

GHOST_TSuccess GHOST_System::pushEvent ( GHOST_IEvent *event ) {
	GHOST_TSuccess success;
	if ( m_eventManager ) {
		success = m_eventManager->pushEvent ( event );
	} else {
		success = GHOST_kFailure;
	}
	return success;
}

GHOST_TSuccess GHOST_System::getCursorPositionClientRelative ( const GHOST_IWindow *window ,
							       int32_t &x ,
							       int32_t &y ) const {
	/* Sub-classes that can implement this directly should do so. */
	int32_t screen_x , screen_y;
	GHOST_TSuccess success = getCursorPosition ( screen_x , screen_y );
	if ( success == GHOST_kSuccess ) {
		window->screenToClient ( screen_x , screen_y , x , y );
	}
	return success;
}

GHOST_TSuccess GHOST_System::setCursorPositionClientRelative ( GHOST_IWindow *window ,
							       int32_t x ,
							       int32_t y ) {
	/* Sub-classes that can implement this directly should do so. */
	int32_t screen_x , screen_y;
	window->clientToScreen ( x , y , screen_x , screen_y );
	return setCursorPosition ( screen_x , screen_y );
}

GHOST_TSuccess GHOST_System::getModifierKeyState ( GHOST_TModifierKey mask , bool &isDown ) const {
	GHOST_ModifierKeys keys;
	/* Get the state of all modifier keys. */
	GHOST_TSuccess success = getModifierKeys ( keys );
	if ( success ) {
		/* Isolate the state of the key requested. */
		isDown = keys.get ( mask );
	}
	return success;
}

GHOST_TSuccess GHOST_System::getButtonState ( GHOST_TButton mask , bool &isDown ) const {
	GHOST_Buttons buttons;
	/* Get the state of all mouse buttons. */
	GHOST_TSuccess success = getButtons ( buttons );
	if ( success ) {
		/* Isolate the state of the mouse button requested. */
		isDown = buttons.get ( mask );
	}
	return success;
}
GHOST_TSuccess GHOST_System::init ( ) {
	m_timerManager = new GHOST_TimerManager ( );
	m_windowManager = new GHOST_WindowManager ( );
	m_eventManager = new GHOST_EventManager ( );

	if ( m_timerManager && m_windowManager && m_eventManager ) {
		return GHOST_kSuccess;
	}
	return GHOST_kFailure;
}

GHOST_TSuccess GHOST_System::exit ( ) {
	if ( getFullScreen ( ) ) {
		endFullScreen ( );
	}

	delete m_displayManager;
	m_displayManager = nullptr;

	delete m_windowManager;
	m_windowManager = nullptr;

	delete m_timerManager;
	m_timerManager = nullptr;

	delete m_eventManager;
	m_eventManager = nullptr;

	return GHOST_kSuccess;
}

GHOST_TSuccess GHOST_System::createFullScreenWindow ( GHOST_Window **window ,
						      const GHOST_DisplaySetting &settings ,
						      const bool stereoVisual ) {
	GHOST_GLSettings glSettings = { 0 };

	if ( stereoVisual ) {
		glSettings.flags |= GHOST_glStereoVisual;
	}
	glSettings.context_type = GHOST_kDrawingContextTypeOpenGL;
	/* NOTE: don't use #getCurrentDisplaySetting() because on X11 we may
	 * be zoomed in and the desktop may be bigger than the viewport. */
	*window = ( GHOST_Window * ) createWindow ( "" ,
						    0 ,
						    0 ,
						    settings.xPixels ,
						    settings.yPixels ,
						    GHOST_kWindowStateNormal ,
						    glSettings ,
						    true /* exclusive */ );
	return ( *window == nullptr ) ? GHOST_kFailure : GHOST_kSuccess;
}

bool GHOST_System::useNativePixel ( ) {
	m_nativePixel = true;
	return true;
}

void GHOST_System::useWindowFocus ( const bool use_focus ) {
	m_windowFocus = use_focus;
}

bool GHOST_System::supportsCursorWarp ( ) {
	return true;
}

bool GHOST_System::supportsWindowPosition ( ) {
	return true;
}
