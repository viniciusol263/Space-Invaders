#pragma once

#include <functional>
#include <array>
#include <string>
#include <future>
#include <iostream>

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"

using namespace std::chrono_literals;

namespace GameUtils
{
    enum class ObjectType : int
    {
        PLAYER = 0,
        PROJECTILE,
        ENEMY
    };

    static std::string ObjectTypeToString(const ObjectType& type) 
    {
        switch(type)
        {
            case ObjectType::PLAYER: return "Player";
            case ObjectType::PROJECTILE: return "Projectile";
            case ObjectType::ENEMY: return "Enemy";
            default: return "";
        }
    }

    constexpr std::array<sf::Keyboard::Scancode,3> Keyboard_Keys = {
        sf::Keyboard::Scancode::A, 
        sf::Keyboard::Scancode::D, 
        sf::Keyboard::Scancode::Space
    };

    constexpr std::array<std::string,3> soundFiles = {"enemy-death.wav", "enemy-shot.wav", "player-shot.wav"};


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


    class Object 
    {
    public:
        Object(std::string id = "UNKNOWN", ObjectType objType = ObjectType::PLAYER, std::string texturePath = "", std::string soundPath = "", std::function<void(GameUtils::Object&)> startupHandler = [](GameUtils::Object&){}, std::function<void(GameUtils::Object&)> logicHandler = [](GameUtils::Object&){}, std::chrono::milliseconds animationFrametime = 166ms) : 
            m_id(id), m_objType(objType), m_soundBufferPath(soundPath), m_startupHandler(startupHandler), m_logicHandler(logicHandler), m_animationFrametime(animationFrametime)
        {
            if(m_id != "UNKNOWN")
            {             
                m_objTexture = std::make_shared<sf::Texture>();
                m_objTexture->loadFromFile(texturePath);
                m_objSprite.setTexture(*m_objTexture);
                m_objSprite.setTextureRect(sf::IntRect{sf::Vector2i{0,0}, sf::Vector2i{32,32}});
                
                if(soundPath != "")
                {
                    m_objSoundBuffer = std::make_shared<sf::SoundBuffer>();
                    m_objSoundBuffer->loadFromFile(soundPath);
                    m_objSound.setBuffer(*m_objSoundBuffer);
                }

                m_startupHandler(*this);
            }
        }

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
        
    };

    class Input
    {
    public:
        Input(sf::Keyboard::Scancode inputIdentifier) : m_inputIdentifier(inputIdentifier)
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
