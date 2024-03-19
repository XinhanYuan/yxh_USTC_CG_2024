#include "view/shapes/rect.h"

#include <imgui.h>

namespace USTC_CG
{
// Draw the rectangle using ImGui
void Rect::draw(const Config& config) const
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    if (fill_flag==false)
    {
        draw_list->AddRect(
            ImVec2(
                config.bias[0] + start_point_x_,
                config.bias[1] + start_point_y_),
            ImVec2(
                config.bias[0] + end_point_x_, config.bias[1] + end_point_y_),
            IM_COL32(
                config.line_color[0],
                config.line_color[1],
                config.line_color[2],
                config.line_color[3]),
            0.f,  // No rounding of corners
            ImDrawFlags_None,
            config.line_thickness);
    }
    else
    {
        draw_list->AddRectFilled(
            ImVec2(
                config.bias[0] + start_point_x_,
                config.bias[1] + start_point_y_),
            ImVec2(
                config.bias[0] + end_point_x_, config.bias[1] + end_point_y_),
            IM_COL32(
                config.line_color[0],
                config.line_color[1],
                config.line_color[2],
                config.line_color[3]),
            0.f,  // No rounding of corners
            ImDrawFlags_None);
    }
    
}

void Rect::update(float x, float y)
{
    end_point_x_ = x;
    end_point_y_ = y;
}

void Rect::restart(float x, float y)
{
    start_point_x_ = x;
    start_point_y_ = y;
}

void Rect::connect()
{
}

void Rect::change_fill_flag()
{
    fill_flag = !fill_flag;
}

void Rect::add_point(float x, float y)
{
}
}  // namespace USTC_CG
