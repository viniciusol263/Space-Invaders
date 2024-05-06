#include "Object.h"

namespace GameUtils
{
    Object::Object(const std::string& id, const ObjectType& objType, const std::string& texturePath, const std::string& soundPath, const std::function<void(GameUtils::Object&)>& startupHandler, const std::function<void(GameUtils::Object&)>& logicHandler, const std::chrono::milliseconds& animationFrametime, const int& hitPoints, const int& scorePoint) : 
            m_id(id), m_objType(objType), m_soundBufferPath(soundPath), m_startupHandler(startupHandler), m_logicHandler(logicHandler), m_animationFrametime(animationFrametime), m_hitPoints(hitPoints), m_scorePoint(scorePoint)
        {
            if(m_id != "UNKNOWN")
            {             
                std::map<ObjectType, sf::IntRect> hitBoxMap = {
                    {ObjectType::PLAYER, sf::IntRect{{5,5},{25,25}}},
                    {ObjectType::ENEMY, sf::IntRect{{4,5},{22,21}}},
                    {ObjectType::PROJECTILE, sf::IntRect{{10,12}, {11, 8}}},
                    {ObjectType::ENEMY_PROJECTILE, sf::IntRect{{10,12}, {11, 8}}},
                    {ObjectType::BOSS, sf::IntRect{{11,20}, {105, 84}}},
                    {ObjectType::BOSS_PROJECTILE, sf::IntRect{{7,8}, {18, 15}}},
                };
                m_objTexture = std::make_shared<sf::Texture>();
                m_objTexture->loadFromFile(texturePath);
                m_objSprite.setTexture(*m_objTexture);
                auto [textureWidth, textureHeigth] = TextureSizeFromObjectType(objType);
                m_objSprite.setTextureRect(sf::IntRect{sf::Vector2i{0,0}, sf::Vector2i{textureWidth,textureHeigth}});
                
                if(soundPath != "")
                {
                    m_objSoundBuffer = std::make_shared<sf::SoundBuffer>();
                    m_objSoundBuffer->loadFromFile(soundPath);
                    m_objSound.setBuffer(*m_objSoundBuffer);
                }

                m_currentRenderRect = m_objSprite.getTextureRect();
                m_textureSize = m_objSprite.getTexture()->getSize();
                m_renderRectSize = m_currentRenderRect.getSize();
                m_frameQuantity = m_textureSize.x/m_renderRectSize.x;
                m_animationHead = 0;
                m_destroyOnFinish = false;
                m_destroy = false;
                m_auxiliarTimestamps["Primary"] = std::chrono::steady_clock::now();
                m_startupHandler(*this);
                m_hitBox = hitBoxMap[objType];
            }
        }


        std::string Object::GetId() const
        {
            return m_id;
        }

        sf::Sprite& Object::GetSprite() 
        {
            return m_objSprite;
        }

        std::shared_ptr<sf::Texture>& Object::GetTexture()
        {
            return m_objTexture;
        }

        std::string Object::GetDefaultSoundFilePath()
        {
            return m_soundBufferPath;
        }

        sf::Sound& Object::GetSound()
        {
            return m_objSound;
        }

        ObjectType Object::GetType() const
        {
            return m_objType;
        }

        std::chrono::milliseconds Object::GetAnimationFrametime() const
        {
            return m_animationFrametime;
        }

        void Object::SetTimer(const std::chrono::milliseconds& time, const bool& continous)
        {
            if(time == 0ms)
            {
                m_timer.reset();
                m_timer = nullptr;
                return;
            }
            m_timer = std::make_shared<std::tuple<std::chrono::milliseconds, std::chrono::steady_clock::time_point, bool>>(time, std::chrono::steady_clock::now(), continous);
        }

        bool Object::TimerOverflown() 
        {
            if(m_timer == nullptr) 
            {
                return false;
            }

            auto [time, timer, continous] = *m_timer;
            if(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - timer) >= time)
            {
                if(continous)
                    m_timer = std::make_shared<std::tuple<std::chrono::milliseconds, std::chrono::steady_clock::time_point, bool>>(time, std::chrono::steady_clock::now(), continous);
                return true;
            }
            return false;
        }

        void Object::SetupAnimatedAction(const int& textureRow, const bool& isLoop, const bool& destroyOnFinish, const bool& onFinishRollback, const int& rollbackTextureRow, const std::function<void()>& destroyAction)
        {
            m_textureRow = textureRow;
            m_isLoop = isLoop;
            m_animRunning = true;
            m_animationStartTime = std::chrono::steady_clock::now();
            m_animationStep = 1;
            m_destroyOnFinish = destroyOnFinish;
            m_onFinishRollback = onFinishRollback;
            m_destroyAction = destroyAction;
            m_previousTextureRow = rollbackTextureRow;
        }

        void Object::DoAnimatedAction()
        {
            if(!m_animRunning) return;
            auto animationFramePeriod = m_animationFrametime/(int)m_frameQuantity;
            if(std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - m_animationStartTime) > animationFramePeriod)
            {
                    if(m_animationStep == m_frameQuantity) 
                    {
                        m_animationStep = 0;
                        if(!m_isLoop)
                        {
                            m_animRunning = false;
                            if(m_destroyOnFinish)
                            {
                                m_destroy = true;
                                m_destroyAction();
                            }
                            if(m_onFinishRollback)
                                SetupAnimatedAction(m_previousTextureRow, true);
                        }
                    }
                    m_animationStartTime = std::chrono::steady_clock::now();
                    m_objSprite.setTextureRect(sf::IntRect{sf::Vector2i{(int)(m_animationStep++ * m_renderRectSize.x), m_textureRow * m_renderRectSize.y}, m_currentRenderRect.getSize()});
            }
        }

        void Object::StopAnimatedAction()
        {
            m_animRunning = 0;
        }

        bool Object::GetDestroy()
        {
            return m_destroy;
        }

        bool Object::GetDestroyOnFinish()
        {
            return m_destroyOnFinish;
        }

        sf::IntRect Object::GetHitBox()
        {
            return m_hitBox;
        }

        std::map<std::string, int>& Object::GetAuxiliarVars()
        {
            return m_auxiliarVariables;
        }

        std::map<std::string, std::chrono::time_point<std::chrono::steady_clock>>& Object::GetAuxiliarTimeStamp()
        {
            return m_auxiliarTimestamps;
        }

        int Object::GetHitPoints() const
        {
            return m_hitPoints;
        }

        void Object::SetHitPoints(const int& value)
        {
            m_hitPoints = value;
        }

        bool Object::GetAnimRunning()
        {
            return m_animRunning;
        }

        void Object::StepLogic()
        {
            m_logicHandler(*this);
        }

        std::pair<int,int> Object::TextureSizeFromObjectType(const ObjectType& type)
        {
            switch(type)
            {
                case ObjectType::BOSS: return std::make_pair(128,128);
                default: return std::make_pair(32,32);
            }
        }
}