/*
	test.c -- tests for i64.c89.h
	
	A straightforward port of Google's original long_test.js.
*/

#include "test_data.h"
#include "i64_c89.h"

#include <stdio.h>
#include <assert.h>
#include <float.h>
#include <math.h>

#define ARRAY_LEN(a) (sizeof a / sizeof a[0])
#define ASSERT_EQUALS(x, y) (assert((x) == (y)))
#define ASSERT_I64_EQUALS(x, y) (assert(i64_isEqual(x, y)))

/* Test i64 comparisons */
void testComparisons() {
	int i, j;
	i64 vi, vj;

	for (i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
        vi = i64_fromBits(TEST_BITS[i + 1], TEST_BITS[i]);
		for (j = 0; j < ARRAY_LEN(TEST_BITS); j += 2) {	
			printf("testComparions: test id %d, %d\n", i, j);
			vj = i64_fromBits(TEST_BITS[j + 1], TEST_BITS[j]);
			ASSERT_EQUALS(i == j, i64_equals(vi, vj));
			ASSERT_EQUALS(i != j, i64_notEquals(vi, vj));
			ASSERT_EQUALS(i < j, i64_lessThan(vi, vj));
			ASSERT_EQUALS(i <= j, i64_lessThanOrEqual(vi, vj));
			ASSERT_EQUALS(i > j, i64_greaterThan(vi, vj));
			ASSERT_EQUALS(i >= j, i64_greaterThanOrEqual(vi, vj));
		}
    }	
	
	printf("testComparions: all tests good\n");
}

/* Test bitwise operations. */
void testBitOperations() {
	int i, j, len;
	i64 vi, vj;
	
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
		printf("testBitOperations: test id %d, *\n", i);
		vi = i64_fromBits(TEST_BITS[i + 1], TEST_BITS[i]);
		ASSERT_EQUALS(~TEST_BITS[i], i64_getHighBits(i64_not(vi)));
		ASSERT_EQUALS(~TEST_BITS[i + 1], i64_getLowBits(i64_not(vi)));
		
		for(j = 0; j < ARRAY_LEN(TEST_BITS); j += 2) {
			printf("testBitOperations: test id %d, %d\n", i, j);
			vj = i64_fromBits(TEST_BITS[j + 1], TEST_BITS[j]);
			ASSERT_EQUALS(TEST_BITS[i] & TEST_BITS[j],
				i64_getHighBits(i64_and(vi, vj)));
			ASSERT_EQUALS(TEST_BITS[i + 1] & TEST_BITS[j + 1],
				i64_getLowBits(i64_and(vi, vj)));
			ASSERT_EQUALS(TEST_BITS[i] | TEST_BITS[j],
				i64_getHighBits(i64_or(vi, vj)));
			ASSERT_EQUALS(TEST_BITS[i + 1] | TEST_BITS[j + 1],
				i64_getLowBits(i64_or(vi, vj)));
			ASSERT_EQUALS(TEST_BITS[i] ^ TEST_BITS[j],
				i64_getHighBits(i64_xor(vi, vj)));
			ASSERT_EQUALS(TEST_BITS[i + 1] ^ TEST_BITS[j + 1],
				i64_getLowBits(i64_xor(vi, vj)));
		}
		
		printf("testBitOperations: test id %d, ** , *\n", i);
		ASSERT_EQUALS(TEST_BITS[i], 
			i64_getHighBits(i64_shiftLeft(vi, 0)));
		ASSERT_EQUALS(TEST_BITS[i + 1], 
			i64_getLowBits(i64_shiftLeft(vi, 0)));
		ASSERT_EQUALS(TEST_BITS[i], 
			i64_getHighBits(i64_shiftRight(vi, 0)));
		ASSERT_EQUALS(TEST_BITS[i + 1],
			i64_getLowBits(i64_shiftRight(vi, 0)));
		ASSERT_EQUALS(TEST_BITS[i], 
			i64_getHighBits(i64_shiftRightUnsigned(vi, 0)));
		ASSERT_EQUALS(TEST_BITS[i + 1],
			i64_getLowBits(i64_shiftRightUnsigned(vi, 0)));
			
		ASSERT_EQUALS(TEST_BITS[i], 
			i64_getHighBits(i64_shiftLeft(vi, 64)));
		ASSERT_EQUALS(TEST_BITS[i + 1], 
			i64_getLowBits(i64_shiftLeft(vi, 64)));
		ASSERT_EQUALS(TEST_BITS[i], 
			i64_getHighBits(i64_shiftRight(vi, 64)));
		ASSERT_EQUALS(TEST_BITS[i + 1],
			i64_getLowBits(i64_shiftRight(vi, 64)));
		ASSERT_EQUALS(TEST_BITS[i], 
			i64_getHighBits(i64_shiftRightUnsigned(vi, 64)));
		ASSERT_EQUALS(TEST_BITS[i + 1],
			i64_getLowBits(i64_shiftRightUnsigned(vi, 64)));
		
		for(len = 1; len < 64; ++len) {
			printf("testBitOperations: test id %d, ***, len %d\n", i, len);
			if(len < 32) {
				ASSERT_EQUALS(
					(TEST_BITS[i] << len) | 
						((i32)(TEST_BITS[i + 1])) >> (32 - len),
					i64_getHighBits(i64_shiftLeft(vi, len))					
				);
				ASSERT_EQUALS(
					TEST_BITS[i + 1] << len,
					i64_getLowBits(i64_shiftLeft(vi, len))
				);
				
				ASSERT_EQUALS(
					TEST_BITS[i] >> len,
					i64_getHighBits(i64_shiftRight(vi, len))
				);
				ASSERT_EQUALS(
					((i32)TEST_BITS[i + 1]) >> len | 
						(TEST_BITS[i] << (32 - len)),
					i64_getLowBits(i64_shiftRight(vi, len))
				);
			} else {
				ASSERT_EQUALS(
					TEST_BITS[i + 1] << (len - 32),
					i64_getHighBits(i64_shiftLeft(vi, len))
				);
				ASSERT_EQUALS(
					0,
					i64_getLowBits(i64_shiftLeft(vi, len))
				);
				
				ASSERT_EQUALS(
					TEST_BITS[i] >= 0 ? 0 : -1,
					i64_getHighBits(i64_shiftRight(vi, len))
				);
				ASSERT_EQUALS(
					TEST_BITS[i] >> (len - 32),
					i64_getLowBits(i64_shiftRight(vi, len))
				);
				
				if(len == 32) {
					ASSERT_EQUALS(
						TEST_BITS[i],
						i64_getLowBits(i64_shiftRightUnsigned(vi, len))
					);
				} else {
					ASSERT_EQUALS(
						((i32)TEST_BITS[i]) >> (len - 32),
						i64_getLowBits(i64_shiftRightUnsigned(vi, len))
					);
				}
			}
		}
	}
	printf("testBitOperations: all tests good\n");
}


/* Test division and modulus. 
void testDivMod() {
	int i, j;
	i64 vi, vj, divResult, modResult;
	int countPerDivMod, countPerDivModCall;
	
	
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
		vi = i64_fromBits(TEST_BITS[i + 1], TEST_BITS[i]);
		for(j = 0; j < ARRAY_LEN(TEST_BITS); j += 2) {
			printf("testDivMOd: test id %d, %d\n", i, j);
			vj = i64_fromBits(TEST_BITS[j + 1], TEST_BITS[j]);
			
		}
	}
	
	printf("testDivMod: all tests good\n");
}

*/

/* test i64_isZero. */
void testIsZero() {
	int i;
	i64 val;
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
		printf("testIsZero: test id %d\n", i);
		val = i64_fromBits(TEST_BITS[i + 1], TEST_BITS[i]);
		ASSERT_EQUALS(
			TEST_BITS[i] == 0 && TEST_BITS[i + 1] == 0,
			i64_isZero(val)
		);
	}
	printf("testIsZero: all tests good\n");
}

/* test i64_isNegative. */
void testIsNegative() {
	int i;
	i64 val;
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
		printf("testIsNegative: test id %d\n", i);
		val = i64_fromBits(TEST_BITS[i + 1], TEST_BITS[i]);
		ASSERT_EQUALS(
			(TEST_BITS[i] >> 31) != 0,
			i64_isNegative(val)
		);
	}
	printf("testIsNegative: all tests good\n");
}

/* test i64_isOdd. */
void testIsOdd() {
	int i;
	i64 val;
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
		printf("testIsOdd: test id %d\n", i);
		val = i64_fromBits(TEST_BITS[i + 1], TEST_BITS[i]);
		ASSERT_EQUALS(
			(TEST_BITS[i + 1] & 1) != 0,
			i64_isOdd(val)
		);
	}
	printf("testIsOdd: all tests good\n");
}

/* test i64_(to|from)String. */


/* test i64_negate. */
void testNegation() {
	int i;
	i64 vi;
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
		printf("testNegation: test id %d\n", i);
		vi = i64_fromBits(TEST_BITS[i + 1], TEST_BITS[i]);
		if(TEST_BITS[i + 1] == 0) {
			/* if the lowest word is all 0s, then it
				behaves very simply when negated. */
			ASSERT_EQUALS(
				(~TEST_BITS[i] + 1) | 0,
				i64_getHighBits(i64_negate(vi))
			);
			ASSERT_EQUALS(
				0,
				i64_getLowBits(i64_negate(vi))
			);
		} else {
			/* this is the case where both words
				are nonzero. */
			ASSERT_EQUALS(
				~TEST_BITS[i],
				i64_getHighBits(i64_negate(vi))
			);
			ASSERT_EQUALS(
				(~TEST_BITS[i + 1] + 1) | 0,
				i64_getLowBits(i64_negate(vi))
			);
		}
	}
	printf("testNegation: all tests good\n");
}

/* test i64_add. */
void testAdd() {
	int i, j, count;
	i64 vi, vj, result;
	
	count = 0;
	/* count essentially indexes into an array of precomputed
		results (TEST_ADD_BITS) */
	
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
		vi = i64_fromBits(TEST_BITS[i + 1], TEST_BITS[i]);
		for(j = 0; j < ARRAY_LEN(TEST_BITS); j += 2) {
			printf("testAdd: test id %d, %d\n", i, j);
			vj = i64_fromBits(TEST_BITS[j + 1], TEST_BITS[j]);
			result = i64_add(vi, vj);
			ASSERT_EQUALS(
				TEST_ADD_BITS[count++], i64_getHighBits(result)
			);
			ASSERT_EQUALS(
				TEST_ADD_BITS[count++], i64_getLowBits(result)
			);
		}
	}
	printf("testAdd: all tests good\n");
}

/* test i64_subtract.
	Implementation differs from goog.math.Long's test
	case */
void testSubtract() {
	int i, j, count;
	i64 vi, vj, result;
	
	count = 0;
	/* count essentially indexes into an array of precomputed
		results (TEST_SUB_BITS) */
	
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
		vi = i64_fromBits(TEST_BITS[i + 1], TEST_BITS[i]);
		for(j = 0; j < ARRAY_LEN(TEST_BITS); j += 2) {
			printf("testSubtract: test id %d, %d\n", i, j);
			vj = i64_fromBits(TEST_BITS[j + 1], TEST_BITS[j]);
			result = i64_subtract(vi, vj);
			ASSERT_EQUALS(
				TEST_SUB_BITS[count++], i64_getHighBits(result)
			);
			ASSERT_EQUALS(
				TEST_SUB_BITS[count++], i64_getLowBits(result)
			);
		}
	}
	printf("testSubtract: all tests good\n");
}

/*
	Test i64_multiply.
*/
void testMultiply() {	int i, j, count;
	i64 vi, vj, result;
	
	count = 0;
	/* count essentially indexes into an array of precomputed
		results (TEST_MUL_BITS) */
	
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
		vi = i64_fromBits(TEST_BITS[i + 1], TEST_BITS[i]);
		for(j = 0; j < ARRAY_LEN(TEST_BITS); j += 2) {
			printf("testMultiply: test id %d, %d\n", i, j);
			vj = i64_fromBits(TEST_BITS[j + 1], TEST_BITS[j]);
			result = i64_multiply(vi, vj);
			ASSERT_EQUALS(
				TEST_MUL_BITS[count++], i64_getHighBits(result)
			);
			ASSERT_EQUALS(
				TEST_MUL_BITS[count++], i64_getLowBits(result)
			);
		}
	}
	printf("testMultiply: all tests good\n");
}

/*
	Test i64_fromBits, and i64_get(High|Low)Bits
	accessors
*/
void testToFromBits() {
	int i;
	i64 val;
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
		printf("testToFromBits: test id %d\n", i);
		val = i64_fromBits(TEST_BITS[i + 1], TEST_BITS[i]);
		ASSERT_EQUALS(TEST_BITS[i], i64_getHighBits(val));
		ASSERT_EQUALS(TEST_BITS[i + 1], i64_getLowBits(val));
	}
	printf("testToFromBits: all tests good\n");
}

/* 
	Test i64_fromInt
*/
void testToFromInt() {
	int i;
	i64 val;
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 1) {
		printf("testToFromInt: test id %d\n", i);
		val = i64_fromInt(TEST_BITS[i]);
		ASSERT_EQUALS(TEST_BITS[i], i64_toInt(val));
	}
	printf("testToFromInt: all tests good\n");
}

/*
	Test i64_(to|from)Double, recalling that
	doubles are ES6 Numbers, and so we test
	the equivalent of goog.math.Long's to/fromNumber
*/

	/* these constants aren't defined in C89, so
		we have to define them ourselves. */
#define DBL_INF ((double)(1.0f / 0.0f))
#define DBL_NAN ((double)(DBL_INF / DBL_INF))

void testToFromDouble() {
	int i;
	double num;
	i64 val;
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
		/* test a round-trip to and from doubles via i64. */
		printf("testToFromDouble: test id %d\n", i);
		num = (TEST_BITS[i] * pow(2, 32) + TEST_BITS[i + 1] >= 0) ?
			TEST_BITS[i + 1] :
			pow(2, 32) + TEST_BITS[i + 1]; 
		ASSERT_EQUALS(0, fmod(num, 1));
		printf("before roundtrip as double: %lf\n", num);
		val = i64_fromDouble(num);
		printf("after fromDouble: "); i64_debugPrintDecimal(val); printf("\n");
		printf("after roundtrip: %lf\n", i64_toDouble(val));
		ASSERT_EQUALS(num, i64_toDouble(val));
	}
	ASSERT_I64_EQUALS(i64_getZero(), i64_fromDouble(DBL_NAN));
	ASSERT_I64_EQUALS(i64_getMaxValue(), i64_fromDouble(DBL_INF));
	ASSERT_I64_EQUALS(i64_getMinValue(), i64_fromDouble(-DBL_INF));
	printf("testToFromDouble: all tests good\n");
}


int main(int argc, char **argv) {
	testComparisons();
	testBitOperations();
	/*testDivMod(); */
	/*testToFromString();*/
	/*testToFromUnsignedString(); */
	testToFromBits();
	testToFromInt();
	testToFromDouble(); /* our equivalent of testToFromNumber */
	/* testFromDecimalCachedValues not defined, since
		caching functionality not needed */	
	
	/* temporarily skip the order of tests for a bit... */
	testIsZero();
	testIsNegative();
	testIsOdd();
	testNegation();
	testAdd();
	testSubtract();
	testMultiply();
	
	/* testIsStringInRange() */
	/* testBase36ToString() */
	/* testBaseDefaultFromString() */
	/* testHashCode not defined, since Long.hashCode not implemented */
	/* testIsSafeInteger(); needs from number */
	
	printf("tests: all tests good.\n");
}
