#include "types.h"

/* Algorithm : Linear Feedback Shift Register Random Number Generator
 * Tap bits : 16 14 13 11
 * Feedback Polynomial : x^16 + x^14 + x^13 + x^11 + 1
 */
ushort generate_random_number(ushort limit) {

    /* Any nonzero start state will work */
    static ushort start_state = 0xACE1u;
    /* 16-bit shift register */
    ushort lfsr;
    /* For storing the result of XORing of tap bits to be appended */
    ushort bit;
    /* Iterator */
    ushort i = 0;

    /* Only non-zero values are acceptable.
     * Set the state to a non-zero value by increment it */
    if (start_state == 0) {
        start_state = start_state + 1;
    }

    /* Load the shift register with the start state */
    lfsr = start_state;
    /* Increment the state for next call */
    start_state = start_state + 1;

    /* Shifting, XORing and Appending
     * Loop until the iterator value reaches the limit */
    do {
        /* XOR the tap bits */
        bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5));
        /* Shift the shift register and append the bit to MSB */
        lfsr = (lfsr >> 1) | (bit << 15);
        /* Increment iterator */
        i++;
    } while (i < limit);

    /* Return the resultant value present in shift register with limit as
     * constraint */
    return (lfsr % limit);
}
