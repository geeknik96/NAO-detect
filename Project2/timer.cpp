//#include "stdafx.h"
#include "Timer.h"

#if defined(_MSC_VER)
#include "windows.h"
#endif

Timer::Timer(int n)
{
#if defined(_MSC_VER)
    this->counters = new  unsigned __int64[n];
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
#endif
}

void Timer::Start(int counter) {
#if defined(_MSC_VER)
    QueryPerformanceCounter((LARGE_INTEGER*)&this->counters[counter]);
#endif
}

float Timer::Get(int counter) {
#if defined(_MSC_VER)
    unsigned __int64 end;
    QueryPerformanceCounter((LARGE_INTEGER*)&end);
    return (float(end - this->counters[counter]) / freq);
#else
    return 0;
#endif
}

Timer::~Timer(void)
{
#if defined(_MSC_VER)
    delete [] this->counters;
#endif
}