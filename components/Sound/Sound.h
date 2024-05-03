#pragma once

#include <array>
#include <string>

namespace GameUtils
{
    const std::array<std::string,3> soundFiles = {"enemy-death.wav", "enemy-shot.wav", "player-shot.wav"};

    
    enum class SoundName : int
    {
        ENEMY_DEATH = 0,
        ENEMY_SHOT,
        PLAYER_SHOT
    };

    static std::string SoundNameToString(const SoundName& name)
    {
        switch(name)
        {
            case SoundName::ENEMY_DEATH: return "enemy-death.wav";
            case SoundName::ENEMY_SHOT: return "enemy-shot.wav";
            case SoundName::PLAYER_SHOT: return "player-shot.wav";
            default: return "unknown";
        }
    }
}