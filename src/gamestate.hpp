//
// Created by fotyev on 2016-10-15.
//

#ifndef ZENGINE_GAMESTATE_HPP
#define ZENGINE_GAMESTATE_HPP

#include "playerinputs.hpp"

#include "entity.hpp"

#include <vector>
#include <memory>

class gamestate_t /*: public entity_t*/
{
public:
    using entity_ptr = std::unique_ptr<entity_t>;
private:
    std::vector<entity_ptr> entities;

public:

    gamestate_t();

    void update(tick_t tick);
    //void update(players_tick_input_t& t);
    void on_input(tick_input_t& input);

    void draw();


    template <class Entity, typename... Args>
    void emplace(Args&&... args)
    {
        entities.emplace_back(std::make_unique<Entity>(std::forward<Args>(args)...));
    }
    void insert(entity_ptr ent)
    {
        entities.push_back(std::move(ent));
    }

    bool remove(entity_t * ent_ptr);

protected:
    // serialize

    /*template <typename Archive>
    virtual void serialize(Archive& ar, const unsigned) = 0;*/
};

class gamestate_simulator
{
public:
    tick_t simulated = 0;
private:
    inputs_t inputs;

private:
    // TODO: add newstate
    gamestate_t oldstate;

public:

    gamestate_t& state() { return oldstate; }

    gamestate_simulator() {};

    void push(tick_input_t ev);

    void update();

    void draw()
    {
        oldstate.draw();
    }

};

#endif //ZENGINE_GAMESTATE_HPP
