#pragma once

#include <Eigen/Core>
#include <Eigen/Sparse>

#define PREC_FLOAT
//#define PREC_DOUBLE

#if (defined(PREC_FLOAT) && defined(PREC_DOUBLE))
	#error "Can not Define PREC_FLOAT and PREC_DOUBLE both at the same time"
#elif defined(PREC_FLOAT)
	using Size_type = size_t;
	using Scalar_type = float;
	using Index_type = int;
	using Mat3_type = Eigen::Matrix3f;
	using Vec2_type = Eigen::Vector2f;
	using Vec3_type = Eigen::Vector3f;
	using VectorX_type = Eigen::VectorXf;
	using MatrixX_type = Eigen::MatrixXf;

	template <int row, int col>
	using Matrix_type = Eigen::Matrix<Scalar_type, row, col>;

	template <int length>
	using Vector_type = Eigen::Vector<Scalar_type, length>;

	using Elements4_type = std::vector<std::tuple<Index_type, Index_type, Index_type, Index_type>>;
	using Elements3_type = std::vector<std::tuple<Index_type, Index_type, Index_type>>;
	using Edges_type = std::vector<std::tuple<Index_type, Index_type>>;
	using Triplet_type = Eigen::Triplet<Scalar_type>;
	using SparseMat_type = Eigen::SparseMatrix<Scalar_type>;

#elif defined(USER_MODE)

#else
	#error "Define PREC_FLOAT or PREC_DOUBLE"
#endif

