#include <thread>
#include <iostream>
#include <algorithm>
#include <array>

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

        auto windowSize = m_window->getSize();

        GenerateSoundChannels();

        for(auto& key : GameUtils::Keyboard_Keys)
            m_keyMaps[key] = std::make_shared<GameUtils::Input>(key);

        //Putting Player ship on the rendering pipeline
        m_objects.emplace_back("0",GameUtils::ObjectType::PLAYER, "../resources/texture/animated-ship.png", "",
            std::bind(&LogicFunctions::PlayerStartup, m_logicFunction, std::placeholders::_1, sf::Vector2i{(int)(windowSize.x/2), (int)(windowSize.y * 0.9)}),
            std::bind(&LogicFunctions::PlayerLogic, m_logicFunction, std::placeholders::_1), 166ms);


        //Putting array of Enemy ships in the rendering pipeline
        CreateArrayObject(4, 4, 
            [this](sf::Vector2i vecPos, std::string id) 
            {
                return GameUtils::Object(id, GameUtils::ObjectType::ENEMY, "../resources/texture/animated-enemy-ship.png", "",
                        std::bind(&LogicFunctions::EnemyStartup, m_logicFunction, std::placeholders::_1, vecPos),
                        std::bind(&LogicFunctions::EnemyLogic, m_logicFunction, std::placeholders::_1), 83ms);
            }
        );

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

    void GameThread::GenerateSoundChannels()
    {    
        for(auto soundFile : GameUtils::soundFiles)
        {
            auto soundBuffer = std::make_shared<sf::SoundBuffer>();
            soundBuffer->loadFromFile("../resources/sfx/" + soundFile);
            m_generalSoundChannels[soundFile] = std::make_pair(soundBuffer,sf::Sound());
            m_generalSoundChannels[soundFile].second.setBuffer(*m_generalSoundChannels[soundFile].first);
        }
    }

    void GameThread::DoSpriteAnimation(GameUtils::Object& obj, sf::Vector2i newSpritePos)
    {
        if(m_auxThread != nullptr) m_auxThread->get();
        m_auxThread = std::make_shared<std::future<void>>(std::async(std::launch::async, [this, &obj, newSpritePos]()
        {
            auto& objSprite = obj.GetSprite();
            auto currentRenderRect = objSprite.getTextureRect();
            auto startTime = std::chrono::steady_clock::now();
            objSprite.setTextureRect(sf::IntRect{newSpritePos, currentRenderRect.getSize()});
            while(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - startTime) <= obj.GetAnimationFrametime());
            objSprite.setTextureRect(currentRenderRect);
        }));
    }

    void GameThread::PlayAudioChannel(GameUtils::SoundName soundName)
    {
        m_generalSoundChannels[GameUtils::SoundNameToString(soundName)].second.play();
    }

    void GameThread::CreateArrayObject(int rows, int columns, std::function<GameUtils::Object(sf::Vector2i, std::string)> objectBuilder)
    {
        auto windowSize = m_window->getSize();
        for(auto row = 0; row < rows; ++row)
        {
            for(auto column = 0; column < columns; ++column)
            {     
                m_objects.push_back(objectBuilder(sf::Vector2i{
                    (int)(windowSize.x * (0.20 * (row + 1))),  // X
                    (int)(windowSize.y * (0.1*(column + 1)))}, // Y
                    std::to_string(row + column*columns))   
                );
            }
        }
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
