#include <thread>
#include <iostream>
#include <algorithm>
#include <array>
#include <cmath>

#include "GameThread.h"
#include "SFML/Graphics.hpp"

using namespace std::chrono_literals;

namespace GameEngine
{
    GameThread::GameThread(const std::shared_ptr<sf::RenderWindow>& window) :
        m_window(std::move(window))
    {
        m_logicFunction = std::make_shared<LogicFunctions>(std::shared_ptr<GameThread>(this));
        InitializeState();
        ClearScreen();

        m_lastFrameTime = std::chrono::steady_clock::now();
    }

    GameThread::~GameThread()
    {
        for(auto index = 0; index < m_auxThreads.size(); ++index)
        {
            if(m_auxThreads[index] != nullptr) 
            {
                if(m_auxThreads[index]->joinable())
                    m_auxThreads[index]->join();
            }
        }
    }

    std::shared_ptr<sf::RenderWindow> GameThread::GetRenderWindow()
    {
        return m_window;
    }

    std::vector<GameUtils::Object>& GameThread::GetObjects()
    {
        return m_objects;
    }

    void GameThread::CreateObject(const std::string& id , const GameUtils::ObjectType& objType, 
            const std::string& texturePath, const std::string& soundPath,
            const std::function<void(GameUtils::Object&)>& startupHandler, const std::function<void(GameUtils::Object&)>& logicHandler, 
            const std::chrono::milliseconds& animationFrametime, const int& hitPoints)
    {
        m_objects.emplace_back(id, objType, texturePath, soundPath, startupHandler, logicHandler, animationFrametime, hitPoints);
    }

    std::unordered_map<sf::Keyboard::Scancode, std::shared_ptr<GameUtils::Input>>& GameThread::GetKeys()
    {
        return m_keyMaps;
    }

    int& GameThread::GetScore()
    {
        return m_score;
    }

    void GameThread::SetScore(const int& score)
    {
        m_score = score;
        if(m_score > m_highscore)
            m_highscore = m_score;
        m_textSprites[GameUtils::TextType::SCORE].setString("Score: " + std::to_string(m_score));
        m_textSprites[GameUtils::TextType::HIGH_SCORE].setString("Highscore: " + std::to_string(m_highscore));
    }

    void GameThread::InitializeState()
    {

        auto windowSize = m_window->getSize();

        m_progression = GameUtils::Progression::NORMAL_GAME;
        m_font.loadFromFile("../resources/fonts/PressStart2P-vaV7.ttf");
        m_textSprites[GameUtils::TextType::SCORE] = {"Score: " + std::to_string(m_score), m_font, 24};
        m_textSprites[GameUtils::TextType::HIGH_SCORE] = {"Highscore: " + std::to_string(m_highscore), m_font, 24};
        m_textSprites[GameUtils::TextType::HIGH_SCORE].setPosition({m_window->getDefaultView().getCenter().x - ((std::string("Highscore: ").size()/2)*m_textSprites[GameUtils::TextType::HIGH_SCORE].getCharacterSize()),m_textSprites[GameUtils::TextType::HIGH_SCORE].getPosition().y});

        GenerateSoundChannels();

        for(auto& key : GameUtils::Keyboard_Keys)
            m_keyMaps[key] = std::make_shared<GameUtils::Input>(key);

        //Putting Player ship on the rendering pipeline
        CreateObject("0",GameUtils::ObjectType::PLAYER, "../resources/texture/multi-anim-ship.png", "",
            std::bind(&LogicFunctions::PlayerStartup, m_logicFunction, std::placeholders::_1, sf::Vector2i{(int)(windowSize.x/2), (int)(windowSize.y * 0.9)}),
            std::bind(&LogicFunctions::PlayerLogic, m_logicFunction, std::placeholders::_1), 100ms);


        //Putting array of Enemy ships in the rendering pipeline
        CreateArrayObject(1, 1, 
            [this](sf::Vector2i vecPos, std::string id) 
            {
                return GameUtils::Object(id, GameUtils::ObjectType::ENEMY, "../resources/texture/animated-enemy-ship.png", "",
                        std::bind(&LogicFunctions::EnemyStartup, m_logicFunction, std::placeholders::_1, vecPos),
                        std::bind(&LogicFunctions::EnemyLogic, m_logicFunction, std::placeholders::_1), 332ms);
            }
        );

    }

    void GameThread::DrawSprites()
    {
        for(auto& sprite : m_objects)
            m_window->draw(sprite.GetSprite());
        for(auto& [_, text] : m_textSprites)
            m_window->draw(text);
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

    void GameThread::PauseLogic()
    {
        if(m_textSprites[GameUtils::TextType::PAUSE].getString() == "GAME OVER")
        {
            for(auto& [_, key] : m_keyMaps)
            {
                if(key->GetPressed())
                {
                    m_score = 0;
                    CleanupGame();
                    InitializeState();
                    break;
                }
            }
        }
        else if((m_paused == 0 || m_paused == 2) && m_keyMaps[sf::Keyboard::Scancode::P]->GetPressed())
        {
            m_paused = (m_paused + 1) % 4;
            m_textSprites.erase(GameUtils::TextType::PAUSE);
        }
        else if((m_paused == 1 || m_paused == 3) && !m_keyMaps[sf::Keyboard::Scancode::P]->GetPressed())
        {
            m_paused = (m_paused + 1) % 4;
            if(m_paused == 2)
            {
                BlockingTextScreen("PAUSED");
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

    void GameThread::ProgressionCheck()
    {   
        auto [enemySize, playerLive, bossLive] = GetLiveObjects();

        if(enemySize == 0)
        {
            if(m_progression == GameUtils::Progression::BOSS_PHASE && bossLive == 0)
            {
                m_progression = GameUtils::Progression::RESPAWN;
                CleanupGame();
                InitializeState();
            }

            else if(m_progression == GameUtils::Progression::NORMAL_GAME)
            {
                m_progression = GameUtils::Progression::BOSS_PHASE;
                CreateObject("1", GameUtils::ObjectType::BOSS, "../resources/texture/animated-boss-ship.png", "", 
                    std::bind(&LogicFunctions::BossStartup, m_logicFunction, std::placeholders::_1, sf::Vector2i{(int)m_window->getDefaultView().getCenter().x, 0}),
                    std::bind(&LogicFunctions::BossLogic, m_logicFunction, std::placeholders::_1), 326ms, 5);
            }

        }
        if(playerLive == 0)
        {
            m_progression = GameUtils::Progression::GAME_OVER;
            BlockingTextScreen("GAME OVER");
            m_paused = 2;
        }
    }

    void GameThread::CleanupPointers()
    {
        if(m_auxThreads.size() == 2)
        {
            for(auto index = 0; index < m_auxThreads.size(); ++index)
            {
                if(m_auxThreads[index] != nullptr) 
                {
                    if(m_auxThreads[index]->joinable())
                        m_auxThreads[index]->join();
                    std::remove_if(m_auxThreads.begin(), m_auxThreads.end(), [this, index](const std::shared_ptr<std::thread>& thread) 
                    -> std::shared_ptr<std::thread>
                    {
                        if(thread.get() == m_auxThreads[index].get())
                            return thread;
                        return nullptr;
                    });
                }
            }
        }
    }

    void GameThread::CleanupGame()
    {
        m_paused = 0;
        for(auto index = 0; index < m_auxThreads.size(); ++index)
        {
            if(m_auxThreads[index] != nullptr) 
            {
                m_auxThreads[index]->join();
            }
        }
        m_auxThreads.clear();
        m_objects.clear();
        m_textSprites.clear();
        m_keyMaps.clear();
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

    void GameThread::DoAnimatedAction(GameUtils::Object& obj, const int& textureRow, const bool& isLoop, const std::function<void()>& actionFunc)
    {
        m_auxThreads.push_back(std::make_shared<std::thread>([this, &obj, isLoop, actionFunc, textureRow]()
        {
            auto& objSprite = obj.GetSprite();
            auto currentRenderRect = objSprite.getTextureRect();
            auto textureSize = objSprite.getTexture()->getSize();
            auto renderRectSize = currentRenderRect.getSize();
            auto frameQuantity = textureSize.x/renderRectSize.x;

            for(auto index = 0; index < frameQuantity; ++index)
            {
                auto startTime = std::chrono::steady_clock::now();
                objSprite.setTextureRect(sf::IntRect{sf::Vector2i{(int)(index * renderRectSize.x), textureRow * renderRectSize.y}, currentRenderRect.getSize()});
                while(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - startTime) <= (obj.GetAnimationFrametime()/(int)frameQuantity));
            }
            if(isLoop)
                objSprite.setTextureRect(currentRenderRect);
            actionFunc();
        }));
    }

    void GameThread::PlayAudioChannel(const GameUtils::SoundName& soundName)
    {
        m_generalSoundChannels[GameUtils::SoundNameToString(soundName)].second.play();
    }

    void GameThread::CreateArrayObject(const int& rows, const int& columns, const std::function<GameUtils::Object(sf::Vector2i, std::string)>& objectBuilder)
    {
        auto windowSize = m_window->getDefaultView().getSize();
        auto additiveRatio = std::floor(windowSize.x / columns);

        for(auto row = 0; row < rows; ++row)
        {
            for(auto column = 0; column < columns; ++column)
            {     
                m_objects.push_back(objectBuilder(sf::Vector2i{
                    (int)(std::floor(additiveRatio/2) + (row * additiveRatio)),  // X
                    (int)(windowSize.y * (0.1*(column + 1)))}, // Y
                    std::to_string(row + column*rows))   
                );
            }
        }
    }

    void GameThread::BlockingTextScreen(const std::string& text)
    {
        m_textSprites[GameUtils::TextType::PAUSE] = {text, m_font, 60};
        auto textSize = m_textSprites[GameUtils::TextType::PAUSE].getCharacterSize();
        auto textLength = m_textSprites[GameUtils::TextType::PAUSE].getString().getSize();
        m_textSprites[GameUtils::TextType::PAUSE].setFillColor(sf::Color::White);
        m_textSprites[GameUtils::TextType::PAUSE].setPosition({(m_window->getDefaultView().getSize().x/2.0f) - (textLength/2)*textSize, (m_window->getDefaultView().getSize().y/2.0f) - textSize});
    }

    std::tuple<int,int,int> GameThread::GetLiveObjects()
    {
        auto quantity = 0, playerExists = 0, bossExists = 0;
        for(auto obj : m_objects)
        {
            switch(obj.GetType())
            {
                case GameUtils::ObjectType::ENEMY:
                    ++quantity;
                    break;
                case GameUtils::ObjectType::PLAYER:
                    ++playerExists;
                    break;
                case GameUtils::ObjectType::BOSS:
                    ++bossExists;
                    break;
            }
        }
        return std::make_tuple(quantity, playerExists, bossExists);
    }

    void GameThread::GameWatcherThread()
    {
        while(m_window->isOpen())
        {

            if(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_lastFrameTime) >= 16ms)
            {
                ProgressionCheck();
                ClearScreen();
                CaptureKeyInput();
                PauseLogic();
                if(m_paused != 2)
                {
                    ExecuteLogic();
                }
                DrawSprites();
                m_window->display(); 
                m_lastFrameTime = std::chrono::steady_clock::now();
                
            }
            CleanupPointers();

        }
    }


}
