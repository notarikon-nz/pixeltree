# cmake/modules/FindSIMD.cmake
include(CheckCXXSourceCompiles)

# Check for SSE2 support
check_cxx_source_compiles("
    #include <emmintrin.h>
    int main() {
        __m128i a = _mm_setzero_si128();
        return 0;
    }
" SIMD_SSE2_FOUND)

# Check for AVX2 support
check_cxx_source_compiles("
    #include <immintrin.h>
    int main() {
        __m256i a = _mm256_setzero_si256();
        return 0;
    }
" SIMD_AVX2_FOUND)

# Set compiler flags for SIMD
if(SIMD_SSE2_FOUND)
    if(MSVC)
        set(SIMD_SSE2_FLAGS "/arch:SSE2")
    else()
        set(SIMD_SSE2_FLAGS "-msse2")
    endif()
endif()

if(SIMD_AVX2_FOUND)
    if(MSVC)
        set(SIMD_AVX2_FLAGS "/arch:AVX2")
    else()
        set(SIMD_AVX2_FLAGS "-mavx2")
    endif()
endif()