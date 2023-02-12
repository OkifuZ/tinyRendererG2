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
	using Index_type = size_t;
	using Mat3_type = Eigen::Matrix3f;
	using Vector_type = Eigen::VectorXf;
	using Elements_type = std::vector<size_t>;
	using Triplet_type = Eigen::Triplet<Scalar_type>;

#elif defined(USER_MODE)

#else
	#error "Define PREC_FLOAT or PREC_DOUBLE"
#endif

