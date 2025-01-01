// Author: Jake Rieger
// Created: 12/12/24.
//

#pragma once

#pragma region Typedefs
#include <cstdint>
#include <cstring>
#include <string>

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

#if defined(__GNUC__) || defined(__clang__)
using u128 = __uint128_t;
using i128 = __int128_t;
#endif

using f32 = float;
using f64 = double;

using cstr = const char*;
using str  = std::string;
using wstr = std::wstring;

#define CAST static_cast
#define CCAST const_cast
#define DCAST dynamic_cast
#define RCAST reinterpret_cast
#pragma endregion

#include <fstream>
#include <vector>
#include <stdexcept>
#include <future>
#include <cstdarg>
#include <iostream>

#ifdef _WIN32
    #include <direct.h>
    #define getcwd _getcwd
    #define PATH_SEPARATOR '\\'
#else
    #include <unistd.h>
    #define PATH_SEPARATOR '/'
#endif

namespace x {
    [[noreturn]] static void
    panic_impl(const char* file, int line, const char* func, const char* fmt, ...) noexcept {
        va_list args;
        va_start(args, fmt);
        char message[1024];
        vsnprintf(message, sizeof(message), fmt, args);
        va_end(args);
        std::cerr << file << "(" << line << ") :: PANIC\n"
                  << "  In: " << func << "\n"
                  << "  Message: " << message << std::endl;
        std::abort();
    }

#ifndef Panic
    /// @brief Terminates the program, displaying the provided error message as well as the file,
    /// line, and function name where the panic occurred.
    #define Panic(fmt, ...) x::panic_impl(__FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__)
#endif

    namespace Filesystem {
        class FileReader {
        public:
            static std::vector<u8> readAllBytes(const str& path);
            static str readAllText(const str& path);
            static std::vector<str> readAllLines(const str& path);
            static std::vector<u8> readBlock(const str& path, size_t size, u64 offset = 0);
            static size_t queryFileSize(const str& path);
        };

        class FileWriter {
        public:
            static bool writeAllBytes(const str& path, const std::vector<u8>& data);
            static bool writeAllText(const str& path, const str& text);
            static bool writeAllLines(const str& path, const std::vector<str>& lines);
            static bool writeBlock(const str& path, const std::vector<u8>& data, u64 offset = 0);
        };

        class AsyncFileReader {
        public:
            static std::future<std::vector<u8>> readAllBytes(const str& path);
            static std::future<str> readAllText(const str& path);
            static std::future<std::vector<str>> readAllLines(const str& path);
            static std::future<std::vector<u8>>
            readBlock(const str& path, size_t size, u64 offset = 0);

        private:
            template<typename Func>
            static auto runAsync(Func&& func) -> std::future<decltype(func())> {
                using ReturnType = decltype(func());
                auto task =
                  std::make_shared<std::packaged_task<ReturnType()>>(std::forward<Func>(func));
                std::future<ReturnType> future = task->get_future();
                std::thread([task]() { (*task)(); }).detach();
                return future;
            }
        };

        class AsyncFileWriter {
        public:
            static std::future<bool> writeAllBytes(const str& path, const std::vector<u8>& data);
            static std::future<bool> writeAllText(const str& path, const str& text);
            static std::future<bool> writeAllLines(const str& path, const std::vector<str>& lines);
            static std::future<bool>
            writeBlock(const str& path, const std::vector<u8>& data, u64 offset = 0);

        private:
            template<typename Func>
            static auto runAsync(Func&& func) -> std::future<decltype(func())> {
                using ReturnType = decltype(func());
                auto task =
                  std::make_shared<std::packaged_task<ReturnType()>>(std::forward<Func>(func));
                std::future<ReturnType> future = task->get_future();
                std::thread([task]() { (*task)(); }).detach();
                return future;
            }
        };

        class StreamReader {};

        class StreamWriter {};

        class Path {
        public:
            explicit Path(const str& path) : path(normalize(path)) {}
            static Path currentPath();

            [[nodiscard]] Path parent() const;
            [[nodiscard]] bool exists() const;
            [[nodiscard]] bool isFile() const;
            [[nodiscard]] bool isDirectory() const;
            [[nodiscard]] bool hasExtension() const;
            [[nodiscard]] str extension() const;
            [[nodiscard]] Path replaceExtension(const str& ext) const;
            [[nodiscard]] Path join(const str& subPath) const;
            [[nodiscard]] Path operator/(const str& subPath) const;
            [[nodiscard]] str string() const;
            [[nodiscard]] const char* cStr() const;
            [[nodiscard]] bool operator==(const Path& other) const;

            bool create() const;
            bool createAll() const;

        private:
            str path;
            static str join(const str& lhs, const str& rhs);
            static str normalize(const str& rawPath);
        };
    };  // namespace Filesystem

#undef Panic
}  // namespace x
