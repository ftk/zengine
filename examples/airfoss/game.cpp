#include "game.hpp"

void gamestate::on_input(tick_input_t input)
{
    IF_EVENT_(input.event, player_join)
    {
        LOGGER(info, "join", input.player);

        respawn(input.player);
    }
    else IF_EVENT_(input.event, player_leave)
    {
        assume(players.count(input.player));
        ecs.destroy(players[input.player]);
        players.erase(input.player);
    }
    else IF_EVENT(input.event, control, c)
    {
        if(!players.count(input.player))
            return;
        plane& p = ecs.get<plane>(players[input.player]);
        p.droll += c->x;
        p.dpitch += c->y;
    }
    else IF_EVENT_(input.event, shoot)
    {
        if(!players.count(input.player))
            return;
        plane& p = ecs.get<plane>(players[input.player]);
        auto bent = ecs.create();
        ecs.assign<expirable>(bent, SCFG(game.bullet.ttl, 60*3u) + input.tick);
        qvm::vec3 vel = p.rotd * qvm::_001() * SCFG(game.bullet.speed, 1.f);
        ecs.assign<bullet>(bent, p.position + vel, vel, input.player);
    }
    else IF_EVENT(input.event, engine, e)
    {
        if(!players.count(input.player))
            return;
        plane& p = ecs.get<plane>(players[input.player]);
        if(e->gas >= 2)
            p.afterburner = e->gas - 2;
        else if(e->gas >= 0)
            p.gas = e->gas;
        else
            p.thrust = -1.f;
    }

}

void gamestate::update(tick_t tick)
{
    ecs.view<plane>().each([this, tick](plane& p) {
        // check collision with enviroment
        if(enviroment_dist(p.position) < 1.f)
        {
            // death
            p.hp = -100;
        }

        p.update(rng);

        if(p.ai)
        {
            p.target_ai(select_ai_target(p));
        }

        // collision with bullets
        ecs.view<const bullet>().each([&p, this](const bullet& b) {
            if(qvm::mag_sqr(b.pos - p.position) < SCFG(game.bullet.hitboxr, 3.f))
            {
                p.hp--;
                scoreboard[b.owner]++;
            }
        });

        // regen hp every 4 seconds
        //if(tick % 240 == 0 && p.hp < 2)
            //p.hp++;

    });


    for(int i = 1; i <= SCFG(game.bots, 5); i++)
        if(tick % 60 == i)
            on_input({i, tick, event::shoot{}});


    ecs.view<expirable>().each([this, tick](auto ent, expirable& c) { if(c.expires == tick) ecs.destroy(ent); });
    ecs.view<bullet>().each([this](bullet& b) {
        b.pos += b.vel;
    });


    for(auto pp : players)
        if(ecs.get<plane>(pp.second).hp < -40)
        {
            respawn(pp.first);
            deaths[pp.first]++;
        }

}

float gamestate::enviroment_dist(qvm::vec3 p)
{
    using namespace qvm;
    using namespace std;

    auto pMod2 = [](vec2& p, vec2 size) -> vec2 {
        vec2 c = (p + size*0.5f), d;
        X(d) = floor(X(c)/X(size));
        Y(d) = floor(Y(c)/Y(size));
        X(p) = X(c) - X(size)*X(d) - X(size*0.5);
        Y(p) = Y(c) - Y(size)*Y(d) - Y(size*0.5);
        return d;
    };

    auto fBox = [](vec3 p, vec3 b) -> float {
        vec3 d = vec3{abs(X(p)), abs(Y(p)), abs(Z(p))} - b;
        
        vec3 dp = d, dn = d;
        X(dp) = max(X(dp), 0.f);
        Y(dp) = max(Y(dp), 0.f);
        Z(dp) = max(Z(dp), 0.f);
        X(dn) = min(X(dn), 0.f);
        Y(dn) = min(Y(dn), 0.f);
        Z(dn) = min(Z(dn), 0.f);
        
        return mag(dp) + max(X(dn), max(Y(dn), Z(dn)));
    };


    vec2 pp = XZ(p);
    vec2 q = pMod2(pp,vec2{11.5,11.5});
    XZ(p) = pp;

    const float TAU = 2 * M_PI;

    float h = 20.  *abs( sin(X(q)/TAU) * sin(Y(q)/TAU) + sin(X(q)/TAU * 3.) * sin(Y(q)/TAU * 2.));
    float d = Y(p) - h;
    //return d;

    //if(d > 4.5) return d;
    float box =  fBox(p,vec3{2.f,h,2.f});
    //if(Y(p) < 20.f)
        //box = std::min(box, 5.f);

    return min(box, Y(p));
}

qvm::vec3 gamestate::select_ai_target(const plane& p)
{
    qvm::vec3 nearest = qvm::vec3{0,40,0},
    direction = p.get_rot() * qvm::_001();
    float maxmetric = -1.f;

    // argmax(b) (cos(dir^b)+1)/|b|

    ecs.view<const plane>().each([&](const plane& p2) {
        if(p2.ai && SCFG(game.ai_allied, false))
            return;
        if(p.position == p2.position)
            return;
        float metric = (qvm::dot(direction, qvm::normalized(p2.position - p.position))+1) / qvm::mag(p2.position - p.position);
        if(metric > maxmetric)
        {
            maxmetric = metric;
            nearest = p2.position;
        }
    });
    return nearest;
}

gamestate::gamestate()
{
    for(int i = 0; i < SCFG(game.bots, 5); i++)
        on_input(tick_input_t{i+1, 0, event::player_join{}});
}

void gamestate::respawn(net_node_id player)
{
    if(players.count(player))
        ecs.destroy(players[player]);
    auto pent = ecs.create();
    players[player] = pent;

    ecs.assign<plane>(pent);
    plane& p = ecs.get<plane>(pent);

    p.position = qvm::vec3{std::uniform_real_distribution<float>{-400.f, 400.f}(rng),
                           60.f,
                           std::uniform_real_distribution<float>{-400.f, 400.f}(rng)};

    float angle = std::uniform_real_distribution<float>{-M_PI, M_PI}(rng);
    p.rotd = p.rotq = qvm::roty_quat(angle);

    if(player <= SCFG(game.bots, 5))
        p.ai = p.gas = true;

}

qvm::vec3 plane::euler_rot() const
{
    using namespace qvm;

    const vec3 forward = _001(), /*up = _010(),*/ right = _100();
    vec3 direction = rotd * forward;

    float yaw = atan2(X(direction), Z(direction));
    float pitch = acos(dot(roty_quat(yaw) * forward, direction));
    if(mag_sqr(roty_quat(yaw) * rotx_quat(pitch) * forward - direction) > 0.000001f)
        pitch = -pitch;
    float roll = acos(dot(roty_quat(yaw) * rotx_quat(pitch) * right, rotd * right));
    if(mag_sqr(roty_quat(yaw) * rotx_quat(pitch) * rotz_quat(roll) * right - rotd * right) > 0.00001f)
        roll = -roll;

    return vec3{roll, pitch, yaw};

}

void plane::update(minstd_rand& rng)
{
    using namespace qvm;
    const vec3 direction = rotd * _001();//get_rot() * _001();
    const vec3 velocity = speed * direction;

    const float aspeed = std::abs(speed);

    position += velocity;

    // apply smoothed rotation
    const float bias = SCFG(game.ema, 0.98f);

    rotd = slerp(rotd, rotq, 1.f - bias);

    // calculate air density
    const float alt = Y(position);
    const float density = 1.0f;// - alt / 600.f;


    {
        const float control = clamp(aspeed * density * SCFG(game.max.c, 0.2f), 0.f, 1.f);
        const float dr = SCFG(game.max.r, 0.1f),
        dy = SCFG(game.max.y, 0.1f),
        dp = SCFG(game.max.p, 0.1f);

        // apply delta rotation
        //rotate_zyx(rot, control * clamp(droll, -dr, dr), control * clamp(dyaw, -dy, dy), control * clamp(dpitch, -dp, dp));
        rotate_z(rotq, control * clamp(droll, -dr, dr));
        rotate_y(rotq, control * clamp(dyaw, -dy, dy));
        rotate_x(rotq, control * clamp(dpitch, -dp, dp));

        droll = dyaw = dpitch = 0;
    }

    // apply thrust
    float accel;
    {
        if(hp <= 0)
            thrust = -1.f;
        else if(gas)
            thrust += 1.f / 60.f / 5;
        else
            thrust -= 1.f / 60.f / 5;
        thrust = clamp(thrust, -1.f, 3.f);
        accel = clamp(thrust / 3.f, 0.f, 1.f) * SCFG(game.thrust, 1.f);
        if(afterburner) accel *= SCFG(game.afterburner, 3.f);
    }
    speed += accel;

    // apply drag
    float drag = speed * speed * density * SCFG(game.drag, .01f);
    if(speed > 0.f)
        speed -= drag;
    else
        speed += drag * 1.5f;
    //force  -= velocity *

    // apply gravity
    speed -= Y(direction) * SCFG(game.gravity, 0.f);

    // pull nose down
    if(aspeed < SCFG(game.control, 0.2f))
    {
        float s = SCFG(game.control, 0.2f) - aspeed;
        float dy = clamp(s * s, 0.f, SCFG(game.max.y, 0.1f)) * (1 - Y(direction)*Y(direction)); // what is this even? cos y ?
        float sign = (Y(direction) < SCFG(game.masscenter, 0.7f)) ? 1.f : -1.f; // pull nose down or up
        vec3 axis = cross(_010(), direction);
        //rot = rot_mat<3>(axis, dy * sign) * rot; //rotate along Y x dir axis
        rotq = rot_quat(axis, dy * sign) * rotq;
    }
    // turbulence
    else if(aspeed > SCFG(game.turbulence, 0.6f))
    {
        float s = aspeed - SCFG(game.control, 0.6f);
        s *= s / 4.f;


        droll += 4 * s * std::uniform_real_distribution{-0.5, 0.5}(rng);
        dyaw += s * std::uniform_real_distribution{-0.5, 0.5}(rng);
        dpitch += s * std::uniform_real_distribution{-0.5, 0.5}(rng);

    }
}

void plane::target_ai(qvm::vec3 target)
{
    if(target == position)
        return;

    using namespace qvm;


    const vec3 forward = _001();
    vec3 direction = rotd * forward;

    vec3 tdirection = normalized(target - position);

    vec3 diff = tdirection - direction;

    dpitch = -Y(diff);

    dyaw = -atan2( X(direction), Z(direction)) + atan2(X(tdirection), Z(tdirection));
    if(dyaw < -M_PI)
        dyaw += 2* M_PI;
    else if(dyaw > M_PI)
        dyaw -= 2* M_PI;
}

