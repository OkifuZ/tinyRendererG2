#pragma once

#include "simulation_type.h"

#include <fstream>

inline void log_sparse_mat(const std::string& log_path, SparseMat_type& mat, const std::string& desc) {
	std::ofstream of(log_path, std::ios::trunc);
	try {
		if (of.is_open()) {
			of << desc << "\n";
			of << "cols: " << mat.cols() << ", rows: " << mat.rows() << "\n";
			of << "nonzeros: " << mat.nonZeros() << "\n";
			for (int i = 0; i < mat.outerSize(); i++) {
				for (SparseMat_type::InnerIterator it(mat, i); it; ++it) {
					of << "(" << it.col() << ", " << it.row() << ", " << it.value() << ")\n";
				}
			}
		}
	}
	catch (...) {}
}