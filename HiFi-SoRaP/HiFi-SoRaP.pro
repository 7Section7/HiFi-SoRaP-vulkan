#-------------------------------------------------
#
# Project created by QtCreator 2018-12-10T13:03:41
#
#-------------------------------------------------

QT       += core gui opengl
#QT       += widgets
QT       += datavisualization

win32 {
LIBS += $$PWD/Lib/glext.lib
}

contains(QT_ARCH, i386) {
    message("32 bits")
#    LIBS += -L$$(VK_SDK_PATH)/Lib32
} else {
#    LIBS += -L$$(VK_SDK_PATH)/Lib
}

LIBS += -L$$(VK_SDK_PATH)/Lib32 -lvulkan-1

CONFIG += c++17

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HiFi-SoRaP
TEMPLATE = app

#QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


#INCLUDEPATH += /usr/include/eigen3/
#INCLUDEPATH += /usr/local/lib/python2.7/dist-packages/tensorflow/include/

#INCLUDEPATH += Lib/eigen3/
#INCLUDEPATH += Lib/eigen3/unsupported/
#INCLUDEPATH += Lib/eigen3/Eigen/
#INCLUDEPATH += Lib/eigen3/test/
#INCLUDEPATH += Lib/eigen3/bench/
#INCLUDEPATH += Lib/eigen3/bench/btl/generic_bench/
#INCLUDEPATH += Lib/eigen3/bench/btl/generic_bench/utils/
#INCLUDEPATH +=s Lib/eigen3/bench/btl/libs/
#INCLUDEPATH += Lib/eigen3/bench/btl/libs/STL/
#INCLUDEPATH += Lib/eigen3/bench/btl/libs/eigen3/
#INCLUDEPATH += Lib/eigen3/bench/btl/libs/eigen2/
#INCLUDEPATH += Lib/eigen3/bench/btl/actions/
#INCLUDEPATH += Lib/eigen3/Eigen/src/Eigen2Support/
#INCLUDEPATH += Lib/eigen3/Eigen/src/Core/
#INCLUDEPATH += Lib/eigen3/Eigen/Core

INCLUDEPATH += Lib/glm/Core


#INCLUDEPATH += /usr/local/Cellar/glew/2.1.0/lib/
#LIBS += /usr/local/Cellar/glew/2.1.0/lib/libGLEW.dylib

#LIBS += -lGLEW

#CONFIG(QMAKE_CXXFLAGS_WARN_ON)

#QMAKE_CXXFLAGS += -I "Lib/eigen3/Eigen" -I "GLVisualization"
#QMAKE_CXXFLAGS += -Wp
#QMAKE_CXXFLAGS += -Wexpansion-to-defined -Wpedantic -Wextra
#CFLAGS += -Wexpansion-to-defined -Wpedantic -Wextra

SOURCES +=  \
	SRP/CPU_SRP/cannonball.cpp \
    SRP/Compute_SRP/computegpu.cpp \
    VulkanVisualization/vkvisualization.cpp \
    VulkanVisualization/vulkanrenderer.cpp \
    VulkanVisualization/vulkanwindow.cpp \
        main.cpp \
        mainwindow.cpp \
    MeshObjects/material.cpp \
    MeshObjects/triangle.cpp \
    MeshObjects/trianglemesh.cpp \
    MeshObjects/object.cpp \
    MeshObjects/lineobject.cpp \
    SRP/advancedsrp.cpp \
    SRP/basicsrp.cpp \
    SRP/srp.cpp \
    DataVisualization/visualizationwindow.cpp \
    DataVisualization/grid.cpp \
    DataVisualization/output.cpp \
    GLVisualization/light.cpp \
    GLVisualization/camera.cpp \
    GLVisualization/glwindow.cpp \
    GLVisualization/glwidget.cpp \
    SRP/GPU_SRP/advancedgpu.cpp \
    SRP/CPU_SRP/nplate.cpp \
    SRP/CPU_SRP/raytracecpu.cpp \
    Lib/glm/core/func_common.inl \
    Lib/glm/core/func_exponential.inl \
    Lib/glm/core/func_geometric.inl \
    Lib/glm/core/func_integer.inl \
    Lib/glm/core/func_matrix.inl \
    Lib/glm/core/func_noise.inl \
    Lib/glm/core/func_packing.inl \
    Lib/glm/core/func_trigonometric.inl \
    Lib/glm/core/func_vector_relational.inl \
    Lib/glm/core/intrinsic_common.inl \
    #Lib/glm/core/intrinsic_exponential.inl \
    Lib/glm/core/intrinsic_geometric.inl \
    Lib/glm/core/intrinsic_matrix.inl \
    Lib/glm/core/intrinsic_trigonometric.inl \
    Lib/glm/core/intrinsic_vector_relational.inl \
    #Lib/glm/core/type_gentype.inl \
    Lib/glm/core/type_half.inl \
    #Lib/glm/core/type_mat.inl \
    Lib/glm/core/type_mat2x2.inl \
    Lib/glm/core/type_mat2x3.inl \
    Lib/glm/core/type_mat2x4.inl \
    Lib/glm/core/type_mat3x2.inl \
    Lib/glm/core/type_mat3x3.inl \
    Lib/glm/core/type_mat3x4.inl \
    Lib/glm/core/type_mat4x2.inl \
    Lib/glm/core/type_mat4x3.inl \
    Lib/glm/core/type_mat4x4.inl \
    #Lib/glm/core/type_vec.inl \
    Lib/glm/core/type_vec1.inl \
    Lib/glm/core/type_vec2.inl \
    Lib/glm/core/type_vec3.inl \
    Lib/glm/core/type_vec4.inl \
    Lib/glm/gtc/constants.inl \
    Lib/glm/gtc/epsilon.inl \
    Lib/glm/gtc/half_float.inl \
    Lib/glm/gtc/matrix_access.inl \
    Lib/glm/gtc/matrix_inverse.inl \
    Lib/glm/gtc/matrix_transform.inl \
    Lib/glm/gtc/noise.inl \
    Lib/glm/gtc/quaternion.inl \
    Lib/glm/gtc/random.inl \
    Lib/glm/gtc/reciprocal.inl \
    Lib/glm/gtc/swizzle.inl \
    Lib/glm/gtc/type_precision.inl \
    Lib/glm/gtc/type_ptr.inl \
    Lib/glm/gtc/ulp.inl \
    Lib/glm/gtx/associated_min_max.inl \
    Lib/glm/gtx/bit.inl \
    Lib/glm/gtx/closest_point.inl \
    Lib/glm/gtx/color_cast.inl \
    Lib/glm/gtx/color_space.inl \
    Lib/glm/gtx/color_space_YCoCg.inl \
    Lib/glm/gtx/compatibility.inl \
    Lib/glm/gtx/component_wise.inl \
    Lib/glm/gtx/euler_angles.inl \
    Lib/glm/gtx/extend.inl \
    Lib/glm/gtx/extented_min_max.inl \
    Lib/glm/gtx/fast_exponential.inl \
    Lib/glm/gtx/fast_square_root.inl \
    Lib/glm/gtx/fast_trigonometry.inl \
    Lib/glm/gtx/gradient_paint.inl \
    Lib/glm/gtx/handed_coordinate_space.inl \
    Lib/glm/gtx/inertia.inl \
    Lib/glm/gtx/int_10_10_10_2.inl \
    Lib/glm/gtx/integer.inl \
    Lib/glm/gtx/intersect.inl \
    Lib/glm/gtx/log_base.inl \
    Lib/glm/gtx/matrix_cross_product.inl \
    Lib/glm/gtx/matrix_interpolation.inl \
    Lib/glm/gtx/matrix_major_storage.inl \
    Lib/glm/gtx/matrix_operation.inl \
    Lib/glm/gtx/matrix_query.inl \
    Lib/glm/gtx/mixed_product.inl \
    Lib/glm/gtx/multiple.inl \
    Lib/glm/gtx/norm.inl \
    Lib/glm/gtx/normal.inl \
    Lib/glm/gtx/normalize_dot.inl \
    Lib/glm/gtx/number_precision.inl \
    Lib/glm/gtx/ocl_type.inl \
    Lib/glm/gtx/optimum_pow.inl \
    Lib/glm/gtx/orthonormalize.inl \
    Lib/glm/gtx/perpendicular.inl \
    Lib/glm/gtx/polar_coordinates.inl \
    Lib/glm/gtx/projection.inl \
    Lib/glm/gtx/quaternion.inl \
    Lib/glm/gtx/raw_data.inl \
    Lib/glm/gtx/rotate_vector.inl \
    Lib/glm/gtx/simd_mat4.inl \
    Lib/glm/gtx/simd_vec4.inl \
    Lib/glm/gtx/spline.inl \
    Lib/glm/gtx/std_based_type.inl \
    Lib/glm/gtx/string_cast.inl \
    Lib/glm/gtx/transform.inl \
    Lib/glm/gtx/transform2.inl \
    #Lib/glm/gtx/unsigned_int.inl \
    Lib/glm/gtx/vec1.inl \
    Lib/glm/gtx/vector_access.inl \
    Lib/glm/gtx/vector_angle.inl \
    Lib/glm/gtx/vector_query.inl \
    Lib/glm/gtx/verbose_operator.inl \
    Lib/glm/gtx/wrap.inl \
    Comparison/comparisonwindow.cpp \
    Comparison/result.cpp \
    Comparison/categoryresult.cpp \
    SRP/GPU_SRP/raytracegputextures.cpp \
    SRP/GPU_SRP/raytracegpu.cpp \
    MeshObjects/quad.cpp \
    SRP/GPU_SRP/render.cpp


HEADERS += \
	SRP/CPU_SRP/cannonball.h \
    SRP/Compute_SRP/computegpu.h \
    VulkanVisualization/vkvisualization.h \
    VulkanVisualization/vulkanrenderer.h \
    VulkanVisualization/vulkanwindow.h \
        mainwindow.h \
    MeshObjects/triangle.h \
    MeshObjects/material.h \
    MeshObjects/trianglemesh.h \
    MeshObjects/object.h \
    MeshObjects/lineobject.h \
    SRP/srp.h \
    SRP/advancedsrp.h \
    SRP/basicsrp.h \
    DataVisualization/visualizationwindow.h \
    DataVisualization/grid.h \
    DataVisualization/output.h \
    Lib/common.h \
    Lib/vec.h \
    Lib/mat.h \
    GLVisualization/camera.h \
    GLVisualization/glwindow.h \
    GLVisualization/light.h \
    GLVisualization/glwidget.h \
    SRP/GPU_SRP/advancedgpu.h \
    SRP/CPU_SRP/nplate.h \
    SRP/CPU_SRP/raytracecpu.h \
    Lib/glm/core/_detail.hpp \
    Lib/glm/core/_fixes.hpp \
    Lib/glm/core/_swizzle.hpp \
    Lib/glm/core/_swizzle_func.hpp \
    Lib/glm/core/_vectorize.hpp \
    Lib/glm/core/func_common.hpp \
    Lib/glm/core/func_exponential.hpp \
    Lib/glm/core/func_geometric.hpp \
    Lib/glm/core/func_integer.hpp \
    Lib/glm/core/func_matrix.hpp \
    Lib/glm/core/func_noise.hpp \
    Lib/glm/core/func_packing.hpp \
    Lib/glm/core/func_trigonometric.hpp \
    Lib/glm/core/func_vector_relational.hpp \
    Lib/glm/core/hint.hpp \
    Lib/glm/core/intrinsic_common.hpp \
    Lib/glm/core/intrinsic_exponential.hpp \
    Lib/glm/core/intrinsic_geometric.hpp \
    Lib/glm/core/intrinsic_matrix.hpp \
    Lib/glm/core/intrinsic_trigonometric.hpp \
    Lib/glm/core/intrinsic_vector_relational.hpp \
    Lib/glm/core/setup.hpp \
    Lib/glm/core/type.hpp \
    Lib/glm/core/type_float.hpp \
    Lib/glm/core/type_gentype.hpp \
    Lib/glm/core/type_half.hpp \
    Lib/glm/core/type_int.hpp \
    Lib/glm/core/type_mat.hpp \
    Lib/glm/core/type_mat2x2.hpp \
    Lib/glm/core/type_mat2x3.hpp \
    Lib/glm/core/type_mat2x4.hpp \
    Lib/glm/core/type_mat3x2.hpp \
    Lib/glm/core/type_mat3x3.hpp \
    Lib/glm/core/type_mat3x4.hpp \
    Lib/glm/core/type_mat4x2.hpp \
    Lib/glm/core/type_mat4x3.hpp \
    Lib/glm/core/type_mat4x4.hpp \
    Lib/glm/core/type_size.hpp \
    Lib/glm/core/type_vec.hpp \
    Lib/glm/core/type_vec1.hpp \
    Lib/glm/core/type_vec2.hpp \
    Lib/glm/core/type_vec3.hpp \
    Lib/glm/core/type_vec4.hpp \
    Lib/glm/gtc/constants.hpp \
    Lib/glm/gtc/epsilon.hpp \
    Lib/glm/gtc/half_float.hpp \
    Lib/glm/gtc/matrix_access.hpp \
    Lib/glm/gtc/matrix_integer.hpp \
    Lib/glm/gtc/matrix_inverse.hpp \
    Lib/glm/gtc/matrix_transform.hpp \
    Lib/glm/gtc/noise.hpp \
    Lib/glm/gtc/quaternion.hpp \
    Lib/glm/gtc/random.hpp \
    Lib/glm/gtc/reciprocal.hpp \
    Lib/glm/gtc/swizzle.hpp \
    Lib/glm/gtc/type_precision.hpp \
    Lib/glm/gtc/type_ptr.hpp \
    Lib/glm/gtc/ulp.hpp \
    Lib/glm/gtx/associated_min_max.hpp \
    Lib/glm/gtx/bit.hpp \
    Lib/glm/gtx/closest_point.hpp \
    Lib/glm/gtx/color_cast.hpp \
    Lib/glm/gtx/color_space.hpp \
    Lib/glm/gtx/color_space_YCoCg.hpp \
    Lib/glm/gtx/compatibility.hpp \
    Lib/glm/gtx/component_wise.hpp \
    Lib/glm/gtx/constants.hpp \
    Lib/glm/gtx/epsilon.hpp \
    Lib/glm/gtx/euler_angles.hpp \
    Lib/glm/gtx/extend.hpp \
    Lib/glm/gtx/extented_min_max.hpp \
    Lib/glm/gtx/fast_exponential.hpp \
    Lib/glm/gtx/fast_square_root.hpp \
    Lib/glm/gtx/fast_trigonometry.hpp \
    Lib/glm/gtx/gradient_paint.hpp \
    Lib/glm/gtx/handed_coordinate_space.hpp \
    Lib/glm/gtx/inertia.hpp \
    Lib/glm/gtx/int_10_10_10_2.hpp \
    Lib/glm/gtx/integer.hpp \
    Lib/glm/gtx/intersect.hpp \
    Lib/glm/gtx/log_base.hpp \
    Lib/glm/gtx/matrix_cross_product.hpp \
    Lib/glm/gtx/matrix_interpolation.hpp \
    Lib/glm/gtx/matrix_major_storage.hpp \
    Lib/glm/gtx/matrix_operation.hpp \
    Lib/glm/gtx/matrix_query.hpp \
    Lib/glm/gtx/mixed_product.hpp \
    Lib/glm/gtx/multiple.hpp \
    Lib/glm/gtx/noise.hpp \
    Lib/glm/gtx/norm.hpp \
    Lib/glm/gtx/normal.hpp \
    Lib/glm/gtx/normalize_dot.hpp \
    Lib/glm/gtx/number_precision.hpp \
    Lib/glm/gtx/ocl_type.hpp \
    Lib/glm/gtx/optimum_pow.hpp \
    Lib/glm/gtx/orthonormalize.hpp \
    Lib/glm/gtx/perpendicular.hpp \
    Lib/glm/gtx/polar_coordinates.hpp \
    Lib/glm/gtx/projection.hpp \
    Lib/glm/gtx/quaternion.hpp \
    Lib/glm/gtx/random.hpp \
    Lib/glm/gtx/raw_data.hpp \
    Lib/glm/gtx/reciprocal.hpp \
    Lib/glm/gtx/rotate_vector.hpp \
    Lib/glm/gtx/simd_mat4.hpp \
    Lib/glm/gtx/simd_vec4.hpp \
    Lib/glm/gtx/spline.hpp \
    Lib/glm/gtx/std_based_type.hpp \
    Lib/glm/gtx/string_cast.hpp \
    Lib/glm/gtx/transform.hpp \
    Lib/glm/gtx/transform2.hpp \
    Lib/glm/gtx/ulp.hpp \
    Lib/glm/gtx/unsigned_int.hpp \
    Lib/glm/gtx/vec1.hpp \
    Lib/glm/gtx/vector_access.hpp \
    Lib/glm/gtx/vector_angle.hpp \
    Lib/glm/gtx/vector_query.hpp \
    Lib/glm/gtx/verbose_operator.hpp \
    Lib/glm/gtx/wrap.hpp \
    Lib/glm/virtrev/xstream.hpp \
    Lib/glm/ext.hpp \
    Lib/glm/glm.hpp \
    Comparison/comparisonwindow.h \
    Comparison/result.h \
    Comparison/categoryresult.h \
    SRP/GPU_SRP/raytracegputextures.h \
    SRP/GPU_SRP/raytracegpu.h \
    MeshObjects/quad.h \
    SRP/GPU_SRP/render.h

#CONFIG(warn_off){

HEADERS += \
    Lib/eigen3/Eigen/src/Cholesky/LDLT.h \
    Lib/eigen3/Eigen/src/Cholesky/LLT.h \
    Lib/eigen3/Eigen/src/Cholesky/LLT_MKL.h \
    Lib/eigen3/Eigen/src/CholmodSupport/CholmodSupport.h \
    Lib/eigen3/Eigen/src/Core/arch/AltiVec/Complex.h \
    Lib/eigen3/Eigen/src/Core/arch/AltiVec/MathFunctions.h \
    Lib/eigen3/Eigen/src/Core/arch/AltiVec/PacketMath.h \
    Lib/eigen3/Eigen/src/Core/arch/AVX/Complex.h \
    Lib/eigen3/Eigen/src/Core/arch/AVX/MathFunctions.h \
    Lib/eigen3/Eigen/src/Core/arch/AVX/PacketMath.h \
    Lib/eigen3/Eigen/src/Core/arch/AVX/TypeCasting.h \
    Lib/eigen3/Eigen/src/Core/arch/CUDA/MathFunctions.h \
    Lib/eigen3/Eigen/src/Core/arch/CUDA/PacketMath.h \
    Lib/eigen3/Eigen/src/Core/arch/Default/Settings.h \
    Lib/eigen3/Eigen/src/Core/arch/NEON/Complex.h \
    Lib/eigen3/Eigen/src/Core/arch/NEON/MathFunctions.h \
    Lib/eigen3/Eigen/src/Core/arch/NEON/PacketMath.h \
    Lib/eigen3/Eigen/src/Core/arch/SSE/Complex.h \
    Lib/eigen3/Eigen/src/Core/arch/SSE/MathFunctions.h \
    Lib/eigen3/Eigen/src/Core/arch/SSE/PacketMath.h \
    Lib/eigen3/Eigen/src/Core/arch/SSE/TypeCasting.h \
    Lib/eigen3/Eigen/src/Core/functors/AssignmentFunctors.h \
    Lib/eigen3/Eigen/src/Core/functors/BinaryFunctors.h \
    Lib/eigen3/Eigen/src/Core/functors/NullaryFunctors.h \
    Lib/eigen3/Eigen/src/Core/functors/StlFunctors.h \
    Lib/eigen3/Eigen/src/Core/functors/UnaryFunctors.h \
    Lib/eigen3/Eigen/src/Core/products/GeneralBlockPanelKernel.h \
    Lib/eigen3/Eigen/src/Core/products/GeneralMatrixMatrix.h \
    Lib/eigen3/Eigen/src/Core/products/GeneralMatrixMatrix_MKL.h \
    Lib/eigen3/Eigen/src/Core/products/GeneralMatrixMatrixTriangular.h \
    Lib/eigen3/Eigen/src/Core/products/GeneralMatrixMatrixTriangular_MKL.h \
    Lib/eigen3/Eigen/src/Core/products/GeneralMatrixVector.h \
    Lib/eigen3/Eigen/src/Core/products/GeneralMatrixVector_MKL.h \
    Lib/eigen3/Eigen/src/Core/products/Parallelizer.h \
    Lib/eigen3/Eigen/src/Core/products/SelfadjointMatrixMatrix.h \
    Lib/eigen3/Eigen/src/Core/products/SelfadjointMatrixMatrix_MKL.h \
    Lib/eigen3/Eigen/src/Core/products/SelfadjointMatrixVector.h \
    Lib/eigen3/Eigen/src/Core/products/SelfadjointMatrixVector_MKL.h \
    Lib/eigen3/Eigen/src/Core/products/SelfadjointProduct.h \
    Lib/eigen3/Eigen/src/Core/products/SelfadjointRank2Update.h \
    Lib/eigen3/Eigen/src/Core/products/TriangularMatrixMatrix.h \
    Lib/eigen3/Eigen/src/Core/products/TriangularMatrixMatrix_MKL.h \
    Lib/eigen3/Eigen/src/Core/products/TriangularMatrixVector.h \
    Lib/eigen3/Eigen/src/Core/products/TriangularMatrixVector_MKL.h \
    Lib/eigen3/Eigen/src/Core/products/TriangularSolverMatrix.h \
    Lib/eigen3/Eigen/src/Core/products/TriangularSolverMatrix_MKL.h \
    Lib/eigen3/Eigen/src/Core/products/TriangularSolverVector.h \
    Lib/eigen3/Eigen/src/Core/util/BlasUtil.h \
    Lib/eigen3/Eigen/src/Core/util/Constants.h \
    Lib/eigen3/Eigen/src/Core/util/DisableStupidWarnings.h \
    Lib/eigen3/Eigen/src/Core/util/ForwardDeclarations.h \
    Lib/eigen3/Eigen/src/Core/util/Macros.h \
    Lib/eigen3/Eigen/src/Core/util/Memory.h \
    Lib/eigen3/Eigen/src/Core/util/Meta.h \
    Lib/eigen3/Eigen/src/Core/util/MKL_support.h \
    Lib/eigen3/Eigen/src/Core/util/NonMPL2.h \
    Lib/eigen3/Eigen/src/Core/util/ReenableStupidWarnings.h \
    Lib/eigen3/Eigen/src/Core/util/StaticAssert.h \
    Lib/eigen3/Eigen/src/Core/util/XprHelper.h \
    Lib/eigen3/Eigen/src/Core/Array.h \
    Lib/eigen3/Eigen/src/Core/ArrayBase.h \
    Lib/eigen3/Eigen/src/Core/ArrayWrapper.h \
    Lib/eigen3/Eigen/src/Core/Assign.h \
    Lib/eigen3/Eigen/src/Core/Assign_MKL.h \
    Lib/eigen3/Eigen/src/Core/AssignEvaluator.h \
    Lib/eigen3/Eigen/src/Core/BandMatrix.h \
    Lib/eigen3/Eigen/src/Core/Block.h \
    Lib/eigen3/Eigen/src/Core/BooleanRedux.h \
    Lib/eigen3/Eigen/src/Core/CommaInitializer.h \
    Lib/eigen3/Eigen/src/Core/CoreEvaluators.h \
    Lib/eigen3/Eigen/src/Core/CoreIterators.h \
    Lib/eigen3/Eigen/src/Core/CwiseBinaryOp.h \
    Lib/eigen3/Eigen/src/Core/CwiseNullaryOp.h \
    Lib/eigen3/Eigen/src/Core/CwiseUnaryOp.h \
    Lib/eigen3/Eigen/src/Core/CwiseUnaryView.h \
    Lib/eigen3/Eigen/src/Core/DenseBase.h \
    Lib/eigen3/Eigen/src/Core/DenseCoeffsBase.h \
    Lib/eigen3/Eigen/src/Core/DenseStorage.h \
    Lib/eigen3/Eigen/src/Core/Diagonal.h \
    Lib/eigen3/Eigen/src/Core/DiagonalMatrix.h \
    Lib/eigen3/Eigen/src/Core/DiagonalProduct.h \
    Lib/eigen3/Eigen/src/Core/Dot.h \
    Lib/eigen3/Eigen/src/Core/EigenBase.h \
    Lib/eigen3/Eigen/src/Core/ForceAlignedAccess.h \
    Lib/eigen3/Eigen/src/Core/Fuzzy.h \
    Lib/eigen3/Eigen/src/Core/GeneralProduct.h \
    Lib/eigen3/Eigen/src/Core/GenericPacketMath.h \
    Lib/eigen3/Eigen/src/Core/GlobalFunctions.h \
    Lib/eigen3/Eigen/src/Core/Inverse.h \
    Lib/eigen3/Eigen/src/Core/IO.h \
    Lib/eigen3/Eigen/src/Core/Map.h \
    Lib/eigen3/Eigen/src/Core/MapBase.h \
    Lib/eigen3/Eigen/src/Core/MathFunctions.h \
    Lib/eigen3/Eigen/src/Core/Matrix.h \
    Lib/eigen3/Eigen/src/Core/MatrixBase.h \
    Lib/eigen3/Eigen/src/Core/NestByValue.h \
    Lib/eigen3/Eigen/src/Core/NoAlias.h \
    Lib/eigen3/Eigen/src/Core/NumTraits.h \
    Lib/eigen3/Eigen/src/Core/PermutationMatrix.h \
    Lib/eigen3/Eigen/src/Core/PlainObjectBase.h \
    Lib/eigen3/Eigen/src/Core/Product.h \
    Lib/eigen3/Eigen/src/Core/ProductEvaluators.h \
    Lib/eigen3/Eigen/src/Core/Random.h \
    Lib/eigen3/Eigen/src/Core/Redux.h \
    Lib/eigen3/Eigen/src/Core/Ref.h \
    Lib/eigen3/Eigen/src/Core/Replicate.h \
    Lib/eigen3/Eigen/src/Core/ReturnByValue.h \
    Lib/eigen3/Eigen/src/Core/Reverse.h \
    Lib/eigen3/Eigen/src/Core/Select.h \
    Lib/eigen3/Eigen/src/Core/SelfAdjointView.h \
    Lib/eigen3/Eigen/src/Core/SelfCwiseBinaryOp.h \
    Lib/eigen3/Eigen/src/Core/Solve.h \
    Lib/eigen3/Eigen/src/Core/SolverBase.h \
    Lib/eigen3/Eigen/src/Core/SolveTriangular.h \
    Lib/eigen3/Eigen/src/Core/SpecialFunctions.h \
    Lib/eigen3/Eigen/src/Core/StableNorm.h \
    Lib/eigen3/Eigen/src/Core/Stride.h \
    Lib/eigen3/Eigen/src/Core/Swap.h \
    Lib/eigen3/Eigen/src/Core/Transpose.h \
    Lib/eigen3/Eigen/src/Core/Transpositions.h \
    Lib/eigen3/Eigen/src/Core/TriangularMatrix.h \
    Lib/eigen3/Eigen/src/Core/VectorBlock.h \
    Lib/eigen3/Eigen/src/Core/VectorwiseOp.h \
    Lib/eigen3/Eigen/src/Core/Visitor.h \
    Lib/eigen3/Eigen/src/Eigenvalues/ComplexEigenSolver.h \
    Lib/eigen3/Eigen/src/Eigenvalues/ComplexSchur.h \
    Lib/eigen3/Eigen/src/Eigenvalues/ComplexSchur_MKL.h \
    Lib/eigen3/Eigen/src/Eigenvalues/EigenSolver.h \
    Lib/eigen3/Eigen/src/Eigenvalues/GeneralizedEigenSolver.h \
    Lib/eigen3/Eigen/src/Eigenvalues/GeneralizedSelfAdjointEigenSolver.h \
    Lib/eigen3/Eigen/src/Eigenvalues/HessenbergDecomposition.h \
    Lib/eigen3/Eigen/src/Eigenvalues/MatrixBaseEigenvalues.h \
    Lib/eigen3/Eigen/src/Eigenvalues/RealQZ.h \
    Lib/eigen3/Eigen/src/Eigenvalues/RealSchur.h \
    Lib/eigen3/Eigen/src/Eigenvalues/RealSchur_MKL.h \
    Lib/eigen3/Eigen/src/Eigenvalues/SelfAdjointEigenSolver.h \
    Lib/eigen3/Eigen/src/Eigenvalues/SelfAdjointEigenSolver_MKL.h \
    Lib/eigen3/Eigen/src/Eigenvalues/Tridiagonalization.h \
    Lib/eigen3/Eigen/src/Geometry/arch/Geometry_SSE.h \
    Lib/eigen3/Eigen/src/Geometry/AlignedBox.h \
    Lib/eigen3/Eigen/src/Geometry/AngleAxis.h \
    Lib/eigen3/Eigen/src/Geometry/EulerAngles.h \
    Lib/eigen3/Eigen/src/Geometry/Homogeneous.h \
    Lib/eigen3/Eigen/src/Geometry/Hyperplane.h \
    Lib/eigen3/Eigen/src/Geometry/OrthoMethods.h \
    Lib/eigen3/Eigen/src/Geometry/ParametrizedLine.h \
    Lib/eigen3/Eigen/src/Geometry/Quaternion.h \
    Lib/eigen3/Eigen/src/Geometry/Rotation2D.h \
    Lib/eigen3/Eigen/src/Geometry/RotationBase.h \
    Lib/eigen3/Eigen/src/Geometry/Scaling.h \
    Lib/eigen3/Eigen/src/Geometry/Transform.h \
    Lib/eigen3/Eigen/src/Geometry/Translation.h \
    Lib/eigen3/Eigen/src/Geometry/Umeyama.h \
    Lib/eigen3/Eigen/src/Householder/BlockHouseholder.h \
    Lib/eigen3/Eigen/src/Householder/Householder.h \
    Lib/eigen3/Eigen/src/Householder/HouseholderSequence.h \
    Lib/eigen3/Eigen/src/IterativeLinearSolvers/BasicPreconditioners.h \
    Lib/eigen3/Eigen/src/IterativeLinearSolvers/BiCGSTAB.h \
    Lib/eigen3/Eigen/src/IterativeLinearSolvers/ConjugateGradient.h \
    Lib/eigen3/Eigen/src/IterativeLinearSolvers/IncompleteCholesky.h \
    Lib/eigen3/Eigen/src/IterativeLinearSolvers/IncompleteLUT.h \
    Lib/eigen3/Eigen/src/IterativeLinearSolvers/IterativeSolverBase.h \
    Lib/eigen3/Eigen/src/IterativeLinearSolvers/LeastSquareConjugateGradient.h \
    Lib/eigen3/Eigen/src/IterativeLinearSolvers/SolveWithGuess.h \
    Lib/eigen3/Eigen/src/Jacobi/Jacobi.h \
    Lib/eigen3/Eigen/src/LU/arch/Inverse_SSE.h \
    Lib/eigen3/Eigen/src/LU/Determinant.h \
    Lib/eigen3/Eigen/src/LU/FullPivLU.h \
    Lib/eigen3/Eigen/src/LU/InverseImpl.h \
    Lib/eigen3/Eigen/src/LU/PartialPivLU.h \
    Lib/eigen3/Eigen/src/LU/PartialPivLU_MKL.h \
    Lib/eigen3/Eigen/src/MetisSupport/MetisSupport.h \
    Lib/eigen3/Eigen/src/misc/blas.h \
    Lib/eigen3/Eigen/src/misc/Image.h \
    Lib/eigen3/Eigen/src/misc/Kernel.h \
    Lib/eigen3/Eigen/src/OrderingMethods/Amd.h \
    Lib/eigen3/Eigen/src/OrderingMethods/Eigen_Colamd.h \
    Lib/eigen3/Eigen/src/OrderingMethods/Ordering.h \
    Lib/eigen3/Eigen/src/PardisoSupport/PardisoSupport.h \
    Lib/eigen3/Eigen/src/PaStiXSupport/PaStiXSupport.h \
    Lib/eigen3/Eigen/src/plugins/ArrayCwiseBinaryOps.h \
    Lib/eigen3/Eigen/src/plugins/ArrayCwiseUnaryOps.h \
    Lib/eigen3/Eigen/src/plugins/BlockMethods.h \
    Lib/eigen3/Eigen/src/plugins/CommonCwiseBinaryOps.h \
    Lib/eigen3/Eigen/src/plugins/CommonCwiseUnaryOps.h \
    Lib/eigen3/Eigen/src/plugins/MatrixCwiseBinaryOps.h \
    Lib/eigen3/Eigen/src/plugins/MatrixCwiseUnaryOps.h \
    Lib/eigen3/Eigen/src/QR/ColPivHouseholderQR.h \
    Lib/eigen3/Eigen/src/QR/ColPivHouseholderQR_MKL.h \
    Lib/eigen3/Eigen/src/QR/FullPivHouseholderQR.h \
    Lib/eigen3/Eigen/src/QR/HouseholderQR.h \
    Lib/eigen3/Eigen/src/QR/HouseholderQR_MKL.h \
    Lib/eigen3/Eigen/src/SparseCholesky/SimplicialCholesky.h \
    Lib/eigen3/Eigen/src/SparseCholesky/SimplicialCholesky_impl.h \
    Lib/eigen3/Eigen/src/SparseCore/AmbiVector.h \
    Lib/eigen3/Eigen/src/SparseCore/CompressedStorage.h \
    Lib/eigen3/Eigen/src/SparseCore/ConservativeSparseSparseProduct.h \
    Lib/eigen3/Eigen/src/SparseCore/MappedSparseMatrix.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseAssign.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseBlock.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseColEtree.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseCompressedBase.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseCwiseBinaryOp.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseCwiseUnaryOp.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseDenseProduct.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseDiagonalProduct.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseDot.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseFuzzy.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseMap.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseMatrix.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseMatrixBase.h \
    Lib/eigen3/Eigen/src/SparseCore/SparsePermutation.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseProduct.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseRedux.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseRef.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseSelfAdjointView.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseSolverBase.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseSparseProductWithPruning.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseTranspose.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseTriangularView.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseUtil.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseVector.h \
    Lib/eigen3/Eigen/src/SparseCore/SparseView.h \
    Lib/eigen3/Eigen/src/SparseCore/TriangularSolver.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_column_bmod.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_column_dfs.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_copy_to_ucol.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_gemm_kernel.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_heap_relax_snode.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_kernel_bmod.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_Memory.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_panel_bmod.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_panel_dfs.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_pivotL.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_pruneL.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_relax_snode.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_Structs.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_SupernodalMatrix.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLU_Utils.h \
    Lib/eigen3/Eigen/src/SparseLU/SparseLUImpl.h \
    Lib/eigen3/Eigen/src/SparseQR/SparseQR.h \
    Lib/eigen3/Eigen/src/SPQRSupport/SuiteSparseQRSupport.h \
    Lib/eigen3/Eigen/src/StlSupport/details.h \
    Lib/eigen3/Eigen/src/StlSupport/StdDeque.h \
    Lib/eigen3/Eigen/src/StlSupport/StdList.h \
    Lib/eigen3/Eigen/src/StlSupport/StdVector.h \
    Lib/eigen3/Eigen/src/SuperLUSupport/SuperLUSupport.h \
    Lib/eigen3/Eigen/src/SVD/BDCSVD.h \
    Lib/eigen3/Eigen/src/SVD/JacobiSVD.h \
    Lib/eigen3/Eigen/src/SVD/JacobiSVD_MKL.h \
    Lib/eigen3/Eigen/src/SVD/SVDBase.h \
    Lib/eigen3/Eigen/src/SVD/UpperBidiagonalization.h \
    Lib/eigen3/Eigen/src/UmfPackSupport/UmfPackSupport.h \
    Lib/eigen3/Eigen/Cholesky \
    Lib/eigen3/Eigen/CholmodSupport \
    Lib/eigen3/Eigen/Core \
    Lib/eigen3/Eigen/Dense \
    Lib/eigen3/Eigen/Eigen \
    Lib/eigen3/Eigen/Eigenvalues \
    Lib/eigen3/Eigen/Geometry \
    Lib/eigen3/Eigen/Householder \
    Lib/eigen3/Eigen/IterativeLinearSolvers \
    Lib/eigen3/Eigen/Jacobi \
    Lib/eigen3/Eigen/LU \
    Lib/eigen3/Eigen/MetisSupport \
    Lib/eigen3/Eigen/OrderingMethods \
    Lib/eigen3/Eigen/PardisoSupport \
    Lib/eigen3/Eigen/PaStiXSupport \
    Lib/eigen3/Eigen/QR \
    Lib/eigen3/Eigen/QtAlignedMalloc \
    Lib/eigen3/Eigen/Sparse \
    Lib/eigen3/Eigen/SparseCholesky \
    Lib/eigen3/Eigen/SparseCore \
    Lib/eigen3/Eigen/SparseLU \
    Lib/eigen3/Eigen/SparseQR \
    Lib/eigen3/Eigen/SPQRSupport \
    Lib/eigen3/Eigen/StdDeque \
    Lib/eigen3/Eigen/StdList \
    Lib/eigen3/Eigen/StdVector \
    Lib/eigen3/Eigen/SuperLUSupport \
    Lib/eigen3/Eigen/SVD \
    Lib/eigen3/Eigen/UmfPackSupport \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Core/util/CXX11Meta.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Core/util/CXX11Workarounds.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Core/util/EmulateArray.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Core/util/EmulateCXX11Meta.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/Tensor.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorArgMax.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorAssign.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorBase.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorBroadcasting.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorChipping.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorConcatenation.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorContraction.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorContractionCuda.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorContractionThreadPool.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorConversion.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorConvolution.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorCustomOp.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorDevice.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorDeviceCuda.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorDeviceDefault.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorDeviceThreadPool.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorDimensionList.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorDimensions.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorEvalTo.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorEvaluator.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorExecutor.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorExpr.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorFFT.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorFixedSize.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorForcedEval.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorForwardDeclarations.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorFunctors.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorGenerator.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorImagePatch.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorIndexList.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorInflation.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorInitializer.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorIntDiv.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorIO.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorLayoutSwap.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorMacros.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorMap.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorMeta.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorMorphing.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorPadding.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorPatch.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorReduction.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorReductionCuda.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorRef.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorReverse.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorShuffling.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorStorage.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorStriding.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorTraits.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorUInt128.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/Tensor/TensorVolumePatch.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/TensorSymmetry/util/TemplateGroupTheory.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/TensorSymmetry/DynamicSymmetry.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/TensorSymmetry/StaticSymmetry.h \
    Lib/eigen3/unsupported/Eigen/CXX11/src/TensorSymmetry/Symmetry.h \
    Lib/eigen3/unsupported/Eigen/CXX11/Core \
    Lib/eigen3/unsupported/Eigen/CXX11/Tensor \
    Lib/eigen3/unsupported/Eigen/CXX11/TensorSymmetry \
    Lib/eigen3/unsupported/Eigen/src/AutoDiff/AutoDiffJacobian.h \
    Lib/eigen3/unsupported/Eigen/src/AutoDiff/AutoDiffScalar.h \
    Lib/eigen3/unsupported/Eigen/src/AutoDiff/AutoDiffVector.h \
    Lib/eigen3/unsupported/Eigen/src/BVH/BVAlgorithms.h \
    Lib/eigen3/unsupported/Eigen/src/BVH/KdBVH.h \
    Lib/eigen3/unsupported/Eigen/src/Eigenvalues/ArpackSelfAdjointEigenSolver.h \
    Lib/eigen3/unsupported/Eigen/src/FFT/ei_fftw_impl.h \
    Lib/eigen3/unsupported/Eigen/src/FFT/ei_kissfft_impl.h \
    Lib/eigen3/unsupported/Eigen/src/IterativeSolvers/ConstrainedConjGrad.h \
    Lib/eigen3/unsupported/Eigen/src/IterativeSolvers/DGMRES.h \
    Lib/eigen3/unsupported/Eigen/src/IterativeSolvers/GMRES.h \
    Lib/eigen3/unsupported/Eigen/src/IterativeSolvers/IncompleteLU.h \
    Lib/eigen3/unsupported/Eigen/src/IterativeSolvers/IterationController.h \
    Lib/eigen3/unsupported/Eigen/src/IterativeSolvers/MINRES.h \
    Lib/eigen3/unsupported/Eigen/src/IterativeSolvers/Scaling.h \
    Lib/eigen3/unsupported/Eigen/src/KroneckerProduct/KroneckerTensorProduct.h \
    Lib/eigen3/unsupported/Eigen/src/LevenbergMarquardt/LevenbergMarquardt.h \
    Lib/eigen3/unsupported/Eigen/src/LevenbergMarquardt/LMcovar.h \
    Lib/eigen3/unsupported/Eigen/src/LevenbergMarquardt/LMonestep.h \
    Lib/eigen3/unsupported/Eigen/src/LevenbergMarquardt/LMpar.h \
    Lib/eigen3/unsupported/Eigen/src/LevenbergMarquardt/LMqrsolv.h \
    Lib/eigen3/unsupported/Eigen/src/MatrixFunctions/MatrixExponential.h \
    Lib/eigen3/unsupported/Eigen/src/MatrixFunctions/MatrixFunction.h \
    Lib/eigen3/unsupported/Eigen/src/MatrixFunctions/MatrixLogarithm.h \
    Lib/eigen3/unsupported/Eigen/src/MatrixFunctions/MatrixPower.h \
    Lib/eigen3/unsupported/Eigen/src/MatrixFunctions/MatrixSquareRoot.h \
    Lib/eigen3/unsupported/Eigen/src/MatrixFunctions/StemFunction.h \
    Lib/eigen3/unsupported/Eigen/src/MoreVectorization/MathFunctions.h \
    Lib/eigen3/unsupported/Eigen/src/NonLinearOptimization/chkder.h \
    Lib/eigen3/unsupported/Eigen/src/NonLinearOptimization/covar.h \
    Lib/eigen3/unsupported/Eigen/src/NonLinearOptimization/dogleg.h \
    Lib/eigen3/unsupported/Eigen/src/NonLinearOptimization/fdjac1.h \
    Lib/eigen3/unsupported/Eigen/src/NonLinearOptimization/HybridNonLinearSolver.h \
    Lib/eigen3/unsupported/Eigen/src/NonLinearOptimization/LevenbergMarquardt.h \
    Lib/eigen3/unsupported/Eigen/src/NonLinearOptimization/lmpar.h \
    Lib/eigen3/unsupported/Eigen/src/NonLinearOptimization/qrsolv.h \
    Lib/eigen3/unsupported/Eigen/src/NonLinearOptimization/r1mpyq.h \
    Lib/eigen3/unsupported/Eigen/src/NonLinearOptimization/r1updt.h \
    Lib/eigen3/unsupported/Eigen/src/NonLinearOptimization/rwupdt.h \
    Lib/eigen3/unsupported/Eigen/src/NumericalDiff/NumericalDiff.h \
    Lib/eigen3/unsupported/Eigen/src/Polynomials/Companion.h \
    Lib/eigen3/unsupported/Eigen/src/Polynomials/PolynomialSolver.h \
    Lib/eigen3/unsupported/Eigen/src/Polynomials/PolynomialUtils.h \
    Lib/eigen3/unsupported/Eigen/src/Skyline/SkylineInplaceLU.h \
    Lib/eigen3/unsupported/Eigen/src/Skyline/SkylineMatrix.h \
    Lib/eigen3/unsupported/Eigen/src/Skyline/SkylineMatrixBase.h \
    Lib/eigen3/unsupported/Eigen/src/Skyline/SkylineProduct.h \
    Lib/eigen3/unsupported/Eigen/src/Skyline/SkylineStorage.h \
    Lib/eigen3/unsupported/Eigen/src/Skyline/SkylineUtil.h \
    Lib/eigen3/unsupported/Eigen/src/SparseExtra/BlockOfDynamicSparseMatrix.h \
    Lib/eigen3/unsupported/Eigen/src/SparseExtra/BlockSparseMatrix.h \
    Lib/eigen3/unsupported/Eigen/src/SparseExtra/DynamicSparseMatrix.h \
    Lib/eigen3/unsupported/Eigen/src/SparseExtra/MarketIO.h \
    Lib/eigen3/unsupported/Eigen/src/SparseExtra/MatrixMarketIterator.h \
    Lib/eigen3/unsupported/Eigen/src/SparseExtra/RandomSetter.h \
    Lib/eigen3/unsupported/Eigen/src/Splines/Spline.h \
    Lib/eigen3/unsupported/Eigen/src/Splines/SplineFitting.h \
    Lib/eigen3/unsupported/Eigen/src/Splines/SplineFwd.h \
    Lib/eigen3/unsupported/Eigen/AdolcForward \
    Lib/eigen3/unsupported/Eigen/AlignedVector3 \
    Lib/eigen3/unsupported/Eigen/ArpackSupport \
    Lib/eigen3/unsupported/Eigen/AutoDiff \
    Lib/eigen3/unsupported/Eigen/BVH \
    Lib/eigen3/unsupported/Eigen/FFT \
    Lib/eigen3/unsupported/Eigen/IterativeSolvers \
    Lib/eigen3/unsupported/Eigen/KroneckerProduct \
    Lib/eigen3/unsupported/Eigen/LevenbergMarquardt \
    Lib/eigen3/unsupported/Eigen/MatrixFunctions \
    Lib/eigen3/unsupported/Eigen/MoreVectorization \
    Lib/eigen3/unsupported/Eigen/MPRealSupport \
    Lib/eigen3/unsupported/Eigen/NonLinearOptimization \
    Lib/eigen3/unsupported/Eigen/NumericalDiff \
    Lib/eigen3/unsupported/Eigen/OpenGLSupport \
    Lib/eigen3/unsupported/Eigen/Polynomials \
    Lib/eigen3/unsupported/Eigen/Skyline \
    Lib/eigen3/unsupported/Eigen/SparseExtra \
    Lib/eigen3/unsupported/Eigen/Splines
#}

FORMS += \
        mainwindow.ui

OTHER_FILES += \
    vshader1.glsl \
    fshader1.glsl

RESOURCES += \
    resources.qrc

DISTFILES += \
    resources/shaders/fshader1.glsl \
    resources/shaders/raytrace_improved_fshader.glsl \
    resources/shaders/raytrace_rand_xorshift_fshader.glsl \
    resources/shaders/vshader1.glsl \
    resources/images/chartIcon.png \
    resources/images/satelliteIcon.png \
    resources/images/satelliteIcon2.png \
    resources/shaders/zbuffer_fshader.glsl \
    resources/shaders/raytrace_multiple_scattering_fshader.glsl \
    resources/shaders/render_fshader.glsl
    #Lib/eigen3/signature_of_eigen3_matrix_library

