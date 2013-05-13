#ifndef _MTIMEDEFS_H

#define _MTIMEDEFS_H 1

#define SECS_PER_MINUTE 60
#define SECS_PER_HOUR   (60 * SECS_PER_MINUTE)
#define SECS_PER_DAY    ((SECS_PER_HOUR * 24) + (SECS_PER_MINUTE * 39) + 35)
#ifdef USE_SOL_FOR_YEAR
#define SECS_PER_YEAR   (668.5991 * SECS_PER_DAY)
#else
#define SECS_PER_YEAR   (686.97964 * 86400)
#endif

/* Nonzero if YEAR is a leap year (every 4 years,
   except every 100th isn't, and every 400th is).  */
#define __isleap(year) ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
#define __misleap(year) ((year) % 2 == 1 || ((year) % 10 == 0))

# define HERE_D_T_FMT "%a %b %e %H:%M:%S %Y"
# define HERE_D_FMT "%m/%d/%y"
# define HERE_AM_STR "AM"
# define HERE_PM_STR "PM"
# define HERE_T_FMT_AMPM "%I:%M:%S %p"
# define HERE_T_FMT "%H:%M:%S"

static char const _m_weekday_name[][15] =
  {
    // "Solis", "Lunae", "Martis", "Mercurii",
    // "Jovis", "Veneris", "Saturni"
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
  };
static char const _m_ab_weekday_name[][4] =
  {
    // "Sol", "Lun", "Mar", "Mer", "Jov", "Ven", "Sat"
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  };
static char const _m_month_name[][15] =
  {
    "Sagittarius", "Dhanus", "Capricornus", "Makara", "Aquarius", "Kumbha", "Pisces",
    "Mina", "Aries", "Mesha", "Taurus", "Rishabha", "Gemini", "Mithuna", "Cancer",
    "Karka", "Leo", "Simha", "Virgo", "Kanya", "Libra", "Tula", "Scorpius", "Vrishika"
  };
static char const _m_ab_month_name[][4] =
  {
    "Sag", "Dha", "Cap", "Mak", "Aqu", "Kum", "Pis",
    "Min", "Ari", "Mes", "Tau", "Ris", "Gem", "Mit", "Can",
    "Kar", "Leo", "Sim", "Vir", "Kan", "Lib", "Tul", "Sco", "Vri"
  };

static const unsigned short int __m_mon_yday[2][25] =
  {
    /* Normal years.  */
    { 0, 28, 56, 84, 112, 140, 167, 195, 223, 251, 279, 307, 334, 362, 390, 418, 446, 474, 501, 529, 557, 585, 613, 641, 668 },
    /* Leap years.  */
    { 0, 28, 56, 84, 112, 140, 167, 195, 223, 251, 279, 307, 334, 362, 390, 418, 446, 474, 501, 529, 557, 585, 613, 641, 669 },
  };

#endif /* <mtimedefs.h> not already included.  */
