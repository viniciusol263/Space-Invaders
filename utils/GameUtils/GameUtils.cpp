#include "GameUtils.h"

namespace GameUtils
{
    bool IsExpired(const std::chrono::time_point<std::chrono::steady_clock>& auxiliarTimestamp, const int& ticks) 
    {
      auto test = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - auxiliarTimestamp);
      auto result = (test >= (globalFrametime * ticks)) ? true : false;
      return result;
    }
}