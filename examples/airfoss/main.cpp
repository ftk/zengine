#include "game.hpp"

#include "util/serialization.hpp"
#include <cereal/cereal.hpp>
#include <cereal/types/unordered_map.hpp>


#include "util/semiconfig.hpp"

#include "components/input.hpp"
#include "components/window.hpp"
#include "components/resources.hpp"
#include "components/netgame.hpp"


#include "util/static_resource.hpp"


#include "main.hpp"

#include "util/log.hpp"

#include "opengl/stlloader.hpp"


#include <fstream>
//#include <cereal/archives/xml.hpp>
#include "gamestate.hpp"
#include "playercontroller.hpp"


#include "raudio.h"

class planeaudio
{

    Sound estart = LoadSound("resources/snd/start2.ogg");
    Sound eloop = LoadSound("resources/snd/loop3.ogg");
    Sound estop = LoadSound("resources/snd/stop2.ogg");
    int state = 0;


    Sound wind = LoadSound("resources/snd/ambient.ogg");
    Sound ab = LoadSound("resources/snd/afterburner.ogg");


    Sound cannon = LoadSound("resources/snd/autocannon_1p.wav");

    Sound breaking = LoadSound("resources/snd/glass_break.wav");

    int prev_hp = 0;


public:

    planeaudio() { InitAudioDevice(); }

    void shoot() { if(!IsSoundPlaying(cannon)) PlaySound(cannon); }

    void update(const plane& p)
    {
        float engine_vol = SCFG(sound.volume.engine, 0.5f);
        if(!state && p.thrust > -1.f)
        {
            PlaySound(estart);
            SetSoundVolume(estart, engine_vol);
            state++;
        }
        if(!IsSoundPlaying(estart) && !IsSoundPlaying(eloop)  && state == 1)
        {

            LOGGER(info, "playing loop");
            PlaySound(eloop);
            SetSoundLoop(eloop, true);
            SetSoundVolume(eloop, engine_vol);
        }

        if(state == 1)
        {
            if(p.thrust == -1.f)
            {
                StopSound(eloop);
                StopSound(estart);
                PlaySound(estop);
                SetSoundVolume(estop, engine_vol);

                state++;
            }
            else
            {
                SetSoundPitch(eloop, qvm::clamp(p.thrust / 3.f, 0.5f, 1.f));
            }
        }

        if(state == 2 && !IsSoundPlaying(estop))
        {
            state = 0;
        }


        if(!IsSoundPlaying(wind))
        {
            PlaySound(wind);
            SetSoundLoop(wind, true);
        }
        SetSoundVolume(wind, qvm::clamp(std::abs(p.speed) * 2.f, 0.f, 1.f));

        if(IsSoundPlaying(ab) != p.afterburner)
        {
            if(p.afterburner && state == 1)
            {
                PlaySound(ab);
                SetSoundLoop(ab, true);
            }
            else
                StopSound(ab);
        }

        if(prev_hp > p.hp)
            PlaySound(breaking);
        prev_hp = p.hp;


    }
};



class camera
{
public:
    qvm::mat4 proj;
    camera()
    {
        proj = qvm::perspective_lh<float>(M_PI/2.f, g_app->window->aspect(), SCFG(video.near, 1.f), SCFG(video.far, 1000.f));
    }
};

class controller
{
public:
    using gs = gamestate;//recorder_gamestate<gamestate>;
    multiplayer_game<gamestate_simulator2<
            gs
    >> game {g_app->netgame.get()};
public:
    camera cam;

    std::vector<net_node_id> hosts;

    controller()
    {
        g_app->netgame->on_event.connect([this](tick_input_t inp) {
            IF_EVENT_(inp.event, host_adv) hosts.push_back(inp.player);
        });

        plane_rnd.load("resources/mesh/SU35S-low.stl"); // 0
        plane_rnd.load("resources/mesh/bullet-low.stl"); // 1
    }

public:
    void join(net_node_id remote)
    {
        LOGGER(info, "sending join to", remote);
        game.join(remote);
    }

    void join_random()
    {
        if(hosts.empty())
            LOGGER(warn, "no hosts found, nodes connected:", g_app->netgame->nodes_list().size());
        else
        {
            stop();
            join(hosts[rand() % hosts.size()]);
        }
    }

    void host()
    {
        LOGGER(info, "starting game");
        game.host();

        g_app->netgame->on_event.connect([this](tick_input_t inp){
            //if(game.state != decltype(game)::HOST) return;
            IF_EVENT_(inp.event, node_connect)
            {
                // send host advertisment
                //=- register_event(name=>'host_adv');
                g_app->netgame->send_input(inp.player, {g_app->netgame->id(), 0, event::host_adv{}});
            }
        });

        g_app->netgame->send_input(g_app->netgame->nodes_list(), {g_app->netgame->id(), 0, event::host_adv{}});

    }

    void stop()
    {
        game.stop();
    }

    void draw(net_node_id);

    void send_event(event_t event)
    {
        if(game.playing())
            game.push_event(std::move(event));
    }

    //bool playing() const { return game.playing(); }
    planeaudio pa;

    stl_renderer plane_rnd {"resources/shd/plane.glsl"};
};

void controller::draw(net_node_id player)
{
    using namespace qvm;


    auto& gamestate = game.sim.gamestate();
    auto& ecs = gamestate.ecs;

    if(!gamestate.players.count(player))
        return;

    auto& env = static_resource<toy_renderer>::get(ID("resources/shd/env.glsl"));
    env.shader.bind();
    GET_UNIFORM(env.shader, u_sky) = static_resource<cubemap_texture>::get(ID("resources/sky/miramar_%%.png")).bind();

    const plane& p = ecs.get<plane>(gamestate.players[player]);

    GET_UNIFORM(env.shader, u_pos) = p.position;
    GET_UNIFORM(env.shader, u_inverse_view) = (p.get_rot());
    env.draw();

    plane_rnd.shader.bind();

    mat4 viewp = cam.proj * transposed(p.get_rot4()) * translation_mat(-p.position);

    ecs.view<plane>().each([&viewp, &p, this](const plane& p2){
        if(p2.position == p.position) return;
        GET_UNIFORM(plane_rnd.shader, u_mvp) = viewp * translation_mat(p2.position) * p2.get_rot4();
        plane_rnd.draw();
    });
    ecs.view<bullet>().each([&viewp, this](const bullet& b){
        GET_UNIFORM(plane_rnd.shader, u_mvp) = viewp * translation_mat(b.pos);
        plane_rnd.draw(1);
    });


    if(SCFG(debug, false))
    {
        auto rot = p.euler_rot();
        char txt[64];
        snprintf(txt, sizeof txt, "SPD: %.1f HP: %d D: %d\nP: %.0f %.0f %.0f\nR: %.0f %.0f %.0f",
                 p.speed, p.hp, (int)gamestate::enviroment_dist(p.position),
                 X(p.position), Y(p.position), Z(p.position),
                 X(rot) * 180 / M_PI, Y(rot) * 180 / M_PI, Z(rot) * 180 / M_PI);

        g_app->window->render_text_box({-1.f, -1.f}, {1., 0.15}, txt, 3);
    }

    if(SCFG(scoreboard, true))
    {
        std::string board;
        for(const auto& player_score : gamestate.scoreboard)
        {
            char txt[64];
            snprintf(txt, sizeof txt, "%10llu %3dH %3dD\n", player_score.first, player_score.second, gamestate.deaths[player_score.first]);
            board += txt;
        }
        g_app->window->render_text_box({0.5, -1.f}, {1., 0.05f * gamestate.scoreboard.size()}, board, gamestate.scoreboard.size());

    }

        // audio update
    pa.update(p);
}

int main( int argc, const char ** argv)
{
    using namespace qvm;
    z_main app(argc, argv);
    g_app->init_components();

    auto c = controller{};

    c.host();


    g_app->input->button("multiplayer", "F12").connect([&c](bool t) { if(t) c.join_random(); });



    gl::feature(GL_BLEND, SCFG(graphics.blend, true));
    gl::feature(GL_DEPTH_TEST, SCFG(graphics.depthtest, true));
    gl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl::ClearColor(0,0,0,0);

    //=- register_event(name=>'control', params=>[['float', 'x'], ['float', 'y']]);
    g_app->window->mouse_move_delta.connect([&c](qvm::vec2 delta) {
        c.send_event(event::control{-X(delta), Y(delta)});
    });

    //=- register_event(name=>'shoot');
    g_app->input->button("shoot", "MOUSE 0").connect([&c](bool t){
        if(t) {c.send_event(event::shoot{}); c.pa.shoot();};
    });
    //=- register_event(name=>'engine', params=>[['int', 'gas']]);

    g_app->input->button("accelerate", "W").connect([&c](bool t) {c.send_event(event::engine{t});});
    g_app->input->button("engine_off", "S").connect([&c](bool t) {if(t) c.send_event(event::engine{-1});});
    g_app->input->button("afterburner", "LEFT_SHIFT").connect([&c](bool t) {c.send_event(event::engine{t+2});});
    //g_app->input->button("left", "A").connect([&c](bool t) {c.dirz = -t;});
    //g_app->input->button("right", "D").connect([&c](bool t) {c.dirz = t;});

    g_app->input->button("mouse_capture", "LEFT_CONTROL").connect([](bool t) {if(t) g_app->window->toggle_cursor(false);});
    g_app->input->button("mouse_release", "LEFT_ALT").connect([](bool t) {if(t) g_app->window->toggle_cursor(true);});

        g_app->input->button("quit", "ESCAPE").connect([&c](bool t) {
        if(t)
        {
            g_app->window->close();
        }
    });



    g_app->window->draw.connect([&c]() {
        try
        {
            c.game.update();
            c.draw(g_app->netgame->id());
        }
        catch(const qvm::error& e)
        {
            LOGGER(error, "qvm error (div by zero?)", e.what());
        }


        if(SCFG(showfps, false))
        {
            static time_t t ;
            static unsigned frames = 0, fps = 0;
            if(time(nullptr) > t)
            {
                t = time(nullptr);
                fps = frames;
                frames = 0;
            }
            frames++;
            g_app->window->render_text_box({-1, 1 - 0.05}, {0.1, 0.05}, boost::lexical_cast<std::string>(fps));
        }

    });

    app.run();
}