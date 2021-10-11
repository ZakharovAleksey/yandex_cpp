//
// Created by azakharov on 10/11/2021.
//

#include "sprint_11_tasks.h"

namespace sprint_11 {

namespace {

void PrintTreeHelper(std::ostream& dst, const path& p, int depth) {
    auto directory_content = std::filesystem::directory_iterator(p);

    std::vector<path> dir_content_names;
    for (const auto& current_path : directory_content)
        dir_content_names.emplace_back(current_path.path().filename());

    std::sort(dir_content_names.begin(), dir_content_names.end(),
              [](const path& left, const path& right) { return right < left; });

    for (const auto current_path : dir_content_names) {
        auto status = std::filesystem::status(p / current_path);

        if (status.type() == std::filesystem::file_type::regular) {
            dst << std::string(depth, ' ') << current_path.string() << '\n';
        }

        if (status.type() == std::filesystem::file_type::directory) {
            dst << std::string(depth, ' ') << current_path.string() << '\n';
            PrintTreeHelper(dst, p / current_path, depth + 2);
        }
    }
}
}  // namespace

void PrintTree(std::ostream& out, const path& root_path) {
    out << root_path.filename().string() << '\n';
    PrintTreeHelper(out, root_path, 2);
}


}  // namespace sprint_11