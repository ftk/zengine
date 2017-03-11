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
protected:
    std::vector<entity_ptr> entities;
    friend struct entity_t;
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
        entities.back()->gamestate = this;
    }
    void insert(entity_ptr ent)
    {
        entities.push_back(std::move(ent));
        entities.back()->gamestate = this;
    }

    bool remove(entity_t * ent_ptr);

public:
    // serialize

    template <typename Archive>
    void serialize(Archive& ar, const unsigned);

    void operator = (gamestate_t& rhs);
};

class old_input_exc {};

class gamestate_simulator
{
public:
    tick_t lag = 1;
private:
    tick_t simulated_old = 0;
    tick_t simulated_new = 0; // invariant: should always be simulated_old + lag
    volatile bool newstate_invalidated = true;
    inputs_t inputs;

private:
    gamestate_t oldstate;
    gamestate_t newstate;

public:

    gamestate_t& state() { return oldstate; }

    gamestate_simulator() {}

    void start()
    {
        simulated_old = simulated_new = get_tick();
    }

    void push(tick_input_t ev);

    void update();

    void draw()
    {
        oldstate.draw();//newstate.draw();
    }

};

#endif //ZENGINE_GAMESTATE_HPP
