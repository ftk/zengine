//
// Created by fotyev on 2017-02-15.
//

#ifndef ZENGINE_CONTROLLER_HPP
#define ZENGINE_CONTROLLER_HPP

#include "main.hpp"
#include "components/collections.hpp"
#include "components/window.hpp"
#include "components/netgame.hpp"
#include "components/script.hpp"
#include "gamestate.hpp"
#include "playercontroller.hpp"

#include "util/log.hpp"
#include "util/assert.hpp"

#include <unordered_map>
#include <bitset>
#include "modules/basic_module.hpp"

#include "opengl/math.hpp"

#include <memory>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/binary.hpp>

class world;

struct entity
{
    typedef unsigned int id_t;

    qvm::vec2 pos {0,0}, size {0, 0}, vel{0, 0};
    id_t id = 0; // id == 0 => entity will be destroyed next time

    SERIALIZABLE2(pos,size,vel,id)

    virtual void update(world& w, tick_t tick) = 0;
    virtual void draw() {};

    void destroy() { id = 0; }
};

class entity_manager
{
public:
    using id_t = entity::id_t;
private:
    id_t max_id = 1;
    std::vector<std::unique_ptr<entity>> entities;
public:
    SERIALIZABLE2(max_id, entities)

    id_t add(entity * ptr)
    {
        ptr->id = max_id;
        entities.push_back(std::unique_ptr<entity>{ptr}); // transfer ownership
        return max_id++;
    }

    template <class Entity = entity>
    Entity * get(id_t id) const
    {
        //assume(id != 0);
        if(id == 0)
            return nullptr;
        auto it = std::lower_bound(entities.begin(), entities.end(), id, [](const std::unique_ptr<entity>& ptr, id_t id){
            return ptr->id < id;
        });
        return it == entities.end() ? nullptr : dynamic_cast<Entity *>(it->get()); // TODO: static_cast
    }

    bool remove(id_t id)
    {
        auto e = get(id);
        if(!e) return false;
        e->id = 0;
        return true;
    }


    template <typename Callable>
    void for_each(const Callable& f)
    {
        for(auto it = entities.begin(); it != entities.end(); )
        {
            if((**it).id == 0)
            {
                it = entities.erase(it);
                continue;
            }
            f(**it);
            ++it;
        }
    }

    template <typename Callable>
    void for_each(const Callable& f) const
    {
        for(const auto& e : entities) f(*e);
    }
};

class world
{
    static const unsigned gw = 40, gh = 40;
    const qvm::vec2 size { 2.f / gw, 2.f / gh }; //blcck size

    std::bitset<gw * gh> w;

public:
    entity_manager ents;

    SERIALIZABLE2(w,ents)

    world()
    {

        for(int i = 0; i < gw; i++)
            w[i * gh + 1] = true;
    }

    void update(tick_t tick)
    {
        ents.for_each([this,tick] (entity& e) {
            e.update(*this, tick);
            // stop
            using namespace qvm;
            vec2 force = check_collision(e.pos, size);
            if(force != vec2{0,0})
                e.vel *= 0.5;
            e.vel += force;
        });
    }

    void draw()
    {
        ents.for_each([this] (entity& e) { e.draw(); });

        auto& texture = g_app->textures->get_from_file("resources/block.png");

        using namespace qvm;

        for(int i = 0; i < gw; i++)
        {
            for(int j = 0; j < gh; j++)
            {
                if(w[i * gh + j])
                {
                    vec2 pos = {-1, -1};
                    X(pos) += X(size) * i;
                    Y(pos) += Y(size) * j;

                    g_app->window->render.copy2(texture, pos, size);
                }
            }

        }
    }

    bool solid(qvm::vec2 pos) const
    {
        using namespace qvm;
        pos += vec2{1, 1};
        int bx = X(pos) / X(size);
        int by = Y(pos) / Y(size);
        if(bx < 0 || bx >= (int)gw || by < 0 || by >= (int)gh) return true;
        return w[bx * gh + by];
    }

    void flip(qvm::vec2 pos)
    {
        using namespace qvm;
        pos += vec2{1, 1};
        int bx = X(pos) / X(size);
        int by = Y(pos) / Y(size);
        if(bx < 0 || bx >= (int)gw || by < 0 || by >= (int)gh) return;
        w[bx * gh + by] = !w[bx * gh + by];
    }

    // pos is center
    qvm::vec2 check_collision(qvm::vec2 pos, qvm::vec2 size) // returns vector of counter force is object is stuck somewhere, or 0,0
    {
        using namespace qvm;
        vec2 f {0,0 };
        unsigned c = 0;
        if(solid(pos)) // bottom left corner stuck
        {
            f += vec2{1, 1};
            c++;
        }
        if(solid(pos + X0(size))) // bottom right
        {
            f += vec2{-1, 1};
            c++;
        }
        if(solid(pos + _0Y(size))) // up left
        {
            f += vec2{1, -1};
            c++;
        }
        if(solid(pos + size)) // up right
        {
            f += vec2{-1, -1};
            c++;
        }

        if(!c)
            return vec2{0, 0};

        if(f == vec2{0, 0} && c)
            f = -pos; // push to 0,0 if all 4 corners are stuck
        return normalized(f) / 500;
    }

};

class character : public entity
{
public:
    int dir = 0;
    SERIALIZABLE2(cereal::base_class<entity>(this), dir)

    character()
    {
        this->size = qvm::vec2{0.05, 0.05};
    }

    void draw() override
    {
        auto& texture = g_app->textures->get_from_file("resources/zee.png");
        g_app->window->render.copy2(texture, pos, size);
    }

    void update(world& w, tick_t tick) override
    {
        using namespace qvm;
        pos += vel;

        // friction...
        vel *= 0.96;

        // gravity
        vel += vec2{0,-0.0005};


        if(dir && abs(X(vel)) < 0.004)
            X(vel) += dir / 400.f;
    }

    //=- register_event(name=>'movement', params=>[[qw(int32_t x)], [qw(int32_t y)], [qw(uint8_t down)]]);
    void move(event::movement m)
    {
        if(m.y == 0)
        {
            dir = m.down ? m.x : 0;
        }
        else if(m.down)
        {
            qvm::Y(vel) += m.y / 70.f;
        }
    }
};

class bullet : public entity
{
public:
    SERIALIZABLE2(cereal::base_class<entity>(this))

    bullet()
    {
        this->size = qvm::vec2{0.005, 0.005};
    }

    void draw() override
    {
        auto& texture = g_app->textures->get_from_file("resources/bullet.png");
        g_app->window->render.copy2(texture, pos, size);
    }

    void update(world& w, tick_t tick) override
    {
        using namespace qvm;
        pos += vel;
        vel += vec2{0, -0.00001}; // slight gravity
    }

};

CEREAL_REGISTER_TYPE(character)
CEREAL_REGISTER_TYPE(bullet)

class character_manager
{
    // player_id -> character_id
    std::unordered_map<net_node_id, entity::id_t> players;

public:
    SERIALIZABLE((players))

    character * get(entity_manager& e, net_node_id player) const
    {
        return e.get<character>(players.count(player)?players.at(player):0);
    }

    void kill_char(character * ch)
    {
        if(!ch) return;
        for(auto it = players.begin(); it != players.end(); ++it)
        {
            if(it->second == ch->id)
            {
                players.erase(it);
                return;
            }
        }
        ch->destroy();
    }

    void spawn_char(entity_manager& ents, net_node_id player)
    {
        players[player] = ents.add(new character{});
    }


    void on_input(entity_manager& ents, const tick_input_t& input)
    {
        if(boost::get<event::player_join>(&input.event))
        {
            // new player
            spawn_char(ents, input.player);
        }
        else if(boost::get<event::player_leave>(&input.event))
        {
            kill_char(get(ents, input.player));
        }
        else if(const auto * ev = boost::get<event::movement>(&input.event))
        {
            if(auto * c = get(ents, input.player))
                c->move(*ev);
        }
    }
};

class zwstate
{
public:
    world w;
    character_manager chrs;

    zwstate()
    {
    }

    void update(tick_t tick)
    {
        w.update(tick);
    }

    void draw()
    {
        // draw world
        w.draw();
    }

    void on_input(const tick_input_t& input)
    {
        chrs.on_input(w.ents, input);
        //=- register_event(name=>'click', params=>[[qw(float x)], [qw(float y)]]);
        if(const event::click * ev = boost::get<event::click>(&input.event))
        {
            w.flip({ev->x, ev->y});
        }
        //=- register_event(name=>'shoot', params=>[[qw(float x)], [qw(float y)]]);
        if(const auto * ev = boost::get<event::shoot>(&input.event))
        {
            auto b = new bullet{};
            auto chpos = chrs.get(w.ents, input.player)->pos;
            b->vel = {ev->x, ev->y};
            b->vel = qvm::normalized(b->vel - chpos) * 0.01;
            b->pos = chpos + b->vel * 10;
            w.ents.add(b);
        }

    }

    SERIALIZABLE((w)(chrs))
};

/*< register_module(name=>'controller', class=>'controller', scriptexport=>[qw(join host stop send_event playing)]); >*/
class controller : public basic_module
{
    multiplayer_game<gamestate_simulator2<zwstate>> game {g_app->netgame.get()};

    decltype(game.state) prevstate = game.state;

public:
    void join(net_node_id remote)
    {
        LOGGER(info, "sending join to", remote);
        game.join(remote);
    }

    void host()
    {
        LOGGER(info, "starting game");
        game.host();
    }

    void stop()
    {
        game.stop();
    }

    void draw() override
    {
        game.sim.draw();
        game.update();

        auto newstate = game.state;

        if(prevstate != newstate)
        {
            switch(newstate)
            {
                case decltype(game)::CLIENT:
                    {
                        //=- register_callback('void ()', 'on_connect');
                        g_app->script->on_connect();
                    }break;
                case decltype(game)::STOPPED:
                    {
                        //=- register_callback('void ()', 'on_disconnect');
                        g_app->script->on_disconnect();
                    }break;
                case decltype(game)::HOST:
                    {
                        //=- register_callback('void ()', 'on_host');
                        g_app->script->on_host();
                    }break;
                default: break;
            }
        }

        prevstate = newstate;
    }

    void send_event(event_t event)
    {
        if(game.playing())
            game.push_event(std::move(event));
    }

    bool playing() const { return game.playing(); }

};
#endif //ZENGINE_CONTROLLER_HPP
