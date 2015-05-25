#pragma once

class Timer
{
private:
#if defined(_MSC_VER)
    unsigned __int64* counters;
    unsigned __int64 freq;
#endif
public:
    Timer(int = 1);
    void Start(int = 0);
    float Get(int = 0);
public:
    ~Timer(void);
};
