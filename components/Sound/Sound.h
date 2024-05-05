#pragma once

#include <array>
#include <string>

namespace GameUtils
{
    const std::array<std::string,5> soundFiles = {"enemy-death.wav", "enemy-shot.wav", "player-shot.wav", "win.wav", "lose.wav"};

    
    enum class SoundName : int
    {
        ENEMY_DEATH = 0,
        ENEMY_SHOT,
        PLAYER_SHOT,
        WIN,
        LOSE
    };

    static std::string SoundNameToString(const SoundName& name)
    {
        switch(name)
        {
            case SoundName::ENEMY_DEATH: return "enemy-death.wav";
            case SoundName::ENEMY_SHOT: return "enemy-shot.wav";
            case SoundName::PLAYER_SHOT: return "player-shot.wav";
            case SoundName::WIN: return "win.wav";
            case SoundName::LOSE: return "lose.wav";
            default: return "unknown";
        }
    }
}