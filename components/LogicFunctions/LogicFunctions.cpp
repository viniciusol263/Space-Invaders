#include <cmath>
#include <iostream>
#include <random>

#include "LogicFunctions.h"
#include "Input/Input.h"
#include "GameThread/GameThread.h"

namespace GameEngine
{

    void LogicFunctions::PlayerStartup(GameUtils::Object& obj, const sf::Vector2i& initialPos)
    {
        auto posX = std::abs(initialPos.x - obj.GetSprite().getTextureRect().getSize().x/2);
        auto posY = std::abs(initialPos.y - obj.GetSprite().getTextureRect().getSize().y/2);
 
        obj.GetSprite().setPosition(posX, posY);
        m_logicAssists[projectileMapIndex] = DefaultAssists[GameUtils::ObjectType::PROJECTILE];
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
        if(projectile && m_logicAssists[projectileMapIndex].auxVariables[0] == 0)
        {
            m_logicAssists[projectileMapIndex].auxVariables[0] = 1;
            m_gameThread->CreateObjectAnimated("1", GameUtils::ObjectType::PROJECTILE, "../resources/texture/animated-projectile.png", "../resources/sfx/player-shot.wav",
                std::bind(LogicFunctions::ProjectileSetup, this, std::placeholders::_1),
                std::bind(LogicFunctions::ProjectileLogic, this, std::placeholders::_1), 150ms, 1 , 0, true);
        }
    }
    void LogicFunctions::EnemyStartup(GameUtils::Object& obj, const sf::Vector2i& initialPos)
    {
        auto enemyInstance = std::make_pair(GameUtils::ObjectType::ENEMY, stoi(obj.GetId()));
        m_logicAssists[enemyInstance] = DefaultAssists[GameUtils::ObjectType::ENEMY];
        m_logicAssists[enemyProjectileMapIndex] = DefaultAssists[GameUtils::ObjectType::ENEMY_PROJECTILE];

        auto posX = std::abs(initialPos.x - obj.GetSprite().getTextureRect().getSize().x/2);
        auto posY = std::abs(initialPos.y - obj.GetSprite().getTextureRect().getSize().y/2);
 
        obj.GetSprite().setPosition(posX, posY);
        obj.SetTimer(obj.GetAnimationFrametime(), true);
    }

    void LogicFunctions::EnemyLogic(GameUtils::Object& obj)
    {
        auto enemyInstance = std::make_pair(GameUtils::ObjectType::ENEMY, stoi(obj.GetId()));
        
        if(m_logicAssists[enemyInstance].auxVariables[0]++ == 30)
        {
            m_logicAssists[enemyInstance].auxVariables[1] = -m_logicAssists[enemyInstance].auxVariables[1];
            m_logicAssists[enemyInstance].auxVariables[0] = 0;
        }
        int nextPosition = obj.GetSprite().getPosition().x - m_logicAssists[enemyInstance].auxVariables[1];
        if(nextPosition > m_gameThread->GetRenderWindow()->getSize().x) nextPosition = 0;
        if(nextPosition <= 0) nextPosition = m_gameThread->GetRenderWindow()->getSize().x;
        obj.GetSprite().setPosition(nextPosition, obj.GetSprite().getPosition().y);

        auto position = obj.GetSprite().getPosition();

        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist(1,100);

        if(dist(rng) >= 80 && m_logicAssists[enemyProjectileMapIndex].auxVariables[0] == 0)
        {
            m_logicAssists[enemyProjectileMapIndex].auxVariables[0] = 1;
            m_gameThread->CreateObjectAnimated("1", GameUtils::ObjectType::ENEMY_PROJECTILE, "../resources/texture/animated-enemy-projectile.png", "../resources/sfx/enemy-shot.wav",
                std::bind(LogicFunctions::EnemyProjectileSetup, this, std::placeholders::_1, sf::Vector2i{position.x,position.y}, enemyInstance),
                std::bind(LogicFunctions::EnemyProjectileLogic, this, std::placeholders::_1), 150ms, 1, 0, true);
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
            if(m_logicAssists[projectileMapIndex].auxVariables[1] == 0)
                obj.GetSprite().setPosition(currentPosition.x, currentPosition.y - 7);
        }
        else 
        {
            m_logicAssists[projectileMapIndex].auxVariables[0] = 0;
            DestroyObject(obj);
            return;
        }

        ObjectCollison(obj, {GameUtils::ObjectType::ENEMY, GameUtils::ObjectType::BOSS}, projectileMapIndex, GameUtils::SoundName::ENEMY_DEATH);
    }

    void LogicFunctions::EnemyProjectileSetup(GameUtils::Object& obj, const sf::Vector2i& initialPos, const std::pair<GameUtils::ObjectType, int>& assistId)
    {
        m_gameThread->PlayAudioChannel(GameUtils::SoundName::ENEMY_SHOT); 
        obj.GetSprite().setPosition(initialPos.x, initialPos.y + obj.GetSprite().getGlobalBounds().getSize().y);
    }

    void LogicFunctions::EnemyProjectileLogic(GameUtils::Object& obj)
    {
        auto currentPosition = obj.GetSprite().getPosition();

        if(currentPosition.y < m_gameThread->GetRenderWindow()->getDefaultView().getSize().y)
        {
            if(m_logicAssists[enemyProjectileMapIndex].auxVariables[1] == 0)
                obj.GetSprite().setPosition(currentPosition.x, currentPosition.y + 7);
        }
        else 
        {
            m_logicAssists[enemyProjectileMapIndex].auxVariables[0] = 0;
            DestroyObject(obj);
            return;
        }

        ObjectCollison(obj, {GameUtils::ObjectType::PLAYER}, enemyProjectileMapIndex, GameUtils::SoundName::ENEMY_DEATH, 1);
    }
    
    void LogicFunctions::BossStartup(GameUtils::Object& obj, const sf::Vector2i& initialPos)
    {
        auto posX = std::abs(initialPos.x - obj.GetSprite().getTextureRect().getSize().x/2);
        auto posY = std::abs(initialPos.y - obj.GetSprite().getTextureRect().getSize().y/2);
 
        obj.GetSprite().setPosition(posX, posY);
        m_logicAssists[bossMapIndex] = DefaultAssists[GameUtils::ObjectType::BOSS];
        obj.SetTimer(obj.GetAnimationFrametime(), true);
    }

    void LogicFunctions::BossLogic(GameUtils::Object& obj)
    {   
        const int movementRange = (m_gameThread->GetRenderWindow()->getDefaultView().getSize().x/4);

        if(obj.GetSprite().getPosition().x <= (movementRange - (obj.GetSprite().getTextureRect().getSize().x/2)) || obj.GetSprite().getPosition().x >= (3*movementRange))
        {
            m_logicAssists[bossMapIndex].auxVariables[1] = -m_logicAssists[bossMapIndex].auxVariables[1];
        }
        int nextPosition = obj.GetSprite().getPosition().x - m_logicAssists[bossMapIndex].auxVariables[1];
        if(nextPosition > m_gameThread->GetRenderWindow()->getSize().x) nextPosition = 0;
        if(nextPosition <= 0) nextPosition = m_gameThread->GetRenderWindow()->getSize().x;
        obj.GetSprite().setPosition(nextPosition, obj.GetSprite().getPosition().y);
    }

    std::vector<GameUtils::Object> LogicFunctions::GetAllObjectByTypes(const std::vector<GameUtils::ObjectType>& types)
    {
        std::vector<GameUtils::Object> m_vector;
        for(auto index = 0; index < m_gameThread->GetObjects().size(); ++index)
        {
            for(auto type : types)
                if(m_gameThread->GetObjects()[index].GetType() == type)
                    m_vector.push_back(m_gameThread->GetObjects()[index]);
        }
        return m_vector;
    }

    GameUtils::Object& LogicFunctions::GetObjectReference(const GameUtils::Object& obj)
    {
        return *std::find(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), obj);
    }

    void LogicFunctions::DestroyObject(const GameUtils::Object& obj)
    {
        m_gameThread->GetObjects().erase(std::find(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), obj));
    }

    void LogicFunctions::ObjectCollison(GameUtils::Object& obj, const std::vector<GameUtils::ObjectType>& objTypes, const std::pair<GameUtils::ObjectType, int>& logicAssist, const GameUtils::SoundName& soundName, const int& textureRow)
    {
        auto currentPosition = obj.GetSprite().getPosition();
        auto enemyObjs = GetAllObjectByTypes(objTypes);
        if(enemyObjs.size() > 0 && m_logicAssists[logicAssist].auxVariables[1] == 0)
        {
            for(auto index = 0; index < enemyObjs.size(); ++index)
            {
                auto enemyPosition = enemyObjs[index].GetSprite().getPosition();
                auto dx = (int)std::abs(currentPosition.x - enemyPosition.x);
                auto dy = (int)std::abs(currentPosition.y - enemyPosition.y);
                auto r = (int)enemyObjs[index].GetSprite().getTextureRect().getSize().x;


                if(std::pow(dx,2) + std::pow(dy, 2) <= std::pow(r, 2)) 
                {
                    m_logicAssists[logicAssist].auxVariables[1] = 1;
                    obj.SetupAnimatedAction(textureRow, false, true, [this, logicAssist] {
                        m_logicAssists[logicAssist].auxVariables[0] = 0;
                        m_logicAssists[logicAssist].auxVariables[1] = 0;
                    });

                    auto& objRef = GetObjectReference(enemyObjs[index]);
                    m_gameThread->PlayAudioChannel(soundName);
                    objRef.SetHitPoints(objRef.GetHitPoints() - 1);
                    if(objRef.GetHitPoints() <= 0)
                    {
                        objRef.SetHitPoints(999);
                        objRef.SetupAnimatedAction(textureRow, false, true, [this, logicAssist, objTypes]() {
                            for(auto objType : objTypes)
                                if(objType == GameUtils::ObjectType::ENEMY)
                                    m_gameThread->SetScore(++m_gameThread->GetScore());
                        });
                    }
                    return;
                }   
            }
        }
    }
}