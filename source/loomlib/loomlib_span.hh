#pragma once

#include "loomlib_utildefines.h"

#include "loomlib_index_range.hh"
#include "loomlib_memory_utils.hh"

#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <vector>

namespace loom {

template<typename _Tp> class Span {
   public:
	using value_type = _Tp;
	using pointer = _Tp *;
	using const_pointer = const _Tp *;
	using reference = _Tp &;
	using const_reference = const _Tp &;
	using iterator = const _Tp *;
	using size_type = size_t;

   protected:
	const _Tp *mData = nullptr;
	size_t mSize = 0;

   public:
	// Create a reference to an empty array.
	constexpr Span() = default;

	constexpr Span(const _Tp *start, size_t size) : mData(start), mSize(size)
	{
	}

	template<typename U,
			 LOOM_ENABLE_IF((is_span_convertible_pointer_v<U, _Tp>))>
	constexpr Span(const U *start, size_t size)
		: mData(static_cast<const _Tp *>(start)), mSize(size)
	{
	}

	/** Reference an initializer_list. Note that the data in the
	 * initializer_list is only valid until the expression containing it is
	 * fully computed.
	 *
	 * Do:
	 *  call_function_with_array({1, 2, 3, 4});
	 *
	 * Don't:
	 *  Span<int> span = {1, 2, 3, 4};
	 *  call_function_with_array(span); */
	constexpr Span(const std::initializer_list<_Tp> &list)
		: Span(list.begin(), static_cast<size_t>(list.size()))
	{
	}

	constexpr Span(const std::vector<_Tp> &vector)
		: Span(vector.data(), static_cast<size_t>(vector.size()))
	{
	}

	template<std::size_t N>
	constexpr Span(const std::array<_Tp, N> &array) : Span(array.data(), N)
	{
	}

	/** Support implicit conversions like the one below:
	 *   Span<T *> -> Span<const T *> */
	template<typename U,
			 LOOM_ENABLE_IF((is_span_convertible_pointer_v<U, _Tp>))>
	constexpr Span(Span<U> span)
		: mData(static_cast<const _Tp *>(span.data())), mSize(span.size())
	{
	}

	/** Returns a contiguous part of the array. This invokes undefined behavior
	 * when the start or size is negative. */
	constexpr Span Slice(size_t start, size_t size) const
	{
		const size_t new_size = std::max<size_t>(0,
												 std::min(size, mSize - start));
		return Span(mData + start, new_size);
	}

	constexpr Span Slice(IndexRange range) const
	{
		return this->slice(range.Start(), range.Size());
	}

	/** Returns a new Span with n elements removed from the beginning. This
	 * invokes undefined behavior when n is negative. */
	constexpr Span DropFront(size_t n) const
	{
		const size_t new_size = std::max<size_t>(0, mSize - n);
		return Span(mData + n, new_size);
	}

	/** Returns a new Span with n elements removed from the beginning. This
	 * invokes undefined behavior when n is negative. */
	constexpr Span DropBack(size_t n) const
	{
		const size_t new_size = std::max<size_t>(0, mSize - n);
		return Span(mData, new_size);
	}

	/** Returns a new Span that only contains the first n elements. This invokes
	 * undefined behavior when n is negative. */
	constexpr Span TakeFront(size_t n) const
	{
		const size_t new_size = std::min<size_t>(mSize, n);
		return Span(mData, new_size);
	}

	/** Returns a new Span that only contains the last n elements. This invokes
	 * undefined behavior when n is negative. */
	constexpr Span TakeBack(size_t n) const
	{
		const size_t new_size = std::min<size_t>(mSize, n);
		return Span(mData + mSize - new_size, new_size);
	}

	/** Returns the pointer to the beginning of the referenced array. This may
	 * be nullptr when the size is zero. */
	constexpr const _Tp *Data() const
	{
		return mData;
	}

	constexpr const _Tp *Begin() const
	{
		return mData;
	}
	constexpr const _Tp *End() const
	{
		return mData + mSize;
	}

	constexpr std::reverse_iterator<const _Tp *> ReverseBegin() const
	{
		return std::reverse_iterator<const _Tp *>(this->End());
	}
	constexpr std::reverse_iterator<const _Tp *> rend() const
	{
		return std::reverse_iterator<const _Tp *>(this->Begin());
	}

	/** Access an element in the array. This invokes undefined behavior when the
	 * index is out of bounds. */
	constexpr const _Tp &operator[](size_t index) const
	{
		LOOM_assert(index < mSize);
		return mData[index];
	}

	// Returns the number of elements in the referenced array.
	constexpr size_t Size() const
	{
		return mSize;
	}

	// Returns true if the size is zero.
	constexpr bool IsEmpty() const
	{
		return mSize == 0;
	}

	// Returns the number of bytes referenced by this Span.
	constexpr size_t SizeInBytes() const
	{
		return sizeof(_Tp) * mSize;
	}

	/** Does a linear search to see of the value is in the array.
	 * Returns true if it is, otherwise false. */
	constexpr bool Contains(const _Tp &value) const
	{
		for (const size_t i : this->IndexRange()) {
			if (this->mData[i] == value) {
				return true;
			}
		}
		return false;
	}

	/** Does a constant time check to see if the pointer points to a value in
	 * the referenced array. Return true if it is, otherwise false. */
	constexpr bool ContainsPtr(const _Tp *ptr) const
	{
		return (this->begin() <= ptr) && (ptr < this->end());
	}

	/** Does a linear search to count how often the value is in the array.
	 * Returns the number of occurrences. */
	constexpr size_t Count(const _Tp &value) const
	{
		size_t counter = 0;
		for (const size_t i : this->IndexRange()) {
			if (this->mData[i] == value) {
				counter++;
			}
		}
		return counter;
	}

	/** Return a reference to the first element in the array. This invokes
	 * undefined behavior when the array is empty. */
	constexpr const _Tp &First() const
	{
		return mData[0];
	}

	/** Returns a reference to the nth last element. This invokes undefined
	 * behavior when the span is too short. */
	constexpr const _Tp &Last(const size_t n = 0) const
	{
		LOOM_assert(n < mSize);
		return mData[mSize - 1 - n];
	}

	/** Returns the element at the given index. If the index is out of range,
	 * return the fallback value. */
	constexpr _Tp Get(size_t index, const _Tp &fallback) const
	{
		if (index < mSize && index >= 0) {
			return mData[index];
		}
		return fallback;
	}

	/** Check if the array contains duplicates. Does a linear search for every
	 * element. So the total running time is O(n^2). Only use this for small
	 * arrays. */
	constexpr bool HasDuplicates__linear_search() const
	{
		/* The size should really be smaller than that. If it is not, the
		 * calling code should be changed. */
		LOOM_assert(mSize < 1000);

		for (size_t i = 0; i < mSize; i++) {
			const _Tp &value = mData[i];
			for (size_t j = i + 1; j < mSize; j++) {
				if (value == mData[j]) {
					return true;
				}
			}
		}
		return false;
	}

	/** Returns true when this and the other array have an element in common.
	 * This should only be called on small arrays, because it has a running time
	 * of O(n*m) where n and m are the sizes of the arrays. */
	constexpr bool Intersects__linear_search(Span other) const
	{
		/* The size should really be smaller than that. If it is not, the
		 * calling code should be changed. */
		LOOM_assert(mSize < 1000);

		for (size_t i = 0; i < mSize; i++) {
			const _Tp &value = mData[i];
			if (other.contains(value)) {
				return true;
			}
		}
		return false;
	}

	/** Returns the index of the first occurrence of the given value. This
	 * invokes undefined behavior when the value is not in the array. */
	constexpr size_t FirstIndex(const _Tp &search_value) const
	{
		const size_t index = this->first_index_try(search_value);
		return index;
	}

	// Returns the index of the first occurrence of the given value or -1 if it
	// does not exist.
	constexpr size_t FirstIndexTry(const _Tp &search_value) const
	{
		for (size_t i = 0; i < mSize; i++) {
			if (mData[i] == search_value) {
				return i;
			}
		}
		return -1;
	}

	/** Utility to make it more convenient to iterate over all indices that can
	 * be used with this array. */
	constexpr IndexRange IndexRange() const
	{
		return IndexRange(mSize);
	}

	/**
	 * Returns a new Span to the same underlying memory buffer. No conversions
	 * are done.
	 */
	template<typename NewT> Span<NewT> constexpr Cast() const
	{
		LOOM_assert((mSize * sizeof(_Tp)) % sizeof(NewT) == 0);
		int64_t new_size = mSize * sizeof(_Tp) / sizeof(NewT);
		return Span<NewT>(reinterpret_cast<const NewT *>(mData), new_size);
	}

	friend bool operator==(const Span<_Tp> a, const Span<_Tp> b)
	{
		if (a.Size() != b.Size()) {
			return false;
		}
		return std::equal(a.Begin(), a.End(), b.Begin());
	}

	friend bool operator!=(const Span<_Tp> a, const Span<_Tp> b)
	{
		return !(a == b);
	}
};

/** Mostly the same as Span, except that one can change the array elements
 * through a MutableSpan. */
template<typename _Tp> class MutableSpan {
   public:
	using value_type = _Tp;
	using pointer = _Tp *;
	using const_pointer = const _Tp *;
	using reference = _Tp &;
	using const_reference = const _Tp &;
	using iterator = _Tp *;
	using size_type = size_t;

   protected:
	_Tp *mData = nullptr;
	size_t mSize = 0;

   public:
	constexpr MutableSpan() = default;

	constexpr MutableSpan(_Tp *start, const size_t size)
		: mData(start), mSize(size)
	{
	}

	constexpr MutableSpan(std::vector<_Tp> &vector)
		: MutableSpan(vector.data(), vector.size())
	{
	}

	template<std::size_t N>
	constexpr MutableSpan(std::array<_Tp, N> &array)
		: MutableSpan(array.data(), N)
	{
	}

	/**
	 * Support implicit conversions like the one below:
	 *   MutableSpan<T *> -> MutableSpan<const T *>
	 */
	template<typename U,
			 LOOM_ENABLE_IF((is_span_convertible_pointer_v<U, _Tp>))>
	constexpr MutableSpan(MutableSpan<U> span)
		: mData(static_cast<_Tp *>(span.data())), mSize(span.size())
	{
	}

	constexpr operator Span<_Tp>() const
	{
		return Span<_Tp>(mData, mSize);
	}

	template<typename U,
			 LOOM_ENABLE_IF((is_span_convertible_pointer_v<_Tp, U>))>
	constexpr operator Span<U>() const
	{
		return Span<U>(static_cast<const U *>(mData), mSize);
	}

	// Returns the number of elements in the array.
	constexpr size_t Size() const
	{
		return mSize;
	}

	// Returns true if the size is zero.
	constexpr bool IsEmpty() const
	{
		return mSize == 0;
	}

	// Replace all elements in the referenced array with the given value.
	constexpr void Fill(const _Tp &value)
	{
		initialized_fill_n(mData, mSize, value);
	}

	/** Replace a subset of all elements with the given value. This invokes
	 * undefined behavior when one of the indices is out of bounds. */
	constexpr void FillIndices(Span<size_t> indices, const _Tp &value)
	{
		for (size_t i : indices) {
			LOOM_assert(i < mSize);
			mData[i] = value;
		}
	}

	/** Returns a pointer to the beginning of the referenced array. This may be
	 * nullptr, when the size is zero. */
	constexpr _Tp *Data() const
	{
		return mData;
	}

	constexpr _Tp *Begin() const
	{
		return mData;
	}
	constexpr _Tp *End() const
	{
		return mData + mSize;
	}

	constexpr std::reverse_iterator<_Tp *> ReverseBegin() const
	{
		return std::reverse_iterator<_Tp *>(this->End());
	}
	constexpr std::reverse_iterator<_Tp *> ReverseEnd() const
	{
		return std::reverse_iterator<_Tp *>(this->Begin());
	}

	constexpr _Tp &operator[](const size_t index) const
	{
		LOOM_assert(index < this->Size());
		return mData[index];
	}

	/** Returns a contiguous part of the array. This invokes undefined behavior
	 * when the start or size is negative. */
	constexpr MutableSpan Slice(const size_t start, const size_t size) const
	{
		const size_t new_size = std::max<size_t>(0,
												 std::min(size, mSize - start));
		return MutableSpan(mData + start, new_size);
	}

	constexpr MutableSpan Slice(IndexRange range) const
	{
		return this->Slice(range.Start(), range.Size());
	}

	/** Returns a new MutableSpan with n elements removed from the beginning.
	 * This invokes undefined behavior when n is negative. */
	constexpr MutableSpan DropFront(const size_t n) const
	{
		const size_t new_size = std::max<size_t>(0, mSize - n);
		return MutableSpan(mData + n, new_size);
	}

	/** Returns a new MutableSpan with n elements removed from the end. This
	 * invokes undefined behavior when n is negative. */
	constexpr MutableSpan DropBack(const size_t n) const
	{
		const size_t new_size = std::max<size_t>(0, mSize - n);
		return MutableSpan(mData, new_size);
	}

	/** Returns a new MutableSpan that only contains the first n elements. This
	 * invokes undefined behavior when n is negative. */
	constexpr MutableSpan TakeFront(const size_t n) const
	{
		const size_t new_size = std::min<size_t>(mSize, n);
		return MutableSpan(mData, new_size);
	}

	/** Return a new MutableSpan that only contains the last n elements. This
	 * invokes undefined behavior when n is negative. */
	constexpr MutableSpan TakeBack(const size_t n) const
	{
		const size_t new_size = std::min<size_t>(mSize, n);
		return MutableSpan(mData + mSize - new_size, new_size);
	}

	// Reverse the data in the MutableSpan.
	constexpr void Reverse()
	{
		for (const int i : IndexRange(mSize / 2)) {
			std::swap(mData[mSize - 1 - i], mData[i]);
		}
	}

	/** Returns an (immutable) Span that references the same array. This is
	 * usually not needed, due to implicit conversions. However, sometimes
	 * automatic type deduction needs some help. */
	constexpr Span<_Tp> AsSpan() const
	{
		return Span<_Tp>(mData, mSize);
	}

	/** Utility to make it more convenient to iterate over all indices that can
	 * be used with this array. */
	constexpr IndexRange IndexRange() const
	{
		return IndexRange(mSize);
	}

	/** Return a reference to the first element in the array. This invokes
	 * undefined behavior when the array is empty. */
	constexpr _Tp &First() const
	{
		return mData[0];
	}

	/** Returns a reference to the nth last element. This invokes undefined
	 * behavior when the span is too short. */
	constexpr _Tp &Last(const size_t n = 0) const
	{
		LOOM_assert(n < mSize);
		return mData[mSize - 1 - n];
	}

	/** Does a linear search to count how often the value is in the array.
	 * Returns the number of occurrences. */
	constexpr size_t Count(const _Tp &value) const
	{
		size_t counter = 0;
		for (const size_t i : this->IndexRange()) {
			if (this->mData[i] == value) {
				counter++;
			}
		}
		return counter;
	}

	/** Copy all values from another span into this span. This invokes undefined
	 * behavior when the destination contains uninitialized data and T is not
	 * trivially copy constructible. The size of both spans is expected to be
	 * the same. */
	constexpr void CopyFrom(Span<_Tp> values)
	{
		LOOM_assert(mSize == values.Size());
		initialized_copy_n(values.Data(), mSize, mData);
	}

	/** Returns a new span to the same underlying memory buffer. No conversions
	 * are done. The caller is responsible for making sure that the type cast is
	 * valid. */
	template<typename NewT> constexpr MutableSpan<NewT> cast() const
	{
		LOOM_assert((mSize * sizeof(_Tp)) % sizeof(NewT) == 0);
		size_t new_size = mSize * sizeof(_Tp) / sizeof(NewT);
		return MutableSpan<NewT>((NewT *)mData, new_size);
	}
};

// This is defined here, because in `loomlib_index_range.hh` `Span` is not yet
// defined.
inline Span<size_t> IndexRange::AsSpan() const
{
	const size_t min_required_size = mStart + mSize;
	const size_t current_array_size = s_current_array_size.load(
		std::memory_order_acquire);
	const size_t *current_array = s_current_array.load(
		std::memory_order_acquire);
	if (min_required_size <= current_array_size) {
		return Span<size_t>(current_array + mStart, mSize);
	}
	return this->AsSpanInternal();
}

}  // namespace loom
