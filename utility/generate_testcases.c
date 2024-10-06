/*
	generate_testcases.c
	
	Using test vectors in test_data.h and a reference
	uint_64 implementation with twos' complement,
	generate expected results for additions done with i64_add.
	
	_REQUIRES C99.__
*/


#include "../test_data.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define ARRAY_LEN(a) (sizeof a / sizeof a[0])
#define MIN(x, y) (x < y ? x : y)

#define INVALID_ARGS 0
#define TEST_ADD 1
#define TEST_MUL 2
#define TEST_FROMDOUBLE 3

int parseArgs(int argc, char** argv) {
	int arg1Len;
	
	if(argc != 2)
		return INVALID_ARGS;
	
	arg1Len = strlen(argv[1]);
		/* of course, doesn't handle
			the case where argv[1] is
			never terminated. */
	
	if(0 == strncmp("add", argv[1], MIN(3, arg1Len))) {
		return TEST_ADD;
	}
	
	if(0 == strncmp("multiply", argv[1], MIN(8, arg1Len))) {
		return TEST_MUL;
	}
	
		if(0 == strncmp("fromdouble", argv[1], MIN(10, arg1Len))) {
		return TEST_FROMDOUBLE;
	}
	
	/* at this point, invalid args: 2 arguments, but we
		haven't recognized the second one */
	return INVALID_ARGS;
}

int main(int argc, char** argv) {
	int i, j, pairsThisLine;
	int64_t x, y, result;
	int32_t high_w, low_w;
	/* note of course that signed and unsigned
		addition is the same in twos' complement, but
		C downcasting differs. */
		
	int forTests = parseArgs(argc, argv);
	
	if(forTests == INVALID_ARGS) {
		printf("help: generate_testcases (add | multiply | fromdouble)\n");
		return 1;
	}
	
	pairsThisLine = 0;
	printf("{\n");
	for(i = 0; i < ARRAY_LEN(TEST_BITS); i += 2) {
		for(j = 0; j < ARRAY_LEN(TEST_BITS); j += 2) {
			x = (((uint64_t)(TEST_BITS[i])) << 32) 
				+ (TEST_BITS[i + 1] & 0xFFFFFFFF);
			y = (((uint64_t)(TEST_BITS[j])) << 32) 
				+ (TEST_BITS[j + 1] & 0xFFFFFFFF);
				
			if(forTests == TEST_ADD) {
				result = x + y;
			}
			else if(forTests == TEST_MUL) {
				result = x * y;
			}
			/* fromDouble requires that
				doubles have mantissa width less
				than or equal to/than i64 (more 
				specifically, to test whether we are greater
				than values representable in an i64. */
			#if !(DBL_MANT_DIG <= 63)
				#error "Error: i64 conversion from doubles requires that" \
						"doubles with no expontent are representable in" \
						"an i64."
			#endif
			else if(forTests == TEST_FROMDOUBLE) {
				/* This method of creating test doubles differs from that
					of goog.math.Long. */
				double srcDouble = ( ((j/4)%2) ? 1 : -1) *
					TEST_BITS[i];
				
				/* doubles corresponding to non-representable #s in an i64. */
				if(srcDouble > (pow(2, 51) - 1)) {
						/* recall we have one less positive int in 2s complement. */
					/* case 1/2 of large #s */
					result = 0x7FFFFFFFFFFFFFFF;
				} else if (srcDouble == HUGE_VAL) {
					/* case 2/2 of large #s */
					result = 0x7FFFFFFFFFFFFFFF;
				} else if (srcDouble < -1 * pow(2, 51)) {
					/* single case of small #s, since we don't
					 have (in C99) an underflowing long double
					 constant. It's unclear whether this is
					 a sufficient test for all rounding policies
					 available in all possible floating-point
					 environments. */
					result = 0x8000000000000000;
				} else if (srcDouble == NAN) {
					result = 0x0000000000000000;
				}
				
				/* at this point, we assume that srcDouble
					is exactly representable as two 32-bit
					literals. */
				else {
					result = srcDouble;
					/* well, we should test that assumption... */
					assert(result == srcDouble);
				}
			}
			
			high_w = (((uint64_t)result) >> 32); /* unsigned shift needed */
			low_w = result;		
						
			pairsThisLine++;
			printf("0x%.8x, 0x%.8x, ", high_w, low_w);
			if(pairsThisLine == 3) {
				printf("\n");
				pairsThisLine = 0;
			}
		}
	}
	printf("}\n");
	
	return 0;
}