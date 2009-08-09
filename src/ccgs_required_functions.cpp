#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "ccgs_required_functions.h"

/* functions all taken from CISSolve.cxx */
double
factorial(double x)
{
  double ret = 1.0;
  while (x > 0.0)
  {
    ret *= x;
    x -= 1.0;
  }
  return ret;
}

double arbitrary_log(double value, double logbase)
{
  return log(value) / log(logbase);
}

double gcd_pair(double a, double b)
{
  uint32_t ai = (uint32_t)fabs(a), bi = (uint32_t)fabs(b);
  unsigned int shift = 0;
  if (ai == 0)
    return bi;
  if (bi == 0)
    return ai;
#define EVEN(x) ((x&1)==0)
  while (EVEN(ai) && EVEN(bi))
  {
    shift++;
    ai >>= 1;
    bi >>= 1;
  }
  do
  {
    if (EVEN(ai))
      ai >>= 1;
    else if (EVEN(bi))
      bi >>= 1;
    else if (ai >= bi)
      ai = (ai - bi) >> 1;
    else
      bi = (bi - ai) >> 1;
  }
  while (ai > 0);

  return (bi << shift);
}

double lcm_pair(double a, double b)
{
  return (a * b) / gcd_pair(a, b);
}

double gcd_multi(uint32_t count, ...)
{
  va_list val;
  double* storage1, *storage2, *t, ret;
  uint32_t i, j = 0;

  if (count == 0)
    return 1.0;

  storage1 = new double[count];
  assert(storage1 != NULL /* Out of memory? */);
  storage2 = new double[count >> 1];
  assert(storage2 != NULL /* Out of memory? */);

  va_start(val, count);
  for (i = 0; i < count - 1; i += 2)
  {
    double a1, a2;
    a1 = va_arg(val, double);
    a2 = va_arg(val, double);
    storage1[j++] = gcd_pair(a1, a2);
  }
  if (i < count)
    storage1[j++] = va_arg(val, double);
  va_end(val);

  while (j != 1)
  {
    count = j;
    j = 0;

    for (i = 0; i < j - 1; i += 2)
      storage2[j++] = gcd_pair(storage1[i], storage1[i + 1]);
    if (i < j)
      storage2[j++] = storage1[i];

    t = storage1;
    storage1 = storage2;
    storage2 = t;
  }

  ret = storage1[0];
  delete [] storage1;
  delete [] storage2;

  return ret;
}

double lcm_multi(uint32_t count, ...)
{
  va_list val;
  double* storage1, *storage2, *t, ret;
  uint32_t i, j = 0;

  if (count == 0)
    return 1.0;

  storage1 = new double[count];
  assert(storage1 != NULL /* Out of memory? */);
  storage2 = new double[count >> 1];
  assert(storage2 != NULL /* Out of memory? */);

  va_start(val, count);
  for (i = 0; i < count - 1; i += 2)
  {
    double a1, a2;
    a1 = va_arg(val, double);
    a2 = va_arg(val, double);
    storage1[j++] = lcm_pair(a1, a2);
  }
  if (i < count)
    storage1[j++] = va_arg(val, double);
  va_end(val);

  while (j != 1)
  {
    count = j;
    j = 0;

    for (i = 0; i < j - 1; i += 2)
      storage2[j++] = lcm_pair(storage1[i], storage1[i + 1]);
    if (i < j)
      storage2[j++] = storage1[i];

    t = storage1;
    storage1 = storage2;
    storage2 = t;
  }

  ret = storage1[0];
  delete [] storage1;
  delete [] storage2;

  return ret;
}

double multi_min(uint32_t count, ...)
{
  va_list val;
  double best, attempt;
  if (count == 0)
    return strtod("NAN", NULL);
  va_start(val, count);
  best = va_arg(val, double);
  while (--count)
  {
    attempt = va_arg(val, double);
    if (attempt < best)
      best = attempt;
  }
  va_end(val);

  return best;
}

double multi_max(uint32_t count, ...)
{
  va_list val;
  double best, attempt;
  if (count == 0)
    return strtod("NAN", NULL);
  va_start(val, count);
  best = va_arg(val, double);
  while (--count)
  {
    attempt = va_arg(val, double);
    if (attempt > best)
      best = attempt;
  }
  va_end(val);

  return best;
}

double
safe_quotient(double num, double den)
{
  if (!isfinite(num) || !isfinite(den))
    return strtod("NAN", NULL);
  int inum = (int)num, iden = (int)den;
  if (iden == 0)
    return strtod("NAN", NULL);

  return inum / iden;
}

double
safe_remainder(double num, double den)
{
  if (!isfinite(num) || !isfinite(den))
    return strtod("NAN", NULL);
  int inum = (int)num, iden = (int)den;
  if (iden == 0)
    return strtod("NAN", NULL);

  return inum % iden;
}

double
safe_factorof(double num, double den)
{
  if (!isfinite(num) || !isfinite(den))
    return strtod("NAN", NULL);
  int inum = (int)num, iden = (int)den;
  if (iden == 0)
    return strtod("NAN", NULL);

  return ((inum % iden) == 0) ? 1.0 : 0.0;
}

#define NR_RANDOM_STARTS 100
#define NR_MAX_STEPS 1000
#define NR_MAX_STEPS_INITIAL 10

static double
random_double_logUniform()
{
  union
  {
    double asDouble;
#ifdef WIN32
    uint16_t asIntegers[4];
#else
    uint32_t asIntegers[2];
#endif
  } X;

#ifdef WIN32
  uint16_t spareRand;
#else
  uint32_t spareRand;
#endif

  do
  {
    spareRand = rand();
#ifdef WIN32
    X.asIntegers[0] = rand() | ((spareRand & 0x1) << 15);
    X.asIntegers[1] = rand() | ((spareRand & 0x2) << 14);
    X.asIntegers[2] = rand() | ((spareRand & 0x4) << 13);
    X.asIntegers[3] = rand() | ((spareRand & 0x8) << 12);
#else
    X.asIntegers[0] = rand() | ((spareRand & 0x1) << 31);
    X.asIntegers[1] = rand() | ((spareRand & 0x2) << 30);
#endif
  }
  while (!isfinite(X.asDouble));

  return X.asDouble;
}

static double
take_numeric_derivative
(
 double(*func)(double VOI, double *C, double *R, double *S, double *A),
 double VOI,
 double *C,
 double *R,
 double *S,
 double *A,
 double *x
)
{
  double saved_x, value0, value1, value2, delta, slope1, slope2, ratio;
  saved_x = *x;
  /*
   * Since we have only 52 bits of mantissa, we need delta to flip only the
   * last couple of bits.
   */
  delta = saved_x * 1E-10;
  value0 = func(VOI, C, R, S, A);
  *x -= delta;
  value1 = func(VOI, C, R, S, A);
  *x = saved_x + delta;
  value2 = func(VOI, C, R, S, A);
  *x = saved_x;
  /* We take two numeric derivatives, so we can compare them. This stops us
   * from getting trapped at around discontinuities(which otherwise produce
   * very steep trapping slopes).
   */
  slope1 = (value0 - value1) / delta;
  slope2 = (value2 - value0) / delta;

  /* If one slope is zero, return the other... */
  if (slope1 == 0.0 || !isfinite(slope1))
    return slope2;
  if (slope2 == 0.0 || !isfinite(slope2))
    return slope1;

  ratio = slope1 / slope2;
  /* If the slopes are similar, return the average... */
  if (ratio > 0.5 && ratio < 2)
    return (slope1 + slope2) / 2.0;
  /* Otherwise, return the least steep of the two... */
  if (fabs(slope1) < fabs(slope2))
    return slope1;
  return slope2;
}

void
NR_MINIMISE
(
 double(*func)(double VOI, double *C, double *R, double *S, double *A),
 double VOI,
 double *C,
 double *R,
 double *S,
 double *A,
 double *x
)
{
  double best_X = 0.0, best_fX = INFINITY;
  double current_X, current_fX, current_dfX_dX;
  uint32_t steps, maxsteps;
  uint32_t i;

  current_X = *x;

  /* We use a 100 random start Newton-Raphson algorithm... */
  for (i = 0; i < NR_RANDOM_STARTS; i++)
  {
    /* Choose a random X as a starting point, except for the first run. */
    if (i > 0 || !isfinite(current_X))
      current_X = random_double_logUniform();

    maxsteps = NR_MAX_STEPS_INITIAL;
    for (steps = 0; steps < maxsteps; steps++)
    {
      *x = current_X;
      current_fX = func(VOI, C, R, S, A);
      if (!isfinite(current_fX))
      {
        break;
      }

      if (best_fX > current_fX)
      {
        /* We can go past NR_MAX_STEPS_INITIAL steps up to NR_MAX_STEPS steps,
         * but only if we keep improving on our previous answer.
         */
        maxsteps += 2;
        if (maxsteps > NR_MAX_STEPS)
          maxsteps = NR_MAX_STEPS;
        best_fX = current_fX;
        best_X = current_X;
        /* This is quite far from 0(relatively speaking for a double) to avoid
         * numerical instability issues when the minimum doesn't exist but the
         * limit at a point from a particular direction would otherwise be the
         * minimum.
         */
        if (best_fX <= 1E-250)
	{
          break;
	}
      }

      current_dfX_dX = take_numeric_derivative(func, VOI, C, R, S, A, x);
      /* If it is completely flat, or infinite, we are done with this round. */
      if (!isfinite(current_dfX_dX) || current_dfX_dX == 0.0)
      {
        break;
      }

      current_X -= current_fX / current_dfX_dX;
      if (!isfinite(current_X))
      {
        break;
      }
    }
    if (best_fX <= 1E-250)
      break;
  }

  *x = best_X;
}
