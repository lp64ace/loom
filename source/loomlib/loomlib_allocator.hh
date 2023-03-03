#pragma once

#include "guardedalloc/mem_guardedalloc.h"

#include "loomlib_utildefines.h"
#include "loomlib_assert.h"

namespace loom {

class GuardedAllocator {
public:
	void *allocate ( size_t size , size_t alignment , const char *name ) {
		/* Should we use MEM_mallocN, when alignment is small? If yes, how small must
		 * alignment be? */
		return MEM_mallocN_aligned ( size , alignment , name );
	}

	void deallocate ( void *ptr ) {
		MEM_freeN ( ptr );
	}
};

class RawAllocator {
private:
	struct MemHead {
		int offset;
	};

public:
	void *allocate ( size_t size , size_t alignment , const char * ) {
		void *ptr = malloc ( size + alignment + sizeof ( MemHead ) );
		void *used_ptr = reinterpret_cast< void * >( reinterpret_cast< uintptr_t >( POINTER_OFFSET ( ptr , alignment + sizeof ( MemHead ) ) ) & ~( static_cast< uintptr_t >( alignment ) - 1 ) );
		int offset = static_cast< int >( ( intptr_t ) used_ptr - ( intptr_t ) ptr );
		LOOM_assert ( offset >= static_cast< int >( sizeof ( MemHead ) ) );
		( static_cast< MemHead * >( used_ptr ) - 1 )->offset = offset;
		return used_ptr;
	}

	void deallocate ( void *ptr ) {
		MemHead *head = static_cast< MemHead * >( ptr ) - 1;
		int offset = -head->offset;
		void *actual_pointer = POINTER_OFFSET ( ptr , offset );
		free ( actual_pointer );
	}
};

}  // namespace loom
