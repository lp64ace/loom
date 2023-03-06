#pragma once

#include "ghost_types.h"

class GHOST_IntContext {
public:
	/**
	 * Destruct the context.
	 */
	virtual ~GHOST_IntContext ( ) = default;

	/**
	 * Make the specified rendering context the calling thread's current rendering context.
	 * All subsequent context calls made by the thread will be drawn on the specified device. 
	 * \return If the function succeds, the return value is nonzero, otherwise the return 
	 * value is GHOST_kFailure.
	 */
	virtual GHOST_TStatus ActivateDrawingContext ( ) = 0;

	/**
	 * Makes the specified rendering context so that it's no longer current. 
	 * \return If the function succeds, the return value is nonzero, otherwise the return 
	 * value is GHOST_kFailure.
	 */
	virtual GHOST_TStatus ReleaseDrawingContext ( ) = 0;

	/**
	 * Swaps the front and back buffers of the device.
	 * \return If the function succeds, the return value is nonzero, otherwise the return 
	 * value is GHOST_kFailure.
	 */
	virtual GHOST_TStatus SwapBuffers ( ) = 0;
};
