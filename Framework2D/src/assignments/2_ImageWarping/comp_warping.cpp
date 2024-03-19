#include "comp_warping.h"

#include <cmath>
#include <iostream>

#include"imgui.h"
#include "IDW.h"
#include "RBF.h"

#include "annoylib.h"
#include "kissrandom.h"

namespace USTC_CG
{
using uchar = unsigned char;

CompWarping::CompWarping(const std::string& label, const std::string& filename)
    : ImageEditor(label, filename)
{
    if (data_)
        back_up_ = std::make_shared<Image>(*data_);
}

void CompWarping::draw()
{
    // Draw the image
    ImageEditor::draw();
    // Draw the canvas
    if (flag_enable_selecting_points_)
        select_points();
}

void CompWarping::invert()
{
    for (int i = 0; i < data_->width(); ++i)
    {
        for (int j = 0; j < data_->height(); ++j)
        {
            const auto color = data_->get_pixel(i, j);
            data_->set_pixel(
                i,
                j,
                { static_cast<uchar>(255 - color[0]),
                  static_cast<uchar>(255 - color[1]),
                  static_cast<uchar>(255 - color[2]) });
        }
    }
    // After change the image, we should reload the image data to the renderer
    update();
}
void CompWarping::mirror(bool is_horizontal, bool is_vertical)
{
    Image image_tmp(*data_);
    int width = data_->width();
    int height = data_->height();

    if (is_horizontal)
    {
        if (is_vertical)
        {
            for (int i = 0; i < width; ++i)
            {
                for (int j = 0; j < height; ++j)
                {
                    data_->set_pixel(
                        i,
                        j,
                        image_tmp.get_pixel(width - 1 - i, height - 1 - j));
                }
            }
        }
        else
        {
            for (int i = 0; i < width; ++i)
            {
                for (int j = 0; j < height; ++j)
                {
                    data_->set_pixel(
                        i, j, image_tmp.get_pixel(width - 1 - i, j));
                }
            }
        }
    }
    else
    {
        if (is_vertical)
        {
            for (int i = 0; i < width; ++i)
            {
                for (int j = 0; j < height; ++j)
                {
                    data_->set_pixel(
                        i, j, image_tmp.get_pixel(i, height - 1 - j));
                }
            }
        }
    }

    // After change the image, we should reload the image data to the renderer
    update();
}
void CompWarping::gray_scale()
{
    for (int i = 0; i < data_->width(); ++i)
    {
        for (int j = 0; j < data_->height(); ++j)
        {
            const auto color = data_->get_pixel(i, j);
            uchar gray_value = (color[0] + color[1] + color[2]) / 3;
            data_->set_pixel(i, j, { gray_value, gray_value, gray_value });
        }
    }
    // After change the image, we should reload the image data to the renderer
    update();
}
void CompWarping::warping()
{
    // Do nothing if there is no control point
    // or selecting no type for warpping
    if (start_points_.size() == 0)
        return;
    if (warp_type_ == USTC_CG::CompWarping::kDefault)
        return;
    // HW2_TODO: You should implement your own warping function that interpolate
    // the selected points.
    // You can design a class for such warping operations, utilizing the
    // encapsulation, inheritance, and polymorphism features of C++. More files
    // like "*.h", "*.cpp" can be added to this directory or anywhere you like.

    // Create a new image to store the result
    Image warped_image(*data_);

    // Record the filled status of each pixel
    MatrixXi filled_status = 
        MatrixXi::Zero(data_->width(), data_->height());

    // Initialize gap_filling module
    int f = 2;
    Annoy::AnnoyIndex<
        int,
        float,
        Annoy::Euclidean,
        //Annoy::Manhattan,
        Annoy::Kiss32Random,
        Annoy::AnnoyIndexSingleThreadedBuildPolicy>
        index(f);  // using Euclidean distance

    // Initialize the color of result image
    for (int y = 0; y < data_->height(); ++y)
    {
        for (int x = 0; x < data_->width(); ++x)
        {
            warped_image.set_pixel(x, y, { 255, 255, 255 });
        }
    }
    // Construct different warpper according to the type
    if (warp_type_ == USTC_CG::CompWarping::kIDW)
    {
        warpper = std::make_shared<IDW>(start_points_, end_points_);
        std::cout << "Warpping by IDW method." << std::endl;
    }
    else
    {
        warpper = std::make_shared<RBF>(start_points_, end_points_);
        std::cout << "Warpping by RBF method." << std::endl;
    }
    int num = 0;
    for (int y = 0; y < data_->height(); ++y)
    {
        for (int x = 0; x < data_->width(); ++x)
        {
            // Apply warping function to (x, y), and we can get (x', y')
            auto [new_x, new_y] = warpper->warp(x, y);

            // Copy the color from the original image to the result image.
            if (new_x >= 0 && new_x < data_->width() && new_y >= 0 &&
                new_y < data_->height())
            {
                std::vector<unsigned char> pixel = data_->get_pixel(x, y);
                warped_image.set_pixel(new_x, new_y, pixel);

                // Store the image_point data. Help to gap filling.
                std::vector<float> pix(2);
                pix = { float(new_x), float(new_y) };
                index.add_item(num, pix.data());
                num++;

                // Store the filled status
                filled_status(new_x, new_y) = 1;
            }
        }
    }
    // Store the image with gap
    with_gap = std::make_shared<Image>(warped_image);

    // Build up the search trees for the index
    // Related to the searching speed of ANN
    index.build(3);  
    
    // Action of filling the gap 
    for (int y = 0; y < data_->height(); ++y)
    {
        for (int x = 0; x < data_->width(); ++x)
        {
            // comfirm the stutus unfilled
            if (filled_status(x, y) == 0)
            {
                float v[2]= { x, y };
                std::vector<int> closest_items;
                std::vector<float> distances;
                float p[2];

                // Find nearest neighbors of the vector
                index.get_nns_by_vector(v, 1, -1, &closest_items, &distances);
                index.get_item(closest_items[0], p);
                if (
                    distances[0] < 3
                    //true
                    )
                {
                    // Copy the pixel information from the nearest
                    // that has been found
                    std::vector<unsigned char> pixel =
                        warped_image.get_pixel(round(p[0]), round(p[1]));
                    warped_image.set_pixel(x, y, pixel);
                }
                closest_items.clear();
                distances.clear();
            }
        }
    }

    // Store the image without gap
    // to make sure you can change the filling mode
    without_gap = std::make_shared<Image>(warped_image);

    // Image Replacing
    std::cout << "Warpping action done !" << std::endl;
    *data_ = *with_gap;
    update();
}
void CompWarping::restore()
{
    *data_ = *back_up_;
    fillgap = false;
    update();
}
void CompWarping::enable_selecting(bool flag)
{
    flag_enable_selecting_points_ = flag;
}
void CompWarping::select_points()
{
    /// Invisible button over the canvas to capture mouse interactions.
    ImGui::SetCursorScreenPos(position_);
    ImGui::InvisibleButton(
        label_.c_str(),
        ImVec2(
            static_cast<float>(image_width_),
            static_cast<float>(image_height_)),
        ImGuiButtonFlags_MouseButtonLeft);
    // Record the current status of the invisible button
    bool is_hovered_ = ImGui::IsItemHovered();
    // Selections
    ImGuiIO& io = ImGui::GetIO();
    if (is_hovered_ && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        draw_status_ = true;
        start_ = end_ =
            ImVec2(io.MousePos.x - position_.x, io.MousePos.y - position_.y);
    }
    if (draw_status_)
    {
        end_ = ImVec2(io.MousePos.x - position_.x, io.MousePos.y - position_.y);
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            start_points_.push_back(start_);
            end_points_.push_back(end_);
            draw_status_ = false;
        }
    }
    // Visualization
    auto draw_list = ImGui::GetWindowDrawList();
    for (size_t i = 0; i < start_points_.size(); ++i)
    {
        ImVec2 s(
            start_points_[i].x + position_.x, start_points_[i].y + position_.y);
        ImVec2 e(
            end_points_[i].x + position_.x, end_points_[i].y + position_.y);
        draw_list->AddLine(s, e, IM_COL32(255, 0, 0, 255), 2.0f);
        draw_list->AddCircleFilled(s, 4.0f, IM_COL32(0, 0, 255, 255));
        draw_list->AddCircleFilled(e, 4.0f, IM_COL32(0, 255, 0, 255));
    }
    if (draw_status_)
    {
        ImVec2 s(start_.x + position_.x, start_.y + position_.y);
        ImVec2 e(end_.x + position_.x, end_.y + position_.y);
        draw_list->AddLine(s, e, IM_COL32(255, 0, 0, 255), 2.0f);
        draw_list->AddCircleFilled(s, 4.0f, IM_COL32(0, 0, 255, 255));
    }
}
void CompWarping::init_selections()
{
    start_points_.clear();
    end_points_.clear();
}

std::pair<int, int>
CompWarping::fisheye_warping(int x, int y, int width, int height)
{
    float center_x = width / 2.0f;
    float center_y = height / 2.0f;
    float dx = x - center_x;
    float dy = y - center_y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // Simple non-linear transformation r -> r' = f(r)
    float new_distance = std::sqrt(distance) * 10;

    if (distance == 0)
    {
        return { static_cast<int>(center_x), static_cast<int>(center_y) };
    }
    // (x', y')
    float ratio = new_distance / distance;
    int new_x = static_cast<int>(center_x + dx * ratio);
    int new_y = static_cast<int>(center_y + dy * ratio);

    return { new_x, new_y };
}

void CompWarping::set_idw()
{
    warp_type_ = kIDW;
}

void CompWarping::set_rbf()
{
    warp_type_ = kRBF;
}
void CompWarping::set_gap()
{
    if (fillgap == false)
    {
        *data_ = *without_gap;
        fillgap = true;
        update();
    }
    else
    {
        *data_ = *with_gap;
        fillgap = false;
        update();
    }
}
}  // namespace USTC_CG