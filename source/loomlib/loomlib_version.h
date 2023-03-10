#pragma once

#define LOOM_VERSION_MAJOR 0
#define LOOM_VERSION_MINOR 0
#define LOOM_VERSION_REVISION 0
#define LOOM_VERSION_STATUS "-dev"

#define LOOMAUX_STR_EXP(__A) #__A
#define LOOMAUX_STR(__A) LOOMAUX_STR_EXP(__A)

#define LOOMAUX_STRW_EXP(__A) L## #__A
#define LOOMAUX_STRW(__A) LOOMAUX_STRW_EXP(__A)

#define ROSE_VERSION \
	LOOMAUX_STR(LOOM_VERSION_MAJOR) \
	"." LOOMAUX_STR(LOOM_VERSION_MINOR) "." LOOMAUX_STR(LOOM_VERSION_REVISION) \
		LOOM_VERSION_STATUS

#define LOOM_MAJOR_VERSION LOOM_VERSION_MAJOR
#define LOOM_MINOR_VERSION LOOM_VERSION_MINOR
#define LOOM_SUBMINOR_VERSION LOOM_VERSION_REVISION
