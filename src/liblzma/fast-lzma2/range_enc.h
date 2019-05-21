/*
* Bitwise range encoder by Igor Pavlov
* Modified by Conor McCarthy
*
* Public domain
*/

#ifndef RANGE_ENCODER_H
#define RANGE_ENCODER_H

#include "range_common.h"
#include "price.h"

#define RC_PROB_INIT_VALUE (RC_BIT_MODEL_TOTAL >> 1U)
#define RC_PRICE_TABLE_SIZE (RC_BIT_MODEL_TOTAL >> RC_MOVE_REDUCING_BITS)

#if 0
void rc_print_price_table();
#endif

typedef struct
{
	uint8_t *out_buffer;
	size_t out_index;
	uint64_t cache_size;
	uint64_t low;
	uint32_t range;
	uint8_t cache;
} lzma_range_fast_enc;

void rcf_reset(lzma_range_fast_enc* const rc);

void rcf_set_output_buffer(lzma_range_fast_enc* const rc, uint8_t *const out_buffer);

void FORCE_NOINLINE rcf_shift_low(lzma_range_fast_enc* const rc);

void rcf_bittree(lzma_range_fast_enc* const rc, probability *const probs, unsigned bit_count, unsigned symbol);

void rcf_bittree_reverse(lzma_range_fast_enc* const rc, probability *const probs, unsigned bit_count, unsigned symbol);

void FORCE_NOINLINE rcf_direct(lzma_range_fast_enc* const rc, unsigned value, unsigned bit_count);

HINT_INLINE
void rcf_bit_0(lzma_range_fast_enc* const rc, probability *const rprob)
{
	unsigned prob = *rprob;
    rc->range = (rc->range >> RC_BIT_MODEL_TOTAL_BITS) * prob;
	prob += (RC_BIT_MODEL_TOTAL - prob) >> RC_MOVE_BITS;
	*rprob = (probability)prob;
	if (rc->range < RC_TOP_VALUE) {
        rc->range <<= 8;
		rcf_shift_low(rc);
	}
}

HINT_INLINE
void rcf_bit_1(lzma_range_fast_enc* const rc, probability *const rprob)
{
	unsigned prob = *rprob;
	uint32_t new_bound = (rc->range >> RC_BIT_MODEL_TOTAL_BITS) * prob;
    rc->low += new_bound;
    rc->range -= new_bound;
	prob -= prob >> RC_MOVE_BITS;
	*rprob = (probability)prob;
	if (rc->range < RC_TOP_VALUE) {
        rc->range <<= 8;
		rcf_shift_low(rc);
	}
}

HINT_INLINE
void rcf_bit(lzma_range_fast_enc* const rc, probability *const rprob, unsigned const bit)
{
	unsigned prob = *rprob;
	if (bit != 0) {
		uint32_t const new_bound = (rc->range >> RC_BIT_MODEL_TOTAL_BITS) * prob;
        rc->low += new_bound;
        rc->range -= new_bound;
		prob -= prob >> RC_MOVE_BITS;
	}
	else {
        rc->range = (rc->range >> RC_BIT_MODEL_TOTAL_BITS) * prob;
		prob += (RC_BIT_MODEL_TOTAL - prob) >> RC_MOVE_BITS;
	}
	*rprob = (probability)prob;
	if (rc->range < RC_TOP_VALUE) {
        rc->range <<= 8;
		rcf_shift_low(rc);
	}
}

#define GET_PRICE(prob, symbol) \
  lzma_rc_prices[symbol][(prob) >> RC_MOVE_REDUCING_BITS]

#define GET_PRICE_0(prob) lzma_rc_prices[0][(prob) >> RC_MOVE_REDUCING_BITS]

#define GET_PRICE_1(prob) lzma_rc_prices[1][(prob) >> RC_MOVE_REDUCING_BITS]

#define MIN_LITERAL_PRICE 8U

HINT_INLINE
void rcf_flush(lzma_range_fast_enc* const rc)
{
    for (int i = 0; i < 5; ++i)
        rcf_shift_low(rc);
}


#endif /* RANGE_ENCODER_H */