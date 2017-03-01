/*
 * Copyright 2017 Till Kolditz <till.kolditz@gmail.com>.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * File:   SSECMP.hpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 24. Februar 2017, 14:07
 */

#ifndef SSECMP_HPP
#define SSECMP_HPP

#include <immintrin.h>

#include <column_operators/SSE.hpp>

namespace v2 {
    namespace bat {
        namespace ops {

            namespace Private {

                inline uint8_t v2_mm128_compact_mask_uint16_t(uint16_t & mask) {
                    mask = mask & 0x5555;
                    mask = ((mask >> 1) | mask) & 0x3333;
                    mask = ((mask >> 2) | mask) & 0x0F0F;
                    mask = ((mask >> 4) | mask) & 0x00FF;
                    return static_cast<uint8_t>(mask);
                }

                inline uint8_t v2_mm128_compact_mask_uint32_t(uint16_t & mask) {
                    mask = mask & 0x1111;
                    mask = ((mask >> 3) | mask) & 0x0303;
                    mask = ((mask >> 6) | mask) & 0x000F;
                    return static_cast<uint8_t>(mask);
                }

                inline uint8_t v2_mm128_compact_mask_uint64_t(uint16_t & mask) {
                    mask = mask & 0x0101;
                    mask = ((mask >> 7) | mask) & 0x0003;
                    return static_cast<uint8_t>(mask);
                }
            }

            template<typename T, template<typename > class Op>
            struct v2_mm128_cmp;

            template<>
            struct v2_mm128_cmp<uint8_t, std::greater> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    return _mm_cmplt_epi8(b, a);
                }

                static inline uint16_t cmp_mask(__m128i & a, __m128i & b) {
                    auto mask = static_cast<uint16_t>(_mm_movemask_epi8(cmp(a, b)));
                    return mask;
                }
            };

            template<>
            struct v2_mm128_cmp<uint8_t, std::greater_equal> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    auto mm = v2_mm128<uint8_t>::max(a, b);
                    return _mm_cmpeq_epi8(a, mm);
                }

                static inline uint16_t cmp_mask(__m128i & a, __m128i & b) {
                    auto mask = static_cast<uint16_t>(_mm_movemask_epi8(cmp(a, b)));
                    return mask;
                }
            };

            template<>
            struct v2_mm128_cmp<uint8_t, std::less> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    return _mm_cmplt_epi8(a, b);
                }

                static inline uint16_t cmp_mask(__m128i & a, __m128i & b) {
                    auto mask = static_cast<uint16_t>(_mm_movemask_epi8(cmp(a, b)));
                    return mask;
                }
            };

            template<>
            struct v2_mm128_cmp<uint8_t, std::less_equal> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    auto mm = v2_mm128<uint8_t>::min(a, b);
                    return _mm_cmpeq_epi8(a, mm);
                }

                static inline uint16_t cmp_mask(__m128i & a, __m128i & b) {
                    auto mask = static_cast<uint16_t>(_mm_movemask_epi8(cmp(a, b)));
                    return mask;
                }
            };

            template<>
            struct v2_mm128_cmp<uint8_t, std::equal_to> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    return _mm_cmpeq_epi8(a, b);
                }

                static inline uint16_t cmp_mask(__m128i & a, __m128i & b) {
                    auto mask = static_cast<uint16_t>(_mm_movemask_epi8(cmp(a, b)));
                    return mask;
                }
            };

            template<>
            struct v2_mm128_cmp<uint16_t, std::greater> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    return _mm_cmplt_epi16(b, a);
                }

                static inline uint8_t cmp_mask(__m128i & a, __m128i & b) {
                    auto mask = static_cast<uint16_t>(_mm_movemask_epi8(cmp(a, b)));
                    return Private::v2_mm128_compact_mask_uint16_t(mask);
                }
            };

            template<>
            struct v2_mm128_cmp<uint16_t, std::greater_equal> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    auto mm = v2_mm128<uint16_t>::max(a, b);
                    return _mm_cmpeq_epi16(a, mm);
                }

                static inline uint8_t cmp_mask(__m128i & a, __m128i & b) {
                    auto mask = static_cast<uint16_t>(_mm_movemask_epi8(cmp(a, b)));
                    return Private::v2_mm128_compact_mask_uint16_t(mask);
                }
            };

            template<>
            struct v2_mm128_cmp<uint16_t, std::less> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    return _mm_cmplt_epi16(a, b);
                }

                static inline uint8_t cmp_mask(__m128i & a, __m128i & b) {
                    auto mask = static_cast<uint16_t>(_mm_movemask_epi8(cmp(a, b)));
                    return Private::v2_mm128_compact_mask_uint16_t(mask);
                }
            };

            template<>
            struct v2_mm128_cmp<uint16_t, std::less_equal> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    auto mm = v2_mm128<uint16_t>::min(a, b);
                    return _mm_cmpeq_epi16(a, mm);
                }

                static inline uint8_t cmp_mask(__m128i & a, __m128i & b) {
                    auto mask = static_cast<uint16_t>(_mm_movemask_epi8(cmp(a, b)));
                    return Private::v2_mm128_compact_mask_uint16_t(mask);
                }
            };

            template<>
            struct v2_mm128_cmp<uint16_t, std::equal_to> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    return _mm_cmpeq_epi16(a, b);
                }

                static inline uint8_t cmp_mask(__m128i & a, __m128i & b) {
                    auto mask = static_cast<uint16_t>(_mm_movemask_epi8(cmp(a, b)));
                    return Private::v2_mm128_compact_mask_uint16_t(mask);
                }
            };

            template<>
            struct v2_mm128_cmp<uint32_t, std::equal_to> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    return _mm_cmpeq_epi32(a, b);
                }

                static inline uint8_t cmp_mask(__m128i & a, __m128i & b) {
                    auto mask = static_cast<uint16_t>(_mm_movemask_epi8(cmp(a, b)));
                    return Private::v2_mm128_compact_mask_uint32_t(mask);
                }
            };

            template<>
            struct v2_mm128_cmp<uint64_t, std::greater_equal> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    auto mm = v2_mm128<uint64_t>::max(a, b);
                    return _mm_cmpeq_epi64(a, mm);
                }

                static inline uint8_t cmp_mask(__m128i & a, __m128i & b) {
                    return static_cast<uint8_t>(_mm_movemask_pd(_mm_castsi128_pd(cmp(a, b))));
                }
            };

            template<>
            struct v2_mm128_cmp<uint64_t, std::equal_to> {

                static inline __m128i cmp(__m128i & a, __m128i & b) {
                    return _mm_cmpeq_epi64(a, b);
                }

                static inline uint8_t cmp_mask(__m128i & a, __m128i & b) {
                    auto mask = static_cast<uint16_t>(_mm_movemask_epi8(cmp(a, b)));
                    return Private::v2_mm128_compact_mask_uint64_t(mask);
                }
            };
        }
    }
}

#endif /* SSECMP_HPP */
