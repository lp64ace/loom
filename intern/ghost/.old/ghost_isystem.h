#pragma once

#include "ghost_types.h"

class GHOST_IntTimerTask;
class GHOST_IntWindow;
class GHOST_IntEventConsumer;

class GHOST_IntSystem {
	static GHOST_IntSystem *s_GhostSystem;
protected:
	
public:
	/**
	 * Create the one and only 'GHOST' system, GHOST_System constructor is protected so 
	 * that we can only create a system using this method, denying duplicates. This will 
	 * select the apropriate system implementation based on the platform.
	 * \return If the function succeds, the return value is nonzero, otherwise the return 
	 * is GOHST_kFailure.
	 */
	static GHOST_TStatus CreateSystem ( );

	/** 
	 * Property disposes and frees all specified system resources of 'GHOST', calling this 
	 * without first calling #CreateSystem will cause this function to fail.
	 * \return If the function succeds, the return value is nonzero, otherwise the return 
	 * is GHOST_kFailure.
	 */
	static GHOST_TStatus DisposeSystem ( );

	/**
	 * Returns a pointer to the 'GHOST' system previously initialized with #CreateSystem. 
	 * \return If a system has been created, the return value is a pointer to the system, 
	 * otherwise this function will return NULL.
	 */
	static GHOST_IntSystem *GetSystem ( );
public:
	/** 
	 * Mark the application's exit flag, use #GetSystemShouldExit to get the value 
	 * of this flag. This flag is usually set by the system when a 'quit' message 
	 * is received, the application though may choose to ignore this if need be. 
	 */
	virtual void SetSystemShouldExit ( bool should_exit ) = 0;

	/**
	 * Returns the application's exit flag, you can use #SetSystemShouldExit to 
	 * manipulate this flag, this value is purely for user usage and will not 
	 * shut down the system by itself.
	 * \return Returns \c True if the application should exit, \c False otherwise.
	 */
	virtual bool GetSystemShouldExit ( void ) = 0;
public:
	/**
	 * Show a system message box with the specified title, message and options. 
	 * \param title The title of the message box.
	 * \param message The message to display in the message box.
	 * \param help_label The label for the help button.
	 * \param continue_label The label for the continue button.
	 * \param link The link to open when the help button is pressed.
	 * \param options The options for the message box, any combination of #GHOST_DialogOptions.
	 */
	virtual GHOST_TStatus ShowMessageBox ( const char *title ,
					       const char *message ,
					       const char *help_label ,
					       const char *continue_label ,
					       const char *link ,
					       GHOST_Enum options ) const = 0;

	/** 
	 * Set the console window's state.
	 * \param show If \c True the console window will be shown, if \c False the console.
	 * \return If the function succeds, the return value is nonzero, otherwise the return 
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetConsoleWindowState ( bool show ) const = 0;
public:
	/**
	 * Returns the system time, this is the time since the system was started, based 
	 * on ANSI clock() routine, this is more accurate than GetMilliseconds.
	 * \return The number of seconds elapsed since the start of the process.
	 */
	virtual GHOST_TimePoint GetTime ( ) const = 0;

	/** 
	 * Returns the system time, return the number of milliseconds elapsed since the start 
	 * of the system process, based on WIN32 GetTickCount64() routine, this is less accurate than 
	 * #GetTime.
	 * \return The number of elapsed milliseconds since the start of the process.
	 */
	virtual uint64_t GetMilliseconds ( ) const = 0;

	/** 
	 * Install a timer to the system, note that on most operating systems, messages need to be 
	 * processed in order for the timer callbacks to be invoked.
	 * \param delay The time to wait for the first call to \a proc ( in milliseconds )
	 * \param interval The time to wait between calls to \a proc ( in milliseconds )
	 * \param proc The callback function to call when the timer expires.
	 * \param user The user data to pass to the callback function.
	 * \return A timer task (NULL if timer task installation failed).
	 */
	virtual GHOST_IntTimerTask *InstallTimer ( uint64_t delay ,
						   uint64_t interval ,
						   GHOST_TimerProcPtr proc ,
						   GHOST_UserDataPtr user ) = 0;

	/**
	 * Remove a timer from the timer manager of this system, similar to #KillTimer from WIN32. 
	 * \param tiemr The timer task to be removed. 
	 * \return If the function succeds the return value is nonzero, otherwise the return 
	 * value is GHOST_kFailure indicating failure.
	 */
	virtual GHOST_TStatus RemoveTimer ( GHOST_IntTimerTask *timer ) = 0;
public:
	/** 
	 * Returns the number of displays installed in this system.
	 * \return Returns the number of monitors currently active on the system.
	 */
	virtual uint8_t GetNumDisplays ( ) const = 0;

	/** 
	 * Retrieves the dimensions of the main display on this system. 
	 * \param width A pointer to the variable that should receive the width of the 
	 * main display.
	 * \param height A pointer to the variable that should receive the height of the 
	 * main dispaly.
	 */
	virtual void GetMainDisplayDimensions ( uint32_t *width , uint32_t *height ) const = 0;
	
	/**
	 * Retrieves the combine dimensions of all monitors on this system. 
	 * \param width A pointer to the variable that should receive the width of all the 
	 * monitors.
	 * \param height A pointer to the variable that should receive the height of all the 
	 * monitors.
	 */
	virtual void GetAllDisplayDimensions ( uint32_t *width , uint32_t *height ) const = 0;

	/** 
	 * Find the widnow under the cursor coordinate and return the window pointer.
	 * \param x The x coordinate of the cursor.
	 * \param y The y coordinate of the cursor.
	 * \return Returns the window under the specified coordinates or NULL if no window 
	 * was found at the specified location.
	 */
	virtual GHOST_IntWindow *GetWindowUnderCursor ( uint32_t x , uint32_t y ) const = 0;
public:
	/** 
	 * Returns the current location of the cursor (location is in screen coordinates) 
	 * \param x The x-coordinate of the cursor.
	 * \param y The y-coordinate of the cursor.
	 * \return If the function succeds the return value is GHOST_kSuccess, otherwise 
	 * the return value is GHOST_kFailure.
	 */
	virtual GHOST_TStatus GetCurrentPosition ( int *x , int *y ) const = 0;

	/**
	 * Sets the current location of the cursor (location is in screen coordinates)
	 * \param x The x-coordinate of the cursor.
	 * \param y The y-coordinate of the cursor.
	 * \return If the function succeds the return value is GHOST_kSuccess, otherwise
	 * the return value is GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetCursorPosition ( int x , int y ) = 0;
public:
	/** 
	 * Retrieves events from the system and stores them in the queue.
	 * \param wait If \c True the function will wait for an event to occur, if \c False 
	 * the function will return immediately if no events were registered on the queue.
	 * \return Returns \c True if any events were retireved from the system, \c False 
	 * otherwise.
	 */
	virtual bool ProcessEvents ( bool wait ) = 0;

	/**
	 * Dispatch all the events in the queue to the registered event consumers.
	 */
	virtual void DispatchEvents ( ) = 0;
public:
	/** 
	 * Create a new window. The new window is added to the list of windows managed. 
	 * Never explicity delete the window, use #DisposeWindow instead.
	 * \param title The title of the window.
	 * \param left The left coordinate of the window.
	 * \param top The top coordinate of the window. 
	 * \param width The width of the window.
	 * \param height The height of the window.
	 * \param state The state of the window, one of #GHOST_kWindowState.
	 * \param type The type of drawing context installed in this window, one of #GHOST_kDrawingContextType.
	 * \param parent The parent window of this window, if this is a subwindow.
	 * \return Returns the new window if the window was created successfully, otherwise 
	 * the return value is NULL.
	 */
	virtual GHOST_IntWindow *SpawnWindow (
		const char *title ,
		int left,
		int top,
		int width,
		int height,
		GHOST_TWindowState state = GHOST_kWindowStateNormal ,
		GHOST_TDrawingContextType type = GHOST_kDrawingContextTypeNone ,
		const GHOST_IntWindow *parent = NULL ) = 0;

	/**
	 * Checks if the window is registered and active.
	 * \return Returns \c True if the window is listed and the quick os check returned successfully, or 
	 * \c False otherwise.
	 */
	virtual bool IsWindowValid ( GHOST_IntWindow *window ) const = 0;

	/**
	 * Dipose a window.
	 * \note Also deletes the window, the window memory is invalidated after this function exits.
	 * \param window Pointer to teh window we want to dispose.
	 * \return If the function succeds the return value is GHOST_kSuccess, otherwise
	 * the return value is GHOST_kFailure.
	 */
	virtual GHOST_TStatus DisposeWindow ( GHOST_IntWindow *window ) = 0;
public:
	/** 
	 * Adds a consumer to the list of event consumers.
	 * \param consumer The consumer we want to add to the list.
	 * \return If the function succeds the return value is GHOST_kSuccess, otherwise
	 * the return value is GHOST_kFailure.
	 */
	virtual GHOST_TStatus AddConsumer ( GHOST_IntEventConsumer *consumer ) = 0;

	/**
	 * Removes a consumer from the list of event consumers.
	 * \param consumer The consumer we want to remove from the list.
	 * \return If the function succeds the return value is GHOST_kSuccess, otherwise
	 * the return value is GHOST_kFailure.
	 */
	virtual GHOST_TStatus RemoveConsumer ( GHOST_IntEventConsumer *consumer ) = 0;
protected:
	/** 
	 * This is protected so that the user will use the #CreateSystem method instead. 
	 * To delete the system use the dispose system method.
	 */
	GHOST_IntSystem ( ) = default;

	/**
	 * The system should be created though the #CreateSystem method and deleted through 
	 * the dispose system method.
	 */
	virtual ~GHOST_IntSystem ( ) = default;
protected:
	/** 
	 * Called right after the system was created and the default managers have all been 
	 * created.
	 * \return If the function succeds the return value is GHOST_kSuccess, otherwise
	 * the return value is GHOST_kFailure.
	 */
	virtual GHOST_TStatus Init ( ) { return GHOST_kSuccess; }

	/** 
	 * Called before the system is disposed, it can be assumed that the managers have not 
	 * yet been deleted.
	 * \return If the function succeds the return value is GHOST_kSuccess, otherwise
	 * the return value is GHOST_kFailure.
	 */
	virtual GHOST_TStatus Exit ( ) { return GHOST_kFailure; }
};
