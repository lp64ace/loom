#pragma once

#include "loomlib_assert.h"
#include "loomlib_utildefines.h"

#include "loomlib_allocator.hh"
#include "loomlib_index_range.hh"
#include "loomlib_memory_utils.hh"
#include "loomlib_span.hh"

namespace loom {

template<
	/**
	 * The type of the values stored in the array.
	 */
	typename _Tp,
	/**
	 * The number of values that can be stored in the array, without doing a
	 * heap allocation.
	 */
	size_t _InlineBufferCapacity = default_inline_buffer_capacity(sizeof(_Tp)),
	/**
	 * The allocator used by this array. Should rarely be changed, except when
	 * you don't want that MEM_* functions are used internally.
	 */
	typename _Allocator = GuardedAllocator>
class Array {
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
	// The beginning of the array. It might point into the inline buffer.
	_Tp *mData;

	// Number of elements in the array.
	size_t mSize;

	// Used for allocations when the inline buffer is too small.
	LOOM_NO_UNIQUE_ADDRESS _Allocator mAllocator;

	// A placeholder buffer that will remain uninitialized until it is used.
	LOOM_NO_UNIQUE_ADDRESS TypedBuffer<_Tp, _InlineBufferCapacity>
		mInlineBuffer;

   public:
	// By default an empty array is created.
	Array(_Allocator allocator = {}) noexcept : mAllocator(allocator)
	{
		mData = mInlineBuffer;
		mSize = 0;
	}

	// Create a new array that contains copies of all values.
	template<typename U, LOOM_ENABLE_IF((std::is_convertible_v<U, _Tp>))>
	Array(Span<U> values, _Allocator allocator = {}) : Array(allocator)
	{
		const size_t size = values.size();
		mData = this->GetBufferForSize(size);
		uninitialized_convert_n<U, _Tp>(values.data(), size, mData);
		mSize = size;
	}

	// Create a new array that contains copies of all values.
	template<typename U, LOOM_ENABLE_IF((std::is_convertible_v<U, _Tp>))>
	Array(const std::initializer_list<U> &values, _Allocator allocator = {})
		: Array(Span<U>(values), allocator)
	{
	}

	Array(const std::initializer_list<_Tp> &values, _Allocator allocator = {})
		: Array(Span<_Tp>(values), allocator)
	{
	}

	/** Create a new array with the given size. All values will be default
	 * constructed. For trivial types like int, default construction does
	 * nothing.
	 *
	 * We might want another version of this in the future, that does not do
	 * default construction even for non-trivial types. This should not be the
	 * default though, because one can easily mess up when dealing with
	 * uninitialized memory.
	 */
	explicit Array(int64_t size, _Allocator allocator = {})
		: Array(NoExceptConstructor(), allocator)
	{
		mData = this->GetBufferForSize(size);
		default_construct_n(mData, size);
		mSize = size;
	}

	/** Create a new array with the given size. All values will be initialized
	 * by copying the given default. */
	Array(int64_t size, const _Tp &value, _Allocator allocator = {})
		: Array(NoExceptConstructor(), allocator)
	{
		LOOM_assert(size >= 0);
		mData = this->GetBufferForSize(size);
		uninitialized_fill_n(mData, size, value);
		mSize = size;
	}

	/** Create a new array with uninitialized elements. The caller is
	 * responsible for constructing the elements. Moving, copying or destructing
	 * an Array with uninitialized elements invokes undefined behavior.
	 *
	 * This should be used very rarely. Note, that the normal size-constructor
	 * also does not initialize the elements when _Tp is trivially
	 * constructible. Therefore, it only makes sense to use this with non
	 * trivially constructible types.
	 *
	 * Usage:
	 *  Array<std::string> my_strings(10, NoInitialization()); */
	Array(int64_t size, NoInitialization, _Allocator allocator = {})
		: Array(NoExceptConstructor(), allocator)
	{
		LOOM_assert(size >= 0);
		mData = this->GetBufferForSize(size);
		mSize = size;
	}

	Array(const Array &other) : Array(other.as_span(), other.mAllocator)
	{
	}

	Array(Array &&other) noexcept(std::is_nothrow_move_constructible_v<_Tp>)
		: Array(NoExceptConstructor(), other.mAllocator)
	{
		if (other.mData == other.mInlineBuffer) {
			uninitialized_relocate_n(other.mData, other.mSize, mData);
		}
		else {
			mData = other.mData;
		}
		mSize = other.mSize;

		other.mData = other.mInlineBuffer;
		other.mSize = 0;
	}

	~Array()
	{
		destruct_n(mData, mSize);
		this->deallocate_if_not_inline(mData);
	}

	Array &operator=(const Array &other)
	{
		return copy_assign_container(*this, other);
	}

	Array &operator=(Array &&other) noexcept(
		std::is_nothrow_move_constructible_v<_Tp>)
	{
		return move_assign_container(*this, std::move(other));
	}

	_Tp &operator[](size_t index)
	{
		LOOM_assert(index < mSize);
		return mData[index];
	}

	const _Tp &operator[](size_t index) const
	{
		LOOM_assert(index < mSize);
		return mData[index];
	}

	operator Span<_Tp>() const
	{
		return Span<_Tp>(mData, mSize);
	}

	operator MutableSpan<_Tp>()
	{
		return MutableSpan<_Tp>(mData, mSize);
	}

	template<typename U,
			 LOOM_ENABLE_IF((is_span_convertible_pointer_v<_Tp, U>))>
	operator Span<U>() const
	{
		return Span<U>(mData, mSize);
	}

	template<typename U,
			 LOOM_ENABLE_IF((is_span_convertible_pointer_v<_Tp, U>))>
	operator MutableSpan<U>()
	{
		return MutableSpan<U>(mData, mSize);
	}

	Span<_Tp> AsSpan() const
	{
		return *this;
	}

	MutableSpan<_Tp> AsMutableSpan()
	{
		return *this;
	}

	// Returns the number of elements in the array.
	size_t Size() const
	{
		return mSize;
	}

	// Returns true when the number of elements in the array is zero.
	bool IsEmpty() const
	{
		return mSize == 0;
	}

	// Copies the given value to every element in the array.
	void Fill(const _Tp &value) const
	{
		initialized_fill_n(mData, mSize, value);
	}

	/** Return a reference to the first element in the array.
	 * This invokes undefined behavior when the array is empty. */
	const _Tp &First() const
	{
		LOOM_assert(mSize > 0);
		return *mData;
	}

	_Tp &First()
	{
		LOOM_assert(mSize > 0);
		return *mData;
	}

	/** Return a reference to the nth last element.
	 * This invokes undefined behavior when the array is too short. */
	const _Tp &Last(const size_t n = 0) const
	{
		LOOM_assert(n < mSize);
		return *(mData + mSize - 1 - n);
	}

	_Tp &Last(const size_t n = 0)
	{
		LOOM_assert(n < mSize);
		return *(mData + mSize - 1 - n);
	}

	// Get a pointer to the beginning of the array.
	const _Tp *Data() const
	{
		return mData;
	}

	_Tp *Data()
	{
		return mData;
	}

	const _Tp *Begin() const
	{
		return mData;
	}

	const _Tp *End() const
	{
		return mData + mSize;
	}

	_Tp *Begin()
	{
		return mData;
	}

	_Tp *End()
	{
		return mData + mSize;
	}

	std::reverse_iterator<_Tp *> ReverseBegin()
	{
		return std::reverse_iterator<_Tp *>(this->End());
	}
	std::reverse_iterator<_Tp *> ReverseEnd()
	{
		return std::reverse_iterator<_Tp *>(this->Begin());
	}

	std::reverse_iterator<const _Tp *> ReverseBegin() const
	{
		return std::reverse_iterator<_Tp *>(this->End());
	}
	std::reverse_iterator<const _Tp *> ReverseEnd() const
	{
		return std::reverse_iterator<_Tp *>(this->Begin());
	}

	// Get an index range containing all valid indices for this array.
	IndexRange IndexRange() const
	{
		return IndexRange(mSize);
	}

	/** Sets the size to zero. This should only be used when you have manually
	 * destructed all elements in the array beforehand. Use with care. */
	void ClearWithoutDestruct()
	{
		mSize = 0;
	}

	// Access the allocator used by this array.
	_Allocator &Allocator()
	{
		return mAllocator;
	}

	const _Allocator &Allocator() const
	{
		return mAllocator;
	}

	/** Get the value of the InlineBufferCapacity template argument. This is the
	 * number of elements that can be stored without doing an allocation. */
	static size_t InlineBufferCapacity()
	{
		return _InlineBufferCapacity;
	}

	/** Destruct values and create a new array of the given size. The values in
	 * the new array are default constructed. */
	void Reinitialize(const size_t new_size)
	{
		size_t old_size = mSize;

		destruct_n(mData, mSize);
		mSize = 0;

		if (new_size <= old_size) {
			default_construct_n(mData, new_size);
		}
		else {
			_Tp *new_data = this->GetBufferForSize(new_size);
			try {
				default_construct_n(new_data, new_size);
			}
			catch (...) {
				this->DeallocateIfNotInline(new_data);
				throw;
			}
			this->DeallocateIfNotInline(mData);
			mData = new_data;
		}

		mSize = new_size;
	}

   private:
	_Tp *GetBufferForSize(size_t size)
	{
		if (size <= _InlineBufferCapacity) {
			return mInlineBuffer;
		}
		else {
			return this->allocate(size);
		}
	}

	_Tp *Allocate(size_t size)
	{
		return static_cast<_Tp *>(mAllocator.allocate(
			static_cast<size_t>(size) * sizeof(_Tp), alignof(_Tp), AT));
	}

	void DeallocateIfNotInline(_Tp *ptr)
	{
		if (ptr != mInlineBuffer) {
			mAllocator.deallocate(ptr);
		}
	}
};

}  // namespace loom
