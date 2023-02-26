#pragma once
#include <string>
#include <filesystem>
#include "json11.hpp"

bool save_str_to_file(const std::filesystem::path& fpath, const std::string& content);

bool exist_file(const std::string& fpath);

std::string read_file_as_str(const std::string& fpath);

std::string current_path();

std::string get_relative_path(std::string fpath);