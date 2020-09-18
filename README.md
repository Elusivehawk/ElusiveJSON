# ElusiveJSON

## What is it?

ElusiveJSON is yet another library for JSON/JSON5 parsing. Its:
* **Minimalist**: Only does what it needs to do, nothing more. Nothing crazy, no template hell.
* **Header-only**: The entire code base is in a single C++ header file.
* **Fast**: Benchmarks pending, but the design is meant to be lightweight and speedy. Branches are only taken when needed, and memory is pre-allocated to both prevent memory leaks and reduce memory allocations in total. Speed is the #1 concern.
* **Small**: The header file takes up less than 1000 LOC.
* **Compatible**: Uses C++11, has no dependencies, and is architecture agnostic (x86, ARM, etc.).
* **FOSS**: Licensed under GNU GPL v3. Probably subject to change.

## How to use

ElusiveJSON is meant to be easy to use, and scalable with your project.

```
#include "hawkutils/elusivejson.h"
using namespace ElusiveJSON;
std::string jsonData = readFile("example.json");//replace with your file reading method of choice.
JMalloc* malloc = new JMalloc();
JParser reader(malloc, &jsonData);//create actual reader
JValue* jobj = nullptr;
try {
	jobj = reader.parseJValue();
} catch (std::exception& e) {
	std::cout << e.what() << std::endl;
}
```

In just 11 lines of code, you can have a JSON file read and parsed. From here, methods can be called to check the actual type of JValue and retrieve values.

## Contributing

For now the project uses Visual Studio 2019, so development is limited to Windows. A future update may include CMake support. Pull requests are welcome and appreciated. Until a code style guide is made, I'll just reformat code as it comes in.

## To-Do

* CMake support
* Unit testing
* Full UTF-8 support
* Documentation
* Tutorials