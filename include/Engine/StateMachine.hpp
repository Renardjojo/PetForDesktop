#pragma once

#include "Game/GameData.hpp"
#include "Engine/Utilities.hpp"

#include <memory>
#include <vector>
#include <assert.h>

class StateMachine
{
public:
    struct Node
    {
        struct Transition
        {
            Node*                              pOwner;
            std::vector<std::shared_ptr<Node>> to;

            virtual void onEnter(GameData& blackBoard){};
            virtual void onUpdate(GameData& blackBoard, double dt){};
            virtual void onExit(GameData& blackBoard){};

            virtual bool canTransition(GameData& blackBoard)
            {
                return false;
            };
        };

        std::vector<std::shared_ptr<Transition>> transitions;

        void AddTransition(std::shared_ptr<Transition> transition)
        {
            transition->pOwner = this;
            transitions.emplace_back(transition);
        }

        virtual void onEnter(GameData& blackBoard)
        {
            for (auto&& transition : transitions)
            {
                transition->onEnter(blackBoard);
            }
        };
        virtual void onUpdate(GameData& blackBoard, double dt)
        {
            for (auto&& transition : transitions)
            {
                transition->onUpdate(blackBoard, dt);
            }
        };
        virtual void onExit(GameData& blackBoard)
        {
            for (auto&& transition : transitions)
            {
                transition->onExit(blackBoard);
            }
        };
    };

protected:
    std::shared_ptr<Node> pCurrentNode = nullptr;

    GameData& blackBoard;

public:
    StateMachine(GameData& inBlackBoard) : blackBoard{inBlackBoard}
    {
    }

    void init(const std::shared_ptr<Node>& initialState)
    {
        assert(initialState != nullptr);
        pCurrentNode = initialState;
        pCurrentNode->onEnter(blackBoard);
    }

    void update(double dt)
    {
        assert(pCurrentNode != nullptr);

        pCurrentNode->onUpdate(blackBoard, dt);

        for (auto&& pNodeTransition : pCurrentNode->transitions)
        {
            assert(pNodeTransition != nullptr);

            if (pNodeTransition->canTransition(blackBoard))
            {
                pCurrentNode->onExit(blackBoard);

                assert(!pNodeTransition->to.empty());
                std::shared_ptr<Node>& to = pNodeTransition->to[randNum(0, pNodeTransition->to.size() - 1)];
                assert(to != nullptr);
                pCurrentNode = to;
                pCurrentNode->onEnter(blackBoard);
                break;
            }
        }
    }
};