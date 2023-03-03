#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Link {
	struct Link *prev , *next;
} Link;

typedef struct LinkData {
	struct Link *prev , *next;
	void *data;
} LinkData;

typedef struct ListBase {
	void *first , *last;
} ListBase;

#ifdef __cplusplus
}
#endif
