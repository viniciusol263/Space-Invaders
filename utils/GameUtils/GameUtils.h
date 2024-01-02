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
    enum class TextType : int
    {
        SCORE = 0,
        PAUSE,
        GAME_OVER,
        HIGH_SCORE
    }; 

    enum class ObjectType : int
    {
        PLAYER = 0,
        PROJECTILE,
        ENEMY,
        ENEMY_PROJECTILE,
        BOSS
    };

    static std::string ObjectTypeToString(const ObjectType& type) 
    {
        switch(type)
        {
            case ObjectType::PLAYER: return "Player";
            case ObjectType::PROJECTILE: return "Projectile";
            case ObjectType::ENEMY: return "Enemy";
            case ObjectType::ENEMY_PROJECTILE: return "Enemy Projectile";
            case ObjectType::BOSS: return "Boss";
            default: return "";
        }
    }

    static std::pair<int,int> TextureSizeFromObjectType(const ObjectType& type)
    {
        switch(type)
        {
            case ObjectType::BOSS: return std::make_pair(128,128);
            default: return std::make_pair(32,32);
        }
    }

    constexpr std::array<sf::Keyboard::Scancode,4> Keyboard_Keys = {
        sf::Keyboard::Scancode::A, 
        sf::Keyboard::Scancode::D, 
        sf::Keyboard::Scancode::Space,
        sf::Keyboard::Scancode::P
    };

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

    enum class Progression : int
    {
      NORMAL_GAME = 0,
      BOSS_PHASE,
      RESPAWN,
      GAME_OVER  
    };


    class Object 
    {
    public:
        Object(const std::string& id = "UNKNOWN", const ObjectType& objType = ObjectType::PLAYER, const std::string& texturePath = "", const std::string& soundPath = "", const std::function<void(GameUtils::Object&)>& startupHandler = [](GameUtils::Object&){}, const std::function<void(GameUtils::Object&)>& logicHandler = [](GameUtils::Object&){}, const std::chrono::milliseconds& animationFrametime = 166ms, const int& hitPoints = 1) : 
            m_id(id), m_objType(objType), m_soundBufferPath(soundPath), m_startupHandler(startupHandler), m_logicHandler(logicHandler), m_animationFrametime(animationFrametime), m_hitPoints(hitPoints)
        {
            if(m_id != "UNKNOWN")
            {             
                m_objTexture = std::make_shared<sf::Texture>();
                m_objTexture->loadFromFile(texturePath);
                m_objSprite.setTexture(*m_objTexture);
                auto [textureWidth, textureHeigth] = TextureSizeFromObjectType(objType);
                m_objSprite.setTextureRect(sf::IntRect{sf::Vector2i{0,0}, sf::Vector2i{textureWidth,textureHeigth}});
                
                if(soundPath != "")
                {
                    m_objSoundBuffer = std::make_shared<sf::SoundBuffer>();
                    m_objSoundBuffer->loadFromFile(soundPath);
                    m_objSound.setBuffer(*m_objSoundBuffer);
                }

                m_startupHandler(*this);
            }
        }

        ~Object() = default;

        constexpr bool operator==(const Object& other) const 
        {
            return (m_id == other.m_id) &&
                    (m_objType == other.m_objType);
        }

        std::string GetId() const
        {
            return m_id;
        }

        sf::Sprite& GetSprite() 
        {
            return m_objSprite;
        }

        std::string GetDefaultSoundFilePath()
        {
            return m_soundBufferPath;
        }

        sf::Sound& GetSound()
        {
            return m_objSound;
        }

        ObjectType GetType() const
        {
            return m_objType;
        }

        std::chrono::milliseconds GetAnimationFrametime() const
        {
            return m_animationFrametime;
        }

        void SetTimer(const std::chrono::milliseconds& time, const bool& continous)
        {
            if(time == 0ms)
            {
                m_timer = nullptr;
                return;
            }
            m_timer = std::make_shared<std::tuple<std::chrono::milliseconds, std::chrono::steady_clock::time_point, bool>>(time, std::chrono::steady_clock::now(), continous);
        }

        bool TimerOverflown() 
        {
            if(m_timer == nullptr) 
            {
                return false;
            }

            auto [time, timer, continous] = *m_timer;
            if(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - timer) >= time)
            {
                if(continous)
                    m_timer = std::make_shared<std::tuple<std::chrono::milliseconds, std::chrono::steady_clock::time_point, bool>>(time, std::chrono::steady_clock::now(), continous);
                return true;
            }
            return false;
        }

        int GetHitPoints() const
        {
            return m_hitPoints;
        }

        void SetHitPoints(const int& value)
        {
            m_hitPoints = value;
        }

        bool GetAnimRunning()
        {
            return m_animRunning;
        }

        void SetAnimRunning(const bool& value)
        {
            m_animRunning = value;
        }

        void StepLogic()
        {
            m_logicHandler(*this);
        }

    private:
        std::string m_id;
        std::string m_soundBufferPath;
        ObjectType m_objType;
        std::function<void(GameUtils::Object&)> m_startupHandler;
        std::function<void(GameUtils::Object&)> m_logicHandler;
        std::shared_ptr<sf::Texture> m_objTexture;
        std::shared_ptr<sf::SoundBuffer> m_objSoundBuffer;
        sf::Sprite m_objSprite;
        sf::Sound m_objSound;
        std::chrono::milliseconds m_animationFrametime;
        std::shared_ptr<std::tuple<std::chrono::milliseconds, std::chrono::steady_clock::time_point, bool>> m_timer;
        int m_hitPoints;
        bool m_animRunning = false;
        
    };

    class Input
    {
    public:
        Input(const sf::Keyboard::Scancode& inputIdentifier) : m_inputIdentifier(inputIdentifier)
        {}

        void SetPressed(bool isPressed)
        {
            m_isPressed = isPressed;
        }

        bool GetPressed()
        {
            return m_isPressed;
        }

        friend std::ostream& operator<<(std::ostream& o, Input& input)
        {
            o << "Key " + sf::Keyboard::getDescription(input.m_inputIdentifier).toAnsiString() + " is " << ((input.m_isPressed) ? "pressed" : "not pressed") << std::endl; 
            return o;
        } 
    private:
        sf::Keyboard::Scancode m_inputIdentifier;
        bool m_isPressed = false;
    };


}
