rm -rf build-mingw
cmake -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DKISAK_PLATFORM=mingw \
  -DCMAKE_C_COMPILER=i686-w64-mingw32-clang \
  -DCMAKE_CXX_COMPILER=i686-w64-mingw32-clang++ \
  -DCMAKE_RC_COMPILER=i686-w64-mingw32-llvm-rc \
  -DCMAKE_SYSTEM_NAME=Windows \
  -DKISAK_SOUND=ON \
  -DCMAKE_SYSTEM_PROCESSOR=x86 \
  -B build-mingw 2>&1 | tee build-mingw.log

ninja -C build-mingw KisakCOD-mp
