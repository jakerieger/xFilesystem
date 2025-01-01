// Author: Jake Rieger
// Created: 12/12/24.
//

#include "Filesystem.hpp"
#include <sstream>

#ifdef _WIN32
    // Windows does not define the S_ISREG and S_ISDIR macros in stat.h, so we do.
    // We have to define _CRT_INTERNAL_NONSTDC_NAMES 1 before #including sys/stat.h
    // in order for Microsoft's stat.h to define names like S_IFMT, S_IFREG, and S_IFDIR,
    // rather than just defining  _S_IFMT, _S_IFREG, and _S_IFDIR as it normally does.
    #define _CRT_INTERNAL_NONSTDC_NAMES 1
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h>
    #include <sys/stat.h>
    #if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
        #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
    #endif
    #if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
        #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
    #endif
#else
    #include <sys/stat.h>
#endif

namespace x::Filesystem {
#pragma region FileReader
    std::vector<u8> FileReader::readAllBytes(const str& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) { return {}; }
        const std::streamsize fileSize = file.tellg();
        std::vector<u8> bytes(fileSize);
        file.seekg(0, std::ios::beg);
        if (!file.read(reinterpret_cast<char*>(bytes.data()), fileSize)) { return {}; }
        file.close();
        return bytes;
    }

    str FileReader::readAllText(const str& path) {
        const std::ifstream file(path);
        if (!file.is_open()) { return {}; }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::vector<str> FileReader::readAllLines(const str& path) {
        std::ifstream file(path);
        std::vector<str> lines;
        if (!file.is_open()) { return {}; }
        str line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
        return lines;
    }

    std::vector<u8> FileReader::readBlock(const str& path, size_t size, u64 offset) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file) { return {}; }
        const std::streamsize fileSize = file.tellg();
        if (offset >= (u64)fileSize || size == 0 || offset + size > (u64)fileSize) { return {}; }
        file.seekg((std::streamsize)offset, std::ios::beg);
        if (!file) { return {}; }
        std::vector<u8> buffer(size);
        file.read(reinterpret_cast<char*>(buffer.data()), (std::streamsize)size);
        if (!file) { return {}; }
        return buffer;
    }

    size_t FileReader::queryFileSize(const str& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open()) { return 0; }
        const std::streamsize fileSize = file.tellg();
        return fileSize;
    }
#pragma endregion

#pragma region FileWriter
    bool FileWriter::writeAllBytes(const str& path, const std::vector<u8>& data) {
        std::ofstream file(path,
                           std::ios::binary | std::ios::trunc);  // Overwrite existing file
        if (!file) return false;
        file.write(RCAST<const char*>(data.data()), CAST<std::streamsize>(data.size()));
        return file.good();
    }

    bool FileWriter::writeAllText(const str& path, const str& text) {
        std::ofstream file(path, std::ios::out | std::ios::trunc);
        if (!file) return false;
        file << text;
        return file.good();
    }

    bool FileWriter::writeAllLines(const str& path, const std::vector<str>& lines) {
        std::ofstream file(path, std::ios::out | std::ios::trunc);
        if (!file) return false;
        for (const auto& line : lines) {
            file << line << '\n';
            if (!file.good()) { return false; }
        }
        return file.good();
    }

    bool FileWriter::writeBlock(const str& path, const std::vector<u8>& data, u64 offset) {
        std::ofstream file(path,
                           std::ios::binary | std::ios::in |
                             std::ios::out);  // Open in binary read/write mode
        if (!file) return false;
        file.seekp(CAST<std::streampos>(offset), std::ios::beg);  // seek to offset
        if (!file) return false;                                  // Failed to seek
        file.write(RCAST<const char*>(data.data()), CAST<std::streamsize>(data.size()));
        return file.good();
    }

    std::future<std::vector<u8>> AsyncFileReader::readAllBytes(const str& path) {
        return runAsync([path]() { return FileReader::readAllBytes(path); });
    }

    std::future<str> AsyncFileReader::readAllText(const str& path) {
        return runAsync([path]() { return FileReader::readAllText(path); });
    }

    std::future<std::vector<str>> AsyncFileReader::readAllLines(const str& path) {
        return runAsync([path]() { return FileReader::readAllLines(path); });
    }

    std::future<std::vector<u8>>
    AsyncFileReader::readBlock(const str& path, size_t size, u64 offset) {
        return runAsync(
          [path, size, offset]() { return FileReader::readBlock(path, size, offset); });
    }

    std::future<bool> AsyncFileWriter::writeAllBytes(const str& path, const std::vector<u8>& data) {
        return runAsync([path, data]() { return FileWriter::writeAllBytes(path, data); });
    }

    std::future<bool> AsyncFileWriter::writeAllText(const str& path, const str& text) {
        return runAsync([path, text]() { return FileWriter::writeAllText(path, text); });
    }

    std::future<bool> AsyncFileWriter::writeAllLines(const str& path,
                                                     const std::vector<str>& lines) {
        return runAsync([path, lines]() { return FileWriter::writeAllLines(path, lines); });
    }

    std::future<bool>
    AsyncFileWriter::writeBlock(const str& path, const std::vector<u8>& data, u64 offset) {
        return runAsync(
          [path, data, offset]() { return FileWriter::writeBlock(path, data, offset); });
    }
#pragma endregion

#pragma region Path
    Path Path::currentPath() {
        char buffer[1024];
        if (!getcwd(buffer, sizeof(buffer))) {
            throw std::runtime_error("Failed to get current working directory");
        }
        return Path(buffer);
    }

    Path Path::parent() const {
        const size_t lastSeparator = path.find_last_of(PATH_SEPARATOR);
        if (lastSeparator == std::string::npos || lastSeparator == 0) {
            return Path(std::to_string(PATH_SEPARATOR));
        }
        return Path(path.substr(0, lastSeparator));
    }

    bool Path::exists() const {
        struct stat info {};
        return stat(path.c_str(), &info) == 0;
    }

    bool Path::isFile() const {
        struct stat info {};
        if (stat(path.c_str(), &info) != 0) {
            std::perror(path.c_str());
            return false;
        }
        return S_ISREG(info.st_mode);
    }

    bool Path::isDirectory() const {
        struct stat info {};
        if (stat(path.c_str(), &info) != 0) {
            std::perror(path.c_str());
            return false;
        }
        return S_ISDIR(info.st_mode);
    }

    bool Path::hasExtension() const {
        const size_t pos = path.find_last_of('.');
        const size_t sep = path.find_last_of(PATH_SEPARATOR);
        return pos != str::npos && (sep == str::npos || pos > sep);
    }

    str Path::extension() const {
        if (!hasExtension()) { return ""; }
        return path.substr(path.find_last_of('.') + 1);
    }

    Path Path::replaceExtension(const str& ext) const {
        if (!hasExtension()) return Path(path + "." + ext);
        return Path(path.substr(0, path.find_last_of('.')) + "." + ext);
    }

    Path Path::join(const str& subPath) const {
        return Path(join(path, subPath));
    }

    Path Path::operator/(const str& subPath) const {
        return Path(join(path, subPath));
    }

    str Path::string() const {
        return path;
    }

    const char* Path::cStr() const {
        return path.c_str();
    }

    bool Path::operator==(const Path& other) const {
        return path == other.path;
    }

    bool Path::create() const {
        if (exists()) return true;

#ifdef _WIN32
        if (!CreateDirectoryA(path.c_str(), nullptr)) {
            const DWORD error = GetLastError();
            if (error != ERROR_ALREADY_EXISTS) { return false; }
        }
#else
        if (mkdir(path.c_str(), 0755) != 0) {
            if (errno != EEXIST) { return false; }
        }
#endif
        return true;
    }

    bool Path::createAll() const {
        if (exists()) return true;

        if (path != str(1, PATH_SEPARATOR)) {
            Path parentPath = parent();
            if (!parentPath.exists()) {
                if (!parentPath.createAll()) return false;
            }
        }

        return create();
    }

    str Path::join(const str& lhs, const str& rhs) {
        if (lhs.empty()) { return lhs; }
        if (rhs.empty()) { return rhs; }
        if (lhs.back() == PATH_SEPARATOR) return lhs + rhs;
        return lhs + PATH_SEPARATOR + rhs;
    }

    str Path::normalize(const str& rawPath) {
        str result;
        std::vector<str> parts;
        size_t start = 0;
        while (start < rawPath.size()) {
            size_t end = rawPath.find(PATH_SEPARATOR, start);
            if (end == std::string::npos) { end = rawPath.size(); }
            str part = rawPath.substr(start, end - start);
            if (part == ".." && !parts.empty() && parts.back() != "..") {
                parts.pop_back();
            } else if (!part.empty() && part != ".") {
                parts.push_back(part);
            }
            start = end + 1;
        }
        for (const auto& part : parts) {
            result += PATH_SEPARATOR + part;
        }

#ifdef _WIN32
        // Remove the first '/' if Windows path
        result = result.substr(1, result.size() - 1);
#endif

        return result.empty() ? str(1, PATH_SEPARATOR) : result;
    }
#pragma endregion
}  // namespace x::Filesystem
