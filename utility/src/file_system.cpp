#include "file_system.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <exception>

bool save_str_to_file(const std::filesystem::path& fpath, const std::string& content) {
    try {
        std::ofstream of(fpath, std::ios::trunc);
        if (!of.is_open()) return false;
        of << content << std::endl;
        of.close();
    }
    catch (...) { return false; }
    return true;
}

std::string read_file_as_str(const std::string& fpath) {
    std::filesystem::path fp(fpath);
    fp = fp.make_preferred().lexically_normal();
    try {
        if (!std::filesystem::exists(fp)) {
            printf("[FileSystem] \"%s\" does not exists.\n", fp.string().c_str());
            return "";
        }
        if (!std::filesystem::is_regular_file(fp)) {
            printf("[FileSystem] \"%s\" is not a regular file.\n", fp.string().c_str());
            return "";
        }
        std::ifstream t(fp.string());
        std::stringstream buffer;
        buffer << t.rdbuf();
        return buffer.str();
    }
    catch (...) {
        printf("[FileSystem] \"%s\" is not a good path.\n", fp.string().c_str());
        return "";
    }
}

bool exist_file(const std::string& fpath) {
    try {
        std::filesystem::path fp(fpath);
        fp = fp.make_preferred().lexically_normal();
        return std::filesystem::exists(fp);
    }
    catch (...) {
        return false;
    }
}


std::string current_path() {
    std::error_code es;
    std::string res = std::filesystem::current_path(es).string();
    if (es) {
        printf("[FileSystem] error on fetching current path.\n");
        return "";
    }
    return res;
}

std::string get_relative_path(std::string fpath) {
    std::filesystem::path fp(fpath);
    fp = fp.make_preferred().lexically_normal();
    std::error_code es;
    std::string& res = std::filesystem::relative(fp, es).string();
    if (es) {
        printf("[FileSystem] \"%s\" is not related to current dir \"%s\".\n",
            fp.string().c_str(), current_path().c_str());
    }
    return res;
}

