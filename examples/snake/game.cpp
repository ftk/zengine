
#include "gamestate.hpp"
#include "snake.hpp"
#include "main.hpp"

#include "util/static_resource.hpp"

void snakegame::draw(net_node_id player)
{
    // draw bg
    auto& bg = static_resource<toy_renderer>::get(ID("resources/shd/coolbg.glsl"));
    bg.shader.bind();
    static_resource<texture>::get(ID("resources/cooltoybgtex.jpg")).bind(0);

    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GET_UNIFORM(bg.shader, u_texture) = qvm::ivec1{0};
    GET_UNIFORM(bg.shader, u_time) = qvm::vec1{(float) glfwGetTime()};
    bg.draw();

    using namespace qvm;

    auto draw_snake = [this](entity_t snake, texture& tex) -> float {
        // draw snake ... from head to tail
        float total_length = 0.f;
        float t = 0.0;
        for(entity_t ent = snake; ent != entt::null; ent = ecs.get<snake_part_c>(ent).next_part)
        {
            const auto& segment = ecs.get<segment_c>(ent);

            vec2 pos = segment.pos + segment.dir;
            float len = segment.length();
            t += len;

            for(; t > 0.f; t -= snake_width)
            {
                vec2 size = {snake_width/* / g_app->window->aspect()*/, snake_width};
                pos = segment.pos + t * segment.dir / len;
                g_app->window->render.copy(
                        tex,
                        pos - size/2,
                        size
                );

                //LOGGER(debug2, "drawing snake circle at", X(pos), Y(pos));

            }
            total_length += len;
        }
        return total_length;
    };

    float total_result = 0;
    for(auto snake : ecs.view<snake_char_c>())
    {
        float len = draw_snake(snake, static_resource<texture>::get(ID("resources/sn2.png")));
        const auto& segment = ecs.get<segment_c>(snake);

        total_result += len;
//#ifndef NDEBUG
        // draw score
        char txt[64];
        snprintf(txt, 64, "LEN: %.1f", 10 * len);
        g_app->window->render_text_box(segment.pos + segment.dir, {0.3, 0.025}, txt);
        snprintf(txt, 64, "SPD: %d", int(10000 * ecs.get<snake_char_c>(snake).speed));
        g_app->window->render_text_box(segment.pos + segment.dir + vec2{0, 0.025}, {0.3, 0.025}, txt);
//#endif
    }
    for(auto snake : ecs.view<dead_snake_c>())
    {
        draw_snake(snake, static_resource<texture>::get(ID("resources/sn1.png")));
    }

    for(const auto& food : ecs.raw_view<food_c>())
    {
        const vec2 size = {food_size, food_size};
        g_app->window->render.copy(
                //g_app->resources->textures.get("resources/food.png"),
                static_resource<texture>::get(ID("resources/food.png")),
                food.pos - size/2,
                size
        );
    }

    // draw food cnt
    if(player_food.count(player))
    {
        char txt[64];
        snprintf(txt, 64, "Food: %d", player_food[player]);
        g_app->window->render_text_box({-1, -1}, {0.3, 0.05}, txt);
    }
    // draw score
    {
        char txt[64];
        snprintf(txt, 64, "Score: %d", (int)(10 * total_result));
        g_app->window->render_text_box({-0.3, -1}, {0.6, 0.05}, txt);
    }
}

class controller
{
    multiplayer_game<gamestate_simulator2<snakegame>> game {g_app->netgame.get()};

    std::vector<net_node_id> hosts;
public:
    controller()
    {
        // how many ticks behind oldstate follows newstate
        // larger values allow worse network conditions, lower values improve performace
        game.sim.lag = g_app->config->get("network.simlag", 20);

        g_app->netgame->on_event.connect([this](tick_input_t inp) {
            IF_EVENT_(inp.event, host_adv) hosts.push_back(inp.player);
        });
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
            if(game.state != decltype(game)::HOST) return;
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

    void draw()
    {
        if(game.playing())
        {
            game.update();
            game.sim.gamestate().draw(g_app->netgame->id());
        }
    }

    void send_event(event_t event)
    {
        if(game.playing())
            game.push_event(std::move(event));
    }

    bool playing() const { return game.playing(); }

};


void init_game2()
{
    auto c = new controller;
    c->host();

    g_app->window->draw.connect([c]() { c->draw();});
    g_app->window->key.connect([c](auto k){
        if(k.action != GLFW_PRESS/* && k.action != GLFW_RELEASE*/)
            return;
        if(k.key == GLFW_KEY_ESCAPE)
            g_app->window->close();

        if(k.key == GLFW_KEY_F12)
        {
            c->join_random();
        }
        if(k.key == GLFW_KEY_F1)
        {
            //=- register_event(name=>'make_new_snake');
            c->send_event(event::make_new_snake{});
        }

        LOGGER(info, "pressed",k.key);
    });

    g_app->window->mouse_click.connect([c](auto click){
        //=- register_event(name=>'new_food', params=>[['float','x'], ['float','y']]);
        if(click.action == GLFW_PRESS && click.button == GLFW_MOUSE_BUTTON_LEFT)
            c->send_event(event::new_food{qvm::X(click.pos), qvm::Y(click.pos)});
    });

}

void init_game()
{
    gl::Enable(GL_BLEND);

    gl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gl::ClearColor(0,0,0,0);
    g_app->window->draw.connect([](){
        static bool initialized = false;
        if(glfwGetTime() < 3.f) // logo
            g_app->window->render.copy_center(g_app->resources->textures.get("resources/splash.jpg"));
        else if(!initialized)
        {
            initialized = true;
            init_game2();
        }
    });
}
