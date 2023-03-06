#pragma once

#include "ghost/ghost_types.h"
#include "ghost_displaymanagernull.h"
#include "ghost_system.h"
#include "ghost_windownull.h"
#include "ghost_contextnone.h"

class GHOST_WindowNULL;

class GHOST_SystemHeadless : public GHOST_System {
public:
	GHOST_SystemHeadless ( ) : GHOST_System ( ) { }
	~GHOST_SystemHeadless ( ) override = default;

	bool processEvents ( bool /*waitForEvent*/ ) override {
		return false;
	}
	bool setConsoleWindowState ( GHOST_TConsoleWindowState /*action*/ ) override {
		return 0;
	}
	GHOST_TSuccess getModifierKeys ( GHOST_ModifierKeys & /*keys*/ ) const override {
		return GHOST_kSuccess;
	}
	GHOST_TSuccess getButtons ( GHOST_Buttons & /*buttons*/ ) const override {
		return GHOST_kSuccess;
	}
	char *getClipboard ( bool /*selection*/ ) const override {
		return nullptr;
	}
	void putClipboard ( const char * /*buffer*/ , bool /*selection*/ ) const override {
	}
	uint64_t getMilliSeconds ( ) const override {
		return 0;
	}
	uint8_t getNumDisplays ( ) const override {
		return uint8_t ( 1 );
	}
	GHOST_TSuccess getCursorPosition ( int32_t & /*x*/ , int32_t & /*y*/ ) const override {
		return GHOST_kFailure;
	}
	GHOST_TSuccess setCursorPosition ( int32_t /*x*/ , int32_t /*y*/ ) override {
		return GHOST_kFailure;
	}
	void getMainDisplayDimensions ( uint32_t & /*width*/ , uint32_t & /*height*/ ) const override { /* nop */
	}
	void getAllDisplayDimensions ( uint32_t & /*width*/ , uint32_t & /*height*/ ) const override { /* nop */
	}
	GHOST_IContext *createOffscreenContext ( GHOST_GLSettings /*glSettings*/ ) override {
		return nullptr;
	}
	GHOST_TSuccess disposeContext ( GHOST_IContext *context ) override {
		delete context;
		return GHOST_kSuccess;
	}
	
	GHOST_TSuccess init ( ) override {
		GHOST_TSuccess success = GHOST_System::init ( );

		if ( success ) {
			m_displayManager = new GHOST_DisplayManagerNULL ( );

			if ( m_displayManager ) {
				return GHOST_kSuccess;
			}
		}

		return GHOST_kFailure;
	}

	GHOST_IWindow *createWindow ( const char *title ,
				      int32_t left ,
				      int32_t top ,
				      uint32_t width ,
				      uint32_t height ,
				      GHOST_TWindowState state ,
				      GHOST_GLSettings glSettings ,
				      const bool /*exclusive*/ ,
				      const bool /*is_dialog*/ ,
				      const GHOST_IWindow *parentWindow ) override {
		return new GHOST_WindowNULL ( title ,
					      left ,
					      top ,
					      width ,
					      height ,
					      state ,
					      parentWindow ,
					      glSettings.context_type ,
					      ( ( glSettings.flags & GHOST_glStereoVisual ) != 0 ) );
	}

	GHOST_IWindow *getWindowUnderCursor ( int32_t /*x*/ , int32_t /*y*/ ) override {
		return nullptr;
	}
};
