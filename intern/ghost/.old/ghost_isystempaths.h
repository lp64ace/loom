#pragma once

#include "ghost_types.h"

class GHOST_IntSystemPaths {
	static GHOST_IntSystemPaths *s_SystemPaths;
public:
	/**
	 * Creates the one and only system for system paths.
	 */
	static GHOST_IntSystemPaths *Create ( );

	/**
	 * Disposes of the one and only system.
	 */
	static GHOST_TStatus Dispose ( );

	/**
	 * Returns a pointer to the system ( NULL if it hasn't been created ).
	 * \return A pointer to the system.
	 */
	static GHOST_IntSystemPaths *Get ( );
public:
	/**
	 * Determine the base dir in which shared resources are located. It will first try to use
	 * "unpack and run" path, then look for properly installed path, including versioning.
	 * \return Unsigned char string pointing to system dir (eg /usr/share/rose/).
	 */
	virtual const char *GetSystemDir ( int version , const char *versionstr ) const = 0;

	/**
	 * Determine the base dir in which user configuration is stored, including versioning.
	 * If needed, it will create the base directory.
	 * \return Unsigned char string pointing to user dir (eg ~/.rose/).
	 */
	virtual const char *GetUserDir ( int version , const char *versionstr ) const = 0;

	/**
	 * Determine a special ("well known") and easy to reach user directory.
	 * \return Unsigned char string pointing to user dir (eg `~/Documents/`).
	 */
	virtual const char *GetUserSpecialDir ( GHOST_TUserSpecialDirTypes type ) const = 0;

	/**
	 * Determine the directory of the current binary
	 * \return Unsigned char string pointing to the binary dir
	 */
	virtual const char *GetBinaryDir ( ) const = 0;

	/**
	 * Add the file to the operating system most recently used files
	 */
	virtual void AddToSystemRecentFiles ( const char *filename ) const = 0;
protected:
	GHOST_IntSystemPaths ( ) = default;
	virtual ~GHOST_IntSystemPaths ( ) = default;
};
