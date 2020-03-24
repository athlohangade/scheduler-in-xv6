#include "types.h"

ushort generate_random_number(ushort limit) {

    static ushort start_state = 0xACE1u;    /* Any nonzero start state will work */
    ushort lfsr;                            /* Shift register */
    ushort bit;                             /* Result of XORing of tap bits to be appended */
    ushort i = 0;                           /* Iterator */

    if (start_state == 0) {                 /* Only non-zero values acceptable */
        start_state = start_state + 1;
    }
    lfsr = start_state;                     /* Load the shift register */
    start_state = start_state + 1;

    /* Shifting, XORing and Appending */
    do {   
        /* taps: 16 14 13 11; feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1 */
        bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5));
        lfsr = (lfsr >> 1) | (bit << 15);
        i++;
    } while (i < limit);

    /* Return the resultant value present in shift register */
    return (lfsr % limit);
}
