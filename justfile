# Tests

[working-directory: 'tests/interlang']
test-interlang-codegen:
    python ../../src/blitzbuffers schema.bzb \
        -l rust ./rust/src/schema.rs \
        -l cpp ./cpp/schema.h

[working-directory: 'tests/interlang/rust']
test-interlang-rust *args:
    cargo run -- {{args}}

[working-directory: 'tests/interlang/cpp']
test-interlang-cpp-build:
    cmake -B build
    cmake --build build

[working-directory: 'tests/interlang/cpp']
[windows]
test-interlang-cpp *args: test-interlang-cpp-build
    ./build/Debug/cpp_tests.exe {{args}}

[working-directory: 'tests/interlang/cpp']
[linux]
test-interlang-cpp *args: test-interlang-cpp-build
    ./build/Debug/cpp_tests {{args}}


test-interlang-gen: test-interlang-codegen test-interlang-cpp test-interlang-rust

test-interlang-check: (test-interlang-cpp "c") (test-interlang-rust "c")

test: test-interlang-gen test-interlang-check


# Benchmarking

[working-directory: 'benchmarks']
bench-codegen:
    python ../src/blitzbuffers ./schemas/blitzbuffers.bzb \
        -l rust ./rust/src/blitzbuffers_generated.rs \
        -l cpp ./cpp/blitzbuffers_generated.h

[working-directory: 'benchmarks/rust']
bench-rust *args:
    cargo run --release -- {{args}}

[working-directory: 'benchmarks/cpp']
bench-cpp-build:
    cmake -B build
    cmake --build build --config Release

[working-directory: 'benchmarks/cpp']
[windows]
bench-cpp *args: bench-cpp-build
    ./build/Release/cpp_benchmarks.exe {{args}}

[working-directory: 'benchmarks/cpp']
[linux]
bench-cpp *args: bench-cpp-build
    ./build/Release/cpp_benchmarks {{args}}

bench: bench-codegen bench-cpp bench-rust

build:
    rm -rf dist
    py -m build --sdist --wheel
