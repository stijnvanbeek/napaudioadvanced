/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <utility/dllexport.h>

#include <audio/utility/simde/x86/avx2.h>
#include <audio/utility/simde/x86/sse2.h>

//#if defined(__AVX__)
//	#include <immintrin.h> // AVX (float8) intrinsics
//#else
//	#error "AVX instructions are disabled. Something is wrong in your CMake config."
//#endif

//#include <emmintrin.h> // SSE2 (float4) intrinsics

namespace nap
{
    
    // define types
	
	typedef simde__m128 float4_value;
    
    struct NAPAPI float4
    {
        float4_value value;
        
        float4()
        {
        }
        
        explicit float4(const int in_f)
        {
            const float f = (float)in_f;
			
			value = simde_mm_set1_ps(f);
        }
        
        explicit float4(const float f)
        {
			value = simde_mm_set1_ps(f);
        }
        
        explicit float4(const float f1, const float f2, const float f3, const float f4)
        {
			value = simde_mm_set_ps(f4, f3, f2, f1);
        }
        
        explicit float4(const double in_f)
        {
            const float f = (float)in_f;
			
			value = simde_mm_set1_ps(f);
        }
        
        explicit float4(float4_value in_value)
        {
            value = in_value;
        }
        
        explicit float4(const float * __restrict f)
        {
			value = simde_mm_loadu_ps(f);
        }
        
        float4 operator+(const float4 other) const
        {
			return float4(simde_mm_add_ps(value, other.value));
        }
        
        float4 operator-(const float4 other) const
        {
			return float4(simde_mm_sub_ps(value, other.value));
        }
        
        float4 operator*(const float4 other) const
        {
			return float4(simde_mm_mul_ps(value, other.value));
        }
        
        float4 operator/(const float4 other) const
        {
			return float4(simde_mm_div_ps(value, other.value));
        }
        
        float4 operator*(const float other) const
        {
            return *this * float4(other);
        }
        
        float4 operator*(const int other) const
        {
            return *this * float4(other);
        }
        
        float& operator[](const int index)
        {
            return reinterpret_cast<float*>(&value)[index];
        }
        
        const float& operator[](const int index) const
        {
            return reinterpret_cast<const float*>(&value)[index];
        }
    };
	
	typedef simde__m256 float8_value;
	
    struct NAPAPI float8
    {
        float8_value value;
        
        float8()
        {
        }
        
        explicit float8(const int in_f)
        {
            const float f = (float)in_f;
			
			value = simde_mm256_set1_ps(f);
        }
        
        explicit float8(const float f)
        {
			value = simde_mm256_set1_ps(f);
        }
        
        explicit float8(const float f1, const float f2, const float f3, const float f4, const float f5, const float f6, const float f7, const float f8)
        {
			value = simde_mm256_set_ps(f8, f7, f6, f5, f4, f3, f2, f1);
        }
        
        explicit float8(const double in_f)
        {
            const float f = (float)in_f;
			
			value = simde_mm256_set1_ps(f);
        }
        
        explicit float8(float8_value in_value)
        {
            value = in_value;
        }
        
        explicit float8(const float * __restrict f)
        {
			value = simde_mm256_loadu_ps(f);
        }
        
        float8 operator+(const float8 other) const
        {
			return float8(simde_mm256_add_ps(value, other.value));
        }
        
        float8 operator-(const float8 other) const
        {
			return float8(simde_mm256_sub_ps(value, other.value));
        }
        
        float8 operator*(const float8 other) const
        {
			return float8(simde_mm256_mul_ps(value, other.value));
        }
        
        float8 operator/(const float8 other) const
        {
			return float8(simde_mm256_div_ps(value, other.value));
        }
        
        float8 operator*(const float other) const
        {
            return *this * float8(other);
        }
        
        float8 operator*(const int other) const
        {
            return *this * float8(other);
        }
        
        float& operator[](const int index)
        {
            return reinterpret_cast<float*>(&value)[index];
        }
        
        bool operator==(const float8 other)
        {
		#if defined(__AVX__)
			auto mask = _mm256_cmp_ps(value, other.value, _CMP_NEQ_OQ);
			return _mm256_movemask_ps(mask) == 0;
		#else
            for (auto i = 0; i < 8; ++i)
                if ((*this)[i] != other[i])
                    return false;
            return true;
		#endif
        }
        
        bool operator!=(const float8 other)
        {
		#if defined(__AVX__)
			auto mask = _mm256_cmp_ps(value, other.value, _CMP_NEQ_OQ);
			return _mm256_movemask_ps(mask) != 0;
		#else
            return !((*this) == other);
		#endif
        }
        
        const float& operator[](const int index) const
        {
            return reinterpret_cast<const float*>(&value)[index];
        }
    };
    
    
    float4 NAPAPI tanVec(const float4 value);
    float8 NAPAPI tanVec(const float8 value);
    float4 NAPAPI sinVec(const float4 value);
    float8 NAPAPI sinVec(const float8 value);
    float4 NAPAPI cosVec(const float4 value);
    float8 NAPAPI cosVec(const float8 value);
	float NAPAPI powVec(const float value, const float power);
    float4 NAPAPI powVec(const float4 value, const float4 power);
    float8 NAPAPI powVec(const float8 value, const float8 power);

	inline void NAPAPI vectorAdd(float8 * __restrict destination, const float8 * __restrict a, const int vectorSize)
    {
    	const int vectorSize_4 = vectorSize >> 2;
		
    	for (int i = 0; i < vectorSize_4; ++i)
    	{
    		const float8 d1 = destination[i*4+0];
    		const float8 d2 = destination[i*4+1];
    		const float8 d3 = destination[i*4+2];
    		const float8 d4 = destination[i*4+3];
    		const float8 a1 = a[i*4+0];
    		const float8 a2 = a[i*4+1];
    		const float8 a3 = a[i*4+2];
    		const float8 a4 = a[i*4+3];
			
			destination[i*4+0] = d1 + a1;
			destination[i*4+1] = d2 + a2;
			destination[i*4+2] = d3 + a3;
			destination[i*4+3] = d4 + a4;
		}
		
		for (int i = vectorSize_4 << 2; i < vectorSize; ++i)
			destination[i] = destination[i] + a[i];
	}
    
}
