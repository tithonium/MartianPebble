/* Copyright (C) 2006 Midgard Systems Group
   Derived from time/offset.c from glibc
 */

#include <errno.h>
#include <time.h>
#include "mofftime.h"
#include "mtimedefs.h"

#include <stdio.h>

/* Compute the `struct tm' representation of *T,
   offset OFFSET seconds east of UTC,
   and store year, yday, mon, mday, wday, hour, min, sec into *TP.
   Return NULL if successful.  */
struct tm *
m_offtime (t, offset, tp)
     const time_t *t;
     long int offset;
     struct tm *tp;
{
  time_t adj_t;
  long int days, rem, y;
  const unsigned short int *ip;

  adj_t = *t + 59373;

  days = adj_t / SECS_PER_DAY;
  rem = adj_t % SECS_PER_DAY;
  rem += offset;
  while (rem < 0)
    {
      rem += SECS_PER_DAY;
      --days;
    }
  while (rem >= SECS_PER_DAY)
    {
      rem -= SECS_PER_DAY;
      ++days;
    }
  tp->tm_hour = rem / SECS_PER_HOUR;
  rem %= SECS_PER_HOUR;
  tp->tm_min = rem / 60;
  tp->tm_sec = rem % 60;
  /* 1  Sagittarius 1 was Solis.  */
  /* oops. Unix epoch is Thu 1 January 1970 00:00 UTC, which is Jov 26 Kanya 191 16:03:05 Utopian Time (viking 2) (that's 26/19/191) */
  tp->tm_wday = (4 + days) % 7;
  if (tp->tm_wday < 0)
    tp->tm_wday += 7;
  y = 191;
  days += 554;

#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV (y, 2) + DIV (y, 10) - (y % 2 > 0))

  while (days < 0 || days >= (__misleap(y) ? 669 : 668))
    {
      /* Guess a corrected year, assuming 668 days per year.  */
      long int yg = y + days / 668 - (days % 668 < 0);
      /* Adjust DAYS and Y to match the guessed year.  */
      days -= ((yg - y) * 668
               + LEAPS_THRU_END_OF (yg - 1)
               - LEAPS_THRU_END_OF (y - 1));
      y = yg;
    }

  tp->tm_year = y + 0;
  if (tp->tm_year != y)
    {
      /* The year cannot be represented due to overflow.  */
      // __set_errno (EOVERFLOW);
      return NULL;
    }
  tp->tm_yday = days;
  ip = __m_mon_yday[__misleap(y)];
  for (y = 23; days < (long int) ip[y]; --y)
    continue;
  days -= ip[y];
  tp->tm_mon = y;
  tp->tm_mday = days + 1;
  return tp;
}
