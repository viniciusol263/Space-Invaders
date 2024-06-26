#pragma once

#include <array>
#include <mutex>
#include "SFML/Window.hpp"

namespace GameUtils
{   
    constexpr std::array<sf::Keyboard::Scancode,5> Keyboard_Keys = {
        sf::Keyboard::Scancode::A, 
        sf::Keyboard::Scancode::D, 
        sf::Keyboard::Scancode::Space,
        sf::Keyboard::Scancode::P,
        sf::Keyboard::Scancode::Enter
    };

    class Input
    {
    public:
        Input(const sf::Keyboard::Scancode& inputIdentifier) : m_inputIdentifier(inputIdentifier)
        {}

        friend std::ostream& operator<<(std::ostream& o, Input& input)
        {
            o << "Key " + sf::Keyboard::getDescription(input.m_inputIdentifier).toAnsiString() + " is " << ((input.m_isPressed) ? "pressed" : "not pressed") << std::endl; 
            return o;
        } 

        void SetPressed(bool isPressed);
        bool GetPressed();
    private:
        sf::Keyboard::Scancode m_inputIdentifier;
        bool m_isPressed = false;
    };
}