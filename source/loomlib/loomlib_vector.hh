#pragma once

#include "loomlib_utildefines.h"

#include "loomlib_allocator.hh"
#include "loomlib_index_range.hh"
#include "loomlib_memory_utils.hh"
#include "loomlib_span.hh"

namespace loom {

template<
	// Type of the values stored in this vector. It has to be movable.
	typename _Tp ,
	/** The number of values that can be stored in this vector, without doing a heap
	 * allocation. Sometimes it makes sense to increase this value a lot. The memory in the
	 * inline buffer is not initialized when it is not needed.
	 *
	 * When _Tpis large, the small buffer optimization is disabled by default to avoid large
	 * unexpected allocations on the stack. It can still be enabled explicitly though. */
	size_t _InlineBufferCapacity = default_inline_buffer_capacity ( sizeof ( _Tp ) ) ,
	/** The allocator used by this vector. Should rarely be changed, except when you don't want
	 * that MEM_* is used internally. */
	typename _Allocator = GuardedAllocator>
class Vector {
public:
	using value_type = _Tp;
	using pointer = _Tp *;
	using const_pointer = const _Tp *;
	using reference = _Tp &;
	using const_reference = const _Tp &;
	using iterator = _Tp *;
	using const_iterator = const _Tp *;
	using size_type = size_t;

private:
	/** Use pointers instead of storing the size explicitly. This reduces the number of
	 * instructions in `append`.
	 *
	 * The pointers might point to the memory in the inline buffer. */
	_Tp *mBegin;
	_Tp *mEnd;
	_Tp *mCapacityEnd;

	// Used for allocations when the inline buffer is too small.
	LOOM_NO_UNIQUE_ADDRESS _Allocator mAllocator;

	// A placeholder buffer that will remain uninitialized until it is used.
	LOOM_NO_UNIQUE_ADDRESS TypedBuffer<_Tp , _InlineBufferCapacity> mInlineBuffer;

	/** Be a friend with other vector instantiations. This is necessary to implement some
	 * memory management logic. */
	template<typename Other_Tp , size_t OtherInlineBufferCapacity , typename OtherAllocator> friend class Vector;

public:
	// Create an empty vector.
	Vector ( _Allocator allocator = {} ) noexcept : mAllocator ( allocator ) {
		mBegin = mInlineBuffer;
		mEnd = mBegin;
		mCapacityEnd = mBegin + _InlineBufferCapacity;
	}

	Vector ( NoExceptConstructor , _Allocator allocator = {} ) noexcept : Vector ( allocator ) {
	}

	/** Create a vector with a specific size.
	 * The elements will be default constructed.
	 * If _Tpis trivially constructible, the elements in the vector are not touched. */
	explicit Vector ( size_t size , _Allocator allocator = {} ) : Vector ( allocator ) {
		this->Resize ( size );
	}

	// Create a vector filled with a specific value.
	Vector ( size_t size , const _Tp &value , _Allocator allocator = {} ) : Vector ( allocator ) {
		this->Resize ( size , value );
	}

	// Create a vector from a span. The values in the vector are copy constructed.
	template<typename U , LOOM_ENABLE_IF ( ( std::is_convertible_v<U , _Tp> ) )> Vector ( Span<U> values , _Allocator allocator = {} ) : Vector ( NoExceptConstructor ( ) , allocator ) {
		const size_t size = values.Size ( );
		this->Reserve ( size );
		uninitialized_convert_n<U , _Tp> ( values.Data ( ) , size , mBegin );
		this->IncreaseSizeByUnchecked ( size );
	}

	/** Create a vector that contains copies of the values in the initialized list.
	 * This allows you to write code like:
	 * Vector<int> vec = {3, 4, 5}; */
	template<typename U , LOOM_ENABLE_IF ( ( std::is_convertible_v<U , _Tp> ) )> Vector ( const std::initializer_list<U> &values ) : Vector ( Span<U> ( values ) ) {
	}

	Vector ( const std::initializer_list<_Tp> &values ) : Vector ( Span<_Tp> ( values ) ) {
	}

	template<typename U , size_t N , LOOM_ENABLE_IF ( ( std::is_convertible_v<U , _Tp> ) )> Vector ( const std::array<U , N> &values ) : Vector ( Span ( values ) ) {
	}

	template<typename InputIt ,
		/* This constructor should not be called with e.g. Vector(3, 10), because that is
		 * expected to produce the vector (10, 10, 10). */
		LOOM_ENABLE_IF ( ( !std::is_convertible_v<InputIt , int> ) )>
	Vector ( InputIt first , InputIt last , _Allocator allocator = {} ) : Vector ( NoExceptConstructor ( ) , allocator ) {
		for ( InputIt current = first; current != last; ++current ) {
			this->append ( *current );
		}
	}

	/** Create a copy of another vector. The other vector will not be changed. If the other
	 * vector has less than InlineBufferCapacity elements, no allocation will be made. */
	Vector ( const Vector &other ) : Vector ( other.AsSpan ( ) , other.mAllocator ) {
	}

	/** Create a copy of a vector with a different InlineBufferCapacity. This needs to be
	 * handled separately, so that the other one is a valid copy constructor. */
	template<size_t OtherInlineBufferCapacity> Vector ( const Vector<_Tp , OtherInlineBufferCapacity , _Allocator> &other ) : Vector ( other.AsSpan ( ) , other.mAllocator ) {
	}

	/** Steal the elements from another vector. This does not do an allocation. The other
	 * vector will have zero elements afterwards. */
	template<size_t OtherInlineBufferCapacity>
	Vector ( Vector<_Tp , OtherInlineBufferCapacity , _Allocator> &&other ) noexcept( std::is_nothrow_move_constructible_v<_Tp> ) : Vector ( NoExceptConstructor ( ) , other.mAllocator ) {
		const size_t size = other.Size ( );

		if ( other.IsInline ( ) ) {
			if ( size <= _InlineBufferCapacity ) {
				/* Copy between inline buffers. */
				uninitialized_relocate_n ( other.mBegin , size , mBegin );
				mEnd = mBegin + size;
			} else {
				/* Copy from inline buffer to newly allocated buffer. */
				const size_t capacity = size;
				mBegin = static_cast< _Tp * >( mAllocator.allocate ( sizeof ( _Tp ) * static_cast< size_t >( capacity ) , alignof( _Tp ) , AT ) );
				mCapacityEnd = mBegin + capacity;
				uninitialized_relocate_n ( other.mBegin , size , mBegin );
				mEnd = mBegin + size;
			}
		} else {
			/* Steal the pointer. */
			mBegin = other.mBegin;
			mEnd = other.mEnd;
			mCapacityEnd = other.mCapacityEnd;
		}

		other.mBegin = other.mInlineBuffer;
		other.mEnd = other.mBegin;
		other.mCapacityEnd = other.mBegin + OtherInlineBufferCapacity;
	}

	~Vector ( ) {
		destruct_n ( mBegin , this->Size ( ) );
		if ( !this->IsInline ( ) ) {
			mAllocator.deallocate ( mBegin );
		}
	}

	Vector &operator=( const Vector &other ) {
		return copy_assign_container ( *this , other );
	}

	Vector &operator=( Vector &&other ) {
		return move_assign_container ( *this , std::move ( other ) );
	}

	// Get the value at the given index. This invokes undefined behavior when the index is out
	// of bounds.
	const _Tp &operator[]( size_t index ) const {
		LOOM_assert ( index < this->Size ( ) );
		return mBegin [ index ];
	}

	// Get the value at the given index. This invokes undefined behavior when the index is out
	// of bounds.
	_Tp &operator[]( size_t index ) {
		LOOM_assert ( index < this->Size ( ) );
		return mBegin [ index ];
	}

	operator Span<_Tp> ( ) const {
		return Span<_Tp> ( mBegin , this->Size ( ) );
	}

	operator MutableSpan<_Tp> ( ) {
		return MutableSpan<_Tp> ( mBegin , this->Size ( ) );
	}

	template<typename U , LOOM_ENABLE_IF ( ( is_span_convertible_pointer_v<_Tp , U> ) )> operator Span<U> ( ) const {
		return Span<U> ( mBegin , this->Size ( ) );
	}

	template<typename U , LOOM_ENABLE_IF ( ( is_span_convertible_pointer_v<_Tp , U> ) )> operator MutableSpan<U> ( ) {
		return MutableSpan<U> ( mBegin , this->Size ( ) );
	}

	Span<_Tp> AsSpan ( ) const {
		return *this;
	}

	MutableSpan<_Tp> AsMutableSpan ( ) {
		return *this;
	}

	/** Make sure that enough memory is allocated to hold min_capacity elements.
	 * This won't necessarily make an allocation when min_capacity is small.
	 * The actual size of the vector does not change. */
	void Reserve ( const size_t min_capacity ) {
		if ( min_capacity > this->Capacity ( ) ) {
			this->ReallocToAtLeast ( min_capacity );
		}
	}

	/** Change the size of the vector so that it contains new_size elements.
	 * If new_size is smaller than the old size, the elements at the end of the vector are
	 * destructed. If new_size is larger than the old size, the new elements at the end are
	 * default constructed. If _Tpis trivially constructible, the memory is not touched by this
	 * function. */
	void Resize ( const size_t new_size ) {
		LOOM_assert ( new_size >= 0 );
		const size_t old_size = this->Size ( );
		if ( new_size > old_size ) {
			this->Reserve ( new_size );
			default_construct_n ( mBegin + old_size , new_size - old_size );
		} else {
			destruct_n ( mBegin + new_size , old_size - new_size );
		}
		mEnd = mBegin + new_size;
	}

	/** Change the size of the vector so that it contains new_size elements.
	 * If new_size is smaller than the old size, the elements at the end of the vector are
	 * destructed. If new_size is larger than the old size, the new elements will be copy
	 * constructed from the given value. */
	void Resize ( const size_t new_size , const _Tp &value ) {
		LOOM_assert ( new_size >= 0 );
		const size_t old_size = this->Size ( );
		if ( new_size > old_size ) {
			this->Reserve ( new_size );
			uninitialized_fill_n ( mBegin + old_size , new_size - old_size , value );
		} else {
			destruct_n ( mBegin + new_size , old_size - new_size );
		}
		mEnd = mBegin + new_size;
	}

	/** Reset the size of the vector so that it contains new_size elements.
	 * All existing elements are destructed, and not copied if the data must be reallocated. */
	void Reinitialize ( const size_t new_size ) {
		this->Clear ( );
		this->Resize ( new_size );
	}

	/** Afterwards the vector has 0 elements, but will still have
	 * memory to be refilled again. */
	void Clear ( ) {
		destruct_n ( mBegin , this->Size ( ) );
		mEnd = mBegin;
	}

	/** Afterwards the vector has 0 elements and any allocated memory
	 * will be freed. */
	void ClearAndMakeInline ( ) {
		destruct_n ( mBegin , this->Size ( ) );
		if ( !this->IsInline ( ) ) {
			mAllocator.deallocate ( mBegin );
		}

		mBegin = mInlineBuffer;
		mEnd = mBegin;
		mCapacityEnd = mBegin + _InlineBufferCapacity;
	}

	/** Insert a new element at the end of the vector.
	 * This might cause a reallocation with the capacity is exceeded.
	 *
	 * This is similar to std::vector::push_back. */
	void Append ( const _Tp &value ) {
		this->AppendAs ( value );
	}

	void Append ( _Tp &&value ) {
		this->AppendAs ( std::move ( value ) );
	}

	// This is similar to `std::vector::emplace_back`.
	template<typename... ForwardValue> void AppendAs ( ForwardValue &&...value ) {
		this->EnsureSpaceForOne ( );
		this->AppendUncheckedAs ( std::forward<ForwardValue> ( value )... );
	}

	/** Append the value to the vector and return the index that can be used to access the
	 * newly added value. */
	size_t AppendAndGetIndex ( const _Tp &value ) {
		return this->AppendAndGetIndexAs ( value );
	}

	size_t AppendAndGetIndex ( _Tp &&value ) {
		return this->AppendAndGetIndexAs ( std::move ( value ) );
	}

	template<typename... ForwardValue> size_t AppendAndGetIndexAs ( ForwardValue &&...value ) {
		const size_t index = this->size ( );
		this->AppendAs ( std::forward<ForwardValue> ( value )... );
		return index;
	}

	/** Append the value if it is not yet in the vector. This has to do a linear search to
	 * check if the value is in the vector. Therefore, this should only be called when it is
	 * known that the vector is small. */
	void AppendNonDuplicates ( const _Tp &value ) {
		if ( !this->Contains ( value ) ) {
			this->Append ( value );
		}
	}

	/** Append the value and assume that vector has enough memory reserved. This invokes
	 * undefined behavior when not enough capacity has been reserved beforehand. Only use this
	 * in performance critical code. */
	void AppendUnchecked ( const _Tp &value ) {
		this->AppendUncheckedAs ( value );
	}

	void AppendUnchecked ( _Tp &&value ) {
		this->AppendUncheckedAs ( std::move ( value ) );
	}

	template<typename... ForwardT> void AppendUncheckedAs ( ForwardT &&...value ) {
		LOOM_assert ( mEnd < mCapacityEnd );
		new ( mEnd ) _Tp ( std::forward<ForwardT> ( value )... );
		mEnd++;
	}

	/** Insert the same element n times at the end of the vector.
	 * This might result in a reallocation internally. */
	void AppendNTimes ( const _Tp &value , const size_t n ) {
		LOOM_assert ( n >= 0 );
		this->Reserve ( this->Size ( ) + n );
		uninitialized_fill_n ( mEnd , n , value );
		this->IncreaseSizeByUnchecked ( n );
	}

	/** Enlarges the size of the internal buffer that is considered to be initialized.
	 * This invokes undefined behavior when the new size is larger than the capacity.
	 * The method can be useful when you want to call constructors in the vector yourself.
	 * This should only be done in very rare cases and has to be justified every time. */
	void IncreaseSizeByUnchecked ( const size_t n ) noexcept {
		LOOM_assert ( mEnd + n <= mCapacityEnd );
		mEnd += n;
	}

	/** Copy the elements of another array to the end of this vector.
	 * This can be used to emulate parts of std::vector::insert. */
	void Extend ( Span<_Tp> array ) {
		this->Extend ( array.Data ( ) , array.Size ( ) );
	}

	void Extend ( const _Tp *start , size_t amount ) {
		this->Reserve ( this->Size ( ) + amount );
		this->ExtendEndUnchecked ( start , amount );
	}

	/** Adds all elements from the array that are not already in the vector. This is an
	 * expensive operation when the vector is large, but can be very cheap when it is known
	 * that the vector is small. */
	void ExtendEndNonDuplicates ( Span<_Tp> arr ) {
		for ( size_t i = 0; i < arr.Size ( ); i++ ) {
			this->AppendNonDuplicates ( arr [ i ] );
		}
	}

	/** Extend the vector without bounds checking. It is assumed that enough memory has been
	 * reserved beforehand. Only use this in performance critical code. */
	void ExtendEndUnchecked ( Span<_Tp> array ) {
		this->ExtendEndUnchecked ( array.Data ( ) , array.Size ( ) );
	}

	void ExtendEndUnchecked ( const _Tp *start , size_t amount ) {
		LOOM_assert ( amount >= 0 );
		LOOM_assert ( mBegin + amount <= mCapacityEnd );
		uninitialized_copy_n ( start , amount , mEnd );
		mEnd += amount;
	}

	template<typename InputIt> void Extend ( InputIt first , InputIt last ) {
		this->Insert ( this->End ( ) , first , last );
	}

	/** Insert elements into the vector at the specified position. This has a running time of
	 * O(n) where n is the number of values that have to be moved. Undefined behavior is
	 * invoked when the insert position is out of bounds. */
	void Insert ( const size_t insert_index , const _Tp &value ) {
		this->Insert ( insert_index , Span<_Tp> ( &value , 1 ) );
	}

	void Insert ( const size_t insert_index , _Tp &&value ) {
		this->Insert ( insert_index , std::make_move_iterator ( &value ) , std::make_move_iterator ( &value + 1 ) );
	}

	void Insert ( const size_t insert_index , Span<_Tp> array ) {
		this->Insert ( mBegin + insert_index , array.Begin ( ) , array.End ( ) );
	}

	template<typename InputIt> void Insert ( const _Tp *insert_position , InputIt first , InputIt last ) {
		const size_t insert_index = insert_position - mBegin;
		this->Insert ( insert_index , first , last );
	}

	template<typename InputIt> void Insert ( const size_t insert_index , InputIt first , InputIt last ) {
		LOOM_assert ( insert_index >= 0 );
		LOOM_assert ( insert_index <= this->Size ( ) );

		const size_t insert_amount = std::distance ( first , last );
		const size_t old_size = this->Size ( );
		const size_t new_size = old_size + insert_amount;
		const size_t move_amount = old_size - insert_index;

		this->Reserve ( new_size );
		for ( size_t i = 0; i < move_amount; i++ ) {
			const size_t src_index = insert_index + move_amount - i - 1;
			const size_t dst_index = new_size - i - 1;
			try {
				new ( static_cast< void * >( mBegin + dst_index ) ) _Tp ( std::move ( mBegin [ src_index ] ) );
			} catch ( ... ) {
				/* Destruct all values that have been moved already. */
				destruct_n ( mBegin + dst_index + 1 , i );
				mEnd = mBegin + src_index + 1;
				throw;
			}
			mBegin [ src_index ].~_Tp ( );
		}

		try {
			std::uninitialized_copy_n ( first , insert_amount , mBegin + insert_index );
		} catch ( ... ) {
			/* Destruct all values that have been moved. */
			destruct_n ( mBegin + new_size - move_amount , move_amount );
			mEnd = mBegin + insert_index;
			throw;
		}
		mEnd = mBegin + new_size;
	}

	/** Insert values at the beginning of the vector. The has to move all the other elements,
	 * so it has a linear running time. */
	void Prepend ( const _Tp &value ) {
		this->Insert ( 0 , value );
	}

	void Prepend ( _Tp &&value ) {
		this->Insert ( 0 , std::move ( value ) );
	}

	void Prepend ( Span<_Tp> values ) {
		this->Insert ( 0 , values );
	}

	template<typename InputIt> void Prepend ( InputIt first , InputIt last ) {
		this->Insert ( 0 , first , last );
	}

	/** Return a reference to the nth last element.
	 * This invokes undefined behavior when the vector is too short. */
	const _Tp &Last ( const size_t n = 0 ) const {
		LOOM_assert ( n < this->Size ( ) );
		return *( mEnd - 1 - n );
	}

	_Tp &Last ( const size_t n = 0 ) {
		LOOM_assert ( n < this->Size ( ) );
		return *( mEnd - 1 - n );
	}

	/** Return a reference to the first element in the vector.
	 * This invokes undefined behavior when the vector is empty. */
	const _Tp &First ( ) const {
		LOOM_assert ( this->Size ( ) > 0 );
		return *mBegin;
	}

	_Tp &First ( ) {
		LOOM_assert ( this->Size ( ) > 0 );
		return *mBegin;
	}

	// Return how many values are currently stored in the vector.
	size_t Size ( ) const {
		const size_t current_size = static_cast< size_t >( mEnd - mBegin );
		return current_size;
	}

	/** Returns true when the vector contains no elements, otherwise false.
	 * This is the same as std::vector::empty. */
	bool IsEmpty ( ) const {
		return mBegin == mEnd;
	}

	/** Destructs the last element and decreases the size by one. This invokes undefined
	 * behavior when the vector is empty. */
	void RemoveLast ( ) {
		LOOM_assert ( !this->IsEmpty ( ) );
		mEnd--;
		mEnd->~_Tp ( );
	}

	/** Remove the last element from the vector and return it. This invokes undefined behavior
	 * when the vector is empty. This is similar to std::vector::pop_back. */
	_Tp PopLast ( ) {
		LOOM_assert ( !this->IsEmpty ( ) );
		_Tp value = std::move ( *( mEnd - 1 ) );
		mEnd--;
		mEnd->~_Tp ( );
		return value;
	}

	/** Delete any element in the vector. The empty space will be filled by the previously last
	 * element. This takes O(1) time. */
	void RemoveAndReorder ( const size_t index ) {
		LOOM_assert ( index >= 0 );
		LOOM_assert ( index < this->Size ( ) );
		_Tp *element_to_remove = mBegin + index;
		_Tp *last_element = mEnd - 1;
		if ( element_to_remove < last_element ) {
			*element_to_remove = std::move ( *last_element );
		}
		mEnd = last_element;
		last_element->~_Tp ( );
	}

	/** Finds the first occurrence of the value, removes it and copies the last element to the
	 * hole in the vector. This takes O(n) time. */
	void RemoveFirstOccurrenceAndReorder ( const _Tp &value ) {
		const size_t index = this->FirstIndexOf ( value );
		this->RemoveAndReorder ( index );
	}

	/** Remove the element at the given index and move all values coming after it one towards
	 * the front. This takes O(n) time. If the order is not important, remove_and_reorder
	 * should be used instead.
	 *
	 * This is similar to std::vector::erase. */
	void Remove ( const size_t index ) {
		LOOM_assert ( index >= 0 );
		LOOM_assert ( index < this->Size ( ) );
		const size_t last_index = this->Size ( ) - 1;
		for ( size_t i = index; i < last_index; i++ ) {
			mBegin [ i ] = std::move ( mBegin [ i + 1 ] );
		}
		mBegin [ last_index ].~_Tp ( );
		mEnd--;
	}

	/** Remove a contiguous chunk of elements and move all values coming after it towards the
	 * front. This takes O(n) time.
	 *
	 * This is similar to std::vector::erase. */
	void Remove ( const size_t start_index , const size_t amount ) {
		const size_t old_size = this->Size ( );
		LOOM_assert ( start_index >= 0 );
		LOOM_assert ( amount >= 0 );
		LOOM_assert ( start_index + amount <= old_size );
		const size_t move_amount = old_size - start_index - amount;
		for ( size_t i = 0; i < move_amount; i++ ) {
			mBegin [ start_index + i ] = std::move ( mBegin [ start_index + amount + i ] );
		}
		destruct_n ( mEnd - amount , amount );
		mEnd -= amount;
	}

	/** Do a linear search to find the value in the vector.
	 * When found, return the first index, otherwise return -1. */
	size_t FirstIndexOfTry ( const _Tp &value ) const {
		for ( const _Tp *current = mBegin; current != mEnd; current++ ) {
			if ( *current == value ) {
				return static_cast< size_t >( current - mBegin );
			}
		}
		return -1;
	}

	/** Do a linear search to find the value in the vector and return the found index. This
	 * invokes undefined behavior when the value is not in the vector. */
	size_t FirstIndexOf ( const _Tp &value ) const {
		const size_t index = this->FirstIndexOfTry ( value );
		LOOM_assert ( index >= 0 );
		return index;
	}

	/** Do a linear search to see of the value is in the vector.
	 * Return true when it exists, otherwise false. */
	bool Contains ( const _Tp &value ) const {
		return this->FirstIndexOfTry ( value ) != -1;
	}

	// Copies the given value to every element in the vector.
	void Fill ( const _Tp &value ) const {
		initialized_fill_n ( mBegin , this->size ( ) , value );
	}

	// Get access to the underlying array.
	_Tp *Data ( ) {
		return mBegin;
	}

	// Get access to the underlying array.
	const _Tp *Data ( ) const {
		return mBegin;
	}

	_Tp *Begin ( ) {
		return mBegin;
	}

	_Tp *End ( ) {
		return mEnd;
	}

	const _Tp *Begin ( ) const {
		return mBegin;
	}

	const _Tp *End ( ) const {
		return mEnd;
	}

	std::reverse_iterator<_Tp *> ReverseBegin ( ) {
		return std::reverse_iterator<_Tp *> ( this->End ( ) );
	}

	std::reverse_iterator<_Tp *> ReverseEnd ( ) {
		return std::reverse_iterator<_Tp *> ( this->Begin ( ) );
	}

	std::reverse_iterator<const _Tp *> ReverseBegin ( ) const {
		return std::reverse_iterator<_Tp *> ( this->End ( ) );
	}

	std::reverse_iterator<const _Tp *> ReverseEnd ( ) const {
		return std::reverse_iterator<_Tp *> ( this->Begin ( ) );
	}

	/** Get the current capacity of the vector, i.e. the maximum number of elements the vector
	 * can hold, before it has to reallocate. */
	size_t Capacity ( ) const {
		return static_cast< size_t >( mCapacityEnd - mBegin );
	}

	IndexRange IndexRange ( ) const {
		return IndexRange ( this->Size ( ) );
	}

	friend bool operator==( const Vector &a , const Vector &b ) {
		return a.AsSpan ( ) == b.AsSpan ( );
	}

	friend bool operator!=( const Vector &a , const Vector &b ) {
		return !( a == b );
	}

private:
	bool IsInline ( ) const {
		return mBegin == mInlineBuffer;
	}

	void EnsureSpaceForOne ( ) {
		if ( UNLIKELY ( mEnd >= mCapacityEnd ) ) {
			this->ReallocToAtLeast ( this->Size ( ) + 1 );
		}
	}

	void ReallocToAtLeast ( const size_t min_capacity ) {
		if ( this->Capacity ( ) >= min_capacity ) {
			return;
		}

		/* At least double the size of the previous allocation. Otherwise consecutive calls
		 * to grow can cause a reallocation every time even though min_capacity only
		 * increments. */
		const size_t min_new_capacity = this->Capacity ( ) * 2;

		const size_t new_capacity = std::max ( min_capacity , min_new_capacity );
		const size_t size = this->Size ( );

		_Tp *new_array = static_cast< _Tp * >( mAllocator.allocate ( static_cast< size_t >( new_capacity ) * sizeof ( _Tp ) , alignof( _Tp ) , AT ) );
		try {
			uninitialized_relocate_n ( mBegin , size , new_array );
		} catch ( ... ) {
			mAllocator.deallocate ( new_array );
			throw;
		}

		if ( !this->IsInline ( ) ) {
			mAllocator.deallocate ( mBegin );
		}

		mBegin = new_array;
		mEnd = mBegin + size;
		mCapacityEnd = mBegin + new_capacity;
	}
};

}  // namespace loom
