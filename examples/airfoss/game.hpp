#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <random>
#include "opengl/math.hpp"
#include "util/semiconfig.hpp"
#include "util/string.hpp"

#include "util/serialization.hpp"

#include "util/assert.hpp"


// just for serialization
class game_config
{
public:
    template <class Archive>
    void save(Archive& ar) const
    {
        std::unordered_map<std::string, std::string> cfg;
        static_config::for_each([&cfg](std::string k, std::string v){
            if(begins_with(k, "game."))
                cfg[std::move(k)] = std::move(v);
        });
        ar(cfg);
    }

    template <class Archive>
    void load(Archive& ar)
    {
        std::unordered_map<std::string, std::string> cfg;
        ar(cfg);
        for(auto& kv : cfg)
        {
            if(begins_with(kv.first, "game."))
                static_config::set(kv.first, std::move(kv.second));
        }
    }
};


class plane
{
public:
    //qvm::mat3 rot = qvm::identity_mat3();
    //qvm::mat3 rot_ema = qvm::identity_mat3(); // exp mov avg

    qvm::quat rotq = qvm::identity_quat<float>();
    qvm::quat rotd = qvm::identity_quat<float>();

    float dyaw = 0.0, dpitch = 0.0, droll = 0.0;

    qvm::vec3 position {10,40,10};

    float speed = .6;

    float thrust = -1.f;
    bool gas = false;
    bool afterburner = false;

    int hp = 2; // <= 0  -> engine stops working

    bool ai = false; // used by gamestate

    SERIALIZABLE(rotq, rotd, dyaw, dpitch, droll, position, speed, thrust, gas, afterburner)



    qvm::mat3 get_rot() const
    {
        return qvm::convert_to<qvm::mat3>(rotd);
    }
    qvm::mat4 get_rot4() const
    {
        return qvm::convert_to<qvm::mat4>(rotd);
    }

    qvm::vec3 euler_rot() const;

    void update(minstd_rand& rng);

    void target_ai(qvm::vec3 target);


};

#include "entt/entity/registry.hpp"
#include "entt/entity/snapshot.hpp"
#include "util/movable.hpp"
#include "playerinputs.hpp"


using reg_t = entt::registry;
using entity_t = reg_t::entity_type;

struct expirable
{
    tick_t expires;
    SERIALIZABLE(expires)
};

struct bullet
{
    qvm::vec3 pos, vel;
    net_node_id owner;
    SERIALIZABLE(pos, vel, owner)
};

class gamestate
{
    game_config cfg;


public:
    reg_t ecs;
    minstd_rand rng;

    // players -> plane entity
    std::unordered_map<net_node_id, entity_t> players;

    std::unordered_map<net_node_id, int> scoreboard;
    std::unordered_map<net_node_id, int> deaths;

    gamestate();
    ENTT_COPY_SERIALIZABLE(gamestate,
            ecs, (plane, expirable, bullet),
            (cfg, players, rng, scoreboard, deaths)
            )

    void update(tick_t tick);

    void on_input(tick_input_t input);

    static float enviroment_dist(qvm::vec3 p);

    qvm::vec3 select_ai_target(const plane& p);

    void respawn(net_node_id player);

    plane& get_plane(net_node_id player) { assume(players.count(player)); return ecs.get<plane>(players[player]); }

};

