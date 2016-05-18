#ifndef SOBOL_H
#define SOBOL_H

/* maximum allowed space dimension */
#define SOBOL_MAX_DIMENSION 40

/* bit count; assumes sizeof(int) >= 32-bit */
#define SOBOL_BIT_COUNT 30


/* Sobol generator state.
 *   sequence_count       = number of calls with this generator
 *   last_numerator_vec   = last generated numerator vector
 *   last_denominator_inv = 1/denominator for last numerator vector
 *   v_direction          = direction number table
 */
typedef struct
{
  unsigned int  sequence_count;
  double        last_denominator_inv;
  int           last_numerator_vec[SOBOL_MAX_DIMENSION];
  int           v_direction[SOBOL_BIT_COUNT][SOBOL_MAX_DIMENSION];
} sobol_state_t;

#ifdef __cplusplus
extern "C" {
#endif

// static
int sobol_init(void * state, unsigned int dimension);
// static

int sobol_get(void * state, unsigned int dimension, double * v);

#ifdef __cplusplus
}
#endif

#endif
