#include "ghost/ghost_isystempaths.h"

#if defined(WIN32) || defined(_WIN32)
#	include "ghost_systempathswin32.h"
#endif

GHOST_ISystemPaths *GHOST_ISystemPaths::m_systemPaths = NULL;

GHOST_TSuccess GHOST_ISystemPaths::create()
{
	GHOST_TSuccess success;
	if (!m_systemPaths) {
#if defined(WIN32) || defined(_WIN32)
		m_systemPaths = new GHOST_SystemPathsWin32();
#endif
		success = m_systemPaths != nullptr ? GHOST_kSuccess : GHOST_kFailure;
	}
	else {
		success = GHOST_kFailure;
	}
	return success;
}

GHOST_TSuccess GHOST_ISystemPaths::dispose()
{
	GHOST_TSuccess success = GHOST_kSuccess;
	if (m_systemPaths) {
		delete m_systemPaths;
		m_systemPaths = nullptr;
	}
	else {
		success = GHOST_kFailure;
	}
	return success;
}

GHOST_ISystemPaths *GHOST_ISystemPaths::get()
{
	if (!m_systemPaths) {
		create();
	}
	return m_systemPaths;
}
