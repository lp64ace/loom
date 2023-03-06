#include <stdexcept>

#include "ghost/ghost_isystem.h"

#include "ghost_systemheadless.h"

#if defined(WIN32) || defined(_WIN32)
#  include "GHOST_systemwin32.h"
#else
#  error The system we are currently running is not a supported build system for GHOST.
#endif

GHOST_ISystem *GHOST_ISystem::m_system = nullptr;
const char *GHOST_ISystem::m_system_backend_id = nullptr;

GHOST_TBacktraceFn GHOST_ISystem::m_backtrace_fn = nullptr;

GHOST_TSuccess GHOST_ISystem::createSystem ( bool verbose , [[maybe_unused]] bool background ) {
        /* When GHOST fails to start, report the back-ends that were attempted.
         * A Verbose argument could be supported in printing isn't always desired. */
        const char *backends_attempted [ 8 ] = { nullptr };
        int backends_attempted_num = 0;

        GHOST_TSuccess success;
        if ( !m_system ) {

        #if defined(WITH_HEADLESS)
                /* Pass. */
        #elif defined(WIN32) || defined(_WIN32)
                backends_attempted [ backends_attempted_num++ ] = "WIN32";
                m_system = new GHOST_SystemWin32 ( );
        #endif

                if ( m_system ) {
                        m_system_backend_id = backends_attempted [ backends_attempted_num - 1 ];
                } else if ( verbose ) {
                        fprintf ( stderr , "GHOST: failed to initialize display for back-end(s): [" );
                        for ( int i = 0; i < backends_attempted_num; i++ ) {
                                if ( i != 0 ) {
                                        fprintf ( stderr , ", " );
                                }
                                fprintf ( stderr , "'%s'" , backends_attempted [ i ] );
                        }
                        fprintf ( stderr , "]\n" );
                }

                success = m_system != nullptr ? GHOST_kSuccess : GHOST_kFailure;
        } else {
                success = GHOST_kFailure;
        }
        if ( success ) {
                success = m_system->init ( );
        }
        return success;
}

GHOST_TSuccess GHOST_ISystem::createSystemBackground ( ) {
        GHOST_TSuccess success;
        if ( !m_system ) {
        #if !defined(WITH_HEADLESS)
                /* Try to create a off-screen render surface with the graphical systems. */
                success = createSystem ( false , true );
                if ( success ) {
                        return success;
                }
                /* Try to fallback to headless mode if all else fails. */
        #endif
                m_system = new GHOST_SystemHeadless ( );
                success = m_system != nullptr ? GHOST_kSuccess : GHOST_kFailure;
        } else {
                success = GHOST_kFailure;
        }
        if ( success ) {
                success = m_system->init ( );
        }
        return success;
}

GHOST_TSuccess GHOST_ISystem::disposeSystem ( ) {
        GHOST_TSuccess success = GHOST_kSuccess;
        if ( m_system ) {
                delete m_system;
                m_system = nullptr;
        } else {
                success = GHOST_kFailure;
        }
        return success;
}

GHOST_ISystem *GHOST_ISystem::getSystem ( ) {
        return m_system;
}

const char *GHOST_ISystem::getSystemBackend ( ) {
        return m_system_backend_id;
}

GHOST_TBacktraceFn GHOST_ISystem::getBacktraceFn ( ) {
        return GHOST_ISystem::m_backtrace_fn;
}

void GHOST_ISystem::setBacktraceFn ( GHOST_TBacktraceFn backtrace_fn ) {
        GHOST_ISystem::m_backtrace_fn = backtrace_fn;
}

