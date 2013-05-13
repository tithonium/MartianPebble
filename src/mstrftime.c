/* Copyright (C) 2013 Midgard Systems Group
   Derived from time/strftime.c from glibc
 */

#include <stdio.h>
#include <time.h>
#include "mstrftime.h"
#include "mtimedefs.h"

#include <limits.h>
#include <ctype.h>
#include <sys/types.h>      /* Some systems define `time_t' here.  */


#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifndef NULL
# define NULL 0
#endif

#define TYPE_SIGNED(t) ((t) -1 < 0)

/* Bound on length of the string representing an integer value of type t.
   Subtract one for the sign bit if t is signed;
   302 / 1000 is log10 (2) rounded up;
   add one for integer division truncation;
   add one more for a minus sign if t is signed.  */
#define INT_STRLEN_BOUND(t) \
 ((sizeof(t) * 8 - TYPE_SIGNED(t)) * 302 / 1000 + 1 + TYPE_SIGNED(t))

#define memset_space(P, Len) (memset ((P), ' ', (Len)), (P) += (Len))
#define memset_zero(P, Len) (memset ((P), '0', (Len)), (P) += (Len))
#define MEMCPY(d, s, n) memcpy ((d), (s), (n))

#define add(n, f)                                 \
  do                                          \
    {                                         \
      int _n = (n);                               \
      int _delta = width - _n;                            \
      int _incr = _n + (_delta > 0 ? _delta : 0);                 \
      if ((size_t) _incr >= maxsize - i)                      \
    return 0;                                 \
      if (p)                                      \
    {                                     \
      if (_delta > 0)                             \
        {                                     \
          if (pad == '0')                         \
        memset_zero (p, _delta);                      \
          else                                \
        memset_space (p, _delta);                     \
        }                                     \
      f;                                      \
      p += _n;                                \
    }                                     \
      i += _incr;                                 \
    } while (0)

#define cpy(n, s) \
    add ((n),                                     \
     if (to_lowcase)                              \
       memcpy_lowcase (p, (s), _n);                \
     else if (to_uppcase)                             \
       memcpy_uppcase (p, (s), _n);                \
     else                                     \
       MEMCPY ((void *) p, (const void *) (s), _n))

#  define TOUPPER(Ch, L) Ch //(islower (Ch) ? toupper (Ch) : (Ch))
#  define TOLOWER(Ch, L) Ch //(isupper (Ch) ? tolower (Ch) : (Ch))
/* We don't use `isdigit' here since the locale dependent
   interpretation is not what we want here.  We only need to accept
   the arabic digits in the ASCII range.  One day there is perhaps a
   more reliable way to accept other sets of digits.  */
#define ISDIGIT(Ch) ((unsigned int) (Ch) - '0' <= 9)

static char *memcpy_lowcase (char *dest, const char *src, size_t len);

static char *memcpy_lowcase (dest, src, len)
    char *dest;
    const char *src;
    size_t len;
{
 while (len-- > 0)
   dest[len] = TOLOWER ((unsigned char) src[len], loc);
 return dest;
}

static char *memcpy_uppcase (char *dest, const char *src, size_t len);

static char *
memcpy_uppcase (dest, src, len)
    char *dest;
    const char *src;
    size_t len;
{
 while (len-- > 0)
   dest[len] = TOUPPER ((unsigned char) src[len], loc);
 return dest;
}


size_t mstrftime (char *s, size_t maxsize, const char *format, const struct tm *tp) {
  size_t i = 0;
  char *p = s;
  const char *f;

#  define f_wkday (_m_weekday_name[tp->tm_wday])
#  define f_month (_m_ab_month_name[tp->tm_mon])
#  define a_wkday (_m_weekday_name[tp->tm_wday])
#  define a_month (_m_ab_month_name[tp->tm_mon])

  size_t aw_len = 3;
  size_t am_len = 3;

  for (f = format; *f != '\0'; ++f)
    {
      int pad = 0;      /* Padding for number ('-', '_', or 0).  */
      int modifier;     /* Field modifier ('E', 'O', or 0).  */
      int digits;       /* Max digits for numeric format.  */
      int number_value;     /* Numeric value to be printed.  */
      int negative_number;  /* 1 if the number is negative.  */
      const char *subfmt;
      char *bufp;
      char buf[1 + (sizeof (int) < sizeof (time_t)
              ? INT_STRLEN_BOUND (time_t)
              : INT_STRLEN_BOUND (int))];
      int width = -1;
      int to_lowcase = 0;
      int to_uppcase = 0;
      int change_case = 0;
      int format_char;


      /* Either multibyte encodings are not supported, they are
     safe for formats, so any non-'%' byte can be copied through,
     or this is the wide character version.  */
      if (*f != '%')
    {
      add (1, *p = *f);
      continue;
    }

      /* Check for flags that can modify a format.  */
      while (1)
    {
      switch (*++f)
        {
          /* This influences the number formats.  */
        case '_':
        case '-':
        case '0':
          pad = *f;
          continue;

          /* This changes textual output.  */
        case '^':
          to_uppcase = 1;
          continue;
        case '#':
          change_case = 1;
          continue;

        default:
          break;
        }
      break;
    }

      /* As a GNU extension we allow to specify the field width.  */
      if (ISDIGIT (*f))
    {
      width = 0;
      do
        {
          if (width > INT_MAX / 10
          || (width == INT_MAX / 10 && *f - '0') > INT_MAX % 10)
        /* Avoid overflow.  */
        width = INT_MAX;
          else
        {
          width *= 10;
          width += *f - '0';
        }
          ++f;
        }
      while (ISDIGIT (*f));
    }

      /* Check for modifiers.  */
      switch (*f)
    {
    case 'E':
    case 'O':
      modifier = *f++;
      break;

    default:
      modifier = 0;
      break;
    }

      /* Now do the specified format.  */
      format_char = *f;
      switch (format_char)
    {
#define DO_NUMBER(d, v) \
      digits = d > width ? d : width;                     \
      number_value = v; goto do_number
#define DO_NUMBER_SPACEPAD(d, v) \
      digits = d > width ? d : width;                     \
      number_value = v; goto do_number_spacepad

    case '%':
      if (modifier != 0)
        goto bad_format;
      add (1, *p = *f);
      break;

    case 'a':
      if (modifier != 0)
        goto bad_format;
      if (change_case)
        {
          to_uppcase = 1;
          to_lowcase = 0;
        }
      cpy (aw_len, a_wkday);
      break;

    case 'A':
      if (modifier != 0)
        goto bad_format;
      if (change_case)
        {
          to_uppcase = 1;
          to_lowcase = 0;
        }
      cpy (strlen(f_wkday), f_wkday);
      break;

    case 'b':
    case 'h':
      if (change_case)
        {
          to_uppcase = 1;
          to_lowcase = 0;
        }
      if (modifier != 0)
        goto bad_format;
      cpy (am_len, a_month);
      break;

    case 'B':
      if (modifier != 0)
        goto bad_format;
      if (change_case)
        {
          to_uppcase = 1;
          to_lowcase = 0;
        }
      cpy (strlen(f_month), f_month);
      break;

    case 'c':
      if (modifier == 'O')
        goto bad_format;
      subfmt = "%a %b %e %H:%M:%S %Y";

    subformat:
      {
        char *old_start = p;
        size_t len = mstrftime (NULL, (size_t) -1, subfmt,
                          tp);
        add (len, mstrftime (p, maxsize - i, subfmt,
                       tp));

        if (to_uppcase)
          while (old_start < p)
        {
          *old_start = TOUPPER ((unsigned char) *old_start, loc);
          ++old_start;
        }
      }
      break;

    case 'x':
      if (modifier == 'O')
        goto bad_format;
      /* Fall through.  */
    case 'D':
      if (modifier != 0)
        goto bad_format;
      subfmt = "%m/%d/%y";
      goto subformat;

    case 'd':
      if (modifier == 'E')
        goto bad_format;

      DO_NUMBER (2, tp->tm_mday);

    case 'e':
      if (modifier == 'E')
        goto bad_format;

      DO_NUMBER_SPACEPAD (2, tp->tm_mday);

      /* All numeric formats set DIGITS and NUMBER_VALUE and then
         jump to one of these two labels.  */

    do_number_spacepad:
      /* Force `_' flag unless overwritten by `0' or '-' flag.  */
      if (((char)pad != '0') && ((char)pad != '-'))
        pad = '_';

    do_number:
      /* Format the number according to the MODIFIER flag.  */

      {
        unsigned int u = number_value;

        bufp = buf + sizeof (buf) / sizeof (buf[0]);
        negative_number = number_value < 0;

        if (negative_number)
          u = -u;

        do
          *--bufp = u % 10 + '0';
        while ((u /= 10) != 0);
      }

    do_number_sign_and_padding:
      if (negative_number)
        *--bufp = '-';

      if (pad != '-')
        {
          int padding = digits - (buf + (sizeof (buf) / sizeof (buf[0]))
                      - bufp);

          if (padding > 0)
        {
          if (pad == '_')
            {
              if ((size_t) padding >= maxsize - i)
            return 0;

              if (p)
            memset_space (p, padding);
              i += padding;
              width = width > padding ? width - padding : 0;
            }
          else
            {
              if ((size_t) digits >= maxsize - i)
            return 0;

              if (negative_number)
            {
              ++bufp;

              if (p)
                *p++ = '-';
              ++i;
            }

              if (p)
            memset_zero (p, padding);
              i += padding;
              width = 0;
            }
        }
        }

      cpy (buf + sizeof (buf) / sizeof (buf[0]) - bufp, bufp);
      break;

    case 'F':
      if (modifier != 0)
        goto bad_format;
      subfmt = "%Y-%m-%d";
      goto subformat;

    case 'H':
      if (modifier == 'E')
        goto bad_format;

      if(tp->tm_hour == 24) {
        add(1, *p = '-');
        add(1, *p = '-');
        break;
      } else
        DO_NUMBER (2, tp->tm_hour);

    case 'k':       /* GNU extension.  */
      if (modifier == 'E')
        goto bad_format;

      DO_NUMBER_SPACEPAD (2, tp->tm_hour);

    case 'j':
      if (modifier == 'E')
        goto bad_format;

      DO_NUMBER (3, 1 + tp->tm_yday);

    case 'M':
      if (modifier == 'E')
        goto bad_format;

      if(tp->tm_hour == 24) {
        add(1, *p = '-');
        add(1, *p = '-');
        break;
      } else
        DO_NUMBER (2, tp->tm_min);

    case 'm':
      if (modifier == 'E')
        goto bad_format;

      DO_NUMBER (2, tp->tm_mon + 1);

    case 'n':
      add (1, *p = '\n');
      break;

    case 'P':
      to_lowcase = 1;
      /* FALLTHROUGH */

    case 'R':
      subfmt = "%H:%M";
      goto subformat;

    case 'r':
      subfmt = "%H:%M:%S";
      goto subformat;

    case 'S':
      if (modifier == 'E')
        goto bad_format;

      if(tp->tm_hour == 24) {
        add(1, *p = '-');
        add(1, *p = '-');
        break;
      } else
        DO_NUMBER (2, tp->tm_sec);

    case 's':       /* GNU extension.  */
      {
        struct tm ltm;
        time_t t;

        ltm = *tp;
        t = mktime (&ltm);

        /* Generate string value for T using time_t arithmetic;
           this works even if sizeof (long) < sizeof (time_t).  */

        bufp = buf + sizeof (buf) / sizeof (buf[0]);
        negative_number = t < 0;

        do
          {
        int d = t % 10;
        t /= 10;

        if (negative_number)
          {
            d = -d;

            /* Adjust if division truncates to minus infinity.  */
            if (0 < -1 % 10 && d < 0)
              {
            t++;
            d += 10;
              }
          }

        *--bufp = d + '0';
          }
        while (t != 0);

        digits = 1;
        goto do_number_sign_and_padding;
      }

    case 'X':
      if (modifier == 'O')
        goto bad_format;
      /* Fall through.  */
    case 'T':
      subfmt = "%H:%M:%S";
      goto subformat;

    case 't':
      add (1, *p = '\t');
      break;

    case 'u':
      DO_NUMBER (1, (tp->tm_wday - 1 + 7) % 7 + 1);

    case 'U':
      if (modifier == 'E')
        goto bad_format;

      DO_NUMBER (2, (tp->tm_yday - tp->tm_wday + 7) / 7);

    case 'W':
      if (modifier == 'E')
        goto bad_format;

      DO_NUMBER (2, (tp->tm_yday - (tp->tm_wday - 1 + 7) % 7 + 7) / 7);

    case 'w':
      if (modifier == 'E')
        goto bad_format;

      DO_NUMBER (1, tp->tm_wday);

    case 'y':
      if (modifier == 'O')
        goto bad_format;
      else
        DO_NUMBER (2, (tp->tm_year % 100 + 100) % 100);
    case 'Y':
      if (modifier == 'O')
        goto bad_format;
      else
        DO_NUMBER (1, tp->tm_year);

    case '\0':      /* GNU extension: % at end of format.  */
        --f;
        /* Fall through.  */
    default:
      /* Unknown format; output the format, including the '%',
         since this is most likely the right thing to do if a
         multibyte string has been misparsed.  */
    bad_format:
      {
        int flen;
        for (flen = 1; f[1 - flen] != '%'; flen++)
          continue;
        cpy (flen, &f[1 - flen]);
      }
      break;
    }
    }

  if (p && maxsize != 0)
    *p = '\0';
  return i;
}
