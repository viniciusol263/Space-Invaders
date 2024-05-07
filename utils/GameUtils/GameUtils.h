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
    constexpr uint32_t red = 0xAC3232FF;
    constexpr uint32_t darkerRed = 0x822929FF;
    constexpr std::array<int,2> enemyQuantity = {4,4};
    constexpr auto globalFrametime = 16.67ms;
    enum class TextType : int
    {
        SCORE = 0,
        PAUSE,
        GAME_OVER,
        HIGH_SCORE,
        MENU_START,
        MENU_TITLE
    }; 
    
    enum class Progression : int
    {
      NORMAL_GAME = 0,
      BOSS_PHASE,
      RESPAWN,
      GAME_OVER,
      MENU
    };
    
    bool IsExpired(const std::chrono::time_point<std::chrono::steady_clock>& auxiliarTimestamp, const int& ticks);
  
}
