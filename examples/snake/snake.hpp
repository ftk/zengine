#ifndef ZENGINE_SNAKE_HPP
#define ZENGINE_SNAKE_HPP

//#include "cute_headers/cute_c2.h"

#include "entt/entity/registry.hpp"

#include "components/window.hpp"

#include <cereal/cereal.hpp>
//#include <cereal/archives/xml.hpp>
#include <cereal/types/unordered_map.hpp>

#include <boost/container/small_vector.hpp>

#include <random>
#include <algorithm>
#include "events.hpp"

#include "playercontroller.hpp"
#include "gamestate.hpp"
#include "main.hpp"

#include "components/config.hpp"
#include "components/resources.hpp"

#include "util/log.hpp"
#include "util/assert.hpp"


#include "geometry.hpp"

using reg_t = entt::registry<uint32_t>;
using entity_t = reg_t::entity_type;


struct snake_part_c
{
    // snake part always has segment_c
    entity_t next_part = entt::null; // next part towards tail
    entity_t prev_part = entt::null; // next part towards head

    SERIALIZABLE(next_part, prev_part)
};

struct snake_char_c
{
    // this entity signnifies snake's head
    entity_t tail;
    float speed = 0.001;

    SERIALIZABLE(tail, speed)
};

struct dead_snake_c
{
    SERIALIZABLE()
};

struct food_c
{
    vec2 pos;

    SERIALIZABLE(pos)
};

class snakegame
{
private:
    reg_t ecs;
    static constexpr float snake_width = 0.02;
    static constexpr float food_size = 0.03;


    // player -> char
    //std::unordered_map<net_node_id, entity_t> players;
    boost::container::flat_map<net_node_id, int> player_food;

    minstd_rand rng;

public:
    ENTT_SERIALIZABLE(ecs, (segment_c, snake_part_c, snake_char_c, dead_snake_c, food_c), (player_food,rng))


    snakegame()
    {
        // make walls
        ecs.assign<segment_c>(ecs.create(), vec2{-1, -1}, vec2{0, 2}); // left
        ecs.assign<segment_c>(ecs.create(), vec2{-1, -1}, vec2{2, 0}); // bottom
        ecs.assign<segment_c>(ecs.create(), vec2{1, 1}, vec2{-2, 0}); // top
        ecs.assign<segment_c>(ecs.create(), vec2{1, 1}, vec2{0, -2}); // right

    }

    void update(tick_t tick)
    {
        boost::container::small_vector<entity_t, 3> to_delete_tail;
        for(auto snake : ecs.view<snake_char_c>())
        {
            // snake - snake's head

            segment_c& snake_pos = ecs.get<segment_c>(snake);
            auto& snake_char = ecs.get<snake_char_c>(snake);

            // increase speed
            snake_char.speed += 0.000001;

            // extend head part
            snake_pos.dir += normalized(snake_pos.dir) * snake_char.speed;

            vec2 head = snake_pos.pos + snake_pos.dir;
            // check for collisions with food items...
            bool food_seek = false;
            for(auto food_ent : ecs.view<food_c>())
            {
                if(mag(ecs.get<food_c>(food_ent).pos - head) < (snake_width + food_size) / 2)
                {
                    // eat
                    ecs.destroy(food_ent);
                    food_seek = true;
                }
            }

            // check for collisions with other segments, incl itself
            bool collision = false;
            vec2 reflected;
            entity_t next_snake_part = ecs.get<snake_part_c>(snake).next_part;
            entity_t next_next_snake_part = entt::null;
            if(next_snake_part != entt::null)
                next_next_snake_part = ecs.get<snake_part_c>(next_snake_part).next_part;

            for(auto seg : ecs.view<segment_c>())
            {
                // snake can turn only 90 deg, so it cant hit head segment, or the following 2 segments
                if(seg != snake && seg != next_snake_part && seg != next_next_snake_part)
                {
                    const segment_c& segment = ecs.get<segment_c>(seg);
                    auto p = intersect_lines(segment, snake_pos);
                    //if(distance_to_segment(segment, head) < snake_width)
                    if(p.first > 0.f && p.first < 1.f && p.second > 0.f &&
                            p.second < (1.f + snake_width / snake_pos.length()))
                    {

                        //float t2 = intersect_lines(snake_pos, segment).second;
                        //vec2 intersect_point = segment.pos + t2 * segment.dir;

                        vec2 normal = rotate_left(normalized(segment.dir));

                        float product = dot(normalized(snake_pos.dir), normal);
                        // using 2.1 instead of 2 somehow makes snake not bounce within the segment
                        reflected = normalized(snake_pos.dir) - 2.1 * product * normal;
                        collision = true;

                    }
                }
            } // segment loop end
            // can only turn snake once and only within this loop
            if(collision)
            {
                // if angle < 90 deg
                if(dot(snake_pos.dir, reflected) > 0)
                {
                    snake_pos.dir -= normalized(snake_pos.dir) * snake_char.speed; // undo snake extension
                    turn_snake(reflected, snake);
                    LOGGER(info, "snake reflected, angle:",acos(dot(normalized(snake_pos.dir), normalized(reflected)))*180./3.1415);
                }
                else
                {
                    LOGGER(info, "game over?");
                    ecs.remove<snake_char_c>(snake);
                    ecs.assign<dead_snake_c>(snake);
                    add_snake();
                }
            }
            else if(food_seek)
            {
                snake_pos.dir += normalized(snake_pos.dir) * food_size; // extend snake
                head = snake_pos.pos + snake_pos.dir;
                vec2 food = seek_food(head);
                // check if snake can turn to it (<90 deg)
                vec2 dir = food - head;
                if(food != vec2{0,0} && dot(snake_pos.dir, dir) > 0) // cos(a) > 0 <=> a < 90 deg
                {
                    turn_snake(dir, snake);
                }
            }
            else
            {
                // decrease tail part
                auto& tail_pos = ecs.get<segment_c>(snake_char.tail);
                if(snake_char.tail != snake && tail_pos.length() < snake_char.speed)
                {
                    // remove tail
                    to_delete_tail.push_back(snake);
                }
                else
                {
                    tail_pos.pos += normalized(tail_pos.dir) * snake_char.speed;
                    tail_pos.dir -= normalized(tail_pos.dir) * snake_char.speed;
                }
            }

        } // snake loop end
        // delete snake tail segments, cant do that in snake loop
        for(auto snake : to_delete_tail)
        {
            entity_t tail_ent = ecs.get<snake_char_c>(snake).tail;
            entity_t new_tail_ent = ecs.get<snake_part_c>(tail_ent).prev_part;


            ecs.get<snake_part_c>(new_tail_ent).next_part = entt::null;
            ecs.get<snake_char_c>(snake).tail = new_tail_ent;

            ecs.destroy(tail_ent);
        }

        // npc will put out random food every 4 seconds
        if(tick % (TICKS_PER_SECOND * 4) == 0)
        {
            on_input({0,tick,event::new_food{std::uniform_real_distribution<float>{-1, 1}(rng),
                                             std::uniform_real_distribution<float>{-1, 1}(rng)}});
        }

        // give players 1 food every 2 seconds
        if(tick % (2*TICKS_PER_SECOND) == 0)
            for(auto& p : player_food)
                p.second++;
    }

    void on_input(tick_input_t input)
    {
        using namespace qvm;

        IF_EVENT_(input.event, player_join)
        {
            if(player_food.empty()) // seed rng by first player id
                rng.seed(input.player);

            add_snake();

            // give player 10 food items at the start
            player_food[input.player] = 10;

            LOGGER(info, "player joined:", input.player);
        }

        else IF_EVENT_(input.event, player_leave)
        {
            player_food.erase(input.player);
        }

        else IF_EVENT(input.event, new_food, f)
        {
            if(input.player && player_food[input.player] > 0)
                player_food[input.player]--;
            else if(input.player)
                return;

            auto food = ecs.create();
            vec2 pos{f->x, f->y};
            ecs.assign<food_c>(food, pos);


            // try to turn all the snakes
            for(auto snake : ecs.view<snake_char_c>())
            {
                segment_c& snake_pos = ecs.get<segment_c>(snake);
                vec2 head = snake_pos.pos + snake_pos.dir;

                // find the closest food item for this snake
                // is it this one
                if(seek_food(head) == pos)
                {
                    // check if snake can turn to it (<90 deg)
                    vec2 dir = pos - head;
                    if(dot(snake_pos.dir, dir) > 0) // cos(a) > 0 <=> a < 90 deg
                    {
                        turn_snake(dir, snake);
                    }
                    else
                        LOGGER(info, "cant turn snake");
                }

            }

        }
        else IF_EVENT_(input.event, make_new_snake)
        {
            if(input.player && player_food[input.player] >= 10)
            {
                player_food[input.player] -= 10;
                add_snake();
            }
        }


    }

    entity_t add_snake()
    {
        auto e = ecs.create();
        vec2 pos = select_best_space();
        ecs.assign<segment_c>(e, pos, 0.001 * normalized(seek_food(pos) - pos));
        ecs.assign<snake_part_c>(e);
        ecs.assign<snake_char_c>(e, e);
        LOGGER(info, "added snake", e, "@", pos);
        return e;
    }

    vec2 seek_food(vec2 head) const
    {
        // find the closest food item for this snake
        float dist2 = 999.;
        vec2 closest{0,0};
        ecs.view<const food_c>().each([&closest,&dist2,head](auto, const auto& food) {
            float d2 = qvm::mag_sqr(food.pos - head);
            if(d2 < dist2)
            {
                closest = food.pos;
                dist2 = d2;
            }
        });
        return closest;
    }

    void turn_snake(vec2 dir, entity_t snake)
    {
        segment_c& snake_pos = ecs.get<segment_c>(snake);
        vec2 head = snake_pos.pos + snake_pos.dir;
        // now turn it... add a new segment
        auto e = ecs.create();
        ecs.assign<segment_c>(e, head, normalized(dir) * (ecs.get<snake_char_c>(snake).speed));

        ecs.assign<snake_part_c>(e, snake);
        ecs.get<snake_part_c>(snake).prev_part = e;

        ecs.assign<snake_char_c>(e, ecs.get<snake_char_c>(snake));
        ecs.remove<snake_char_c>(snake);
    }

    vec2 select_best_space(unsigned tries = 10)
    {
        // find point furthest away from snakes
        vec2 p {0, 0};
        float dist = 0;
        while(tries--)
        {

            vec2 pp { std::uniform_real_distribution<float>{-0.9, 0.9}(rng),
                      std::uniform_real_distribution<float>{-0.9, 0.9}(rng)};
            float mindist = 999;
            for(auto ent : ecs.view<const snake_part_c>())
            {
                const auto& seg = ecs.get<const segment_c>(ent);
                float d = distance_to_segment(seg, pp);
                if(d < mindist)
                    mindist = d;
            }
            if(mindist > dist)
            {
                dist = mindist;
                p = pp;
            }
        }
        return p;
    }

    void draw(net_node_id player) const;
};





#endif //ZENGINE_ZW_HPP
