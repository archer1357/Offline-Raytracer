#include "timer.h"

#ifndef __cplusplus
#include <stdbool.h>
#endif

#if defined(_MSC_VER)
#include <Winsock2.h>
#include < time.h >
#include <windows.h>
#else
#include <sys/time.h>

#endif

#include <stdint.h>


#if defined(_MSC_VER)

#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
// #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL //non msvc

struct timezone
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv,struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;

  if(NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tmpres /= 10;  /*convert into microseconds*/
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }

  if(NULL != tz)
  {
    if(!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }

  return 0;
}
#endif

uint64_t getRawTime() {
  struct timeval tv;

  if(0!=gettimeofday(&tv, 0)) {
    return 0;
  }

  return (uint64_t) tv.tv_sec * (uint64_t) 1000000 + (uint64_t) tv.tv_usec;
}

double timer() {
  static bool inited=false;
  static uint64_t start;
  uint64_t stop, dif;

  if(!inited) {
    start=getRawTime();
    inited=true;
  }

  stop=getRawTime();
  dif=stop-start;
  return dif/1000000.0;
}


// double timer() {
//   static bool inited=false;
//   static struct timeval start;
//   struct timeval stop, dif;

//   if(!inited) {
//     if(0!=gettimeofday(&start, 0)) {
//       return 0.0;
//     }

//     inited=true;
//   }

//   if(0!=gettimeofday(&stop, 0)) {
//     return 0.0;
//   }

//   timersub(&stop,&start,&dif);
//   uint64_t x=(uint64_t) dif.tv_sec * (uint64_t) 1000000 + (uint64_t) dif.tv_usec;
//   return x/1000000.0;
// }
