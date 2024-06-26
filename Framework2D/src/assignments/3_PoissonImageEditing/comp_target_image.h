#pragma once
#include <cmath>
#include "comp_source_image.h"
#include "view/comp_image.h"
#include "cloner.h"
#include "Eigen/Dense"
#include <Eigen/Sparse>
#include<Eigen/IterativeLinearSolvers>
#include "seamless.h"
#include "mix.h"
using namespace Eigen;
namespace USTC_CG
{
class CompTargetImage : public ImageEditor
{
   public:
    // HW3_TODO: Add more types of cloning
    enum CloneType
    {
        kDefault = 0,
        kPaste = 1,
        kSeamless = 2,
        kMix = 3
    };

    explicit CompTargetImage(
        const std::string& label,
        const std::string& filename);
    virtual ~CompTargetImage() noexcept = default;

    void draw() override;
    // Bind the source image component
    void set_source(std::shared_ptr<CompSourceImage> source);

    // Enable realtime updating
    void set_realtime(bool flag);
    void restore();

    // HW3_TODO: Add more types of cloning
    void set_paste();
    void set_seamless();
    void set_mix();

    // PreDecomposition for Initialization of the Solver
    void PreDecomposition();

    // The clone function
    void clone();

    // function to fix up the value of the solution that is NOT in 0~255
    unsigned char fix(float p);

   private:
    // cloner to get the const part of the linear equation
    std::shared_ptr<Cloner> cloner;

    // Store the original image data
    std::shared_ptr<Image> back_up_;

    // Source image
    std::shared_ptr<CompSourceImage> source_image_;

    // cloner's type
    CloneType clone_type_ = kDefault;

    // Sparse linear system solver
    SparseLU<SparseMatrix<float>> solver;

    // related const and flag
    ImVec2 mouse_position_;
    bool edit_status_ = false;
    bool flag_realtime_updating = false;
};

}  // namespace USTC_CG