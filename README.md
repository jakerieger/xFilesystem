# x::Filesystem

Filesystem is a lightweight alternative to `std::filesystem` for C++17. It's cross-platform and provides common I/O operations:

- Read/write text files
- Read/write binary files
- Read/write chunks of data from a file
- Async read/write
- Query file size
- `Path` abstraction for representing system file paths.

## Quickstart

Simply add `Filesystem.hpp`/`Filesystem.cpp` to your application sources. That's it.

## Usage

```cpp
#include "Filesystem.hpp"
using namespace x::Filesystem;
```

### File I/O

```cpp
// File reading
std::vector<uint8_t> bytes = FileReader::readAllBytes(readmePath.string());

auto asyncResult = AsyncFileReader::readAllText(readmePath.string());
try {
    std::string readText = asyncResult.get();
} catch (...) {}

// File writing
auto writeResult = FileWriter::writeAllBytes((currentPath / "newFile.txt").string(), {0x0});
if (!writeResult) { Panic("Failed to write file bytes"); }

// Async same as above for writing: AsyncFileWriter
```

### `Filesystem::Path` class

```cpp
// Append to path
Path readmePath = currentPath / "README.md";
// or
Path readmePath  = currentPath.join("README.md");

// Path operations
Path parent     = readmePath.parent();
bool exists     = readmePath.exists();
bool hasExt     = readmePath.hasExtension();
bool isFile     = readmePath.isFile();
bool isDir      = readmePath.isDirectory();
bool created    = readmePath.create();
bool createdAll = readmePath.createAll();
std::string ext = readmePath.extension();
readmePath.replaceExtension(".txt");

// String representations
std::string readmeString = readmePath.string();
const char* readmeCString = readmePath.cStr();
```