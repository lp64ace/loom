#pragma once

#include "loomlib_utildefines.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <iostream>

namespace loom {

template<typename _Tp> class Span;

class IndexRange {
private:
	size_t mStart = 0;
	size_t mSize = 0;

public:
	constexpr IndexRange ( ) = default;

	constexpr explicit IndexRange ( size_t size ) : mStart ( 0 ) , mSize ( size ) {
	}

	constexpr explicit IndexRange ( size_t start , size_t size ) : mStart ( start ) , mSize ( size ) {
	}

	class Iterator {
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = size_t;
		using pointer = const size_t *;
		using reference = const size_t &;
		using difference_type = std::ptrdiff_t;

	private:
		size_t _current;

	public:
		constexpr explicit Iterator ( int64_t current ) : _current ( current ) {
		}

		constexpr Iterator &operator++( ) {
			_current++;
			return *this;
		}

		constexpr Iterator operator++( int ) {
			Iterator copied_iterator = *this;
			++( *this );
			return copied_iterator;
		}

		constexpr friend bool operator!=( const Iterator &a , const Iterator &b ) {
			return a._current != b._current;
		}

		constexpr friend bool operator==( const Iterator &a , const Iterator &b ) {
			return a._current == b._current;
		}

		constexpr size_t operator*( ) const {
			return _current;
		}
	};

	/* -------------------------------------------------------------------- */
	/** \name Utility functions
	 * \{ */

	constexpr Iterator begin ( ) const {
		return Iterator ( mStart );
	}

	constexpr Iterator end ( ) const {
		return Iterator ( mStart + mSize );
	}

	/** \} */

	constexpr Iterator Begin ( ) const {
		return Iterator ( mStart );
	}

	constexpr Iterator End ( ) const {
		return Iterator ( mStart + mSize );
	}

	constexpr size_t operator[]( size_t index ) const {
		return mStart + index;
	}

	constexpr friend bool operator==( IndexRange a , IndexRange b ) {
		return ( a.mSize == b.mSize ) && ( a.mStart == b.mStart );
	}

	// Get the first element in the range. The returned value is undefined when the range is
	// empty.
	constexpr size_t Start ( ) const {
		return mStart;
	}

	// Get the amount of numbers in the range.
	constexpr size_t Size ( ) const {
		return mSize;
	}

	// Returns true if the size is zero.
	constexpr size_t Empty ( ) const {
		return mSize == 0;
	}

	// Create a new range starting at the end of the current one.
	constexpr IndexRange After ( size_t n ) const {
		return IndexRange ( mStart + mSize , n );
	}

	// Create a new range that ends at the start of the current one.
	constexpr IndexRange Before ( size_t n ) const {
		return IndexRange ( mStart - n , n );
	}

	// Get the first element in the range. Asserts when the range is empty.
	constexpr size_t First ( ) const {
		return mStart;
	}

	// Get the nth last element in the range. Asserts when the range is empty or when n is
	// negative.
	constexpr size_t Last ( const size_t n = 0 ) const {
		return mStart + mSize - 1 - n;
	}

	/** Get the element one before the beginning. The returned value is undefined when the
	 * range is empty, and the range must start after zero already. */
	constexpr size_t OneBeforeStart ( ) const {
		return mStart - 1;
	}

	// Get the element one after the end. The returned value is undefined when the range is
	// empty.
	constexpr size_t OneAfterLast ( ) const {
		return mStart + mSize;
	}

	// Returns true when the range contains a certain number, otherwise false.
	constexpr bool Contains ( size_t value ) const {
		return mStart <= value && value < mStart + mSize;
	}

	// Returns a new range, that contains a sub-interval of the current one.
	constexpr IndexRange Slice ( size_t start , size_t size ) const {
		size_t new_start = mStart + start;
		return IndexRange ( new_start , size );
	}

	constexpr IndexRange Slice ( IndexRange range ) const {
		return this->Slice ( range.Start ( ) , range.Size ( ) );
	}

	/** Returns a new IndexRange with n elements removed from the beginning of the range.
	 * This invokes undefined behavior when n is negative. */
	constexpr IndexRange DropFront ( size_t n ) const {
		const ptrdiff_t new_size = std::max<ptrdiff_t> ( 0 , mSize - n );
		return IndexRange ( mStart + n , new_size );
	}

	/** Returns a new IndexRange with n elements removed from the end of the range.
	 * This invokes undefined behavior when n is negative. */
	constexpr IndexRange drop_back ( int64_t n ) const {
		const ptrdiff_t new_size = std::max<ptrdiff_t> ( 0 , mSize - n );
		return IndexRange ( mStart , new_size );
	}

	/** Returns a new IndexRange that only contains the first n elements. This invokes
	 * undefined behavior when n is negative. */
	constexpr IndexRange TakeFront ( size_t n ) const {
		const ptrdiff_t new_size = std::min<ptrdiff_t> ( mSize , n );
		return IndexRange ( mStart , new_size );
	}

	/** Returns a new IndexRange that only contains the last n elements. This invokes undefined
	 * behavior when n is negative. */
	constexpr IndexRange TakeBack ( size_t n ) const {
		const ptrdiff_t new_size = std::min<ptrdiff_t> ( mSize , n );
		return IndexRange ( mStart + mSize - new_size , new_size );
	}

	/** Move the range forward or backward within the larger array. The amount may be negative,
	 * but its absolute value cannot be greater than the existing start of the range. */
	constexpr IndexRange Shift ( int64_t n ) const {
		return IndexRange ( mStart + n , mSize );
	}

	// Get read-only access to a memory buffer that contains the range as actual numbers.
	Span<size_t> AsSpan ( ) const;
private:
	static std::atomic<size_t> s_current_array_size;
	static std::atomic<size_t *> s_current_array;

	Span<size_t> AsSpanInternal ( ) const;
};

struct AlignedIndexRanges {
	IndexRange prefix;
	IndexRange aligned;
	IndexRange suffix;
};

/** Split a range into three parts so that the boundaries of the middle part are aligned to some
 * power of two.
 *
 * This can be used when an algorithm can be optimized on aligned indices/memory. The algorithm
 * then needs a slow path for the beginning and end, and a fast path for the aligned elements. */
AlignedIndexRanges split_index_range_by_alignment ( const IndexRange range , const size_t alignment );

}  // namespace loom
