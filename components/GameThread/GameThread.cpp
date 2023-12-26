#include <thread>
#include <iostream>
#include <algorithm>

#include "GameThread.h"
#include "SFML/Graphics.hpp"

using namespace std::chrono_literals;

namespace GameEngine
{
    GameThread::GameThread(std::shared_ptr<sf::RenderWindow> window) :
        m_window(std::move(window))
    {
        m_logicFunction = std::make_shared<LogicFunctions>(std::shared_ptr<GameThread>(this));
        InitializeState();
        ClearScreen();

        for(auto& obj : m_objects)
            m_window->draw(obj.GetSprite());

        m_window->display();

        m_lastFrameTime = std::chrono::steady_clock::now();
    }

    std::shared_ptr<sf::RenderWindow> GameThread::GetRenderWindow()
    {
        return m_window;
    }

    std::vector<GameUtils::Object>& GameThread::GetObjects()
    {
        return m_objects;
    }

    std::unordered_map<sf::Keyboard::Scancode, std::shared_ptr<GameUtils::Input>>& GameThread::GetKeys()
    {
        return m_keyMaps;
    }

    void GameThread::InitializeState()
    {
        for(auto& key : GameUtils::Keyboard_Keys)
            m_keyMaps[key] = std::make_shared<GameUtils::Input>(key);

        m_objects.emplace_back("0",GameUtils::ObjectType::PLAYER, "../resources/texture/ship.png", 
            std::bind(&LogicFunctions::PlayerStartup, m_logicFunction, std::placeholders::_1),
            std::bind(&LogicFunctions::PlayerLogic, m_logicFunction, std::placeholders::_1));

        for(auto index = 1; index <= 4; ++index)
        {
            m_objects.emplace_back(std::to_string(index), GameUtils::ObjectType::ENEMY, "../resources/texture/enemy-ship.png",
                std::bind(&LogicFunctions::EnemyStartup, m_logicFunction, std::placeholders::_1),
                std::bind(&LogicFunctions::EnemyLogic, m_logicFunction, std::placeholders::_1));
        }
    }

    void GameThread::DrawSprites()
    {
        for(auto& sprite : m_objects)
            m_window->draw(sprite.GetSprite());
    }

    void GameThread::CaptureKeyInput()
    {
        if(m_window->pollEvent(m_keyboardEvent))
        {
            if(m_keyboardEvent.type == sf::Event::Closed)
                m_window->close();

            switch(m_keyboardEvent.type)
            {
                case sf::Event::EventType::KeyPressed:
                        if(m_keyMaps[m_keyboardEvent.key.scancode] != nullptr)
                            m_keyMaps[m_keyboardEvent.key.scancode]->SetPressed(true);
                        break;
                case sf::Event::EventType::KeyReleased:
                        if(m_keyMaps[m_keyboardEvent.key.scancode] != nullptr)
                            m_keyMaps[m_keyboardEvent.key.scancode]->SetPressed(false);
                        break;       
            }       
        }
    }

    void GameThread::ExecuteLogic()
    {
        for(auto index = 0; index < m_objects.size(); ++index)
            m_objects[index].StepLogic();
    }

    void GameThread::ClearScreen()
    {
        m_window->clear(sf::Color::Black);
        backgroundTexture = std::make_shared<sf::Texture>();
        backgroundTexture->loadFromFile("../resources/texture/background.png");
        backgroundSprite.setTexture(*backgroundTexture);
        m_window->draw(backgroundSprite);
    }

    void GameThread::GameWatcherThread()
    {
        while(m_window->isOpen())
        {

            if(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_lastFrameTime) >= 16ms)
            {
                ClearScreen();
                CaptureKeyInput();
                ExecuteLogic();
                DrawSprites();
                m_window->display();
                m_lastFrameTime = std::chrono::steady_clock::now();
            }

        }
    }
}
