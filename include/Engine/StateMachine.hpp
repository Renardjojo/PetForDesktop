#pragma once

#include "Engine/Utilities.hpp"
#include "Game/GameData.hpp"

#include <assert.h>
#include <memory>
#include <vector>

class StateMachine
{
public:
    struct Node
    {
        struct Transition
        {
            Node*                              pOwner = nullptr;
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
        bool                                     canUseTransition = true;

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
            if (canUseTransition)
            {
                for (auto&& transition : transitions)
                {
                    transition->onUpdate(blackBoard, dt);
                }
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

    const std::shared_ptr<Node>& getCurrent() const
    {
        return pCurrentNode;
    }

    void setCurrent(const std::shared_ptr<Node>& current)
    {
        init(current);
    }

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

        if (pCurrentNode->canUseTransition)
        {
            for (auto&& pNodeTransition : pCurrentNode->transitions)
            {
                assert(pNodeTransition != nullptr);

                if (pNodeTransition->canTransition(blackBoard))
                {
                    pCurrentNode->onExit(blackBoard);

                    assert(!pNodeTransition->to.empty());
                    std::shared_ptr<Node>& to = pNodeTransition->to[randNum(0, (int)pNodeTransition->to.size() - 1)];
                    assert(to != nullptr);
                    pCurrentNode = to;
                    pCurrentNode->onEnter(blackBoard);
                    break;
                }
            }
        }
    }
};