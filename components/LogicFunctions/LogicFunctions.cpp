#include <cmath>
#include <iostream>

#include "LogicFunctions.h"
#include "GameThread/GameThread.h"

namespace GameEngine
{

    void LogicFunctions::PlayerStartup(GameUtils::Object& obj)
    {
        auto x = m_gameThread->GetRenderWindow()->getSize().x;
        auto y = m_gameThread->GetRenderWindow()->getSize().y;

        obj.GetSprite().setPosition(x/2, y * 0.9f);
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
        if(projectile && std::find_if(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), [](const GameUtils::Object& obj){return obj.GetType() == GameUtils::ObjectType::PROJECTILE;}) == m_gameThread->GetObjects().end())
        {
            m_gameThread->GetObjects().emplace_back("1", GameUtils::ObjectType::PROJECTILE, "../resources/texture/projectile.png", "../resources/sfx/player-shot.wav",
                std::bind(LogicFunctions::ProjectileSetup, this, std::placeholders::_1),
                std::bind(LogicFunctions::ProjectileLogic, this, std::placeholders::_1));
        }
    }
    void LogicFunctions::EnemyStartup(GameUtils::Object& obj)
    {
        auto id = stoi(obj.GetId());
        auto x = m_gameThread->GetRenderWindow()->getSize().x;
        auto y = m_gameThread->GetRenderWindow()->getSize().y;

        obj.GetSprite().setPosition(x - x*(0.3*id), y * 0.1f);
    }

    void LogicFunctions::EnemyLogic(GameUtils::Object& obj)
    {
        if(m_logicAssists[GameUtils::ObjectType::ENEMY].counter++ == 30)
        {
            m_logicAssists[GameUtils::ObjectType::ENEMY].persistent_value = -m_logicAssists[GameUtils::ObjectType::ENEMY].persistent_value;
            m_logicAssists[GameUtils::ObjectType::ENEMY].counter = 0;
        }
        int nextPosition = obj.GetSprite().getPosition().x - m_logicAssists[GameUtils::ObjectType::ENEMY].persistent_value;
        if(nextPosition > m_gameThread->GetRenderWindow()->getSize().x) nextPosition = 0;
        if(nextPosition <= 0) nextPosition = m_gameThread->GetRenderWindow()->getSize().x;
        obj.GetSprite().setPosition(nextPosition, obj.GetSprite().getPosition().y);
    }

    void LogicFunctions::ProjectileSetup(GameUtils::Object& obj)
    {
        auto playerPosition = std::find_if(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), [](const GameUtils::Object& obj){
            return obj.GetType() == GameUtils::ObjectType::PLAYER;
        });

        obj.GetSound().play();
        obj.GetSprite().setPosition(playerPosition->GetSprite().getPosition().x, playerPosition->GetSprite().getPosition().y - obj.GetSprite().getGlobalBounds().getSize().y);
    }

    void LogicFunctions::ProjectileLogic(GameUtils::Object& obj)
    {
        auto currentPosition = obj.GetSprite().getPosition();

        if(currentPosition.y >= 0)
            obj.GetSprite().setPosition(currentPosition.x, currentPosition.y - 10);
        else 
        {
            EraseObject(obj);
            return;
        }

        auto enemyObjs = GetAllObjectByType(GameUtils::ObjectType::ENEMY);
        if(enemyObjs.size() > 0)
        {
            std::cout << enemyObjs.size() << std::endl;
            for(auto index = 0; index < enemyObjs.size(); ++index)
            {
                auto enemyPosition = enemyObjs[index].GetSprite().getPosition();
                auto dx = (int)std::abs(currentPosition.x - enemyPosition.x);
                auto dy = (int)std::abs(currentPosition.y - enemyPosition.y);
                auto r = (int)enemyObjs[index].GetSprite().getGlobalBounds().getSize().x;
                if(std::pow(dx,2) + std::pow(dy, 2) <= std::pow(r, 2)) 
                {
                    EraseObject(enemyObjs[index]);
                    EraseObject(obj);
                    return;
                }   
            }
        }


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

    void LogicFunctions::EraseObjectById(std::string id)
    {
        m_gameThread->GetObjects().erase(std::find_if(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), 
        [id](const GameUtils::Object& obj){
            return obj.GetId() == id;
        }));
    }

    void LogicFunctions::EraseObject(const GameUtils::Object& obj)
    {
        m_gameThread->GetObjects().erase(std::find(m_gameThread->GetObjects().begin(), m_gameThread->GetObjects().end(), obj));
    }

}