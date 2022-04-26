# REIO

> Miniature library for data I/O and primitive (de)serialization,
> built with reverse-engineering file formats in mind.

## Contents

Library provides two basic kinds of abstractions:

   - buffers - used to store/pass around blocks of bytes;
   - streams - sequential I/O primitives providing buffer and number (de)serialization.

Only a few implementations are provided for each category, though that bare
minimum has proved to be highly useful across several personal projects
which involved writing tools for working with reverse-engineered file
formats.

Despite the origins of this library, it is designed to be used easily
in any context that requires reading and writing arbitrary binary data.
By default, **reio** uses a custom exception to express failures,
but the macro-based system used across the library should allow
the programmer to use any kind of irregular error reporting logic
under the hood.

### Usage

**reio** is expected to be built with the host codebase, and to get
statically linked into it. Library's small size and few included headers
shouldn't impact your build times much.

Just put the library's directory into your `third_party`/`external`/etc.
directory and use CMake's `add_subdirectory` to include it into the project.
If you are using a build system other than CMake, use all `includes/**/*.hpp`
files as headers and `src/**/*.cpp` files as sources, and add the `includes/`
directory into your include path.

It requires a modern compiler, used together with the latest language
standard. Two main combinations are supported:

   - Windows / MSVC 2022;
   - Linux / GCC 11.

### Documentation

Headers and sources are pretty heavily documented, so you should be able
to use those as a reference. The extensive test suite provides a lot of
usage samples as well.

To build a static documentation, you can run Doxygen in the project's root.

## License

This library and its test suite is licensed under **MIT License**,
a copy of which should be located in the project's root, and is
also reproduced here:

```
Copyright (c) 2022 Alex Sobolev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

The only dependency, **Catch2**, is licensed under the **Boost Software License**,
a copy of which should be located in `./external/Catch2/LICENSE.txt`.