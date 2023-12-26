#pragma once

#include <string>
#include <variant>
#include <unordered_map>
#include "GameThread/IGameThread.h"

namespace GameEngine
{
    constexpr int AI_WALK_SPEED = 10;

    struct LogicAssist
    {
        int persistent_value;
        int counter;
    };

    class LogicFunctions 
    {
    public:
        LogicFunctions() = delete;
        LogicFunctions(const std::shared_ptr<IGameThread>& gameThread) : m_gameThread(std::move(gameThread))
        {
           m_logicAssists[GameUtils::ObjectType::ENEMY] = LogicAssist{
            .persistent_value = 3,
            .counter = 0
           };
        }

        void PlayerStartup(GameUtils::Object& obj);
        void PlayerLogic(GameUtils::Object& obj);
        void EnemyStartup(GameUtils::Object& obj);
        void EnemyLogic(GameUtils::Object& obj);
        void ProjectileSetup(GameUtils::Object& obj);
        void ProjectileLogic(GameUtils::Object& obj);

    private:
        std::shared_ptr<IGameThread> m_gameThread;
        std::unordered_map<GameUtils::ObjectType, LogicAssist> m_logicAssists;
        
        std::vector<GameUtils::Object> GetAllObjectByType(const GameUtils::ObjectType& type);
        void EraseObjectById(std::string id);
        void EraseObject(const GameUtils::Object& obj);
        
    };


}