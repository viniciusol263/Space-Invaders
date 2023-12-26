#pragma once

#include <functional>
#include <array>
#include <string>

#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

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

    class Object 
    {
    public:
        Object(std::string id = "UNKNOWN", ObjectType objType = ObjectType::PLAYER, std::string texturePath = "", std::function<void(GameUtils::Object&)> startupHandler = [](GameUtils::Object&){}, std::function<void(GameUtils::Object&)> logicHandler = [](GameUtils::Object&){}, std::function<void(GameUtils::Object&)> destructionHandler = [](GameUtils::Object&){}) : 
            m_id(id), m_objType(objType), m_startupHandler(startupHandler), m_logicHandler(logicHandler), m_destructionHandler(destructionHandler)
        {
            if(m_id != "UNKNOWN")
            {             
                m_objTexture = std::make_shared<sf::Texture>();
                m_objTexture->loadFromFile(texturePath);
                m_objSprite.setTexture(*m_objTexture);
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

    ObjectType GetType() const
    {
        return m_objType;
    }

    void StepLogic()
    {
        m_logicHandler(*this);
    }

    private:
        std::string m_id;
        ObjectType m_objType;
        std::function<void(GameUtils::Object&)> m_startupHandler;
        std::function<void(GameUtils::Object&)> m_logicHandler;
        std::function<void(GameUtils::Object&)> m_destructionHandler;
        std::shared_ptr<sf::Texture> m_objTexture;
        sf::Sprite m_objSprite;
        
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
