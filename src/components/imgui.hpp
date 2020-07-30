//
// Created by fotyev on 2020-07-30.
//

#ifndef ZENGINE_IMGUI_HPP
#define ZENGINE_IMGUI_HPP

#include "main.hpp"
#include "window.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_scoped.h"

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

//=- register_component(class=>'imgui_c', name=>'gui', priority=>70);
class imgui_c
{
public:
  imgui_c();

  ~imgui_c();

  static void new_frame()
  {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
  }

  static void draw()
  {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }
};

#endif //ZENGINE_IMGUI_HPP
