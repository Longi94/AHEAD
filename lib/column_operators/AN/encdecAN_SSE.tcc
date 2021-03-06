// Copyright (c) 2016-2017 Till Kolditz
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/*
 * File:   encdecAN_SSE.tcc
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 22. November 2016, 16:22
 */

#ifndef ENCDECAN_SSE_TCC
#define ENCDECAN_SSE_TCC

#include <type_traits>
#include <immintrin.h>

#include <column_storage/TempStorage.hpp>
#include <column_operators/ANbase.hpp>
#include "SSEAN.hpp"
#include "../miscellaneous.hpp"
#include "ANhelper.tcc"

#ifdef __GNUC__
#pragma GCC push_options
#pragma GCC target "sse4.2"
#else
#warning "Forcing SSE 4.2 code is not yet implemented for this compiler"
#endif

namespace ahead {
    namespace bat {
        namespace ops {
            namespace simd {
                namespace sse {

                    /*
                     template<typename Head, typename Tail>
                     BAT<Head, typename TypeMap<Tail>::v2_encoded_t>*
                     encodeAN(BAT<Head, Tail>* arg, typename TypeMap<Tail>::v2_encoded_t::type_t A = std::get<ANParametersSelector<Tail>::As->size() - 1>(*ANParametersSelector<Tail>::As)) {

                     typedef typename TypeMap<Tail>::v2_encoded_t enctail_t;
                     typedef typename enctail_t::type_t tail_t;
                     typedef typename BAT<Head, enctail_t>::coldesc_head_t cd_head_t;
                     typedef typename BAT<Head, enctail_t>::coldesc_tail_t cd_tail_t;

                     static_assert(std::is_base_of<v2_base_t, Head>::value, "Head must be a base type");
                     static_assert(std::is_base_of<v2_base_t, Tail>::value, "Tail must be a base type");

                     auto result = new TempBAT<Head, enctail_t>(cd_head_t(arg->head.metaData),
                     cd_tail_t(ColumnMetaData(sizeof(typename TypeMap<Tail>::v2_encoded_t::type_t), A, ext_euclidean(A, sizeof(tail_t)), enctail_t::UNENC_MAX_U, enctail_t::UNENC_MIN)));
                     result->reserve(arg->size());
                     auto iter = arg->begin();
                     for (; iter->hasNext(); ++*iter) {
                     result->append(std::make_pair(iter->head(), static_cast<tail_t>(iter->tail()) * A));
                     }
                     delete iter;
                     return result; // possibly empty
                     }

                     template<typename Head, typename ResTail>
                     AN_indicator_vector *
                     checkAN(BAT<Head, ResTail>* arg, typename ResTail::type_t aInv = ResTail::A_INV, typename ResTail::type_t unEncMaxU = ResTail::A_UNENC_MAX_U) {

                     typedef typename ResTail::type_t res_t;

                     static_assert(std::is_base_of<v2_base_t, Head>::value, "Head must be a base type");
                     static_assert(std::is_base_of<v2_anencoded_t, ResTail>::value, "ResTail must be an AN-encoded type");

                     res_t Ainv = static_cast<res_t>(arg->tail.metaData.AN_Ainv);

                     auto result = new AN_indicator_vector;
                     auto iter = arg->begin();
                     for (size_t i = 0; iter->hasNext(); ++*iter, ++i) {
                     if (static_cast<res_t>(iter->tail() * aInv) > unEncMaxU) {
                     result->push_back(i * AOID);
                     }
                     }
                     delete iter;
                     return result; // possibly empty
                     }

                     template<typename Head, typename ResTail>
                     std::pair<TempBAT<Head, typename ResTail::v2_unenc_t>*, AN_indicator_vector *> decodeAN(BAT<Head, ResTail>* arg) {

                     typedef typename ResTail::type_t restail_t;
                     typedef typename ResTail::v2_unenc_t Tail;
                     typedef typename Tail::type_t tail_t;

                     static_assert(std::is_base_of<v2_base_t, Head>::value, "Head must be a base type");
                     static_assert(std::is_base_of<v2_anencoded_t, ResTail>::value, "ResTail must be an AN-encoded type");

                     restail_t Ainv = static_cast<restail_t>(arg->tail.metaData.AN_Ainv);
                     restail_t unencMaxU = static_cast<restail_t>(arg->tail.metaData.AN_unencMaxU);
                     auto result = skeletonHead<Head, Tail>(arg);
                     result->reserve(arg->size());
                     auto vec = new AN_indicator_vector;
                     auto iter = arg->begin();
                     size_t pos = 0;
                     for (; iter->hasNext(); ++*iter, ++pos) {
                     auto t = static_cast<restail_t>(iter->tail() * Ainv);
                     result->append(std::make_pair(iter->head(), static_cast<tail_t>(t)));
                     if (t > unencMaxU) {
                     vec->push_back(pos * AOID);
                     }
                     }
                     delete iter;
                     return std::make_pair(result, vec);
                     }
                     */

                    namespace Private {
                        template<typename Head, typename Tail>
                        struct CheckAndDecodeAN {
                            typedef typename Head::type_t head_t;
                            typedef typename Tail::type_t tail_t;
                            typedef typename Head::v2_unenc_t v2_head_unenc_t;
                            typedef typename Tail::v2_unenc_t v2_tail_unenc_t;
                            typedef typename v2_head_unenc_t::type_t head_unenc_t;
                            typedef typename v2_tail_unenc_t::type_t tail_unenc_t;
                            typedef typename smaller_type<head_t, tail_t>::type_t smaller_t;
                            typedef typename smaller_type<head_unenc_t, tail_unenc_t>::type_t smaller_unenc_t;
                            typedef typename larger_type<head_t, tail_t>::type_t larger_t;
                            typedef typename larger_type<head_unenc_t, tail_unenc_t>::type_t larger_unenc_t;
                            typedef typename mmAN<__m128i, smaller_t>::mask_t smaller_mask_t;
                            typedef typename mmAN<__m128i, larger_t>::mask_t larger_mask_t;
                            typedef ANhelper<Head> head_helper_t;
                            typedef ANhelper<Tail> tail_helper_t;
                            typedef typename std::tuple<BAT<v2_head_unenc_t, v2_tail_unenc_t>*, AN_indicator_vector *, AN_indicator_vector *> result_t;

                            static result_t doIt(
                                    BAT<Head, Tail>* arg,
                                    resoid_t AOID) {
                                static_assert(head_helper_t::isEncoded || tail_helper_t::isEncoded, "At least one of Head and Tail must be an AN-encoded type");

                                const constexpr bool isHeadSmaller = larger_type<head_t, tail_t>::isSecondLarger;
                                const constexpr bool isHeadEncoded = head_helper_t::isEncoded;
                                const constexpr bool isTailEncoded = tail_helper_t::isEncoded;
                                const constexpr size_t smallersPerMM128 = sizeof(__m128i) / sizeof (smaller_t);
                                const constexpr size_t smallersUnencPerMM128 = sizeof(__m128i) / sizeof (smaller_unenc_t);
                                const constexpr size_t largersPerMM128 = sizeof(__m128i) / sizeof (larger_t);
                                const constexpr size_t factor = sizeof(larger_t) / sizeof(smaller_t);
                                const constexpr bool isSmallerEncoded = isHeadSmaller ? isHeadEncoded : isTailEncoded;
                                const constexpr bool isLargerEncoded = isHeadSmaller ? isTailEncoded : isHeadEncoded;

                                oid_t szArg = arg->size();

                                AN_indicator_vector * vec1 = head_helper_t::createIndicatorVector();
                                AN_indicator_vector * vec2 = tail_helper_t::createIndicatorVector();
                                auto vecS = isHeadSmaller ? vec1 : vec2;
                                auto vecL = isHeadSmaller ? vec2 : vec1;

                                auto result = new TempBAT<v2_head_unenc_t, v2_tail_unenc_t>();
                                result->reserve(szArg + smallersUnencPerMM128); // reserve more data to compensate for writing after the last bytes, since writing the very last vector will write 16 Bytes and not just the remaining ones

                                auto pmmH = reinterpret_cast<__m128i *>(arg->head.container->data());
                                auto pmmHEnd = reinterpret_cast<__m128i *>(arg->head.container->data() + szArg);
                                auto pmmT = reinterpret_cast<__m128i *>(arg->tail.container->data());
                                auto pmmTEnd = reinterpret_cast<__m128i *>(arg->tail.container->data() + szArg);
                                __m128i * pmmS = isHeadSmaller ? pmmH : pmmT;
                                __m128i * pmmSEnd = isHeadSmaller ? pmmHEnd : pmmTEnd;
                                __m128i * pmmL = isHeadSmaller ? pmmT : pmmH;
                                __m128i mmDecS, mmDecL;
                                auto pRH = reinterpret_cast<head_unenc_t*>(result->head.container->data());
                                auto pRT = reinterpret_cast<tail_unenc_t*>(result->tail.container->data());
                                auto pmmRS = reinterpret_cast<__m128i *>(smaller_type<head_t, tail_t>::get(reinterpret_cast<head_t *>(pRH), reinterpret_cast<tail_t *>(pRT)));
                                auto pmmRL = reinterpret_cast<__m128i *>(larger_type<head_t, tail_t>::get(reinterpret_cast<head_t *>(pRH), reinterpret_cast<tail_t *>(pRT)));

                                head_t hAinv = static_cast<head_t>(arg->head.metaData.AN_Ainv);
                                head_t hUnencMaxU = static_cast<head_t>(arg->head.metaData.AN_unencMaxU);
                                tail_t tAinv = static_cast<tail_t>(arg->tail.metaData.AN_Ainv);
                                tail_t tUnencMaxU = static_cast<tail_t>(arg->tail.metaData.AN_unencMaxU);
                                auto mmASinv = isHeadSmaller ? mm128<head_t>::set1(hAinv) : mm128<tail_t>::set1(tAinv);
                                auto mmALinv = isHeadSmaller ? mm128<tail_t>::set1(tAinv) : mm128<head_t>::set1(hAinv);
                                auto mmASDmax = isHeadSmaller ? mm128<head_t>::set1(hUnencMaxU) : mm128<tail_t>::set1(tUnencMaxU);
                                auto mmALDmax = isHeadSmaller ? mm128<tail_t>::set1(tUnencMaxU) : mm128<head_t>::set1(hUnencMaxU);
                                size_t pos = 0;
                                while (pmmS <= (pmmSEnd - 1)) {
                                    auto mm = _mm_lddqu_si128(pmmS++);
                                    if (isSmallerEncoded) {
                                        mmAN<__m128i, smaller_t>::detect(mmDecS, mm, mmASinv, mmASDmax, vecS, pos, AOID);
                                        mmDecS = mm128<smaller_t, smaller_unenc_t>::convert(mmDecS);
                                    } else {
                                        mmDecS = mm;
                                    }
                                    _mm_storeu_si128(pmmRS, mmDecS);
                                    pmmRS = reinterpret_cast<__m128i *>(reinterpret_cast<smaller_unenc_t *>(pmmRS) + smallersPerMM128);
                                    for (size_t i = 0; i < factor; ++i) {
                                        mm = _mm_lddqu_si128(pmmL++);
                                        if (isLargerEncoded) {
                                            mmAN<__m128i, larger_t>::detect(mmDecL, mm, mmALinv, mmALDmax, vecL, pos, AOID);
                                            mmDecL = mm128<larger_t, larger_unenc_t>::convert(mmDecL);
                                        } else {
                                            mmDecL = mm;
                                        }
                                        pos += largersPerMM128;
                                        _mm_storeu_si128(pmmRL, mmDecL);
                                        pmmRL = reinterpret_cast<__m128i *>(reinterpret_cast<larger_unenc_t *>(pmmRL) + largersPerMM128);
                                    }
                                }
                                result->overwrite_size(pos); // "register" the number of values we added
                                auto iter = arg->begin();
                                for (*iter += pos; iter->hasNext(); ++*iter, ++pos) {
                                    head_t decH = isHeadEncoded ? static_cast<head_t>(iter->head() * hAinv) : iter->head();
                                    tail_t decT = isTailEncoded ? static_cast<tail_t>(iter->tail() * tAinv) : iter->tail();
                                    if (isHeadEncoded && (decH > hUnencMaxU)) {
                                        vec1->push_back(pos * AOID);
                                    }
                                    if (isTailEncoded && (decT > tUnencMaxU)) {
                                        vec2->push_back(pos + AOID);
                                    }
                                    result->append(std::make_pair(static_cast<head_unenc_t>(decH), static_cast<tail_unenc_t>(decT)));
                                }
                                delete iter;
                                return make_tuple(result, vec1, vec2);
                            }
                        };

                        template<typename Tail>
                        struct CheckAndDecodeAN<v2_void_t, Tail> {
                            typedef v2_void_t Head;
                            typedef typename Tail::type_t tail_t;
                            typedef typename Tail::v2_unenc_t v2_tail_unenc_t;
                            typedef typename v2_tail_unenc_t::type_t tail_unenc_t;
                            typedef typename mmAN<__m128i, tail_t>::mask_t tail_mask_t;
                            typedef ANhelper<Tail> tail_helper_t;
                            typedef typename std::tuple<BAT<v2_void_t, v2_tail_unenc_t>*, AN_indicator_vector *, AN_indicator_vector *> result_t;

                            static result_t doIt(
                                    BAT<Head, Tail>* arg,
                                    resoid_t AOID) {
                                static_assert(tail_helper_t::isEncoded, "Tail must be an AN-encoded type");

                                const constexpr size_t tailsPerMM128 = sizeof(__m128i) / sizeof (tail_t);
                                const constexpr size_t unencTailsPerMM128 = sizeof(__m128i) / sizeof (tail_unenc_t);

                                tail_t tAinv = static_cast<tail_t>(arg->tail.metaData.AN_Ainv);
                                tail_t tUnencMaxU = static_cast<tail_t>(arg->tail.metaData.AN_unencMaxU);

                                oid_t szArg = arg->size();
                                AN_indicator_vector * vec = tail_helper_t::createIndicatorVector();
                                auto result = new TempBAT<v2_void_t, v2_tail_unenc_t>();
                                result->reserve(szArg + unencTailsPerMM128); // reserve more data to compensate for writing after the last bytes, since writing the very last vector will write 16 Bytes and not just the remaining ones
                                auto pT = arg->tail.container->data();
                                auto pTEnd = pT + szArg;
                                auto pmmT = reinterpret_cast<__m128i *>(pT);
                                auto pmmTEnd = reinterpret_cast<__m128i *>(pTEnd);
                                auto mmATInv = mm128<tail_t>::set1(tAinv);
                                auto mmATDmax = mm128<tail_t>::set1(tUnencMaxU);
                                __m128i mmDec;
                                auto pRT = reinterpret_cast<tail_unenc_t *>(result->tail.container->data());
                                auto pmmRT = reinterpret_cast<__m128i *>(pRT);

                                size_t pos = 0;
                                for (; pmmT <= (pmmTEnd - 1); ++pmmT, pos += tailsPerMM128) {
                                    mmAN<__m128i, tail_t>::detect(mmDec, _mm_lddqu_si128(pmmT), mmATInv, mmATDmax, vec, pos, AOID);
                                    mmDec = mm128<tail_t, tail_unenc_t>::convert(mmDec);
                                    _mm_storeu_si128(pmmRT, mmDec);
                                    pmmRT = reinterpret_cast<__m128i *>(reinterpret_cast<tail_unenc_t *>(pmmRT) + tailsPerMM128);
                                }
                                result->overwrite_size(pos); // "register" the number of values we added
                                auto iter = arg->begin();
                                for (*iter += pos; iter->hasNext(); ++*iter, ++pos) {
                                    tail_t decT = static_cast<tail_t>(iter->tail() * tAinv);
                                    if (decT > tUnencMaxU) {
                                        vec->push_back(pos * AOID);
                                    }
                                    result->append(static_cast<tail_unenc_t>(decT));
                                }
                                delete iter;
                                return make_tuple(result, nullptr, vec);
                            }
                        };
                    }

                    template<typename Head, typename Tail>
                    std::tuple<BAT<typename Head::v2_unenc_t, typename Tail::v2_unenc_t>*, AN_indicator_vector *, AN_indicator_vector *> checkAndDecodeAN(
                            BAT<Head, Tail>* arg,
                            resoid_t AOID) {
                        return Private::CheckAndDecodeAN<Head, Tail>::doIt(arg, AOID);
                    }

                }
            }
        }
    }
}

#ifdef __GNUC__
#pragma GCC pop_options
#else
#warning "Unforcing scalar code is not yet implemented for this compiler"
#endif

#endif /* ENCDECAN_SSE_TCC */
