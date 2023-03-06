#pragma once

#include "ghost_types.h"

#include <string>

class GHOST_IntWindow {
public:
	GHOST_IntWindow ( ) = default;
	virtual ~GHOST_IntWindow ( ) = default;

	/** 
	 * Returns indication as to wether the window is valid.
	 * \return The validity of the window.
	 */
	virtual bool GetValid ( ) const = 0;

	/** 
	 * Returns the associated OS object/handle.
	 * \return The associated OS object/handle.
	 */
	virtual void *GetOSWindow ( ) const = 0;

	/** 
	 * Returns the type of drawing context used in this window.
	 * \return The current drawing context.
	 */
	virtual GHOST_TDrawingContextType GetDrawingContextType ( ) const = 0;

	/** 
	 * Attempts to install the specified rendering context in this window. 
	 * \param type The type of drawing context to install. 
	 * \return If the function succeds, the return value is nonzero, otherwise the return 
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetDrawingContexType ( GHOST_TDrawingContextType type ) = 0;

	/**
	 * Swaps the front and back buffers of the device.
	 * \return If the function succeds, the return value is nonzero, otherwise the return 
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SwapBuffers ( ) = 0;

	/**
	 * Sets the swap interval for #SwapBuffers.
	 * \param interval The swap interval we want to use.
	 * \return If the function succeds, the return value is nonzero, otherwise the return 
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetSwapInterval ( int interval ) const = 0;

	/** 
	 * Get the current swap interval for #SwapBuffers.
	 * \param interval pointer to location to return swap interval.
	 * \return If the function succeds, the return value is nonzero, otherwise the return 
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus GetSwapInterval ( int *interval ) const = 0;

	/**
	 * Activates the drawing context of this window.
	 * \return If the function succeds, the return value is nonzero, otherwise the return 
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus ActivateDrawingContext ( ) = 0;
	
	/**
	 * Releases the drawing context of this window.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus ReleaseDrawingContext ( ) = 0;
	
	/**
	 * Sets the title displayed in the title bar of the window.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetTitle ( const char *title ) = 0;

	/**
	 * Returns the title displayed in the title bar.
	 * \return The title displayed in the title bar.
	 */
	virtual std::string GetTitle ( ) const = 0;

	/** 
	 * Retrieves the dimensions of the bounding rectanghle of the specified window. 
	 * The dimensions are given in screen coordiantes that are relative to the upper-left 
	 * corner of the screen.
	 * \param bounds The bounding rectangle of the window.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus GetWindowBounds ( GHOST_Rect *bounds ) const = 0;

	/**
	 * Retrieves the coordinates of a window's client area.
	 * The client coordinates specify the upper-left and lower-right corners of the client
	 * area. Because client coordinates are relative to the upper-left corner of a window's
	 * client area, the coordinates of the upper-left corner are (0,0). \param bounds: The
	 * bounding rectangle of the window.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus GetClientBounds ( GHOST_Rect *bounds ) const = 0;

	/**
	 * Resizes client rectangle width.
	 * \param width: The new width of the client area of the window.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetClientWidth ( int width ) const = 0;

	/**
	 * Resizes client rectangle height.
	 * \param height: The new height of the client area of the window.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetClientHeight ( int width ) const = 0;

	/**
	 * Resizes client rectangle.
	 * \param width The new width of the client area of the window.
	 * \param height The new height of the client area of the window.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetClientSize ( int width , int height ) const = 0;

	/**
	 * The #ScreenToClient method converts the screen coordinates of a specified point on the 
	 * screen to client-area coordinates.
	 * \param p_x The x coordinate of the point to be converted, if this is NULL it's value 
	 * is ignored.
	 * \param p_y The y coordinate of the point to be converted, if this is NULL it's value 
	 * is ignored.
	 */
	virtual void ScreenToClient ( int *p_x , int *p_y ) const = 0;

	/**
	 * The #ClientToScreen method converts the client coordinates of a specified point on the 
	 * client to screen-area coordinates.
	 * \param p_x The x coordinate of the point to be converted, if this is NULL it's value 
	 * is ignored.
	 * \param p_y The y coordinate of the point to be converted, if this is NULL it's value 
	 * is ignored.
	 */
	virtual void ClientToScreen ( int *p_x , int *p_y ) const = 0;

	/**
	 * Tells if the ongoing drag'n'drop can be accepted upon mouse drop.
	 */
	virtual void SetAcceptDragOperation ( bool accept ) = 0;

	/**
	 * Returns acceptanc of the dropped object.
	 * Usually called bny the 'object dropped' event handing function.
	 */
	virtual bool CanAcceptDragOperation ( ) const = 0;

	/**
	 * Set the state of the window (normal,minimized,maximized).
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetState ( GHOST_TWindowState ) const = 0;

	/**
	 * Returns the state of the window (normal,minimized,maximized).
	 * \return Returns the state of the window.
	 */
	virtual GHOST_TWindowState GetState ( ) const = 0;

	/**
	 * Set the windows' order (bottom, top).
	 * \param orer The order to set the window to.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetOrder ( GHOST_TWindowOrder order ) = 0;

	/*
	 * Get the current curshop shape.
	 * \return Returns the current shape of the cursor.
	 */
	virtual GHOST_TStandardCursor GetCursorShape ( ) const = 0;

	/** 
	 * Handy operation to set the cursor to any of the standard cursors provided.
	 * \param cursor The cursor shape to set the cursor to.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetCursorShape ( GHOST_TStandardCursor cursor ) = 0;

	/**
	 * Get the cursor's boundaries for the grab operation.
	 * \param bounds The boundaries of the cursor.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus GetCursorGrabBounds ( GHOST_Rect *bounds ) = 0;

	/** 
	 * Get information about the current cursor's grab state.
	 * \param mode A pointer to a GHOST_TGrabCursorMode to store the current grab mode.
	 * \param flag A pointer to a GHOST_TAxisFlag flag to store the current axis flags.
	 * \param bounds A pointer to a GHOST_Rect to store the current cursor boundaries.
	 */
	virtual void GetCursorGrabState ( GHOST_TGrabCursorMode *mode ,
					  GHOST_TAxisFlag *flag ,
					  GHOST_Rect *bounds ,
					  bool *use_software_cursor ) = 0;

	virtual bool GetCursorGrabUseSoftwareDisplay ( ) = 0;

	/**
	 * Set the cursor using custom bitmap data.
	 * \param bitmap The bitmap data for the cursor.
	 * \param mask The mask data for the cursor.
	 * \param sizex The width of the cursor in pixels.
	 * \param sizey The height of the cursor in pixels.
	 * \param hotx The x coordinate of the cursor's hot spot.
	 * \param hoty The y coordinate of the cursor's hot spot.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetCustonCursorShape ( uint8_t *bitmap ,
						     uint8_t *mask ,
						     int sizex ,
						     int sizey ,
						     int hotx ,
						     int hoty ,
						     bool invert ) = 0;

	/**
	 * Retrieve the current cursor's bitmap.
	 * \param bitmap A pointer to a GHOST_CursorBitmapRef to store the current cursor bitmap.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus GetCursorBitmap ( GHOST_CursorBitmapRef *bitmap ) = 0;

	/**
	 * Returns wether or not the cursor is visible on the window.
	 * \return Returns \c True if the cursor is currently visible, \c False otherwise.
	 */
	virtual bool GetCursorVisibility ( ) const = 0;

	/**
	 * Sets the visibility of the cursor on the window.
	 * \param show The visibility of the cursor.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetCursorVisibility ( bool show ) = 0;

	/** 
	 * Start a cursor grab operation.
	 * \param mode The grab mode to use.
	 * \param flag The axis we want to grab the mouse at.
	 * \param rect The boundaries of the cursor.
	 * \param ungrab_xy The x and y coordinates of the cursor when the grab is released.
	 * \return If the function succeds, the return value is nonzero, otherwise the return
	 * will be GHOST_kFailure.
	 */
	virtual GHOST_TStatus SetCursorGrab ( GHOST_TGrabCursorMode mode ,
					      GHOST_TAxisFlag flag ,
					      GHOST_Rect *rect ,
					      int ungrab_xy [ 2 ] ) = 0;

	/** 
	 * \return Returns the user data assigned to the window.
	 */
	virtual GHOST_UserDataPtr GetUserData ( ) const = 0;

	/*
	 * \return Updates the window's user data.
	 */
	virtual void SetUserData ( GHOST_UserDataPtr data ) = 0;
};
