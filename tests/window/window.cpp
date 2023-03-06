#include "ghost/ghost_c-api.h"

int main ( void ) {
	GHOST_HSYSTEM hSystem = GHOST_CreateSystem ( );

	GHOST_GLSettings GHOST_kWindowFeaturesOpenGL = { 0 , GHOST_kDrawingContextTypeOpenGL };
	
	GHOST_HWND hWnd = GHOST_CreateWindow ( hSystem ,
					       NULL ,
					       "Loom" ,
					       240 ,
					       240 ,
					       1280 ,
					       720 ,
					       GHOST_kWindowStateNormal ,
					       false ,
					       GHOST_kWindowFeaturesOpenGL );

	while ( true ) {
		if ( GHOST_ProcessEvents ( hSystem , false ) ) {
			GHOST_DispatchEvents ( hSystem );
		}
	}

	GHOST_DisposeSystem ( hSystem );
	return 0;
}