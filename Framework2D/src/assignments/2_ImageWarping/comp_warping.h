#pragma once

#include "view/comp_image.h"
#include "Warping.h"

namespace USTC_CG
{
// Image component for warping and other functions
class CompWarping : public ImageEditor
{
   public:
    explicit CompWarping(const std::string& label, const std::string& filename);
    virtual ~CompWarping() noexcept = default;

    void draw() override;

    // Simple edit functions
    void invert();
    void mirror(bool is_horizontal, bool is_vertical);
    void gray_scale();
    void warping();
    void restore();

    // Transforming mode
    enum WarpType
    {
        kDefault    = 0,
        kIDW        = 1,
        kRBF        = 2,
    };

    // Point selecting interaction
    void enable_selecting(bool flag);
    void select_points();
    void init_selections();

    // Warp Mode Setting
    void set_idw();
    void set_rbf();

    // Change whether fill the gap
    void set_gap();

   private:
    // Store the original image data
    std::shared_ptr<Image> back_up_;

    // The image data with gap and without gap
    // Control it by flag fillgap
    std::shared_ptr<Image> with_gap;
    std::shared_ptr<Image> without_gap;
    bool fillgap = false; 

    // The selected point couples for image warping
    std::vector<ImVec2> start_points_, end_points_;

    // The warp type and warpper
    WarpType warp_type_ = kDefault;
    std::shared_ptr<Warping> warpper;

    // Parameter for visualization
    ImVec2 start_, end_;
    bool flag_enable_selecting_points_ = false;
    bool draw_status_ = false;

   private:
    // A simple "fish-eye" warping function
    std::pair<int, int> fisheye_warping(int x, int y, int width, int height);
};

}  // namespace USTC_CG