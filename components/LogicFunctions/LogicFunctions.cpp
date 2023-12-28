#include <cmath>
#include <iostream>
#include <random>

#include "LogicFunctions.h"
#include "GameThread/GameThread.h"

namespace GameEngine
{

    void LogicFunctions::PlayerStartup(GameUtils::Object& obj, sf::Vector2i initialPos)
    {
        obj.GetSprite().setPosition(initialPos.x, initialPos.y);
    }

    void LogicFunctions::PlayerLogic(GameUtils::Object& obj)
    {
        //Horizontal Movement
        auto left = (m_gameThread->GetKeys()[sf::Keyboard::Scancode::A]->GetPressed()) ? -1 : 0;
        auto right = (m_gameThread->GetKeys()[sf::Keyboard::Scancode::D]->GetPressed()) ? 1 : 0;
        auto projectile = m_gameThread->GetKeys()[sf::Keyboard::Scancode::Space]->GetPressed();

        auto nextPosition = obj.GetSprite().getPosition().x + ((left + right) * 10);
        if(nextPosition > m_gameThread->GetRenderWindow()->getSize().x) nextPosition = 0;
        if(nextPosition < 0) nextPosition = m_gameThread->GetRenderWindow()->getSize().x;
        obj.GetSprite().setPosition(nextPosition, obj.GetSprite().getPosition().y);

        //Projectile instantiation
        if(projectile && m_logicAssists[projectileMapIndex].counter == 0)
        {
            m_logicAssists[projectileMapIndex].counter = 1;
            m_gameThread->DoAnimatedAction(obj, 0, true, [this](){
                m_gameThread->CreateObject("1", GameUtils::ObjectType::PROJECTILE, "../resources/texture/animated-projectile.png", "../resources/sfx/player-shot.wav",
                    std::bind(LogicFunctions::ProjectileSetup, this, std::placeholders::_1),
                    std::bind(LogicFunctions::ProjectileLogic, this, std::placeholders::_1), 300ms);
            });
        }
    }
    void LogicFunctions::EnemyStartup(GameUtils::Object& obj, sf::Vector2i initialPos)
    {
        auto enemyInstance = std::make_pair(GameUtils::ObjectType::ENEMY, stoi(obj.GetId()));
        m_logicAssists[enemyInstance] = DefaultAssists[GameUtils::ObjectType::ENEMY];
        obj.GetSprite().setPosition(initialPos.x, initialPos.y);
    }

    void LogicFunctions::EnemyLogic(GameUtils::Object& obj)
    {
        auto enemyInstance = std::make_pair(GameUtils::ObjectType::ENEMY, stoi(obj.GetId()));
        
        if(m_logicAssists[enemyInstance].counter++ == 30)
        {
            m_logicAssists[enemyInstance].persistent_value = -m_logicAssists[enemyInstance].persistent_value;
            m_logicAssists[enemyInstance].counter = 0;
        }
        int nextPosition = obj.GetSprite().getPosition().x - m_logicAssists[enemyInstance].persistent_value;
        if(nextPosition > m_gameThread->GetRenderWindow()->getSize().x) nextPosition = 0;
        if(nextPosition <= 0) nextPosition = m_gameThread->GetRenderWindow()->getSize().x;
        obj.GetSprite().setPosition(nextPosition, obj.GetSprite().getPosition().y);

        auto position = obj.GetSprite().getPosition();

        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist(1,100);

        if(dist(rng) >= 80 && m_logicAssists[enemyProjectileMapIndex].counter == 0)
        {
            m_logicAssists[enemyProjectileMapIndex].counter = 1;
            m_gameThread->CreateObject("1", GameUtils::ObjectType::PROJECTILE, "../resources/texture/animated-enemy-projectile.png", "../resources/sfx/enemy-shot.wav",
                std::bind(LogicFunctions::EnemyProjectileSetup, this, std::placeholders::_1, sf::Vector2i{position.x,position.y}),
                std::bind(LogicFunctions::EnemyProjectileLogic, this, std::placeholders::_1), 300ms);
        }


    }

    void LogicFunctions::ProjectileSetup(GameUtils::Object& obj)
    {
        auto playerPosition = std::find_if(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), [](const GameUtils::Object& obj){
            return obj.GetType() == GameUtils::ObjectType::PLAYER;
        });

        m_gameThread->PlayAudioChannel(GameUtils::SoundName::PLAYER_SHOT); 
        obj.GetSprite().setPosition(playerPosition->GetSprite().getPosition().x, playerPosition->GetSprite().getPosition().y - obj.GetSprite().getGlobalBounds().getSize().y);
    }

    void LogicFunctions::ProjectileLogic(GameUtils::Object& obj)
    {
        auto currentPosition = obj.GetSprite().getPosition();

        if(currentPosition.y >= 0)
        {
            if(m_logicAssists[projectileMapIndex].persistent_value == 0)
                obj.GetSprite().setPosition(currentPosition.x, currentPosition.y - 10);
        }
        else 
        {
            m_logicAssists[projectileMapIndex].counter = 0;
            DestroyObject(obj);
            return;
        }

        ObjectCollison(obj, GameUtils::ObjectType::ENEMY, projectileMapIndex, GameUtils::SoundName::ENEMY_DEATH);
    }

    void LogicFunctions::EnemyProjectileSetup(GameUtils::Object& obj, sf::Vector2i initialPos)
    {
        m_gameThread->PlayAudioChannel(GameUtils::SoundName::ENEMY_SHOT); 
        obj.GetSprite().setPosition(initialPos.x, initialPos.y + obj.GetSprite().getGlobalBounds().getSize().y);
    }

    void LogicFunctions::EnemyProjectileLogic(GameUtils::Object& obj)
    {
        auto currentPosition = obj.GetSprite().getPosition();

        if(currentPosition.y < m_gameThread->GetRenderWindow()->getDefaultView().getSize().y)
        {
            if(m_logicAssists[enemyProjectileMapIndex].persistent_value == 0)
                obj.GetSprite().setPosition(currentPosition.x, currentPosition.y + 10);
        }
        else 
        {
            m_logicAssists[enemyProjectileMapIndex].counter = 0;
            DestroyObject(obj);
            return;
        }

        ObjectCollison(obj, GameUtils::ObjectType::PLAYER, enemyProjectileMapIndex, GameUtils::SoundName::ENEMY_DEATH, 1);
    }

    std::vector<GameUtils::Object> LogicFunctions::GetAllObjectByType(const GameUtils::ObjectType& type)
    {
        std::vector<GameUtils::Object> m_vector;
        for(auto index = 0; index < m_gameThread->GetObjects().size(); ++index)
        {
            if(m_gameThread->GetObjects()[index].GetType() == type)
                m_vector.push_back(m_gameThread->GetObjects()[index]);
        }
        return m_vector;
    }

    GameUtils::Object& LogicFunctions::GetObjectReference(GameUtils::Object obj)
    {
        return *std::find(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), obj);
    }

    void LogicFunctions::DestroyObject(const GameUtils::Object& obj)
    {
        m_gameThread->GetObjects().erase(std::find(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), obj));
    }

    void LogicFunctions::ObjectCollison(GameUtils::Object& obj, GameUtils::ObjectType objType, std::pair<GameUtils::ObjectType, int> logicAssist, GameUtils::SoundName soundName, int textureRow)
    {
        auto currentPosition = obj.GetSprite().getPosition();
        auto enemyObjs = GetAllObjectByType(objType);
        if(enemyObjs.size() > 0 && m_logicAssists[logicAssist].persistent_value == 0)
        {
            for(auto index = 0; index < enemyObjs.size(); ++index)
            {
                auto enemyPosition = enemyObjs[index].GetSprite().getPosition();
                auto dx = (int)std::abs(currentPosition.x - enemyPosition.x);
                auto dy = (int)std::abs(currentPosition.y - enemyPosition.y);
                auto r = (int)enemyObjs[index].GetSprite().getGlobalBounds().getSize().x;
                if(std::pow(dx,2) + std::pow(dy, 2) <= std::pow(r, 2)) 
                {
                    m_logicAssists[logicAssist].persistent_value = 1;
                    m_gameThread->DoAnimatedAction(GetObjectReference(obj), false, textureRow, [this, obj, logicAssist](){
                        DestroyObject(GetObjectReference(obj));
                        m_logicAssists[logicAssist].persistent_value = 0;
                    });
                    m_gameThread->DoAnimatedAction(GetObjectReference(enemyObjs[index]), textureRow, false, [this, enemyObjs, objType, index, logicAssist, soundName](){
                        m_gameThread->PlayAudioChannel(soundName);
                        DestroyObject(GetObjectReference(enemyObjs[index]));
                        m_logicAssists[logicAssist].counter = 0;
                        if(objType == GameUtils::ObjectType::ENEMY)
                            m_gameThread->SetScore(++m_gameThread->GetScore());
                    });
                    return;
                }   
            }
        }
    }
}