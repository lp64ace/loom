#pragma once

#include "loomlib/loomlib_utildefines.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GHOST_DECLARE_HANDLE(name) \
  typedef struct name##__ { \
    int __unused; \
  } *name;

GHOST_DECLARE_HANDLE ( GHOST_SystemHandle );
GHOST_DECLARE_HANDLE ( GHOST_TimerTaskHandle );
GHOST_DECLARE_HANDLE ( GHOST_WindowHandle );
GHOST_DECLARE_HANDLE ( GHOST_EventHandle );
GHOST_DECLARE_HANDLE ( GHOST_ContextHandle );
GHOST_DECLARE_HANDLE ( GHOST_EventConsumerHandle );

#ifdef __cplusplus
class GHOST_IntTimerTask;
typedef void ( *GHOST_TimerProcPtr )( GHOST_IntTimerTask *task , uint64_t time );
#else
struct GHOST_TimerTaskHandle;
typedef void ( *GHOST_TimerProcPtr )( struct GHOST_TimerTaskHandle *task , uint64_t time );
#endif

typedef void *GHOST_TEventDataPtr;
typedef void *GHOST_UserDataPtr;

typedef long double GHOST_TimePoint;
typedef unsigned int GHOST_Enum;

typedef enum GHOST_TStatus {
	GHOST_kFailure = 0 , GHOST_kSuccess = 1 ,
} GHOST_TStatus;

typedef enum GHOST_DialogOptions {
	GHOST_DialogWarning = ( 1 << 0 ) ,
	GHOST_DialogError = ( 1 << 1 ) ,
} GHOST_DialogOptions;

ENUM_OPERATORS ( GHOST_DialogOptions , GHOST_DialogError )

typedef enum GHOST_TWindowState {
	GHOST_kWindowStateNormal = 0 ,
	GHOST_kWindowStateMaximized ,
	GHOST_kWindowStateMinimized ,
	GHOST_kWindowStateFullScreen ,
	GHOST_kWindowStateEmbedded ,
} GHOST_TWindowState;

typedef enum GHOST_TDrawingContextType {
	GHOST_kDrawingContextTypeNone = 0 ,
	GHOST_kDrawingContextTypeOpenGL ,
	GHOST_kDrawingContextTypeD3D ,
}GHOST_TDrawingContextType;

typedef enum GHOST_TWindowOrder {
	GHOST_kWindowOrderTop = 0,
	GHOST_kWindowOrderBottom ,
} GHOST_TWindowOrder;

typedef enum GHOST_TStandardCursor {
	GHOST_kStandardCursorFirstCursor = 0 ,
	GHOST_kStandardCursorDefault = 0 ,
	GHOST_kStandardCursorRightArrow ,
	GHOST_kStandardCursorLeftArrow ,
	GHOST_kStandardCursorInfo ,
	GHOST_kStandardCursorDestroy ,
	GHOST_kStandardCursorHelp ,
	GHOST_kStandardCursorWait ,
	GHOST_kStandardCursorText ,
	GHOST_kStandardCursorCrosshair ,
	GHOST_kStandardCursorCrosshairA ,
	GHOST_kStandardCursorCrosshairB ,
	GHOST_kStandardCursorCrosshairC ,
	GHOST_kStandardCursorPencil ,
	GHOST_kStandardCursorUpArrow ,
	GHOST_kStandardCursorDownArrow ,
	GHOST_kStandardCursorVerticalSplit ,
	GHOST_kStandardCursorHorizontalSplit ,
	GHOST_kStandardCursorEraser ,
	GHOST_kStandardCursorKnife ,
	GHOST_kStandardCursorEyedropper ,
	GHOST_kStandardCursorZoomIn ,
	GHOST_kStandardCursorZoomOut ,
	GHOST_kStandardCursorMove ,
	GHOST_kStandardCursorNSEWScroll ,
	GHOST_kStandardCursorNSScroll ,
	GHOST_kStandardCursorEWScroll ,
	GHOST_kStandardCursorStop ,
	GHOST_kStandardCursorUpDown ,
	GHOST_kStandardCursorLeftRight ,
	GHOST_kStandardCursorTopSide ,
	GHOST_kStandardCursorBottomSide ,
	GHOST_kStandardCursorLeftSide ,
	GHOST_kStandardCursorRightSide ,
	GHOST_kStandardCursorTopLeftCorner ,
	GHOST_kStandardCursorTopRightCorner ,
	GHOST_kStandardCursorBottomRightCorner ,
	GHOST_kStandardCursorBottomLeftCorner ,
	GHOST_kStandardCursorCopy ,
	GHOST_kStandardCursorCustom ,

	GHOST_kStandardCursorNumCursors
} GHOST_TStandardCursor;

typedef enum GHOST_TGrabCursorMode {
	GHOST_kGrabDisable = 0 , // Grab not set.
	GHOST_kGrabNormal , // No cursor adjustments.
	GHOST_kGrabWrap , // Wrap the mouse location to prevent limiting screen bounds.
	GHOST_kGrabHide , // Hide the mouse while grabbing and restore the original location on release
} GHOST_TGrabCursorMode;

typedef enum GHOST_TAxisFlag {
	GHOST_kAxisNon = 0 ,
	GHOST_kAxisX = ( 1 << 0 ) ,
	GHOST_kAxisY = ( 1 << 1 ) ,
} GHOST_TAxisFlag;

typedef struct GHOST_CursorBitmapRef {
	const uint8_t *Data;
	int data [ 2 ];
	int hotspot [ 2 ];
} GHOST_CursorBitmapRef;

typedef enum {
	GHOST_kEventUnknown = 0 ,

	GHOST_kEventCursorMove ,
	GHOST_kEventButtonDown ,
	GHOST_kEventButtonUp ,	
	GHOST_kEventWheel ,	
	GHOST_kEventTrackpad ,	

	GHOST_kEventKeyDown ,
	GHOST_kEventKeyUp ,

	GHOST_kEventQuitRequest ,

	GHOST_kEventWindowClose ,
	GHOST_kEventWindowActivate ,
	GHOST_kEventWindowDeactivate ,
	GHOST_kEventWindowUpdate ,
	GHOST_kEventWindowSize ,
	GHOST_kEventWindowMove ,
	GHOST_kEventWindowDPIHintChanged ,

	GHOST_kEventDraggingEntered ,
	GHOST_kEventDraggingUpdated ,
	GHOST_kEventDraggingExited ,
	GHOST_kEventDraggingDropDone ,

	/**
	 * Needed for Cocoa to open double-clicked .blend file at
	 * startup.
	 */
	GHOST_kEventOpenMainFile ,
	/**
	 * Needed for Cocoa when window moves to other display.
	 */
	GHOST_kEventNativeResolutionChange ,

	GHOST_kEventTimer ,

	GHOST_kEventImeCompositionStart ,
	GHOST_kEventImeComposition ,
	GHOST_kEventImeCompositionEnd ,

	GHOST_kNumEventTypes
} GHOST_TEventType;

typedef enum {
	GHOST_kUserSpecialDirDesktop ,
	GHOST_kUserSpecialDirDocuments ,
	GHOST_kUserSpecialDirDownloads ,
	GHOST_kUserSpecialDirMusic ,
	GHOST_kUserSpecialDirPictures ,
	GHOST_kUserSpecialDirVideos ,
	GHOST_kUserSpecialDirCaches ,
} GHOST_TUserSpecialDirTypes;

typedef struct GHOST_Rect {
	int left , top , right , bottom;
} GHOST_Rect;

#ifdef __cplusplus
}
#endif
