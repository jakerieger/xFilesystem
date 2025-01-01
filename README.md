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

int main() {
    // Get current path
    Path currentPath = Path::currentPath();
    
    // Append to path
    Path readmePath = currentPath / "README.md";
    // or
    Path readmePath  = currentPath.join("README.md");
    
    // Path operations
    /**
    readmePath.exists();
    readmePath.extension();
    readmePath.hasExtension();
    readmePath.replaceExtension(".txt");
    readmePath.isFile();
    readmePath.isDirectory();
    readmePath.parent();
    readmePath.create();
    readmePath.createAll();
    **/
    
    // String representations
    std::string readmeString = readmePath.string();
    const char* readmeCString = readmePath.cStr();
    
    // File reading
    std::vector<uint8_t> bytes = FileReader::readAllBytes(readmePath.string());
  
    auto asyncResult = AsyncFileReader::readAllText(readmePath.string());
    try {
        std::string readText = asyncResult.get();
    } catch (...) {}
    
    // File writing
    auto writeResult = FileWriter::writeAllBytes((currentPath / "newFile.txt").string());
    if (!writeResult) { Panic("Failed to write file bytes"); }
    
    // Async same as above for writing: AsyncFileWriter
    
    return 0;
}
```