#include "constraint_PD.h"


SimPD::StrainConstraint::StrainConstraint(
	std::initializer_list<Index_type> indices, Scalar_type k,
	const Vector_type& pos, Scalar_type sigma_min, Scalar_type sigma_max)
	: indices(indices), k(k), sigma_min(sigma_min), sigma_max(sigma_max)
{
	if (indices.size() != 4) {
		printf("[StarinConstraint] indice size error\n");
		exit(-1);
	}

	auto v1 = this->indices.at(0);
	auto v2 = this->indices.at(1);
	auto v3 = this->indices.at(2);
	auto v4 = this->indices.at(3);

	auto p1 = pos.block(v1 * 3, 0, 3, 1);
	auto p2 = pos.block(v2 * 3, 0, 3, 1);
	auto p3 = pos.block(v3 * 3, 0, 3, 1);
	auto p4 = pos.block(v4 * 3, 0, 3, 1);

	Mat3_type Dm;
	Dm.col(0) = p1 - p4;
	Dm.col(1) = p2 - p4;
	Dm.col(2) = p3 - p4;

	this->V_0 = std::abs((1.0f / 6.0f) * Dm.determinant());
	this->Dm_inv = Dm.inverse();
}

std::vector<Triplet_type> SimPD::StrainConstraint::get_v_k_G_GT(const Vector_type& p) const {
    // G_j*G_j^T
    auto const N = p.rows() / 3;
    const Size_type vi = this->indices.at(0) * 3;
    const Size_type vj = this->indices.at(1) * 3;
    const Size_type vk = this->indices.at(2) * 3;
    const Size_type vl = this->indices.at(3) * 3;
   
    auto const weight = this->k * this->V_0;

    const auto& d11 = Dm_inv(0, 0);
    const auto& d21 = Dm_inv(1, 0);
    const auto& d31 = Dm_inv(2, 0);
    const auto& d12 = Dm_inv(0, 1);
    const auto& d22 = Dm_inv(1, 1);
    const auto& d32 = Dm_inv(2, 1);
    const auto& d13 = Dm_inv(0, 2);
    const auto& d23 = Dm_inv(1, 2);
    const auto& d33 = Dm_inv(2, 2);

    // precompute often used quantities
    const Scalar_type _d11_d21_d31 = -d11 - d21 - d31;
    const Scalar_type _d12_d22_d32 = -d12 - d22 - d32;
    const Scalar_type _d13_d23_d33 = -d13 - d23 - d33;

    // col 1
    Scalar_type const s1_1 = d11 * d11 + d12 * d12 + d13 * d13;
    Scalar_type const s4_1 = d11 * d21 + d12 * d22 + d13 * d23;
    Scalar_type const s7_1 = d11 * d31 + d12 * d32 + d13 * d33;
    Scalar_type const s10_1 = d11 * _d11_d21_d31 + d12 * _d12_d22_d32 + d13 * _d13_d23_d33;
    // col 2
    Scalar_type const s2_2 = s1_1;
    Scalar_type const s5_2 = s4_1;
    Scalar_type const s8_2 = s7_1;
    Scalar_type const s11_2 = s10_1;
    // col 3
    Scalar_type const s3_3 = s1_1;
    Scalar_type const s6_3 = s4_1;
    Scalar_type const s9_3 = s7_1;
    Scalar_type const s12_3 = s10_1;
    // col 4
    Scalar_type const s1_4 = d11 * d21 + d12 * d22 + d13 * d23;
    Scalar_type const s4_4 = d21 * d21 + d22 * d22 + d23 * d23;
    Scalar_type const s7_4 = d31 * d21 + d32 * d22 + d33 * d23;
    Scalar_type const s10_4 = d21 * (_d11_d21_d31)+d22 * (_d12_d22_d32)+d23 * (_d13_d23_d33);
    // col 5
    Scalar_type const s2_5 = s1_4;
    Scalar_type const s5_5 = s4_4;
    Scalar_type const s8_5 = s7_4;
    Scalar_type const s11_5 = s10_4;
    // col 6
    Scalar_type const s3_6 = s1_4;
    Scalar_type const s6_6 = s4_4;
    Scalar_type const s9_6 = s7_4;
    Scalar_type const s12_6 = s10_4;

    // col 7
    Scalar_type const s1_7 = d11 * d31 + d12 * d32 + d13 * d33;
    Scalar_type const s4_7 = d21 * d31 + d22 * d32 + d23 * d33;
    Scalar_type const s7_7 = d31 * d31 + d32 * d32 + d33 * d33;
    Scalar_type const s10_7 = _d11_d21_d31 * d31 + _d12_d22_d32 * d32 + _d13_d23_d33 * d33;
    // col 8
    Scalar_type const s2_8 = s1_7;
    Scalar_type const s5_8 = s4_7;
    Scalar_type const s8_8 = s7_7;
    Scalar_type const s11_8 = s10_7;
    // col 9
    Scalar_type const s3_9 = s1_7;
    Scalar_type const s6_9 = s4_7;
    Scalar_type const s9_9 = s7_7;
    Scalar_type const s12_9 = s10_7;
    // col 10
    Scalar_type const s1_10 = _d11_d21_d31 * d11 + _d12_d22_d32 * d12 + _d13_d23_d33 * d13;
    Scalar_type const s4_10 = _d11_d21_d31 * d21 + _d12_d22_d32 * d22 + _d13_d23_d33 * d23;
    Scalar_type const s7_10 = _d11_d21_d31 * d31 + _d12_d22_d32 * d32 + _d13_d23_d33 * d33;
    Scalar_type const s10_10 = (_d11_d21_d31) * (_d11_d21_d31)+(_d12_d22_d32) * (_d12_d22_d32)+
        (_d13_d23_d33) * (_d13_d23_d33);
    // col 11
    Scalar_type const s2_11 = s1_10;
    Scalar_type const s5_11 = s4_10;
    Scalar_type const s8_11 = s7_10;
    Scalar_type const s11_11 = s10_10;

    // col 12
    Scalar_type const s3_12 = s1_10;
    Scalar_type const s6_12 = s4_10;
    Scalar_type const s9_12 = s7_10;
    Scalar_type const s12_12 = s10_10;

    std::array<Eigen::Triplet<Scalar_type>, 12u * 4u> triplets;
    int const row1 = vi;
    int const row2 = vi + 1;
    int const row3 = vi + 2;
    int const row4 = vj;
    int const row5 = vj + 1;
    int const row6 = vj + 2;
    int const row7 = vk;
    int const row8 = vk + 1;
    int const row9 = vk + 2;
    int const row10 = vl;
    int const row11 = vl + 1;
    int const row12 = vl + 2;

    int const col1 = row1;
    int const col2 = row2;
    int const col3 = row3;
    int const col4 = row4;
    int const col5 = row5;
    int const col6 = row6;
    int const col7 = row7;
    int const col8 = row8;
    int const col9 = row9;
    int const col10 = row10;
    int const col11 = row11;
    int const col12 = row12;
    // col 1
    triplets[0] = { row1, col1, weight * s1_1 };
    triplets[1] = { row4, col1, weight * s4_1 };
    triplets[2] = { row7, col1, weight * s7_1 };
    triplets[3] = { row10, col1, weight * s10_1 };
    // col 2
    triplets[4] = { row2, col2, weight * s2_2 };
    triplets[5] = { row5, col2, weight * s5_2 };
    triplets[6] = { row8, col2, weight * s8_2 };
    triplets[7] = { row11, col2, weight * s11_2 };
    // col 3
    triplets[8] = { row3, col3, weight * s3_3 };
    triplets[9] = { row6, col3, weight * s6_3 };
    triplets[10] = { row9, col3, weight * s9_3 };
    triplets[11] = { row12, col3, weight * s12_3 };
    // col 4
    triplets[12] = { row1, col4, weight * s1_4 };
    triplets[13] = { row4, col4, weight * s4_4 };
    triplets[14] = { row7, col4, weight * s7_4 };
    triplets[15] = { row10, col4, weight * s10_4 };
    // col 5
    triplets[16] = { row2, col5, weight * s2_5 };
    triplets[17] = { row5, col5, weight * s5_5 };
    triplets[18] = { row8, col5, weight * s8_5 };
    triplets[19] = { row11, col5, weight * s11_5 };
    // col 6
    triplets[20] = { row3, col6, weight * s3_6 };
    triplets[21] = { row6, col6, weight * s6_6 };
    triplets[22] = { row9, col6, weight * s9_6 };
    triplets[23] = { row12, col6, weight * s12_6 };
    // col 7
    triplets[24] = { row1, col7, weight * s1_7 };
    triplets[25] = { row4, col7, weight * s4_7 };
    triplets[26] = { row7, col7, weight * s7_7 };
    triplets[27] = { row10, col7, weight * s10_7 };
    // col 8
    triplets[28] = { row2, col8, weight * s2_8 };
    triplets[29] = { row5, col8, weight * s5_8 };
    triplets[30] = { row8, col8, weight * s8_8 };
    triplets[31] = { row11, col8, weight * s11_8 };
    // col 9
    triplets[32] = { row3, col9, weight * s3_9 };
    triplets[33] = { row6, col9, weight * s6_9 };
    triplets[34] = { row9, col9, weight * s9_9 };
    triplets[35] = { row12, col9, weight * s12_9 };
    // col 10
    triplets[36] = { row1, col10, weight * s1_10 };
    triplets[37] = { row4, col10, weight * s4_10 };
    triplets[38] = { row7, col10, weight * s7_10 };
    triplets[39] = { row10, col10, weight * s10_10 };
    // col 11
    triplets[40] = { row2, col11, weight * s2_11 };
    triplets[41] = { row5, col11, weight * s5_11 };
    triplets[42] = { row8, col11, weight * s8_11 };
    triplets[43] = { row11, col11, weight * s11_11 };
    // col 12
    triplets[44] = { row3, col12, weight * s3_12 };
    triplets[45] = { row6, col12, weight * s6_12 };
    triplets[46] = { row9, col12, weight * s9_12 };
    triplets[47] = { row12, col12, weight * s12_12 };

    return std::vector<Triplet_type>{triplets.begin(), triplets.end()};
}

void SimPD::StrainConstraint::get_v_k_G_ST_p(const Vector_type& q, Vector_type& b) const {
    // G_j*S_j^T*P

    // project -> p
    auto const N = q.rows() / 3;
    const Size_type vi = this->indices.at(0) * 3;
    const Size_type vj = this->indices.at(1) * 3;
    const Size_type vk = this->indices.at(2) * 3;
    const Size_type vl = this->indices.at(3) * 3;

    Vec3_type const q1 = q.block(vi, 0, 3, 1);
    Vec3_type const q2 = q.block(vj, 0, 3, 1);
    Vec3_type const q3 = q.block(vk, 0, 3, 1);
    Vec3_type const q4 = q.block(vl, 0, 3, 1);

    Mat3_type Ds;
    Ds.col(0) = q1 - q4;
    Ds.col(1) = q2 - q4;
    Ds.col(2) = q3 - q4;

    const Mat3_type F = Ds * this->Dm_inv;

    Scalar_type det_f = F.determinant();
    bool const is_tet_inverted = F.determinant() < Scalar_type{ 0 };
    // if (is_tet_inverted) printf("inverted! F det: %.5f\n", det_f);

    Eigen::JacobiSVD<Mat3_type> SVD(F, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Mat3_type const& U = SVD.matrixU();
    Mat3_type const& V = SVD.matrixV();

    Vec3_type sigma = SVD.singularValues();

    if (is_tet_inverted)
    {
        sigma(2) = -sigma(2);
    } // put this where?

    sigma(0) = std::clamp(sigma(0), this->sigma_min, this->sigma_max);
    sigma(1) = std::clamp(sigma(1), this->sigma_min, this->sigma_max);
    sigma(2) = std::clamp(sigma(2), this->sigma_min, this->sigma_max);

    /*if (is_tet_inverted)
    {
        sigma(2) = -sigma(2);
    }*/ // put this where?

    // p_j
    Mat3_type const Fhat = U * sigma.asDiagonal() * V.transpose();

    // calculate G_j*S_j^T*p_j
    Scalar_type const& p1 = Fhat(0, 0);
    Scalar_type const& p2 = Fhat(1, 0);
    Scalar_type const& p3 = Fhat(2, 0);
    Scalar_type const& p4 = Fhat(0, 1);
    Scalar_type const& p5 = Fhat(1, 1);
    Scalar_type const& p6 = Fhat(2, 1);
    Scalar_type const& p7 = Fhat(0, 2);
    Scalar_type const& p8 = Fhat(1, 2);
    Scalar_type const& p9 = Fhat(2, 2);

    auto const weight = this->k * this->V_0;

    auto const& d11 = this->Dm_inv(0, 0);
    auto const& d21 = this->Dm_inv(1, 0);
    auto const& d31 = this->Dm_inv(2, 0);
    auto const& d12 = this->Dm_inv(0, 1);
    auto const& d22 = this->Dm_inv(1, 1);
    auto const& d32 = this->Dm_inv(2, 1);
    auto const& d13 = this->Dm_inv(0, 2);
    auto const& d23 = this->Dm_inv(1, 2);
    auto const& d33 = this->Dm_inv(2, 2);

    Scalar_type const _d11_d21_d31 = -d11 - d21 - d31;
    Scalar_type const _d12_d22_d32 = -d12 - d22 - d32;
    Scalar_type const _d13_d23_d33 = -d13 - d23 - d33;

    // we have already symbolically computed wi * (Ai*Si)^T * Bi * pi
    Scalar_type const bi0 = (d11 * p1) + (d12 * p4) + (d13 * p7);
    Scalar_type const bi1 = (d11 * p2) + (d12 * p5) + (d13 * p8);
    Scalar_type const bi2 = (d11 * p3) + (d12 * p6) + (d13 * p9);
    Scalar_type const bj0 = (d21 * p1) + (d22 * p4) + (d23 * p7);
    Scalar_type const bj1 = (d21 * p2) + (d22 * p5) + (d23 * p8);
    Scalar_type const bj2 = (d21 * p3) + (d22 * p6) + (d23 * p9);
    Scalar_type const bk0 = (d31 * p1) + (d32 * p4) + (d33 * p7);
    Scalar_type const bk1 = (d31 * p2) + (d32 * p5) + (d33 * p8);
    Scalar_type const bk2 = (d31 * p3) + (d32 * p6) + (d33 * p9);
    Scalar_type const bl0 = p1 * (_d11_d21_d31)+p4 * (_d12_d22_d32)+p7 * (_d13_d23_d33);
    Scalar_type const bl1 = p2 * (_d11_d21_d31)+p5 * (_d12_d22_d32)+p8 * (_d13_d23_d33);
    Scalar_type const bl2 = p3 * (_d11_d21_d31)+p6 * (_d12_d22_d32)+p9 * (_d13_d23_d33);

    b(vi + 0) += weight * bi0;
    b(vi + 1) += weight * bi1;
    b(vi + 2) += weight * bi2;
    b(vj + 0) += weight * bj0;
    b(vj + 1) += weight * bj1;
    b(vj + 2) += weight * bj2;
    b(vk + 0) += weight * bk0;
    b(vk + 1) += weight * bk1;
    b(vk + 2) += weight * bk2;
    b(vl + 0) += weight * bl0;
    b(vl + 1) += weight * bl1;
    b(vl + 2) += weight * bl2;
}