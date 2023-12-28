#pragma once

#include <string>
#include <variant>
#include <map>
#include <utility>
#include "GameThread/IGameThread.h"

namespace GameEngine
{
    constexpr int AI_WALK_SPEED = 10;

    constexpr auto projectileMapIndex = std::make_pair(GameUtils::ObjectType::PROJECTILE, 0);

    struct LogicAssist
    {
        int persistent_value;
        int counter;
    };

    static std::map<GameUtils::ObjectType, LogicAssist> DefaultAssists = {{GameUtils::ObjectType::ENEMY, {3, 0}}, {GameUtils::ObjectType::PROJECTILE, {0,0}}};

    class LogicFunctions 
    {
    public:
        LogicFunctions() = delete;
        LogicFunctions(const std::shared_ptr<IGameThread>& gameThread) 
            : m_gameThread(std::move(gameThread))
        {
            //Logic Assist for Enemies
           for(int index = 0; index <= 4*4; ++index)
           {
            auto number = std::make_pair(GameUtils::ObjectType::ENEMY, index);
            m_logicAssists[number] = LogicAssist{
                .persistent_value = 3,
                .counter = 0
            };
           }

           //Logic Assist for Projectile
           auto number = std::make_pair(GameUtils::ObjectType::PROJECTILE, 0);
           m_logicAssists[number] = LogicAssist{
            .persistent_value = 0,
            .counter = 0
           };
        }

        void PlayerStartup(GameUtils::Object& obj, sf::Vector2i initialPos);
        void PlayerLogic(GameUtils::Object& obj);
        void EnemyStartup(GameUtils::Object& obj, sf::Vector2i initialPos);
        void EnemyLogic(GameUtils::Object& obj);
        void ProjectileSetup(GameUtils::Object& obj);
        void ProjectileLogic(GameUtils::Object& obj);

    private:
        std::shared_ptr<IGameThread> m_gameThread;
        std::map<std::pair<GameUtils::ObjectType, int>, LogicAssist> m_logicAssists;
        
        std::vector<GameUtils::Object> GetAllObjectByType(const GameUtils::ObjectType& type);
        GameUtils::Object& GetObjectReference(GameUtils::Object obj);
        void DestroyObject(const GameUtils::Object& obj);
        
    };


}