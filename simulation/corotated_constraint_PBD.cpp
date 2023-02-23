#include "constraint_PBD.h"
#include <limits>

Scalar_type corotated_constraint_energy(const Mat3_type& F, const Mat3_type& R);
Vector_type<12> coratated_constraint_gradient(const Mat3_type& U, const Vec3_type& S, const Mat3_type& V,
	const Mat3_type& Dm_inv, Scalar_type C);

Matrix_type<9, 12> compute_pF_pX_flattened_9x12(const Mat3_type& Dm_inv);
Matrix_type<9, 9> compute_pS_pF_flattened_9x9(const Mat3_type& U, const Mat3_type& V);
Matrix_type<9, 1> compute_pC_pS_flattened_9x1(const Vec3_type& Sigma, Scalar_type C);

SimPBD::CorotatedConstraint::CorotatedConstraint(std::initializer_list<Index_type> indices, Scalar_type stiff,
	const VectorX_type& pos)
	: Constraint(indices, stiff) {
	const Vec3_type& v1 = pos.block<3, 1>(this->indices[0] * 3, 0);
	const Vec3_type& v2 = pos.block<3, 1>(this->indices[1] * 3, 0);
	const Vec3_type& v3 = pos.block<3, 1>(this->indices[2] * 3, 0);
	const Vec3_type& v4 = pos.block<3, 1>(this->indices[3] * 3, 0);
	this->v_0 = (v2 - v1).cross(v3 - v1).dot(v4 - v1);
	Mat3_type Dm;
	Dm.col(0) = Vec3_type{ v2 - v1 };
	Dm.col(1) = Vec3_type{ v3 - v1 };
	Dm.col(2) = Vec3_type{ v4 - v1 };
	this->Dm_inv = Dm.inverse();
}


void SimPBD::CorotatedConstraint::resolve(VectorX_type& position, const VectorX_type& inv_mass, Scalar_type dt) {
	Index_type v1_ind = this->indices.at(0);
	Index_type v2_ind = this->indices.at(1);
	Index_type v3_ind = this->indices.at(2);
	Index_type v4_ind = this->indices.at(3);
	auto& v1 = position.block<3, 1>(v1_ind * 3, 0);
	auto& v2 = position.block<3, 1>(v2_ind * 3, 0);
	auto& v3 = position.block<3, 1>(v3_ind * 3, 0);
	auto& v4 = position.block<3, 1>(v4_ind * 3, 0);

	Mat3_type Ds;
	Ds.col(0) = v2 - v1;
	Ds.col(1) = v3 - v1;
	Ds.col(2) = v4 - v1;

	Mat3_type F = Ds * this->Dm_inv;

	Scalar_type det_f = F.determinant();
	bool const is_tet_inverted = F.determinant() < Scalar_type{ 0 };

	if (is_tet_inverted) {
		Eigen::JacobiSVD<Mat3_type> SVD(F, Eigen::ComputeFullU | Eigen::ComputeFullV);
		Mat3_type const& U = SVD.matrixU();
		Mat3_type const& V = SVD.matrixV();
		Vec3_type sigma = SVD.singularValues();
		
		/*
		sigma(0) = std::clamp(sigma(0), 0.75f, 1.25f);
		sigma(1) = std::clamp(sigma(1), 0.75f, 1.25f);
		sigma(2) = std::clamp(sigma(2), 0.75f, 1.25f);
		*/

		Scalar_type min_sigma = std::numeric_limits<Scalar_type>::max();
		int min_idx = -1;
		for (int i = 0; i < 3; i++) {
			if (sigma[i] < 0 && min_sigma > std::abs(sigma[i])) {
				min_sigma = std::abs(sigma[i]);
				min_idx = i;
			}
		}
		sigma[min_idx] *= -1.0f;

		F = U * sigma.asDiagonal() * V.transpose();;
	}

	Eigen::JacobiSVD<Mat3_type> SVD(F, Eigen::ComputeFullU | Eigen::ComputeFullV);
	Mat3_type const& U = SVD.matrixU();
	Mat3_type const& V = SVD.matrixV();
	Vec3_type sigma = SVD.singularValues();
	
	Scalar_type C = this->v_0 * corotated_constraint_energy(F, U * V.transpose());
	Vector_type<12> C_gradient = this->v_0 * coratated_constraint_gradient(U, sigma, V, this->Dm_inv, C);

	// gradient of C wrt v1&v2&v3&v4
	const Vec3_type& grad_C_x1 = C_gradient.block<3, 1>(0, 0);
	const Vec3_type& grad_C_x2 = C_gradient.block<3, 1>(3, 0);
	const Vec3_type& grad_C_x3 = C_gradient.block<3, 1>(6, 0);
	const Vec3_type& grad_C_x4 = C_gradient.block<3, 1>(9, 0);

	// lambda
	Scalar_type k1 = inv_mass[v1_ind];
	Scalar_type k2 = inv_mass[v2_ind];
	Scalar_type k3 = inv_mass[v3_ind];
	Scalar_type k4 = inv_mass[v4_ind];
	Scalar_type grad_C_x1_sq = grad_C_x1.dot(grad_C_x1);
	Scalar_type grad_C_x2_sq = grad_C_x2.dot(grad_C_x2);
	Scalar_type grad_C_x3_sq = grad_C_x3.dot(grad_C_x3);
	Scalar_type grad_C_x4_sq = grad_C_x4.dot(grad_C_x4);
	Scalar_type lambda = -C /
		(k1 * grad_C_x1_sq + k2 * grad_C_x2_sq + k3 * grad_C_x3_sq + k4 * grad_C_x4_sq +
			this->alpha / (dt * dt));

	// delta X & update X
	v1 += (lambda * k1 * grad_C_x1);
	v2 += (lambda * k2 * grad_C_x2);
	v3 += (lambda * k3 * grad_C_x3);
	v4 += (lambda * k4 * grad_C_x4);
}


Scalar_type corotated_constraint_energy(const Mat3_type& F, const Mat3_type& R) {
	return (F - R).norm(); // Frobenius norm
}

Vector_type<12> coratated_constraint_gradient(const Mat3_type& U, const Vec3_type& S, const Mat3_type& V, 
	const Mat3_type& Dm_inv, Scalar_type C) 
{
	Matrix_type<9, 12> pF_pX_flattened = compute_pF_pX_flattened_9x12(Dm_inv); // 9 * 12
	Matrix_type<9, 9> pS_pF_flattened = compute_pS_pF_flattened_9x9(U, V); // 9 * 9, S stands for Sigma
	Matrix_type<9, 1> pC_pS_flattened = compute_pC_pS_flattened_9x1(S, C); // 9 * 1

	return pF_pX_flattened.transpose() * pS_pF_flattened.transpose() * pC_pS_flattened;
}


Matrix_type<9, 12> compute_pF_pX_flattened_9x12(const Mat3_type& Dm_inv) {
	const Scalar_type m = Dm_inv(0, 0);
	const Scalar_type n = Dm_inv(0, 1);
	const Scalar_type o = Dm_inv(0, 2);
	const Scalar_type p = Dm_inv(1, 0);
	const Scalar_type q = Dm_inv(1, 1);
	const Scalar_type r = Dm_inv(1, 2);
	const Scalar_type s = Dm_inv(2, 0);
	const Scalar_type t = Dm_inv(2, 1);
	const Scalar_type u = Dm_inv(2, 2);
	
	const Scalar_type t1 = -m - p - s;
	const Scalar_type t2 = -n - q - t;
	const Scalar_type t3 = -o - r - u;
	
	Matrix_type<9, 12> pF_pX;
	pF_pX.setZero();

	pF_pX(0, 0) = t1;
	pF_pX(0, 3) = m;
	pF_pX(0, 6) = p;
	pF_pX(0, 9) = s;
	pF_pX(1, 1) = t1;
	pF_pX(1, 4) = m;
	pF_pX(1, 7) = p;
	pF_pX(1, 10) = s;
	pF_pX(2, 2) = t1;
	pF_pX(2, 5) = m;
	pF_pX(2, 8) = p;
	pF_pX(2, 11) = s;
	pF_pX(3, 0) = t2;
	pF_pX(3, 3) = n;
	pF_pX(3, 6) = q;
	pF_pX(3, 9) = t;
	pF_pX(4, 1) = t2;
	pF_pX(4, 4) = n;
	pF_pX(4, 7) = q;
	pF_pX(4, 10) = t;
	pF_pX(5, 2) = t2;
	pF_pX(5, 5) = n;
	pF_pX(5, 8) = q;
	pF_pX(5, 11) = t;
	pF_pX(6, 0) = t3;
	pF_pX(6, 3) = o;
	pF_pX(6, 6) = r;
	pF_pX(6, 9) = u;
	pF_pX(7, 1) = t3;
	pF_pX(7, 4) = o;
	pF_pX(7, 7) = r;
	pF_pX(7, 10) = u;
	pF_pX(8, 2) = t3;
	pF_pX(8, 5) = o;
	pF_pX(8, 8) = r;
	pF_pX(8, 11) = u;
	
	return pF_pX;
}

Matrix_type<9, 9> compute_pS_pF_flattened_9x9(const Mat3_type& U, const Mat3_type& V) {
	Matrix_type<9, 9> pS_pF;
	for (int j = 0; j < 3; j++) {
		for (int i = 0; i < 3; i++) {
			auto& pS_pFij = pS_pF.col(j * 3 + i); // column major
			// pS/pF_ij = U^T * pF/pF_ij * V 
			// col0
			pS_pFij[0] = U(i, 0) * V(j, 0);
			pS_pFij[1] = U(i, 1) * V(j, 0);
			pS_pFij[2] = U(i, 2) * V(j, 0);
			// col1
			pS_pFij[3] = U(i, 0) * V(j, 1);
			pS_pFij[4] = U(i, 1) * V(j, 1);
			pS_pFij[5] = U(i, 2) * V(j, 1);
			// col2
			pS_pFij[6] = U(i, 0) * V(j, 2);
			pS_pFij[7] = U(i, 1) * V(j, 2);
			pS_pFij[8] = U(i, 2) * V(j, 2);
		}
	}

	return pS_pF;
}

Matrix_type<9, 1> compute_pC_pS_flattened_9x1(const Vec3_type& Sigma, Scalar_type C) {
	Matrix_type<9, 1> pC_pS;
	pC_pS.setZero();
	pC_pS(0, 0) = Sigma[0] - 1;
	pC_pS(4, 0) = Sigma[1] - 1;
	pC_pS(8, 0) = Sigma[2] - 1;
	pC_pS /= C;
	return pC_pS;
}