#include "GameUtils.h"

namespace GameUtils
{
    bool IsExpired(const std::chrono::time_point<std::chrono::steady_clock>& auxiliarTimestamp, const int& ticks) 
    {
      return std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - auxiliarTimestamp) >= (globalFrametime * ticks * 0.75);
    }
}