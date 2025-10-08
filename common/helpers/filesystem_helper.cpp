#include "filesystem_helper.h"

std::vector<std::string> FileSystemHelper::ScanFiles(const std::string& dirPath, const std::string& suffix)
{
    std::vector<std::string> files;

    for (auto& entry : std::filesystem::directory_iterator(dirPath))
    {
        if (entry.is_regular_file())
        {
            auto path = entry.path();
            if (path.extension() == ".bmp" || path.extension() == ".BMP")
            {
                files.push_back(path.string());
            }
        }
    }

    // 按文件名中的数字排序，例如 0.bmp, 1.bmp, 2.bmp...
    std::sort(files.begin(), files.end(),
              [](const std::string& a, const std::string& b) {
                  int na = std::stoi(std::filesystem::path(a).stem().string());
                  int nb = std::stoi(std::filesystem::path(b).stem().string());
                  return na < nb;
              });

    return files;
}
