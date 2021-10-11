#pragma once

#include <filesystem>
#include <fstream>

using std::filesystem::path;

namespace sprint_11 {

/* PRINT DIRECTORIES TREE */

void PrintTree(std::ostream& out, const path& root_path);
}  // namespace sprint_11