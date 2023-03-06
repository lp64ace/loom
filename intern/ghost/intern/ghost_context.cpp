#include "ghost_context.h"

#include <cstdio>
#include <cstring>

#ifdef _WIN32
#  include <epoxy/wgl.h>
#  include <tchar.h>
#
#  ifndef ERROR_PROFILE_DOES_NOT_MATCH_DEVICE
#    define ERROR_PROFILE_DOES_NOT_MATCH_DEVICE 0x7E7
#  endif
#endif

#ifdef _WIN32

bool win32_silent_chk ( bool result ) {
        if ( !result ) {
                SetLastError ( NO_ERROR );
        }

        return result;
}

bool win32_chk ( bool result , const char *file , int line , const char *text ) {
        if ( !result ) {
                LPTSTR formattedMsg = nullptr;

                DWORD error = GetLastError ( );

                const TCHAR *msg;

                DWORD count = 0;

                /* Some drivers returns a HRESULT instead of a standard error message.
                 * i.e: 0xC0072095 instead of 0x2095 for ERROR_INVALID_VERSION_ARB
                 * So strip down the error to the valid error code range. */
                switch ( error & 0x0000FFFF ) {
                        case ERROR_INVALID_VERSION_ARB:
                        msg =
                                _T ( "The specified OpenGL version and feature set are either invalid or not supported.\n" );
                        break;

                        case ERROR_INVALID_PROFILE_ARB:
                        msg =
                                _T ( "The specified OpenGL profile and feature set are either invalid or not supported.\n" );
                        break;

                        case ERROR_INVALID_PIXEL_TYPE_ARB:
                                msg = _T ( "The specified pixel type is invalid.\n" );
                        break;

                        case ERROR_INCOMPATIBLE_DEVICE_CONTEXTS_ARB:
                        msg =
                                _T ( "The device contexts specified are not compatible. "
                                     "This can occur if the device contexts are managed by "
                                     "different drivers or possibly on different graphics adapters.\n" );
                        break;

                #  ifdef WITH_GLEW_ES
                        case ERROR_INCOMPATIBLE_AFFINITY_MASKS_NV:
                        msg = _T ( "The device context(s) and rendering context have non-matching affinity masks.\n" );
                        break;

                        case ERROR_MISSING_AFFINITY_MASK_NV:
                        msg = _T ( "The rendering context does not have an affinity mask set.\n" );
                        break;
                #  endif

                        case ERROR_PROFILE_DOES_NOT_MATCH_DEVICE:
                        msg =
                                _T ( "The specified profile is intended for a device of a "
                                     "different type than the specified device.\n" );
                        break;

                        default: {
                                count = FormatMessage ( ( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                                          FORMAT_MESSAGE_IGNORE_INSERTS ) ,
                                                        nullptr ,
                                                        error ,
                                                        MAKELANGID ( LANG_NEUTRAL , SUBLANG_DEFAULT ) ,
                                                        ( LPTSTR ) ( &formattedMsg ) ,
                                                        0 ,
                                                        nullptr );

                                msg = count > 0 ? formattedMsg : _T ( "<no system message>\n" );
                                break;
                        }
                }

        #  ifndef NDEBUG
                _ftprintf (
                        stderr , _T ( "%s:%d: [%s] -> Win32 Error# (%lu): %s" ) , file , line , text , ( unsigned long ) error , msg );
        #  else
                _ftprintf ( stderr , _T ( "Win32 Error# (%lu): %s" ) , ( unsigned long ) error , msg );
        #  endif

                SetLastError ( NO_ERROR );

                if ( count != 0 ) {
                        LocalFree ( formattedMsg );
                }
        }

        return result;
}

#endif  // _WIN32

void GHOST_Context::initClearGL ( ) {
        glClearColor ( 0.294 , 0.294 , 0.294 , 0.000 );
        glClear ( GL_COLOR_BUFFER_BIT );
        glClearColor ( 0.000 , 0.000 , 0.000 , 0.000 );
}
