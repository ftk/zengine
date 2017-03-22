//
// Created by fotyev on 2017-03-13.
//

#ifndef ZENGINE_CONTROLLER_HPP
#define ZENGINE_CONTROLLER_HPP

#include "entity.hpp"

#include "main.hpp"
#include "components/collections.hpp"
#include "components/window.hpp"
#include "components/netgame.hpp"
#include "components/script.hpp"
#include "gamestate.hpp"
#include "playercontroller.hpp"

#include <vector>

#include <random>
#include <boost/lexical_cast.hpp>

#include "util/log.hpp"
#include "util/assert.hpp"



struct gamefield
{
    unsigned w, h;
    unsigned colors = 5;
    std::vector<std::vector<int>> field;

    gamefield(unsigned w, unsigned h) : w(w), h(h), field(w,std::vector<int>(h,0))
    {
    }

    texture& get_texture(unsigned color) const
    {
        assume(color < colors);
        return g_app->textures->get("ff tex"_fnv + color,
                                    [color](){ return textures_c::load_file("resources/" + std::to_string(color+1) + ".png"); });
    }

    void draw()
    {
        using namespace qvm;
        for(unsigned i = 0; i < w; i++)
        {
            for(unsigned j = 0; j < h; j++)
            {
                const qvm::vec2 size {1.f/w, 1.f/h};
                g_app->window->render.copy2(get_texture(field[i][j]), vec2{X(size) * i, Y(size) * j}, size, nullopt);
            }
        }
    }

    template <typename Callable1, typename Callable2>
    void floodfill(int x, int y, Callable1 f1, Callable2 f2)
    {
        int color = field[x][y];
        std::vector<bool> visited(w*h, false);
        std::function<void(unsigned,unsigned)> dfs = [color, &f1, &f2, &visited, this, &dfs](unsigned x, unsigned y) {
            if(x >= w || y >= h || visited[x*h+y])
                return;
            visited[x*h+y] = true;
            if(field[x][y] != color)
            {
                f2(x,y);
                return;
            }
            f1(x,y);
            dfs(x+1,y);
            dfs(x-1,y);
            dfs(x,y+1);
            dfs(x,y-1);
        };
        dfs(x,y);
    }

    void init(int seed)
    {
        std::minstd_rand generator{seed};
        std::uniform_int_distribution<int> distribution(0,4);
        for(unsigned i = 0; i < w; i++)
        {
            for(unsigned j = 0; j < h; j++)
            {
                field[i][j] = distribution(generator);
            }
        }

    }
};

class player
{
public:
    gamefield& gf;
    net_node_id id;

    unsigned int posx, posy;

    int stats[5] = {0};
    int score = 0;

    player(gamefield& gf, net_node_id id, unsigned posx, unsigned posy) : gf(gf), id(std::move(id)), posx(posx), posy(posy)
    {
        update_stats();
    }

    void update_stats()
    {
        for(int i = 0; i < 5; i++) stats[i] = 0;
        score = 0;
        gf.floodfill(posx, posy,
                      [this](int,int){score++;},
                      [this](int x,int y){stats[gf.field[x][y]]++;}
        );
    }

    void make_turn(unsigned color)
    {
        assume(color < gf.colors);
        gf.floodfill(posx, posy,
                      [color,this](int x, int y) { gf.field[x][y] = color; },
                      [color,this](int x, int y) {}
        );
        update_stats();
    }
};


class floodfill_game
{
    gamefield gf;
    std::vector<player> players;
    int turn = 0;
public:
    floodfill_game(net_node_id id1, net_node_id id2) : gf(40, 40)
    {
        gf.init(id1 + id2);
        players.push_back({gf, id1, 0, 0});
        players.push_back({gf, id2, gf.w-1, gf.h-1});
    }

    void on_input(const tick_input_t& input)
    {
        //=- register_event(name=>'colorize', params=>[[qw(uint32_t c)]]);
        if(auto e = boost::get<event::colorize>(&input.event))
        {
            if(input.player == players[turn].id)
            {
                players[turn].make_turn(e->c);
                turn ^= 1;
            }
        }
    }
    void draw()
    {
        using namespace qvm;
        for(auto p = 0; p < 2; p++)
        {
            for(unsigned i = 0; i <= gf.colors; i++)
            {
                vec2 size{0.03, 0.03},
                        pos{p ? 0.8f : -0.9f, (p?0.8f:-0.5f) - Y(size) * i * 2};
                if(i == gf.colors)
                {
                    if(p == turn)
                    {
                        auto &texture = g_app->textures->get("score"_fnv + players[p].score,
                                                             g_app->fonts->lazy_render("Score: " + std::to_string(players[p].score)));

                        g_app->window->render.copy_y(texture, pos, Y(size));
                    }
                    break;
                }
                g_app->window->render.copy2(gf.get_texture(i), pos, size);
                X(pos) += X(size) * 2;

                int score = players[p].stats[i];
                auto &texture = g_app->textures->get(players[p].stats[i],
                                                     g_app->fonts->lazy_render(boost::lexical_cast<std::string>(score)));

                g_app->window->render.copy2(texture, pos, size, nullopt);

            }
        }
        {
            renderer_2d::transform_t t{g_app->window->render, renderer_2d::transform_t::rect({-0.7, -0.8}, {1.4, 1.6})};
            gf.draw();
        }
    }
    void update(tick_t)
    {
    }
};

class gamecontroller
{
    net_node_id local, remote;
    gamestate_simulator<floodfill_game> sim;
public:
    gamecontroller(net_node_id local, net_node_id remote) : local(local), remote(remote),
                                                            sim(std::min(local, remote),
                                                                std::max(local, remote))
    {
        LOGGER(info, "game started!!!", remote);
        assume(remote != 0 && local != 0);
        assume(local != remote);

        //=- register_callback('void (uint64_t)', 'on_game_start');
        g_app->script->on_game_start(remote);
    }



    bool on_event(const SDL_Event& ev)
    {
        switch(ev.type)
        {
            case SDL_KEYDOWN:
                switch(ev.key.keysym.scancode)
                {
                    case SDL_SCANCODE_1:
                        push_local(event::colorize{0}); return false;
                    case SDL_SCANCODE_2:
                        push_local(event::colorize{1}); return false;
                    case SDL_SCANCODE_3:
                        push_local(event::colorize{2}); return false;
                    case SDL_SCANCODE_4:
                        push_local(event::colorize{3}); return false;
                    case SDL_SCANCODE_5:
                        push_local(event::colorize{4}); return false;
                    default:
                        break;
                }break;

        }

        return true;
    }

    void draw()
    {
        try
        {
            sim.update();
        }
        catch(old_input_exc)
        {
            push_local(event::disconnect{});
//            throw stop_game{};
        }
        sim.draw();
    }

    void push_local(event_t event)
    {
        auto input = tick_input_t{g_app->netgame->id(), get_tick(), event};
        g_app->netgame->send_event(remote, event);
        sim.push(std::move(input));
    }

    template<typename T>
    void on_netevent(const tick_input_t& input, T event)
    {
        sim.push(input);
    }

    ~gamecontroller()
    {
        LOGGER(info, "player disconnected :(");
        // this is called from network thread, no crazy logic here :(
    }
};


/*< register_module(name=>'controller', class=>'controller', scriptexport=>[qw(startgame)]); >*/
class controller : public basic_module
{
    two_player_game<gamecontroller> game {g_app->netgame.get()};
    bool started = false;
public:
    void startgame(net_node_id remote)
    {
        game.send_request(remote);
    }

    bool on_event(const SDL_Event& ev) override
    {
        if(game.gamecontroller)
            return game.gamecontroller->on_event(ev);
        return true;
    }

    void draw() override
    {
        if((bool)game.gamecontroller != started)
        {
            started = (bool)game.gamecontroller;
            //=- register_callback('void ()', 'on_disconnect');

            if(!started)
                g_app->script->on_disconnect();
        }
        if((bool)game.gamecontroller)
            game.gamecontroller->draw();
    }

};

#endif //ZENGINE_CONTROLLER_HPP
