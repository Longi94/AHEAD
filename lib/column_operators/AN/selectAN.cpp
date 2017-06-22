// Copyright (c) 2017 Till Kolditz
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
 * File:   selectAN.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 10-04-2017 22:10
 */

#include <column_operators/ANbase.hpp>
#include <column_operators/functors.hpp>
#include "selectAN.tcc"

namespace ahead {
    namespace bat {
        namespace ops {

#define SELECT1_AN(Head, Tail, Operator) \
template \
std::pair<BAT<typename TypeMap<Head>::v2_encoded_t::v2_select_t, typename Tail::v2_select_t>*, AN_indicator_vector*> \
selectAN<Operator, Head, Tail> ( \
        BAT<Head, Tail> *, \
        typename Tail::type_t && \
        );

#define SELECT1_AN_REENC(Head, Tail, Operator) \
template \
std::pair<BAT<typename TypeMap<Head>::v2_encoded_t::v2_select_t, typename Tail::v2_select_t>*, AN_indicator_vector*> \
selectAN<Operator>( \
        BAT<Head, Tail> *, \
        typename Tail::type_t &&, \
        typename Tail::v2_select_t::type_t, \
        typename Tail::v2_select_t::type_t \
        );

#define SELECT2_AN(Head, Tail, Operator1, Operator2) \
template \
std::pair<BAT<typename TypeMap<Head>::v2_encoded_t::v2_select_t, typename Tail::v2_select_t>*, AN_indicator_vector*> \
selectAN<Operator1, Operator2, AND, Head, Tail>( \
        BAT<Head, Tail> *, \
        typename Tail::type_t &&, \
        typename Tail::type_t && \
        ); \
template \
std::pair<BAT<typename TypeMap<Head>::v2_encoded_t::v2_select_t, typename Tail::v2_select_t>*, AN_indicator_vector*> \
selectAN<Operator1, Operator2, OR, Head, Tail>( \
        BAT<Head, Tail> *, \
        typename Tail::type_t &&, \
        typename Tail::type_t && \
        );

#define SELECT2_AN_REENC(Head, Tail, Operator1, Operator2) \
template \
std::pair<BAT<typename TypeMap<Head>::v2_encoded_t::v2_select_t, typename Tail::v2_select_t>*, AN_indicator_vector*> \
selectAN<Operator1, Operator2, AND, Head, Tail>( \
        BAT<Head, Tail> *, \
        typename Tail::type_t &&, \
        typename Tail::type_t &&, \
        typename Tail::v2_select_t::type_t, \
        typename Tail::v2_select_t::type_t \
        ); \
template \
std::pair<BAT<typename TypeMap<Head>::v2_encoded_t::v2_select_t, typename Tail::v2_select_t>*, AN_indicator_vector*> \
selectAN<Operator1, Operator2, OR, Head, Tail>( \
        BAT<Head, Tail> *, \
        typename Tail::type_t &&, \
        typename Tail::type_t &&, \
        typename Tail::v2_select_t::type_t, \
        typename Tail::v2_select_t::type_t \
        );

#define SELECT2_AN_SUB(Head, Tail, Operator2) \
SELECT2_AN(Head, Tail, std::less, Operator2) \
SELECT2_AN(Head, Tail, std::less_equal, Operator2) \
SELECT2_AN(Head, Tail, std::equal_to, Operator2) \
SELECT2_AN(Head, Tail, std::not_equal_to, Operator2) \
SELECT2_AN(Head, Tail, std::greater, Operator2) \
SELECT2_AN(Head, Tail, std::greater_equal, Operator2)

#define SELECT2_AN_REENC_SUB(Head, Tail, Operator2) \
SELECT2_AN_REENC(Head, Tail, std::less, Operator2) \
SELECT2_AN_REENC(Head, Tail, std::less_equal, Operator2) \
SELECT2_AN_REENC(Head, Tail, std::equal_to, Operator2) \
SELECT2_AN_REENC(Head, Tail, std::not_equal_to, Operator2) \
SELECT2_AN_REENC(Head, Tail, std::greater, Operator2) \
SELECT2_AN_REENC(Head, Tail, std::greater_equal, Operator2)

#define SELECT_AN(Head, Tail) \
SELECT1_AN(Head, Tail, std::less) \
SELECT1_AN(Head, Tail, std::less_equal) \
SELECT1_AN(Head, Tail, std::equal_to) \
SELECT1_AN(Head, Tail, std::not_equal_to) \
SELECT1_AN(Head, Tail, std::greater_equal) \
SELECT1_AN(Head, Tail, std::greater) \
SELECT2_AN_SUB(Head, Tail, std::less) \
SELECT2_AN_SUB(Head, Tail, std::less_equal) \
SELECT2_AN_SUB(Head, Tail, std::equal_to) \
SELECT2_AN_SUB(Head, Tail, std::not_equal_to) \
SELECT2_AN_SUB(Head, Tail, std::greater_equal) \
SELECT2_AN_SUB(Head, Tail, std::greater)

#define SELECT_AN_REENC(Head, Tail) \
SELECT1_AN_REENC(Head, Tail, std::less) \
SELECT1_AN_REENC(Head, Tail, std::less_equal) \
SELECT1_AN_REENC(Head, Tail, std::equal_to) \
SELECT1_AN_REENC(Head, Tail, std::not_equal_to) \
SELECT1_AN_REENC(Head, Tail, std::greater_equal) \
SELECT1_AN_REENC(Head, Tail, std::greater) \
SELECT2_AN_REENC_SUB(Head, Tail, std::less) \
SELECT2_AN_REENC_SUB(Head, Tail, std::less_equal) \
SELECT2_AN_REENC_SUB(Head, Tail, std::equal_to) \
SELECT2_AN_REENC_SUB(Head, Tail, std::not_equal_to) \
SELECT2_AN_REENC_SUB(Head, Tail, std::greater_equal) \
SELECT2_AN_REENC_SUB(Head, Tail, std::greater)

            namespace scalar {
                SELECT_AN (v2_void_t, v2_restiny_t)
                SELECT_AN_REENC (v2_void_t, v2_restiny_t)
                SELECT_AN (v2_void_t, v2_resshort_t)
                SELECT_AN_REENC (v2_void_t, v2_resshort_t)
                SELECT_AN (v2_void_t, v2_resint_t)
                SELECT_AN_REENC (v2_void_t, v2_resint_t)
                SELECT_AN (v2_void_t, v2_str_t)
            }

            namespace sse {
                SELECT_AN (v2_void_t, v2_restiny_t)
                SELECT_AN_REENC (v2_void_t, v2_restiny_t)
                SELECT_AN (v2_void_t, v2_resshort_t)
                SELECT_AN_REENC (v2_void_t, v2_resshort_t)
                SELECT_AN (v2_void_t, v2_resint_t)
                SELECT_AN_REENC (v2_void_t, v2_resint_t)
                SELECT_AN (v2_void_t, v2_str_t)
            }

        }
    }
}
