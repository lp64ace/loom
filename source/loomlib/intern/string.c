#include "guardedalloc/mem_guardedalloc.h"

#include "loomlib/loomlib_assert.h"
#include "loomlib/loomlib_string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *GLU_strdupn(const char *string, size_t len)
{
	char *n = MEM_mallocN(len + 1, __func__);
	memcpy(n, string, len);
	n[len] = '\0';
	return n;
}

char *GLU_strdup(const char *string)
{
	return GLU_strdupn(string, GLU_strlen(string));
}

char *GLU_strdupcat(const char *__restrict str1, const char *__restrict str2)
{
	/* Only include NULL terminator for the second string */
	const size_t str1_len = GLU_strlen(str1);
	const size_t str2_len = GLU_strlen(str2) + 1;
	char *str, *s;

	str = MEM_mallocN(str1_len + str2_len, __func__);
	s = str;

	memcpy(s, str1, str1_len);
	s += str1_len;
	memcpy(s, str2, str2_len);

	return str;
}

size_t GLU_strlen(const char *str)
{
	const char *char_ptr;
	const unsigned long int *longword_ptr;
	unsigned long int longword, magic_bits, himagic, lomagic;

	/* Handle the first few characters by reading one character at a time.
	Do this until CHAR_PTR is aligned on a longword boundary. */
	for (char_ptr = str;
		 ((unsigned long int)char_ptr & (sizeof(longword) - 1)) != 0;
		 ++char_ptr) {
		if (*char_ptr == '\0')
			return char_ptr - str;
	}

	/* All these elucidatory comments refer to 4-byte longwords,
	but the theory applies equally well to 8-byte longwords. */

	longword_ptr = (unsigned long int *)char_ptr;

	/* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
	the "holes."  Note that there is a hole just to the left of
	each byte, with an extra at the end:

	bits:  01111110 11111110 11111110 11111111
	bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

	The 1-bits make sure that carries propagate to the next 0-bit.
	The 0-bits provide holes for carries to fall into. */
	magic_bits = 0x7efefeffL;
	himagic = 0x80808080L;
	lomagic = 0x01010101L;

	if (sizeof(longword) > 4) {
		/* 64-bit version of the magic. */
		/* Do the shift in two steps to avoid a warning if long has 32 bits. */
		magic_bits = ((0x7efefefeL << 16) << 16) | 0xfefefeffL;
		himagic = ((himagic << 16) << 16) | himagic;
		lomagic = ((lomagic << 16) << 16) | lomagic;
	}

	if (sizeof(longword) > 8)
		abort();

	/* Instead of the traditional loop which tests each character,
	we will test a longword at a time.  The tricky part is testing
	if *any of the four* bytes in the longword in question are zero. */
	for (;;) {
		/* We tentatively exit the loop if adding MAGIC_BITS to
		LONGWORD fails to change any of the hole bits of LONGWORD.

		1) Is this safe?  Will it catch all the zero bytes?
		Suppose there is a byte with all zeros. Any carry bits
		propagating from its left will fall into the hole at its
		least significant bit and stop. Since there will be no
		carry from its most significant bit, the LSB of the
		byte to the left will be unchanged, and the zero will be
		detected.

		2) Is this worthwhile? Will it ignore everything except
		zero bytes? Suppose every byte of LONGWORD has a bit set
		somewhere. There will be a carry into bit 8.  If bit 8
		is set, this will carry into bit 16.  If bit 8 is clear,
		one of bits 9-15 must be set, so there will be a carry
		into bit 16. Similarly, there will be a carry into bit
		24.  If one of bits 24-30 is set, there will be a carry
		into bit 31, so all of the hole bits will be changed.

		The one misfire occurs when bits 24-30 are clear and bit
		31 is set; in this case, the hole at bit 31 is not
		changed.  If we had access to the processor carry flag,
		we could close this loophole by putting the fourth hole
		at bit 32!

		So it ignores everything except 128's, when they're aligned
		properly. */
		longword = *longword_ptr++;
		if (((longword - lomagic) & himagic) != 0) {
			/* Which of the bytes was the zero?  If none of them were, it was
			a misfire; continue the search. */

			const char *cp = (const char *)(longword_ptr - 1);

			if (cp[0] == 0)
				return cp - str;
			if (cp[1] == 0)
				return cp - str + 1;
			if (cp[2] == 0)
				return cp - str + 2;
			if (cp[3] == 0)
				return cp - str + 3;
			if (sizeof(longword) > 4) {
				if (cp[4] == 0)
					return cp - str + 4;
				if (cp[5] == 0)
					return cp - str + 5;
				if (cp[6] == 0)
					return cp - str + 6;
				if (cp[7] == 0)
					return cp - str + 7;
			}
		}
	}
}

size_t GLU_strnlen(const char *str, size_t maxn)
{
	const char *char_ptr;

	for (char_ptr = str; char_ptr != str + maxn; char_ptr++) {
		if (!*char_ptr) {
			break;
		}
	}

	return char_ptr - str;
}

char *GLU_strncpy(char *__restrict dst,
				  const char *__restrict src,
				  size_t maxncpy)
{
	size_t srclen = GLU_strnlen(src, maxncpy - 1);
	LOOM_assert(maxncpy != 0);

	memcpy(dst, src, srclen);
	dst[srclen] = 0;
	return dst;
}

char *GLU_strncpy_ensure_pad(char *__restrict dst,
							 const char *__restrict src,
							 char pad,
							 size_t maxncpy)
{
	LOOM_assert(maxncpy != 0);

	if (*src == '\0') {
		*dst = '\0';
	}
	else {
		size_t index = 0;
		size_t srclen;

		if (src[index] != pad) {
			dst[index++] = pad;
			maxncpy--;
		}
		maxncpy--;

		srclen = GLU_strnlen(src, maxncpy);
		if ((src[srclen - 1] != pad) && (srclen == maxncpy)) {
			srclen--;
		}

		memcpy(&dst[index], src, srclen);
		index += srclen;

		if (dst[index - 1] != pad) {
			dst[index++] = pad;
		}
		dst[index] = '\0';
	}

	return dst;
}

size_t GLU_strncpy_rlen(char *__restrict dst,
						const char *__restrict src,
						const size_t maxncpy)
{
	size_t srclen = GLU_strnlen(src, maxncpy - 1);
	LOOM_assert(maxncpy != 0);

	memcpy(dst, src, srclen);
	dst[srclen] = '\0';
	return srclen;
}

size_t GLU_strcpy_rlen(char *__restrict dst, const char *__restrict src)
{
	size_t srclen = GLU_strlen(src);
	memcpy(dst, src, srclen + 1);
	return srclen;
}

int *generate_Z_array(const char *__restrict str, int *z)
{
	size_t n = GLU_strlen(str);

	if (!z) {
		z = MEM_malloc_arrayN(n, sizeof(*z), __func__);
	}

	size_t l = 0, r = 0, k;

	for (size_t i = 1; i < n; i++) {
		/** If i > r nothing matches so we will calculate
		 * z [ i ] using the naive way. */
		if (i > r) {
			l = r = i;
			while (r < n && str[r - l] == str[r]) {
				r++;
			}
			z[i] = r - l;
			r--;
		}
		else {
			/** k = i - l so k corresponds to number which
			 * matches in [l,r] interval. */
			k = i - l;

			/** if z[k] is less than remaining interval then z[i]
			 * will be equal to z[k]. */
			if (z[k] < r - i + 1) {
				z[i] = z[k];
			}
			else {
				l = i;
				while (r < n && str[r - l] == str[r]) {
					r++;
				}
				z[i] = r - l;
				r--;
			}
		}
	}

	return z;
}

char *GLU_str_replaceN(const char *__restrict str,
					   const char *__restrict substr_old,
					   const char *__restrict substr_new)
{
	const size_t oldlen = GLU_strlen(substr_old);
	const size_t newlen = GLU_strlen(substr_new);
	const size_t len = GLU_strlen(str);

	char *concat = GLU_strdupcat(substr_old, str);
	int *z = generate_Z_array(concat, NULL);

	const size_t maxoccur = (len + oldlen - 1 / oldlen);
	const size_t maxoutlen = maxoccur * MAX2(oldlen, newlen);

	char *out = MEM_malloc_arrayN(maxoutlen + 1, sizeof(*out), __func__);
	char *out_itr = out;

	for (const char *itr = concat + oldlen; *itr != '\0';) {
		if (z[itr - concat] >= oldlen) {
			out_itr += GLU_strncpy_rlen(out_itr, substr_new, (size_t)-1);
			itr += oldlen;
		}
		else {
			*out_itr = *itr;
			out_itr++;
			itr++;
		}
	}

	*out_itr = '\0';

	MEM_freeN(concat);
	MEM_freeN(z);

	return out;
}

char *GLU_str_reverse(char *str)
{
	char *itr_l = str, *itr_r = str + GLU_strlen(str) - 1;

	char tmp;

	while (itr_l < itr_r) {
		SWAP_TVAL(tmp, *itr_l, *itr_r);
		itr_l++;
		itr_r--;
	}

	return str;
}

char *GLU_str_reverseN(const char *__restrict str)
{
	size_t len = GLU_strlen(str);

	char *out = MEM_malloc_arrayN(len + 1, sizeof(*out), __func__);
	memcpy(out, str, len + 1);

	char *itr_l = out, *itr_r = out + len - 1;

	char tmp;

	while (itr_l < itr_r) {
		SWAP_TVAL(tmp, *itr_l, *itr_r);
		itr_l++;
		itr_r--;
	}

	return out;
}

size_t GLU_vsnprintf(char *__restrict buffer,
					 size_t maxncpy,
					 const char *ATTR_PRINTF_FORMAT_STRING format,
					 va_list arg)
{
	size_t n;

	LOOM_assert(buffer != NULL);
	LOOM_assert(maxncpy > 0);
	LOOM_assert(format != NULL);

	n = (size_t)vsnprintf(buffer, maxncpy, format, arg);

	if (n != -1 && n < maxncpy) {
		buffer[n] = '\0';
	}
	else {
		buffer[maxncpy - 1] = '\0';
	}

	return n;
}

size_t GLU_vsnprintf_rlen(char *__restrict buffer,
						  size_t maxncpy,
						  const char *__restrict format,
						  va_list arg)
{
	size_t n;

	LOOM_assert(buffer != NULL);
	LOOM_assert(maxncpy > 0);
	LOOM_assert(format != NULL);

	n = (size_t)vsnprintf(buffer, maxncpy, format, arg);

	if (n != -1 && n < maxncpy) {
		/* pass */
	}
	else {
		n = maxncpy - 1;
	}
	buffer[n] = '\0';

	return n;
}

size_t GLU_snprintf(char *__restrict dst,
					size_t maxncpy,
					const char *ATTR_PRINTF_FORMAT_STRING format,
					...)
{
	size_t n;
	va_list arg;

	va_start(arg, format);
	n = GLU_vsnprintf(dst, maxncpy, format, arg);
	va_end(arg);

	return n;
}

size_t GLU_snprintf_rlen(char *__restrict dst,
						 size_t maxncpy,
						 const char *ATTR_PRINTF_FORMAT_STRING format,
						 ...)
{
	size_t n;
	va_list arg;

	va_start(arg, format);
	n = GLU_vsnprintf_rlen(dst, maxncpy, format, arg);
	va_end(arg);

	return n;
}

char *GLU_sprintfN(const char *__restrict format, ...)
{
	size_t n, alloc = DEFAULT_STRING_LEN;
	va_list arg;

	char *out = NULL;

	do {
		out = MEM_reallocN(
			out, (alloc = alloc * (size_t)2) * sizeof(*out), __func__);

		va_start(arg, format);
		n = GLU_vsnprintf(out, alloc, format, arg);
		va_end(arg);
	} while (n == (size_t)-1 || n >= alloc);

	return out;
}
