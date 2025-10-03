# Build & Directory Conventions

This project standardizes all build artifacts under a single hierarchy inside `05-implementation/build/` to eliminate directory sprawl and accidental commits.

## Directory Layout
```
05-implementation/
  build/
    ci/        # CI pipeline builds (Debug unless overridden)
    dev/       # Local developer builds (Debug fast iteration)
    tdd/       # Red/Green/Refactor focused test cycles
    release/   # Optimized Release builds
    coverage/  # Instrumented builds for coverage reports (future)
```

## Naming Rationale
- Single root `build/` (scoped to implementation) keeps artifacts isolated from repository root.
- Subfolders communicate intent (ci, dev, tdd, release) and can be safely removed independently.
- Future tooling (coverage, sanitizers) can add new subfolders without colliding with existing ones.

## Legacy Directories
Previously used directories may still appear locally:
- `build_ci/`
- `build_tdd/`
- `build_tmp/`

They are now ignored via `.gitignore` and should be deleted locally when convenient.

## Generating Builds
From repository root (examples in Bash / Ubuntu runner):
```
cd 05-implementation
cmake -B build/dev -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=17 -DBUILD_TESTING=ON
cmake --build build/dev -- -j$(nproc)
```
Run tests with CTest:
```
cd 05-implementation/build/dev
ctest --output-on-failure
```

## Recommended Profiles
| Profile | Folder     | Flags / Options (suggested)                               | Purpose |
|---------|------------|------------------------------------------------------------|---------|
| CI      | build/ci   | `-DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON`              | Automated validation |
| Dev     | build/dev  | `-DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON`              | Everyday coding |
| TDD     | build/tdd  | `-DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON`              | Focused red/green cycles |
| Release | build/release | `-DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON`         | Packaging, perf checks |
| Coverage| build/coverage | (future) `--coverage` or `-fprofile-arcs -ftest-coverage` | Coverage metrics |

## Cleaning
Remove all build artifacts safely:
```
cd 05-implementation
rm -rf build
```
Or targeted cleanup:
```
rm -rf build/tdd build/dev
```

## Windows (PowerShell) Examples
```
Set-Location 05-implementation
cmake -B build/dev -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=17 -DBUILD_TESTING=ON
cmake --build build/dev --config Debug -- /m
ctest --test-dir build/dev --output-on-failure
```

## Future Extensions
- Add `ENABLE_COVERAGE=ON` CMake option and produce gcovr XML into `build/coverage`.
- Introduce toolchain-specific subfolders if cross-compiling (e.g., `build/arm64/ci`).

## Governance
Changes to this convention must be captured in an ADR and reflected here to maintain traceability.
