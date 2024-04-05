#ifndef COMMON_H
#define COMMON_H

#include <functional>

struct Task
{
    std::function<void()> func;
    long long idx;
};

#endif // COMMON_H
