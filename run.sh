cd build
make -j$(nproc) || exit 1
cd ..
./build/mi_app
