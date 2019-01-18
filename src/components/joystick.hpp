//
// Created by fotyev on 2018-11-23.
//

#ifndef ZENGINE_JOYSTICK_HPP
#define ZENGINE_JOYSTICK_HPP

#include <GLFW/glfw3.h>
#include "util/signals.hpp"
#include "util/log.hpp"

#include "components/window.hpp"
#include "main.hpp"

//=- register_component(class=>'joystick_c', name=>'joystick', priority=>25);
class joystick_c
{
public:
    int js = 0;

    sig::signal<void (unsigned int button, bool state)> button;
    sig::signal<void (unsigned int axel, float state)> axel;

private:
    std::vector<unsigned char> old_buttons;
    std::vector<float> old_axels;

    sig::scoped_connection _poll = g_app->window->draw.connect([this](){poll();});
public:
    joystick_c()
    {
        for(int js = 0; js < 16; js++)
        {
            if(glfwJoystickPresent(js))
            {
                int buttons, axes;
                glfwGetJoystickButtons(js, &buttons);
                glfwGetJoystickAxes(js, &axes);
                LOGGER(info, "Joystick detected", js, glfwGetJoystickName(js),
                       "buttons:", buttons, "axes:", axes/*, "gamepad:", glfwJoystickIsGamepad(js)*/);
            }
        }
#ifndef NDEBUG
        button.connect([](int b, int s) { LOGGER(debug2, "JS:",b,s); });
#endif
    }

    void poll()
    {
        {
            int btn_cnt;
            auto btns = glfwGetJoystickButtons(js, &btn_cnt);
            if(btn_cnt > (int)old_buttons.size())
                old_buttons.resize(btn_cnt, GLFW_RELEASE);
            for(int i = 0; i < btn_cnt; i++)
            {
                if(btns[i] != old_buttons[i])
                {
                    button(i, btns[i] == GLFW_PRESS);
                    old_buttons[i] = btns[i];
                }
            }
        }
        {
            int axe_cnt;
            auto axes = glfwGetJoystickAxes(js, &axe_cnt);
            if(axe_cnt > (int)old_axels.size())
                old_axels.resize(axe_cnt, 0.f);
            for(int i = 0; i < axe_cnt; i++)
            {
                if(axes[i] != old_axels[i])
                {
                    axel(i, axes[i]);
                    old_axels[i] = axes[i];
                }
            }
        }
    }
};

#endif //ZENGINE_JOYSTICK_HPP
