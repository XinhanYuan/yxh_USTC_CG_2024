#include "comp_target_image.h"
#include <iostream>
#include <cmath>
namespace USTC_CG
{
using uchar = unsigned char;

CompTargetImage::CompTargetImage(
    const std::string& label,
    const std::string& filename)
    : ImageEditor(label, filename)
{
    if (data_)
        back_up_ = std::make_shared<Image>(*data_);
}

void CompTargetImage::draw()
{
    // Draw the image
    ImageEditor::draw();
    // Invisible button for interactions
    ImGui::SetCursorScreenPos(position_);
    ImGui::InvisibleButton(
        label_.c_str(),
        ImVec2(
            static_cast<float>(image_width_),
            static_cast<float>(image_height_)),
        ImGuiButtonFlags_MouseButtonLeft);
    bool is_hovered_ = ImGui::IsItemHovered();
    // When the mouse is clicked or moving, we would adapt clone function to
    // copy the selected region to the target.
    ImGuiIO& io = ImGui::GetIO();
    if (is_hovered_ && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        edit_status_ = true;
        mouse_position_ =
            ImVec2(io.MousePos.x - position_.x, io.MousePos.y - position_.y);
        clone();
    }
    if (edit_status_)
    {
        mouse_position_ =
            ImVec2(io.MousePos.x - position_.x, io.MousePos.y - position_.y);
        if (flag_realtime_updating)
            clone();
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            edit_status_ = false;
        }
    }
}

void CompTargetImage::set_source(std::shared_ptr<CompSourceImage> source)
{
    source_image_ = source;
}

void CompTargetImage::set_realtime(bool flag)
{
    flag_realtime_updating = flag;
}

void CompTargetImage::restore()
{
    *data_ = *back_up_;
    update();
}

void CompTargetImage::set_paste()
{
    clone_type_ = kPaste;
    std::cout << "Set Mode To Paste." << std::endl;
}

void CompTargetImage::set_seamless()
{
    clone_type_ = kSeamless;
    std::cout << "Set Mode To Seamless." << std::endl;
    PreDecomposition();
}

void CompTargetImage::set_mix()
{
    clone_type_ = kMix;
    std::cout << "Set Mode To Mixed." << std::endl;
    PreDecomposition();
}

void CompTargetImage::PreDecomposition()
{
    int sel_width = abs(source_image_->get_start().x - source_image_->get_end().x) + 1;
    int sel_height = abs(source_image_->get_start().y - source_image_->get_end().y) + 1;

    // The dimension of the matrix depends on the scale of the selected area
    SparseMatrix<float> mat((sel_width * sel_height), (sel_width * sel_height));

    // Reserve 8 position for element that is not 0 at each row
    // Actually there are no more than 4 elements that are not zero each 4
    mat.reserve(VectorXi::Constant(sel_width * sel_height, 8));

    for (int i = 0; i < sel_height; i++)
    {
        for (int j = 0; j < sel_width; j++)
        {
            int index = i * sel_width + j;

            mat.insert(index, index) = 4;
            if (i == 0 || i == sel_height - 1 || j == 0 || j == sel_width - 1)
            {
            }
            else
            {
                mat.insert(index, index - 1) = -1;
                mat.insert(index, index + 1) = -1;
                mat.insert(index, index + sel_width) = -1;
                mat.insert(index, index - sel_width) = -1;
            }
        }
    }

    // PreDecomposition
    solver.compute(mat);
    std::cout << "PreDecomposition done!" << std::endl;
}
void CompTargetImage::clone()
{
    // The implementation of different types of cloning
    // HW3_TODO: In this function, you should at least implement the "seamless"
    // cloning labeled by `clone_type_ ==kSeamless`.
    //
    // The realtime updating (update when the mouse is moving) is only available
    // when the checkboard is selected. It is required to improve the efficiency
    // of your seamless cloning to achieve realtime editing. (Use decomposition
    // of sparse matrix before solve the linear system)
    std::shared_ptr<Image> mask = source_image_->get_region();

    switch (clone_type_)
    {
        case USTC_CG::CompTargetImage::kDefault: break;
        case USTC_CG::CompTargetImage::kPaste:
        {
            restore();

            for (int i = 0; i < mask->width(); ++i)
            {
                for (int j = 0; j < mask->height(); ++j)
                {
                    int tar_x =
                        static_cast<int>(mouse_position_.x) + i -
                        static_cast<int>(source_image_->get_start().x);
                    int tar_y =
                        static_cast<int>(mouse_position_.y) + j -
                        static_cast<int>(source_image_->get_start().y);
                    if (0 <= tar_x && tar_x < image_width_ && 0 <= tar_y &&
                        tar_y < image_height_ && mask->get_pixel(i, j)[0] > 0)
                    {
                        data_->set_pixel(
                            tar_x,
                            tar_y,
                            source_image_->get_data()->get_pixel(i, j));
                    }
                }
            }
            break;
        }
        case USTC_CG::CompTargetImage::kSeamless:
        {
            cloner = std::make_shared<Seamless>();
            // Check whether the region is completely contain in the target image. 
            if (static_cast<int>(mouse_position_.x) +
                        static_cast<int>(source_image_->get_end().x) -
                        static_cast<int>(source_image_->get_start().x) >= image_width_ 
                ||
                static_cast<int>(mouse_position_.y) +
                        static_cast<int>(source_image_->get_end().y) -
                        static_cast<int>(source_image_->get_start().y) >= image_height_
                ||
                static_cast<int>(mouse_position_.x) <= 0
                ||
                static_cast<int>(mouse_position_.y) <= 0)
            {
                restore();
                return;
            }

            restore();

            int sel_width =
                abs(source_image_->get_start().x - source_image_->get_end().x) + 1;
            int sel_height =
                abs(source_image_->get_start().y - source_image_->get_end().y) + 1;

            // Using cloner to build up the linear system
            VectorXf red_vec = cloner->get_vec(
                sel_width,
                sel_height,
                mouse_position_,
                source_image_,
                data_,
                0);

            VectorXf gre_vec = cloner->get_vec(
                sel_width,
                sel_height,
                mouse_position_,
                source_image_,
                data_,
                1);

            VectorXf blu_vec = cloner->get_vec(
                sel_width,
                sel_height,
                mouse_position_,
                source_image_,
                data_,
                2);

            // Solve the equation with pre-decomposed solver
            VectorXf red_sol = solver.solve(red_vec);
            VectorXf gre_sol = solver.solve(gre_vec);
            VectorXf blu_sol = solver.solve(blu_vec);

            // Change the target image pixel with the solution
            for (int i = 0; i < sel_height; i++)
            {
                for (int j = 0; j < sel_width; j++)
                {
                    int index = i * sel_width + j;
                    int tar_x = j + static_cast<int>(mouse_position_.x);
                    int tar_y = i + static_cast<int>(mouse_position_.y);

                    data_->set_pixel(
                        tar_x,
                        tar_y,
                        { fix(red_sol(index)),
                          fix(gre_sol(index)),
                          fix(blu_sol(index)) });
                    index++;
                }
            }
            break;
        }
        case USTC_CG::CompTargetImage::kMix:
        {
            cloner = std::make_shared<Mix>();
            // Check whether the region is completely contain in the target
            // image.
            if (static_cast<int>(mouse_position_.x) +
                        static_cast<int>(source_image_->get_end().x) -
                        static_cast<int>(source_image_->get_start().x) >=
                    image_width_ ||
                static_cast<int>(mouse_position_.y) +
                        static_cast<int>(source_image_->get_end().y) -
                        static_cast<int>(source_image_->get_start().y) >=
                    image_height_ ||
                static_cast<int>(mouse_position_.x) <= 0 ||
                static_cast<int>(mouse_position_.y) <= 0)
            {
                restore();
                return;
            }

            restore();

            int sel_width =
                abs(source_image_->get_start().x - source_image_->get_end().x) +
                1;
            int sel_height =
                abs(source_image_->get_start().y - source_image_->get_end().y) +
                1;

            VectorXf red_vec = cloner->get_vec(
                sel_width,
                sel_height,
                mouse_position_,
                source_image_,
                data_,
                0);

            VectorXf gre_vec = cloner->get_vec(
                sel_width,
                sel_height,
                mouse_position_,
                source_image_,
                data_,
                1);

            VectorXf blu_vec = cloner->get_vec(
                sel_width,
                sel_height,
                mouse_position_,
                source_image_,
                data_,
                2);

            VectorXf red_sol = solver.solve(red_vec);
            VectorXf gre_sol = solver.solve(gre_vec);
            VectorXf blu_sol = solver.solve(blu_vec);

            for (int i = 0; i < sel_height; i++)
            {
                for (int j = 0; j < sel_width; j++)
                {
                    int index = i * sel_width + j;
                    int tar_x = j + static_cast<int>(mouse_position_.x);
                    int tar_y = i + static_cast<int>(mouse_position_.y);

                    data_->set_pixel(
                        tar_x,
                        tar_y,
                        { fix(red_sol(index)),
                          fix(gre_sol(index)),
                          fix(blu_sol(index)) });
                    index++;
                }
            }
            break;
        }
        default: break;
    }

    update();
}
uchar CompTargetImage::fix(float p)
{
    // fix up the solution
    // if the channel value is nagative, fix it by zero
    // if the channel value exceeds 255(upper bound of uchar), fix it by 255
    // if the channel value is between 0 and 255, change the type directly
    uchar p_;
    if (p <= 0)
        p_ = 0;
    else if (p >= 255)
        p_ = 255;
    else
        p_ = (uchar)p;
    return p_;
}
}  // namespace USTC_CG