#include <cstdlib>
#include <cstring>

#include "ghost/ghost_c-api.h"
#include "ghost/ghost_ievent.h"
#include "ghost/ghost_ieventconsumer.h"
#include "ghost/ghost_isystem.h"
#include "ghost_callbackeventconsumer.h"

GHOST_HSYSTEM GHOST_CreateSystem(void)
{
	GHOST_ISystem::createSystem(true, false);
	GHOST_ISystem *system = GHOST_ISystem::getSystem();

	return (GHOST_HSYSTEM)system;
}

GHOST_HSYSTEM GHOST_CreateSystemBackground(void)
{
	GHOST_ISystem::createSystemBackground();
	GHOST_ISystem *system = GHOST_ISystem::getSystem();

	return (GHOST_HSYSTEM)system;
}

GHOST_TSuccess GHOST_DisposeSystem(GHOST_HSYSTEM system)
{
	GHOST_ISystem::disposeSystem();
	return GHOST_kSuccess;
}

void GHOST_ShowMessageBox(GHOST_HSYSTEM systemhandle,
						  const char *title,
						  const char *message,
						  const char *help,
						  const char *cont,
						  const char *link,
						  GHOST_DialogOptions options)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	system->showMessageBox(title, message, help, cont, link, options);
}

GHOST_HEVTCONSUMER GHOST_CreateEventConsumer(
	GHOST_EventCallbackProcPtr eventcallback, void *userdata)
{
	GHOST_CallbackEventConsumer *consumer = new GHOST_CallbackEventConsumer(
		eventcallback, userdata);
	return (GHOST_HEVTCONSUMER)consumer;
}

GHOST_TSuccess GHOST_DisposeEventConsumer(GHOST_HEVTCONSUMER consumerhandle)
{
	delete ((GHOST_CallbackEventConsumer *)consumerhandle);
	return GHOST_kSuccess;
}

uint64_t GHOST_GetMilliSeconds(GHOST_HSYSTEM systemhandle)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	return system->getMilliSeconds();
}

GHOST_HTTASK GHOST_InstallTimer(GHOST_HSYSTEM systemhandle,
								uint64_t delay,
								uint64_t interval,
								GHOST_TimerProcPtr timerproc,
								GHOST_TUserDataPtr userdata)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	return (GHOST_HTTASK)system->installTimer(
		delay, interval, timerproc, userdata);
}

GHOST_TSuccess GHOST_RemoveTimer(GHOST_HSYSTEM systemhandle,
								 GHOST_HTTASK timertaskhandle)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	GHOST_ITimerTask *timertask = (GHOST_ITimerTask *)timertaskhandle;
	return system->removeTimer(timertask);
}

uint8_t GHOST_GetNumDisplays(GHOST_HSYSTEM systemhandle)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	return system->getNumDisplays();
}

void GHOST_GetMainDisplayDimensions(GHOST_HSYSTEM systemhandle,
									uint32_t *width,
									uint32_t *height)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	system->getMainDisplayDimensions(*width, *height);
}

void GHOST_GetAllDisplayDimensions(GHOST_HSYSTEM systemhandle,
								   uint32_t *width,
								   uint32_t *height)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	system->getAllDisplayDimensions(*width, *height);
}

GHOST_HCONTEXT GHOST_CreateOpenGLContext(GHOST_HSYSTEM systemhandle,
										 GHOST_GLSettings glSettings)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	return (GHOST_HCONTEXT)system->createOffscreenContext(glSettings);
}

GHOST_TSuccess GHOST_DisposeOpenGLContext(GHOST_HSYSTEM systemhandle,
										  GHOST_HCONTEXT contexthandle)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	GHOST_IContext *context = (GHOST_IContext *)contexthandle;
	return system->disposeContext(context);
}

GHOST_HWND GHOST_CreateWindow(GHOST_HSYSTEM systemhandle,
							  GHOST_HWND parent_windowhandle,
							  const char *title,
							  int32_t left,
							  int32_t top,
							  uint32_t width,
							  uint32_t height,
							  GHOST_TWindowState state,
							  bool is_dialog,
							  GHOST_GLSettings glSettings)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;

	return (GHOST_HWND)system->createWindow(
		title,
		left,
		top,
		width,
		height,
		state,
		glSettings,
		false,
		is_dialog,
		(GHOST_IWindow *)parent_windowhandle);
}

GHOST_TUserDataPtr GHOST_GetWindowUserData(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return window->getUserData();
}

void GHOST_SetWindowUserData(GHOST_HWND windowhandle,
							 GHOST_TUserDataPtr userdata)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	window->setUserData(userdata);
}

bool GHOST_IsDialogWindow(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return window->isDialog();
}

GHOST_TSuccess GHOST_DisposeWindow(GHOST_HSYSTEM systemhandle,
								   GHOST_HWND windowhandle)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return system->disposeWindow(window);
}

bool GHOST_ValidWindow(GHOST_HSYSTEM systemhandle, GHOST_HWND windowhandle)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return system->validWindow(window);
}

GHOST_HWND GHOST_BeginFullScreen(GHOST_HSYSTEM systemhandle,
								 GHOST_DisplaySetting *setting,
								 const bool stereoVisual)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	GHOST_IWindow *window = nullptr;
	system->beginFullScreen(*setting, &window, stereoVisual);
	return (GHOST_HWND)window;
}

GHOST_TSuccess GHOST_EndFullScreen(GHOST_HSYSTEM systemhandle)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	return system->endFullScreen();
}

bool GHOST_GetFullScreen(GHOST_HSYSTEM systemhandle)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	return system->getFullScreen();
}

GHOST_HWND GHOST_GetWindowUnderCursor(GHOST_HSYSTEM systemhandle,
									  int32_t x,
									  int32_t y)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	GHOST_IWindow *window = system->getWindowUnderCursor(x, y);

	return (GHOST_HWND)window;
}

bool GHOST_ProcessEvents(GHOST_HSYSTEM systemhandle, bool waitForEvent)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	return system->processEvents(waitForEvent);
}

void GHOST_DispatchEvents(GHOST_HSYSTEM systemhandle)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	system->dispatchEvents();
}

GHOST_TSuccess GHOST_AddEventConsumer(GHOST_HSYSTEM systemhandle,
									  GHOST_HEVTCONSUMER consumerhandle)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	return system->addEventConsumer(
		(GHOST_CallbackEventConsumer *)consumerhandle);
}

GHOST_TSuccess GHOST_RemoveEventConsumer(GHOST_HSYSTEM systemhandle,
										 GHOST_HEVTCONSUMER consumerhandle)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	return system->removeEventConsumer(
		(GHOST_CallbackEventConsumer *)consumerhandle);
}

GHOST_TSuccess GHOST_SetProgressBar(GHOST_HWND windowhandle, float progress)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return window->setProgressBar(progress);
}

GHOST_TSuccess GHOST_EndProgressBar(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return window->endProgressBar();
}

GHOST_TStandardCursor GHOST_GetCursorShape(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return window->getCursorShape();
}

GHOST_TSuccess GHOST_SetCursorShape(GHOST_HWND windowhandle,
									GHOST_TStandardCursor cursorshape)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return window->setCursorShape(cursorshape);
}

GHOST_TSuccess GHOST_HasCursorShape(GHOST_HWND windowhandle,
									GHOST_TStandardCursor cursorshape)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return window->hasCursorShape(cursorshape);
}

GHOST_TSuccess GHOST_SetCustomCursorShape(GHOST_HWND windowhandle,
										  uint8_t *bitmap,
										  uint8_t *mask,
										  int sizex,
										  int sizey,
										  int hotX,
										  int hotY,
										  bool canInvertColor)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return window->setCustomCursorShape(
		bitmap, mask, sizex, sizey, hotX, hotY, canInvertColor);
}

GHOST_TSuccess GHOST_GetCursorBitmap(GHOST_HWND windowhandle,
									 GHOST_CursorBitmapRef *bitmap)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return window->getCursorBitmap(bitmap);
}

bool GHOST_GetCursorVisibility(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->getCursorVisibility();
}

GHOST_TSuccess GHOST_SetCursorVisibility(GHOST_HWND windowhandle, bool visible)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return window->setCursorVisibility(visible);
}

GHOST_TSuccess GHOST_GetCursorPosition(const GHOST_HSYSTEM systemhandle,
									   const GHOST_HWND windowhandle,
									   int32_t *x,
									   int32_t *y)
{
	const GHOST_ISystem *system = (const GHOST_ISystem *)systemhandle;
	const GHOST_IWindow *window = (const GHOST_IWindow *)windowhandle;

	return system->getCursorPositionClientRelative(window, *x, *y);
}

GHOST_TSuccess GHOST_SetCursorPosition(GHOST_HSYSTEM systemhandle,
									   GHOST_HWND windowhandle,
									   int32_t x,
									   int32_t y)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return system->setCursorPositionClientRelative(window, x, y);
}

GHOST_TSuccess GHOST_SetCursorGrab(GHOST_HWND windowhandle,
								   GHOST_TGrabCursorMode mode,
								   GHOST_TAxisFlag wrap_axis,
								   int bounds[4],
								   const int mouse_ungrab_xy[2])
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	GHOST_Rect bounds_rect;
	int32_t mouse_xy[2];

	if (bounds) {
		bounds_rect = GHOST_Rect(bounds[0], bounds[1], bounds[2], bounds[3]);
	}
	if (mouse_ungrab_xy) {
		mouse_xy[0] = mouse_ungrab_xy[0];
		mouse_xy[1] = mouse_ungrab_xy[1];
	}

	return window->setCursorGrab(mode,
								 wrap_axis,
								 bounds ? &bounds_rect : nullptr,
								 mouse_ungrab_xy ? mouse_xy : nullptr);
}

void GHOST_GetCursorGrabState(GHOST_HWND windowhandle,
							  GHOST_TGrabCursorMode *r_mode,
							  GHOST_TAxisFlag *r_axis_flag,
							  int r_bounds[4],
							  bool *r_use_software_cursor)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	GHOST_Rect bounds_rect;
	bool use_software_cursor;
	window->getCursorGrabState(
		*r_mode, *r_axis_flag, bounds_rect, use_software_cursor);
	r_bounds[0] = bounds_rect.m_l;
	r_bounds[1] = bounds_rect.m_t;
	r_bounds[2] = bounds_rect.m_r;
	r_bounds[3] = bounds_rect.m_b;
	*r_use_software_cursor = use_software_cursor;
}

GHOST_TSuccess GHOST_GetModifierKeyState(GHOST_HSYSTEM systemhandle,
										 GHOST_TModifierKey mask,
										 bool *r_is_down)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	GHOST_TSuccess result;
	bool is_down = false;

	result = system->getModifierKeyState(mask, is_down);
	*r_is_down = is_down;

	return result;
}

GHOST_TSuccess GHOST_GetButtonState(GHOST_HSYSTEM systemhandle,
									GHOST_TButton mask,
									bool *r_is_down)
{
	GHOST_ISystem *system = (GHOST_ISystem *)systemhandle;
	GHOST_TSuccess result;
	bool is_down = false;

	result = system->getButtonState(mask, is_down);
	*r_is_down = is_down;

	return result;
}

void GHOST_SetAcceptDragOperation(GHOST_HWND windowhandle, bool can_accept)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	window->setAcceptDragOperation(can_accept);
}

GHOST_TEventType GHOST_GetEventType(GHOST_HEVENT eventhandle)
{
	GHOST_IEvent *event = (GHOST_IEvent *)eventhandle;

	return event->getType();
}

uint64_t GHOST_GetEventTime(GHOST_HEVENT eventhandle)
{
	GHOST_IEvent *event = (GHOST_IEvent *)eventhandle;

	return event->getTime();
}

GHOST_HWND GHOST_GetEventWindow(GHOST_HEVENT eventhandle)
{
	GHOST_IEvent *event = (GHOST_IEvent *)eventhandle;

	return (GHOST_HWND)event->getWindow();
}

GHOST_TEventDataPtr GHOST_GetEventData(GHOST_HEVENT eventhandle)
{
	GHOST_IEvent *event = (GHOST_IEvent *)eventhandle;

	return event->getData();
}

GHOST_TimerProcPtr GHOST_GetTimerProc(GHOST_HTTASK timertaskhandle)
{
	GHOST_ITimerTask *timertask = (GHOST_ITimerTask *)timertaskhandle;

	return timertask->getTimerProc();
}

void GHOST_SetTimerProc(GHOST_HTTASK timertaskhandle,
						GHOST_TimerProcPtr timerproc)
{
	GHOST_ITimerTask *timertask = (GHOST_ITimerTask *)timertaskhandle;

	timertask->setTimerProc(timerproc);
}

GHOST_TUserDataPtr GHOST_GetTimerTaskUserData(GHOST_HTTASK timertaskhandle)
{
	GHOST_ITimerTask *timertask = (GHOST_ITimerTask *)timertaskhandle;

	return timertask->getUserData();
}

void GHOST_SetTimerTaskUserData(GHOST_HTTASK timertaskhandle,
								GHOST_TUserDataPtr userdata)
{
	GHOST_ITimerTask *timertask = (GHOST_ITimerTask *)timertaskhandle;

	timertask->setUserData(userdata);
}

bool GHOST_GetValid(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->getValid();
}

GHOST_TDrawingContextType GHOST_GetDrawingContextType(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->getDrawingContextType();
}

GHOST_TSuccess GHOST_SetDrawingContextType(GHOST_HWND windowhandle,
										   GHOST_TDrawingContextType type)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->setDrawingContextType(type);
}

GHOST_HCONTEXT GHOST_GetDrawingContext(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return (GHOST_HCONTEXT)window->getDrawingContext();
}

void GHOST_SetTitle(GHOST_HWND windowhandle, const char *title)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	window->setTitle(title);
}

char *GHOST_GetTitle(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	std::string title = window->getTitle();

	char *ctitle = (char *)malloc(title.size() + 1);

	if (ctitle == nullptr) {
		return nullptr;
	}

	strcpy_s(ctitle, title.size() + 1, title.c_str());

	return ctitle;
}

GHOST_HRECT GHOST_GetWindowBounds(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	GHOST_Rect *rectangle = nullptr;

	rectangle = new GHOST_Rect();
	window->getWindowBounds(*rectangle);

	return (GHOST_HRECT)rectangle;
}

GHOST_HRECT GHOST_GetClientBounds(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	GHOST_Rect *rectangle = nullptr;

	rectangle = new GHOST_Rect();
	window->getClientBounds(*rectangle);

	return (GHOST_HRECT)rectangle;
}

void GHOST_DisposeRectangle(GHOST_HRECT rectanglehandle)
{
	delete (GHOST_Rect *)rectanglehandle;
}

GHOST_TSuccess GHOST_SetClientWidth(GHOST_HWND windowhandle, uint32_t width)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->setClientWidth(width);
}

GHOST_TSuccess GHOST_SetClientHeight(GHOST_HWND windowhandle, uint32_t height)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->setClientHeight(height);
}

GHOST_TSuccess GHOST_SetClientSize(GHOST_HWND windowhandle,
								   uint32_t width,
								   uint32_t height)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->setClientSize(width, height);
}

void GHOST_ScreenToClient(GHOST_HWND windowhandle,
						  int32_t inX,
						  int32_t inY,
						  int32_t *outX,
						  int32_t *outY)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	window->screenToClient(inX, inY, *outX, *outY);
}

void GHOST_ClientToScreen(GHOST_HWND windowhandle,
						  int32_t inX,
						  int32_t inY,
						  int32_t *outX,
						  int32_t *outY)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	window->clientToScreen(inX, inY, *outX, *outY);
}

GHOST_TWindowState GHOST_GetWindowState(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->getState();
}

GHOST_TSuccess GHOST_SetWindowState(GHOST_HWND windowhandle,
									GHOST_TWindowState state)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->setState(state);
}

GHOST_TSuccess GHOST_SetWindowModifiedState(GHOST_HWND windowhandle,
											bool isUnsavedChanges)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->setModifiedState(isUnsavedChanges);
}

GHOST_TSuccess GHOST_SetWindowOrder(GHOST_HWND windowhandle,
									GHOST_TWindowOrder order)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->setOrder(order);
}

GHOST_TSuccess GHOST_SwapWindowBuffers(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->swapBuffers();
}

GHOST_TSuccess GHOST_SetSwapInterval(GHOST_HWND windowhandle, int interval)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->setSwapInterval(interval);
}

GHOST_TSuccess GHOST_GetSwapInterval(GHOST_HWND windowhandle, int *intervalOut)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->getSwapInterval(*intervalOut);
}

GHOST_TSuccess GHOST_ActivateWindowDrawingContext(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->activateDrawingContext();
}

GHOST_TSuccess GHOST_ActivateOpenGLContext(GHOST_HCONTEXT contexthandle)
{
	GHOST_IContext *context = (GHOST_IContext *)contexthandle;
	if (context) {
		return context->activateDrawingContext();
	}
	return GHOST_kFailure;
}

GHOST_TSuccess GHOST_ReleaseOpenGLContext(GHOST_HCONTEXT contexthandle)
{
	GHOST_IContext *context = (GHOST_IContext *)contexthandle;

	return context->releaseDrawingContext();
}

unsigned int GHOST_GetContextDefaultOpenGLFramebuffer(
	GHOST_HCONTEXT contexthandle)
{
	GHOST_IContext *context = (GHOST_IContext *)contexthandle;

	return context->getDefaultFramebuffer();
}

unsigned int GHOST_GetDefaultOpenGLFramebuffer(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->getDefaultFramebuffer();
}

GHOST_TSuccess GHOST_InvalidateWindow(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;

	return window->invalidate();
}

int32_t GHOST_GetWidthRectangle(GHOST_HRECT rectanglehandle)
{
	return ((GHOST_Rect *)rectanglehandle)->getWidth();
}

int32_t GHOST_GetHeightRectangle(GHOST_HRECT rectanglehandle)
{
	return ((GHOST_Rect *)rectanglehandle)->getHeight();
}

void GHOST_GetRectangle(
	GHOST_HRECT rectanglehandle, int32_t *l, int32_t *t, int32_t *r, int32_t *b)
{
	GHOST_Rect *rect = (GHOST_Rect *)rectanglehandle;

	*l = rect->m_l;
	*t = rect->m_t;
	*r = rect->m_r;
	*b = rect->m_b;
}

void GHOST_SetRectangle(
	GHOST_HRECT rectanglehandle, int32_t l, int32_t t, int32_t r, int32_t b)
{
	((GHOST_Rect *)rectanglehandle)->set(l, t, r, b);
}

GHOST_TSuccess GHOST_IsEmptyRectangle(GHOST_HRECT rectanglehandle)
{
	GHOST_TSuccess result = GHOST_kFailure;

	if (((GHOST_Rect *)rectanglehandle)->isEmpty()) {
		result = GHOST_kSuccess;
	}
	return result;
}

GHOST_TSuccess GHOST_IsValidRectangle(GHOST_HRECT rectanglehandle)
{
	GHOST_TSuccess result = GHOST_kFailure;

	if (((GHOST_Rect *)rectanglehandle)->isValid()) {
		result = GHOST_kSuccess;
	}
	return result;
}

void GHOST_InsetRectangle(GHOST_HRECT rectanglehandle, int32_t i)
{
	((GHOST_Rect *)rectanglehandle)->inset(i);
}

void GHOST_UnionRectangle(GHOST_HRECT rectanglehandle,
						  GHOST_HRECT anotherrectanglehandle)
{
	((GHOST_Rect *)rectanglehandle)
		->unionRect(*(GHOST_Rect *)anotherrectanglehandle);
}

void GHOST_UnionPointRectangle(GHOST_HRECT rectanglehandle,
							   int32_t x,
							   int32_t y)
{
	((GHOST_Rect *)rectanglehandle)->unionPoint(x, y);
}

GHOST_TSuccess GHOST_IsInsideRectangle(GHOST_HRECT rectanglehandle,
									   int32_t x,
									   int32_t y)
{
	GHOST_TSuccess result = GHOST_kFailure;

	if (((GHOST_Rect *)rectanglehandle)->isInside(x, y)) {
		result = GHOST_kSuccess;
	}
	return result;
}

GHOST_TVisibility GHOST_GetRectangleVisibility(
	GHOST_HRECT rectanglehandle, GHOST_HRECT anotherrectanglehandle)
{
	GHOST_TVisibility visible = GHOST_kNotVisible;

	visible = ((GHOST_Rect *)rectanglehandle)
				  ->getVisibility(*(GHOST_Rect *)anotherrectanglehandle);

	return visible;
}

void GHOST_SetCenterRectangle(GHOST_HRECT rectanglehandle,
							  int32_t cx,
							  int32_t cy)
{
	((GHOST_Rect *)rectanglehandle)->setCenter(cx, cy);
}

void GHOST_SetRectangleCenter(
	GHOST_HRECT rectanglehandle, int32_t cx, int32_t cy, int32_t w, int32_t h)
{
	((GHOST_Rect *)rectanglehandle)->setCenter(cx, cy, w, h);
}

GHOST_TSuccess GHOST_ClipRectangle(GHOST_HRECT rectanglehandle,
								   GHOST_HRECT anotherrectanglehandle)
{
	GHOST_TSuccess result = GHOST_kFailure;

	if (((GHOST_Rect *)rectanglehandle)
			->clip(*(GHOST_Rect *)anotherrectanglehandle)) {
		result = GHOST_kSuccess;
	}
	return result;
}

char *GHOST_getClipboard(bool selection)
{
	GHOST_ISystem *system = GHOST_ISystem::getSystem();
	return system->getClipboard(selection);
}

void GHOST_putClipboard(const char *buffer, bool selection)
{
	GHOST_ISystem *system = GHOST_ISystem::getSystem();
	system->putClipboard(buffer, selection);
}

bool GHOST_setConsoleWindowState(GHOST_TConsoleWindowState action)
{
	GHOST_ISystem *system = GHOST_ISystem::getSystem();
	return system->setConsoleWindowState(action);
}

bool GHOST_UseNativePixels(void)
{
	GHOST_ISystem *system = GHOST_ISystem::getSystem();
	return system->useNativePixel();
}

bool GHOST_SupportsCursorWarp(void)
{
	GHOST_ISystem *system = GHOST_ISystem::getSystem();
	return system->supportsCursorWarp();
}

bool GHOST_SupportsWindowPosition(void)
{
	GHOST_ISystem *system = GHOST_ISystem::getSystem();
	return system->supportsWindowPosition();
}

void GHOST_SetBacktraceHandler(GHOST_TBacktraceFn backtrace_fn)
{
	GHOST_ISystem::setBacktraceFn(backtrace_fn);
}

void GHOST_UseWindowFocus(bool use_focus)
{
	GHOST_ISystem *system = GHOST_ISystem::getSystem();
	return system->useWindowFocus(use_focus);
}

float GHOST_GetNativePixelSize(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	if (window) {
		return window->getNativePixelSize();
	}
	return 1.0f;
}

uint16_t GHOST_GetDPIHint(GHOST_HWND windowhandle)
{
	GHOST_IWindow *window = (GHOST_IWindow *)windowhandle;
	return window->getDPIHint();
}
