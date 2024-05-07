#include <thread>
#include <iostream>
#include <algorithm>
#include <array>
#include <cmath>

#include "GameThread.h"
#include "GameUtils/GameUtils.h"
#include "SFML/Graphics.hpp"

using namespace std::chrono_literals;

namespace GameEngine
{
    GameThread::GameThread(const std::shared_ptr<sf::RenderWindow>& window) :
        m_window(std::move(window))
    {
        m_logicFunction = std::make_shared<LogicFunctions>(std::shared_ptr<GameThread>(this));
        InitializeState();
        MenuScreen(); 
        RenderText();
        RenderStage();
        ClearScreen();

        m_lastFrameTime = std::chrono::steady_clock::now();
    }

    GameThread::~GameThread()
    {        
        for(auto soundFile : GameUtils::soundFiles)
        {
            m_generalSoundChannels[soundFile].second.stop();
            while(m_generalSoundChannels[soundFile].second.getStatus() != sf::Sound::Stopped);
            m_generalSoundChannels[soundFile].first.reset();
        }
        m_generalSoundChannels.clear();
    }

    std::shared_ptr<sf::RenderWindow> GameThread::GetRenderWindow()
    {
        return m_window;
    }

    std::vector<GameUtils::Object>& GameThread::GetObjects()
    {
        return m_objects;
    }

    GameUtils::Object& GameThread::CreateObject(const std::string& id , const GameUtils::ObjectType& objType, 
            const std::string& texturePath, const std::string& soundPath,
            const std::function<void(GameUtils::Object&)>& startupHandler, const std::function<void(GameUtils::Object&)>& logicHandler, 
            const std::chrono::milliseconds& animationFrametime, const int& hitPoints, const int& scorePoint)
    {
        m_objects.emplace_back(id, objType, texturePath, soundPath, startupHandler, logicHandler, animationFrametime, hitPoints, scorePoint);
        return m_objects.back();
    }

    GameUtils::Object& GameThread::CreateObjectAnimated(const std::string& id, const GameUtils::ObjectType& objType, 
            const std::string& texturePath, const std::string& soundPath,
            const std::function<void(GameUtils::Object&)>& startupHandler, const std::function<void(GameUtils::Object&)>& logicHandler, 
            const std::chrono::milliseconds& animationFrametime, const int& hitPoints, const int& scorePoint, const int& textureRow, const bool& isLoop) 
    {
        m_objects.emplace_back(id, objType, texturePath, soundPath, startupHandler, logicHandler, animationFrametime, hitPoints, scorePoint);
        m_objects.back().SetupAnimatedAction(textureRow, isLoop);
        return m_objects.back();
    }

    void GameThread::DestroyObject(const GameUtils::Object& obj)
    {
        m_objects.erase(std::find(m_objects.begin(), m_objects.end(), obj));
    }

    void GameThread::DestroyObjectAnimated(const GameUtils::Object& obj, const int& textureRow)
    {
        auto objIterator = std::find(m_objects.begin(), m_objects.end(), obj);
        objIterator->SetupAnimatedAction(textureRow, false, true);
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
        m_font.loadFromFile("../resources/fonts/PressStart2P-vaV7.ttf");

        GenerateSoundChannels();

        for(auto& key : GameUtils::Keyboard_Keys)
            m_keyMaps[key] = std::make_shared<GameUtils::Input>(key);
    }

    void GameThread::RenderStage()
    {
        if(m_progression == GameUtils::Progression::MENU) return;
        m_progression = GameUtils::Progression::NORMAL_GAME;

        RenderText();
        auto windowSize = m_window->getDefaultView().getSize();
  
        //Putting Player ship on the rendering pipeline
        CreateObject("0",GameUtils::ObjectType::PLAYER, "../resources/texture/multi-anim-ship.png", "",
            std::bind(&LogicFunctions::PlayerStartup, m_logicFunction, std::placeholders::_1, sf::Vector2i{(int)(windowSize.x/2), (int)(windowSize.y * 0.9)}),
            std::bind(&LogicFunctions::PlayerLogic, m_logicFunction, std::placeholders::_1), 200ms, 1, 1);


        //Putting array of Enemy ships in the rendering pipeline
        CreateArrayObject(GameUtils::enemyQuantity[0], GameUtils::enemyQuantity[1], 
            [this](sf::Vector2i vecPos, std::string id) 
            {
                return CreateObjectAnimated(id, GameUtils::ObjectType::ENEMY, "../resources/texture/animated-enemy-ship.png", "",
                        std::bind(&LogicFunctions::EnemyStartup, m_logicFunction, std::placeholders::_1, vecPos),
                        std::bind(&LogicFunctions::EnemyLogic, m_logicFunction, std::placeholders::_1), 200ms, 1, 1, true);
            }
        );
    }

    void GameThread::MenuScreen()
    {
        std::string menuTitleString = "SPACE INVADERS";
        auto menuTitleSize = 50;
        m_textSprites[GameUtils::TextType::MENU_TITLE] = {menuTitleString, m_font, menuTitleSize};
        m_textSprites[GameUtils::TextType::MENU_TITLE].setPosition({m_window->getDefaultView().getCenter().x - (menuTitleString.size()/2)*menuTitleSize, m_window->getDefaultView().getCenter().y/2});
        m_textSprites[GameUtils::TextType::MENU_TITLE].setFillColor(sf::Color(GameUtils::red));

        std::string menuStartString = "PRESS ENTER TO START";
        auto menuStartSize = 24;
        m_textSprites[GameUtils::TextType::MENU_START] = {menuStartString, m_font, menuStartSize};
        m_textSprites[GameUtils::TextType::MENU_START].setPosition({m_window->getDefaultView().getCenter().x - (menuStartString.size()/2)*menuStartSize, 3*m_window->getDefaultView().getCenter().y/2});
        m_textSprites[GameUtils::TextType::MENU_START].setFillColor(sf::Color(GameUtils::red));

        m_progression = GameUtils::Progression::MENU;
        // m_textSprites.erase(GameUtils::TextType::MENU_TITLE);
        // m_textSprites.erase(GameUtils::TextType::MENU_START);
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
            for(auto& [scancode, key] : m_keyMaps)
            {
                if(key != nullptr && key->GetPressed() && scancode == sf::Keyboard::Scancode::Enter)
                {
                    m_score = 0;
                    CleanupGame();
                    RenderStage();
                    break;
                }
            }
        }
        if(m_progression == GameUtils::Progression::MENU)
        {
            for(auto& [scancode, key] : m_keyMaps)
            {
                if(key != nullptr && key->GetPressed() && scancode == sf::Keyboard::Scancode::Enter)
                {
                    m_score = 0;
                    m_progression = GameUtils::Progression::NORMAL_GAME;
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
        if(m_paused == 2) return;
        for(auto index = 0; index < m_objects.size(); ++index)
        {
            if(m_objects[index].GetDestroy())
            {
                m_objects.erase(m_objects.begin() + index);
                continue;
            }
            m_objects[index].StepLogic();
            m_objects[index].DoAnimatedAction();
        }
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
                PlayAudioChannel(GameUtils::SoundName::WIN);
                m_progression = GameUtils::Progression::RESPAWN;
                CleanupGame();
                RenderStage();
            }

            else if(m_progression == GameUtils::Progression::NORMAL_GAME)
            {
                m_progression = GameUtils::Progression::BOSS_PHASE;
                CreateObjectAnimated("1", GameUtils::ObjectType::BOSS, "../resources/texture/animated-boss-ship.png", "", 
                    std::bind(&LogicFunctions::BossStartup, m_logicFunction, std::placeholders::_1, sf::Vector2i{(int)m_window->getDefaultView().getCenter().x, 0}),
                    std::bind(&LogicFunctions::BossLogic, m_logicFunction, std::placeholders::_1), 164ms, 5, 5, 1, true);
            }

        }
        if(m_progression != GameUtils::Progression::MENU && playerLive == 0)
        {
            if(m_paused != 2)
                PlayAudioChannel(GameUtils::SoundName::LOSE);
            m_progression = GameUtils::Progression::GAME_OVER;
            BlockingTextScreen("GAME OVER");
            m_paused = 2;
        }
    }


    void GameThread::CleanupGame()
    {
        m_paused = 0;

        m_objects.clear();
        m_textSprites.clear();
    }

    void GameThread::RenderText()
    {   
        if(m_progression == GameUtils::Progression::MENU) return;
        auto windowSize = m_window->getDefaultView().getSize();
        m_textSprites[GameUtils::TextType::SCORE] = {"Score: " + std::to_string(m_score), m_font, 24};
        m_textSprites[GameUtils::TextType::HIGH_SCORE] = {"Highscore: " + std::to_string(m_highscore), m_font, 24};
        m_textSprites[GameUtils::TextType::HIGH_SCORE].setPosition({m_window->getDefaultView().getCenter().x - ((std::string("Highscore: ").size()/2)*m_textSprites[GameUtils::TextType::HIGH_SCORE].getCharacterSize()),
                                                                    m_textSprites[GameUtils::TextType::HIGH_SCORE].getPosition().y});
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
                objectBuilder(sf::Vector2i{
                    (int)(std::floor(additiveRatio/2) + (row * additiveRatio)),  // X
                    (int)(windowSize.y * (0.1*(column + 1)))}, // Y
                    std::to_string(row + column*rows));
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
            if(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_lastFrameTime) >= GameUtils::globalFrametime)
            {
                m_lastFrameTime = std::chrono::steady_clock::now();
                ProgressionCheck();
                ClearScreen();
                CaptureKeyInput();
                PauseLogic();
                ExecuteLogic();
                DrawSprites();
                m_window->display(); 
            }
        }
    }


}
