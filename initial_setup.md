# 1. Create project structure
mkdir -p pixeltree/{cmake/modules,docs,include/pixeltree,src,tests,examples,tools,scripts}

# 2. Initialize git repository
cd pixeltree
git init
git add .
git commit -m "Initial project structure"

# 3. Setup vcpkg (if not already installed)
./scripts/setup_vcpkg.sh

# 4. Configure and build
cmake --preset default
cmake --build build/default

# 5. Run tests
ctest --test-dir build/default --output-on-failure