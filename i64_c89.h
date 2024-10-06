/*
	i64.c89.h
	
	A port of Google's Closure library's implementation of a
	64-bit, 2s complement type, little-endian,
	with 32-bit ints. That is, none of the non-trivial algorithms
	are mine.
*/

#ifndef I64_C89_H
#define I64_C89_H

#include <assert.h>
	/* TODO: remove all assertions (we only use them
		for debugging tests, not the actual tests)
		and remove this include. */

#include <stdio.h>

#include <string.h>
	/* needed for strncmp */
#include <stdlib.h>
	/* needed for integer division */
#include <math.h>
	/* math.h is needed solely for functions
		used to test assertions on doubles --
		eventually remove the assertions and this */

#define SHR32F0(n, m) (((u32)n) >> m)
	/* "Shift Right a i32, and
 	and Fill with 0s. " */
	
/*
	PLATFORM-DEPENDENT DEFINITIONS
	==============================
	Customize for your platform.
	'i32' must have integer division and modulus
	defined on it via '/' and '%/.
		(We can generalize via a macro if need be,
		but let's not get ahead of ourselves.)
*/

/* EDIT for your platform. */
typedef int i32;
typedef unsigned int u32;

/*
	BASE DEFINITIONS
	================
*/

typedef struct {
	i32 low_;
	i32 high_;
} i64;

void i64_debugPrint(i64 this) {
	printf("0x%.8x :: 0x%.8x", this.high_, this.low_);
}

void i64_debugPrintDecimal(i64 this) {
	i32 neg_p = this.high_ & 0x80000000;
	if(neg_p != 0) {
		printf("-((%u * 2^32) + %u + 1)", ~this.high_ , ~this.low_);
	} else {
		printf("((%u * 2^32) + %u)", this.high_, this.low_);
		
	}
}

#define i64_isEqual(this, other) \
	((this.low_ == other.low_) && \
		(this.high_ == other.high_))

#define i64_debugPrintNewlining(this) \
	(printf("\n"), i64_debugPrint(this), printf("\n"))
	
	
i64 i64_fromBits(i32 low, i32 high) {
	i64 ret;
	ret.low_ = low;
	ret.high_ = high;
	return ret;
}

#define i64_getHighBits(this) ((i32)this.high_)
#define i64_getLowBits(this) ((i32)this.low_)

/*
	Get the lowest 32 bits as an _unsigned_ value.
*/
#define i64_getLowBitsUnsigned(this) ((u32)this.low_)

	
/*
	CONSTANTS
	=========
*/

#define TWO_PWR_32_DBL_ ((double)0x100000000)
	/*
		TWO_PWR_32_DBL_ == 2^32, of course
			
	*/

#define TWO_PWR_63_DBL_ ((double)0x8000000000000000)
#define TWO_PWR_62_DBL_ ((double)0x4000000000000000)
	/*
		i64s are in the range
			-(2^62) <= x (2^62 - 1).
	*/

/*
	Get the smallest _in magnitude_ i64.
*/
i64 i64_getMinValue() {
	return i64_fromBits(0, 0x80000000);
}

i64 i64_getMaxValue() {
	return i64_fromBits(0xFFFFFFFF, 0x7FFFFFFF);
}

i64 i64_getZero() {
	return i64_fromBits(0, 0);
}

/*
	the following functions, getOne and getNegOne,
	illustrate little-ending byte ordering used
*/

i64 i64_getOne() {
	return i64_fromBits(1, 0);
}

i64 i64_getNegOne() {
	return i64_fromBits(-1, -1);
}
	

/*
	COMPARISONS
	===========
*/

int i64_isZero(i64 this) {
	return this.low_ == 0 && this.high_ == 0;
}

int i64_isNegative(i64 this) {
	return this.high_ < 0;
}

int i64_isOdd(i64 this) {
	return (this.low_ & 1) == 1;
}

/*
	Compares two 64s a, b and returns one of three results:
	- 0 if a == b,
	- 1 if a > b;
	- (-1) if a < b.
*/
int i64_compare(i64 this, i64 other) {
	if (this.high_ == other.high_) {
		if (this.low_ == other.low_) {
			return 0;
		}
		return i64_getLowBitsUnsigned(this) >
			i64_getLowBitsUnsigned(other) ? 1 : -1;
    }
    return this.high_ > other.high_ ? 1 : -1;
}


int i64_equals(i64 this, i64 other) {
	return (this.low_ == other.low_) && (this.high_ == other.high_);
}

int i64_notEquals(i64 this, i64 other) {
	return !i64_equals(this, other);
}

int i64_lessThan(i64 this, i64 other) {
	return i64_compare(this, other) < 0;
}

int i64_lessThanOrEqual(i64 this, i64 other) {
	return i64_compare(this, other) <= 0;
}

int i64_greaterThan(i64 this, i64 other) {
	return i64_compare(this, other) > 0;
}

int i64_greaterThanOrEqual(i64 this, i64 other) {
	return i64_compare(this, other) >= 0;
}

/*
	ARITHMETIC OPERATIONS _EXCEPT_ DIVISION
	=======================================
	(division is defined at the very end)
*/

i64 i64_negate(i64 this) {
	i64 ret;
	i32 negLow, overflowFromLow, negHigh;

	negLow = (~this.low_ + 1) | 0;
		/* standard 2s-complement negation */
	overflowFromLow = !negLow;
		/* in a 2s complement type with n
		 bits, the lowest representable
		 negative number is -(2^n), but the
		 highest is 2^n - 1; therefore, negating
		 -(2^n) incurs overflow
		 */
	negHigh = (~this.high_ + overflowFromLow) | 0;
		/*  again, 2s-complement negation */
	return i64_fromBits(negLow, negHigh);
}

i64 i64_add(i64 this, i64 other) {
	/* although I've stated above that all of these
		algorithms are essentially ported verbatim
		from Google's goog.math.Long, I should re-emphasize
		that here.
		We sum up chunks of 16 bits, but store these 16
		ints in 32-bit locals (the largest native numeric
		type we have at hand). That way, we can detect
		overflow by simply looking at high bits.*/
		
	/* chunk up 'this' (a) and 'that" (b) */
	i32 a48 = SHR32F0(this.high_, 16);
	i32 a32 = this.high_ & 0xFFFF;
	i32 a16 = SHR32F0(this.low_, 16);
	i32 a00 = this.low_ & 0xFFFF;
	
	i32 b48 = SHR32F0(other.high_, 16);
	i32 b32 = other.high_ & 0xFFFF;
	i32 b16 = SHR32F0(other.low_, 16);
	i32 b00 = other.low_ & 0xFFFF;
	
	/* initialize our result words */
	i32 c48 = 0, c32 = 0, c16 = 0, c00 = 0;
	
	/* okay - this is just adding and carrying */
	c00 += a00 + b00; /* add; */
	c16 += SHR32F0(c00, 16); /* carry c00 into c16; */
	c00 &= 0xFFFF; /* mask off the carry from c00 */
	/* etc. etc. etc. */
	c16 += a16 + b16;
    c32 += SHR32F0(c16, 16);
    c16 &= 0xFFFF;
    c32 += a32 + b32;
    c48 += SHR32F0(c32, 16);
    c32 &= 0xFFFF;
    c48 += a48 + b48;
    c48 &= 0xFFFF;
	
	/* and then finally concat. all of the 16-bit chunks
		(stored in i32s) together. */
	return i64_fromBits(
		(c16 << 16) | c00,
		(c48 << 16) | c32
	);
}

i64 i64_subtract(i64 this, i64 other) {
	return i64_add(this, i64_negate(other));
}

i64 i64_multiply(i64 this, i64 other) {
	/* We do long multiplication: adding (with extended width)
		successive (sets of consecutive) digits.
		In particular, we split in 4 chunks of 16 bits,
		as with i64_add, but do the repeated arithmetic
		with 32-bit integers to handle overflow. */
		
	i32 a48 = SHR32F0(this.high_, 16);
    i32 a32 = this.high_ & 0xFFFF;
    i32 a16 = SHR32F0(this.low_, 16);
    i32 a00 = this.low_ & 0xFFFF;

    i32 b48 = SHR32F0(other.high_, 16);
    i32 b32 = other.high_ & 0xFFFF;
    i32 b16 = SHR32F0(other.low_, 16);
    i32 b00 = other.low_ & 0xFFFF;

    i32 c48 = 0, c32 = 0, c16 = 0, c00 = 0;
	
	if(i64_isZero(this)) {
		return this;
	}
	if(i64_isZero(other)) {
		return other;
	}
	
	/* Okay - now start the long mulitplication. */
	c00 += a00 * b00;
    c16 += SHR32F0(c00, 16);
    c00 &= 0xFFFF;
    c16 += a16 * b00;
	
    c32 += SHR32F0(c16, 16);
    c16 &= 0xFFFF;
    c16 += a00 * b16;
    c32 += SHR32F0(c16, 16);
    c16 &= 0xFFFF;
    c32 += a32 * b00;
	
    c48 += SHR32F0(c32, 16);
    c32 &= 0xFFFF;
    c32 += a16 * b16;
    c48 += SHR32F0(c32, 16);
    c32 &= 0xFFFF;
    c32 += a00 * b32;
    c48 += SHR32F0(c32, 16);
    c32 &= 0xFFFF;
    c48 += a48 * b00 + a32 * b16 + a16 * b32 + a00 * b48;
    c48 &= 0xFFFF;
	
    return i64_fromBits(
		(c16 << 16) | c00,
		(c48 << 16) | c32
	);
}

/* Division. 
   TODO: Currently, we bootstrap from 'double' arithmetic.
   This is not ideal, since we use sprintf to convert double -> i64.
	Ideally, we would use Knuth's Algorithm D (cf. Hacker's
	Delight, 9-2.) */
i64 i64_fromDouble(double value);
double i64_toDouble(i64 value);
	/* TODO: these forward declarations are only temporary
		while we bootstrap i64 division from double division. */
i64 i64_DivMod(i64 this, i64 other, i64* rem) {
	double thisD, otherD, remD, resD;

	/* 'this' and 'other' are both two words.
		Perform division in 'double' land. */
	thisD = i64_toDouble(this);
	otherD = i64_toDouble(other);
	
	remD = fmod(thisD, otherD);
	/* at this point, remD should be an integer; */
	assert(floor(remD) == remD);
	*rem = i64_fromDouble(remD);
	
	resD = floor(thisD / otherD);	
	resD = (resD < 0) ? ceil(resD) : floor(resD);
	/* at this point, resD should be an integer; */
	assert(floor(resD) == resD);
	
	return i64_fromDouble(resD);
}

/*
	TYPE CONVERSIONS
	================
*/

/*
   Returns an i64 representing the 64-bit integer
   that comes by concatenating the given high and low bits.
*/


i64 i64_fromInt(i32 intValue) {
	return i64_fromBits(intValue,
				intValue < 0 ? -1 : 0); 
}

/*
	Assuming the value represented by 'this'
	fits into an i32, return it.
*/
i32 i64_toInt(i64 this) {
	return this.low_;
}

/*
	Returns an i64 representing an integer double
	- the case where an easy conversion works as
	we will explain below.
	
	The C equivalent of goog.math.Long's
	Long.fromNumber() routine, recalling that ES6
	Numbers are just doubles.
	
	'Value' should be integral; otherwise, UB.
	
	Special cases: 
	NaN => 0; 
	-/+ inf => min/max value.
	
	WARNING: as currently implemented,
		this operation is relatively expensive in
		terms of both time and space:
		- it allocates a char[100]
		on the stack,
		- uses sprintf (with a sentintel check);
		- does a lot of i64 arithmetic (to the
		order of log_10(n) iterations, but with
		a single scratch i64.);
*/

i64 i64_fromDouble(double value) {
	char intermediate[50];
	i64 result;
	int idx;
	int sign = 1;
	/* PROLOG:
		We are using 'sprintf'. So
		add a canary value to 'value'. If
		this has been overwritten by the
		end of the routine, we're fucked. */
	intermediate[49] = 0;
		
	
	/* CHECK ASSUMPTIONS: value is an integer */
	printf("i64_toDouble xx: %a == %a = %d\n", value, floor(value), value == floor(value));
	assert(value != value || value == floor(value));
		/* first conditional checks NAN;
			ceil would work as well */
		
	/* We want to split the top and bottom
		words of a double, without
		loss of precision and portably (e.g.
		without directly manipulating representation
		of doubles). The semantically easiest way
		of doing so is by representing doubles
		as string values. */
	sprintf(intermediate, "%#F", value);
		/* obtain output of form
			[-]xxxx.[yyy]
			(note the decimal point is not optional.)
			NANs and INFs are handled portably by %F
				(vs. %f) */
	assert(intermediate[49] == 0);
		/* CHECK: is the string buffer sentinel stil there? 
			Check the canary on our intermediate string
			buffer to make sure 'sprintf' fuckery
			hasn't occurred. If it has, crash FAST. */
			
	/* Pathological cases for 'value'. */
	if(strncmp("NAN", intermediate, 3) == 0) {
		return i64_getZero();
	} else if(strncmp("-INF", intermediate, 4) == 0) {
		return i64_getMinValue();
	} else if(strncmp("INF", intermediate, 3) == 0) {
		return i64_getMaxValue();
	}
		
	/* Initialize string parse state. */
	idx = 0;
	result = i64_getZero();
	
	/* Determine sign of resulting i64. */
	if(intermediate[0] == '-') {
		sign = -1;
		idx++;
	}
	
	/* Shift and add digits until we reach a '0.' */
	for(; (idx < 49) && (intermediate[idx] != '.'); idx++) {
		printf("fromDouble iteration %d: digit %c, result_in = ", idx, intermediate[idx]);
		i64_debugPrintDecimal(result); printf("\n");
		result = i64_multiply(result, i64_fromBits(10, 0));
		printf("fromDouble iteration %d: shift result ", idx);
		i64_debugPrintDecimal(result);
		printf("\n");
		result = i64_add(result, i64_fromBits(intermediate[idx] - '0', 0));
		printf("fromDouble iteration %d: add result ", idx);
		i64_debugPrintDecimal(result);
		printf("\n");
		printf("fromDouble iteration %d: result_out = ", idx);
		i64_debugPrintDecimal(result);
		printf("\n");
	}
	
	/* Negate resulting value if needed */
	if(sign == -1) {
		result = i64_negate(result);
	}
	
	return result;
}

/*
	The C equivalent of goog.math.Long's
	Long.toNumber() routine, recalling that ES6
	Numbers are just doubles.
*/
double i64_toDouble(i64 this) {
	i32 neg_p = this.high_ & 0x80000000;
	double  top, bottom, toRet, scratchf;
	
	if(neg_p != 0) {
		/* this = abs(this) */
		this = i64_negate(this);
		assert((this.high_ & 0x80000000) == 0);
	}

	top = (u32)this.high_ * (double)TWO_PWR_32_DBL_;
	bottom = (u32)this.low_;
	assert((top >= 0) && (bottom >= 0));
	toRet = top + bottom;
	
	if(neg_p != 0) {
		toRet *= -1;
	}

	printf("i64_toDouble absolute values: %f + %f\n", top, bottom);
	
	/* ASSUMPTION: result double has no fractional part */
	assert(modf(toRet, &scratchf) == 0);
	printf("i64_toDouble xx: %a == %a = %d\n", toRet, floor(toRet), toRet == floor(toRet));
	assert(toRet == floor(toRet));
	
	return toRet;
}

/*
	Can we convert from i64 to a double without loss
	of precision? (That is, can we fit our integer
	within the mantissa of a double, with range +/-
	2^53?
*/
int i64_isSafeInteger(i64 this) {
	i32 top11Bits = this.high_ >> 21;
	return
		top11Bits == 0 /* positive case */
		|| (top11Bits = -1 /* negative case... */
				/* ... and exclude -2^53 proper. */
			&& !(this.low_ == 0 & this.high_ == (0xffe00000 | 0)));
}

#define I64_TOFROMSTR_SUCC 0
#define I64_TOFROMSTR_FAIL_RADIX 1

// int i64_toString(i64 this, int radix, char *strOut, int outLen) {
	// /* Check radix in/out of range; */
	// if(radix < 2 || 36 < radix) {
		// strOut[0] = 0;
		// return I64_TOFROMSTR_FAIL_RADIX;
	// }
	
	// /* Begin string conversion by adding a sentinel to the
		// output buffer; */
	// strOut[outLen - 1] = 0;
	
	
	// /* Common cases that correspond to */
	
	// finish_successfully:
	// /* Before finishing (successfully), check to see whether the
		// sentinel is intact; if not, fail fast; */
	// assert(strOut[outLen - 1] == 0);
	// return I64_TOFROMSTR_SUCC;
// }


i64 i64_fromString(int radix, char *strIn, int inLen, int *outStatus);

/*
	BITWISE OPERATIONS
	==================
*/


i64 i64_not(i64 this) {
	return i64_fromBits(~this.low_, ~this.high_);
}

i64 i64_and(i64 this, i64 other) {
	return i64_fromBits
		(this.low_ & other.low_, this.high_ & other.high_);
}

i64 i64_or(i64 this, i64 other) {
	return i64_fromBits
		(this.low_ | other.low_, this.high_ | other.high_);
}

i64 i64_xor(i64 this, i64 other) {
	return i64_fromBits
		(this.low_ ^ other.low_, this.high_ ^ other.high_);
}

/*
	BITSHIFTS (unless specified, ZERO-FILLING)
	==========================================

	NB - we assume we'll only shift by i32s
	(that is, by a values that are 
	  (a) a raw c numeric type, (b) signed,
	  (c) with width at most 32.)
	WATCH OUT: 
	Currently, we don't test whether shifts
	by negative values work as expected.
*/

i64 i64_shiftRight(i64 this, i32 numBits);
	/* Forward ref. needed for left and right
		shift to call each other as needed */
		
i64 i64_shiftLeft(i64 this, i32 numBits) {
	i32 low, high;
	
	/* this next control path, that implements
		shifting by negative values,
		is iffy. */
	if(numBits < 0)
		return i64_shiftRight(this, -numBits);
	
	numBits &= 63;
	if(numBits == 0) {
		return this;
	} else {
		low = this.low_;
		if(numBits < 32) {
			high = this.high_;
			return i64_fromBits(
				low << numBits,
				(high << numBits) | (((i32)low) >> (32 - numBits))
			);
		} else {
			return i64_fromBits(
				0,
				low << (numBits - 32)
			);
		}
	}
}

/* Shift right, retaining the current sign. */
i64 i64_shiftRight(i64 this, i32 numBits) {
	i32 low, high;
	
	/* this next control path, that implements
		shifting by negative values,
		is iffy. */
	if(numBits < 0)
		return i64_shiftLeft(this, -numBits);
	
	numBits &= 63;
	if(numBits == 0) {
		return this;
	} else {
		high = this.high_;
		if(numBits < 32) {
			low = this.low_;
			return i64_fromBits(
				(((i32)low) >> numBits) | (high << (32 - numBits)),
				high >> numBits
			);
		} else {
			return i64_fromBits(
				high >> (numBits - 32),
				high >= 0 ? 0 : -1
			);
		}
	}
}

/* Shift right, replacing leading bits with 0s. */
i64 i64_shiftRightUnsigned(i64 this, i32 numBits) {
	i32 low, high;
	
	/* this next control path, that implements
		shifting by negative values,
		is iffy. */
	if(numBits < 0)
		return i64_shiftLeft(this, -numBits);
	
	numBits &= 63;
	if(numBits == 0) {
		return this;
	} else {
		high = this.high_;
		if(numBits < 32) {
			low = this.low_;
			return i64_fromBits(
				(((i32)low) >> numBits) | (high << (32 - numBits)),
				((i32)high) >> numBits
			);
		} else if(numBits == 32) {
			return i64_fromBits(high, 0);
		} else {
			return i64_fromBits(
				((i32)high) >> (numBits - 32),
				0
			);
		}
	}
}

/*
	DIVISION
	========
	...defined at the end because it uses plenty
	of other routines.
*/

/* i64_div: unlike other routines which are nearly
	always constant time, div is of order O????? */


#endif
