#pragma once

#include "Game/GameData.hpp"

#include <GLFW/glfw3.h>

#include <functional>
#include <queue>
#include <vector>
#include <cmath>

struct TimerTask
{
    std::function<void()> task        = nullptr;
    double                localTimer  = 0.; // if current time egal 1s and local timer egal 0.5 global time egal 1.5
    double                globalTimer = 0.;
    bool                  isLooping   = false;

    TimerTask(const std::function<void()>& task = nullptr, double localTimer = .0, double globalTimer = .0,
              bool isLooping = false)
        : task{task}, localTimer{localTimer}, globalTimer{globalTimer}, isLooping{isLooping}
    {
    }

    bool operator>(const TimerTask& other) const noexcept
    {
        return globalTimer > other.globalTimer;
    }
};

class TimeManager
{
protected:
    double m_time     = glfwGetTime();
    double m_tempTime = m_time;

    double    m_timeAccLoop    = 0.;
    double    m_deltaTime      = 0.;
    double    m_fixedDeltaTime = 1. / 60.;
    GameData& datas;

    std::priority_queue<TimerTask, std::vector<TimerTask>, std::greater<TimerTask>> m_timerQueue;

public:
    TimeManager(GameData& data) : m_fixedDeltaTime{1. / data.FPS}, datas{data}
    {
    }

    // improve first frame accurancy
    void start()
    {
        m_time     = glfwGetTime();
        m_tempTime = m_time;
    }

    inline void emplaceTimer(std::function<void()> functionToExecute, double delay, bool isLooping = false) noexcept
    {
        m_timerQueue.emplace(functionToExecute, delay, delay + datas.timeAcc, isLooping);
    }

    void update(std::function<void(double deltaTime)> unlimitedUpdateFunction,
                std::function<void(double deltaTime)> limitedUpdateFunction)
    {
        /*unfixed update*/
        unlimitedUpdateFunction(m_deltaTime);

        /*Prepar the next frame*/
        m_tempTime  = glfwGetTime();
        m_deltaTime = m_tempTime - m_time;
        m_time      = m_tempTime;

        // This is temporary
        if (m_deltaTime > 0.25)
            m_deltaTime = 0.25;

        /*Add accumulator*/
        datas.timeAcc += m_deltaTime;
        datas.timeAcc *= !isinf(datas.timeAcc); // reset if isInf (avoid conditionnal jump)

        /*Fixed update*/
        m_timeAccLoop += m_deltaTime;

        while (m_timeAccLoop >= m_fixedDeltaTime)
        {
            limitedUpdateFunction(m_fixedDeltaTime);
            m_timeAccLoop -= m_fixedDeltaTime;
        }

        while (!m_timerQueue.empty() && m_timerQueue.top().globalTimer <= datas.timeAcc)
        {
            const TimerTask& timerTask = m_timerQueue.top();
            timerTask.task();

            if (timerTask.isLooping)
            {
                emplaceTimer(timerTask.task, timerTask.localTimer, timerTask.isLooping);
            }
            m_timerQueue.pop();
        }

        while (!datas.deltasCursorPosBuffer.empty() &&
               datas.deltasCursorPosBuffer.top().timer + datas.coyoteTimeCursorPos <= datas.timeAcc)
        {
            const GameData::DeltaCursosPosElem& elem = datas.deltasCursorPosBuffer.top();
            datas.deltaCursorAcc -= elem.pos;
            datas.deltasCursorPosBuffer.pop();
        }
    }
};