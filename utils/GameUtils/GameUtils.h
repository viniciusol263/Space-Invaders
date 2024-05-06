#pragma once

#include <functional>
#include <array>
#include <string>
#include <future>
#include <iostream>
#include <utility>

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"

using namespace std::chrono_literals;

namespace GameUtils
{
    constexpr std::array<int,2> enemyQuantity = {4,4};
    constexpr auto globalFrametime = 16ms;
    enum class TextType : int
    {
        SCORE = 0,
        PAUSE,
        GAME_OVER,
        HIGH_SCORE
    }; 
    
    enum class Progression : int
    {
      NORMAL_GAME = 0,
      BOSS_PHASE,
      RESPAWN,
      GAME_OVER  
    };
    
    bool IsExpired(const std::chrono::time_point<std::chrono::steady_clock>& auxiliarTimestamp, const int& ticks);
  
}
