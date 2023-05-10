# LLVM for LC-3.2

This repository houses a version of LLVM that has a backend for the LC-3.2
architecture. It was modified from LLVM 16.0.0, and much of the documentation
follows from there. In particular, it would be good to read the original
[Getting Started Guide](https://llvm.org/docs/GettingStarted.html) and the
original [Compilation Guide](https://llvm.org/docs/CMake.html).

## Usage

### Build Environment

For ease-of-use, a `docker-compose.yml` script is provided. Its corresponding
`Dockerfile` provides all the packages needed to build LLVM.

### Building

To build this project, check out the repository and navigate into it. Create a
`build` subdirectory and change into it too. Configure with
```
$ cmake -G Ninja \
  -DCMAKE_INSTALL_PREFIX=${LLVM_INSTALL_DIR} \
  -DCMAKE_BUILD_TYPE=Debug -DLLVM_ENABLE_ASSERTIONS=ON \
  -DLLVM_ENABLE_PROJECTS="clang;lld" -DLLVM_ENABLE_RUNTIMES="" \
  -DLLVM_TARGETS_TO_BUILD="" -DLLVM_EXPERIMENTAL_TARGETS_TO_BUILD="LC32" \
  ../llvm/
```
Make sure to set `CMAKE_INSTALL_PREFIX` to the installation directory. Also
change `CMAKE_BUILD_TYPE` as needed. It is recommended to keep assertions `ON`
even when building in `Release` mode on account of this project's instability.

During development, one may enable `LLVM_OPTIMIZED_TABLEGEN`,
`BUILD_SHARED_LIBS`, and `LLVM_USE_SPLIT_DWARF` to reduce build times.
Furthermore, one can build unit tests by enabling `LLVM_BUILD_TESTS`, though
these take a signigicant amount of time to link, and they're not even used at
present.

If memory usage is a concern, consider setting `LLVM_PARALLEL_LINK_JOBS`.

With configuration done, build and install with
```
$ ninja
$ ninja install
```

### Running

Once this project is installed, let's say to `${LLVM_INSTALL_DIR}`, build with
the driver program
```
$ ${LLVM_INSTALL_DIR}/bin/clang --target=lc_3.2 ...
```
By default, it will try to link `libc` and `libm`. Either set `-nostdlib` or
pass `--sysroot` to point the driver to the libraries. It will also try to link
the compiler runtime. Again, either set `-nodefaultlibs` or pass `--sysroot`.

The driver does not link with `crt0`, nor does it have a default linker script.
You must supply both.


## Contributing

To contribute, please file an issue or a pull request on this GitHub.
Alternatively, email the project maintainers directly.
