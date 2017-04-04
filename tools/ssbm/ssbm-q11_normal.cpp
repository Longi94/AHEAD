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
 * File:   ssbm-q11_normal.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 1. August 2016, 12:20
 */

#include "ssbm.hpp"

int main(int argc, char** argv) {
    SSBM_REQUIRED_VARIABLES("SSBM Query 1.1 Normal\n=====================", 24, "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M", "N", "O", "P");

    SSBM_LOAD("date", "lineorder", "SSBM Q1.1:\n"
            "select sum(lo_revenue), d_year, p_brand\n"
            "  from lineorder, part, supplier, date\n"
            "  where lo_orderdate = d_datekey\n"
            "    and d_year = 1993\n"
            "    and lo_discount between 1 and 3\n"
            "    and lo_quantity < 25;");

    /* Measure loading ColumnBats */
    MEASURE_OP(batDYcb, new shortint_colbat_t("date", "year"));
    MEASURE_OP(batDDcb, new int_colbat_t("date", "datekey"));
    MEASURE_OP(batLQcb, new tinyint_colbat_t("lineorder", "quantity"));
    MEASURE_OP(batLDcb, new tinyint_colbat_t("lineorder", "discount"));
    MEASURE_OP(batLOcb, new int_colbat_t("lineorder", "orderdate"));
    MEASURE_OP(batLEcb, new int_colbat_t("lineorder", "extendedprice"));

    /* Measure converting (copying) ColumnBats to TempBats */
    MEASURE_OP(batDY, ahead::bat::ops::copy(batDYcb));
    MEASURE_OP(batDD, ahead::bat::ops::copy(batDDcb));
    MEASURE_OP(batLQ, ahead::bat::ops::copy(batLQcb));
    MEASURE_OP(batLD, ahead::bat::ops::copy(batLDcb));
    MEASURE_OP(batLO, ahead::bat::ops::copy(batLOcb));
    MEASURE_OP(batLE, ahead::bat::ops::copy(batLEcb));

    delete batDYcb;
    delete batDDcb;
    delete batLQcb;
    delete batLDcb;
    delete batLOcb;
    delete batLEcb;

    SSBM_BEFORE_QUERIES;

    for (size_t i = 0; i < CONFIG.NUM_RUNS; ++i) {
        SSBM_BEFORE_QUERY;

        // 1) select from lineorder
        MEASURE_OP(bat1, ahead::bat::ops::select<std::less>(batLQ, 25)); // lo_quantity < 25
        MEASURE_OP(bat2, ahead::bat::ops::select(batLD, 1, 3)); // lo_discount between 1 and 3
        auto bat3 = bat1->mirror_head(); // prepare joined selection (select from lineorder where lo_quantity... and lo_discount)
        delete bat1;
        MEASURE_OP(bat4, ahead::bat::ops::matchjoin(bat3, bat2)); // join selection
        delete bat3;
        delete bat2;
        auto bat5 = bat4->mirror_head(); // prepare joined selection with lo_orderdate (contains positions in tail)
        MEASURE_OP(bat6, ahead::bat::ops::matchjoin(bat5, batLO)); // only those lo_orderdates where lo_quantity... and lo_discount
        delete bat5;

        // 2) select from date (join inbetween to reduce the number of lines we touch in total)
        MEASURE_OP(bat7, ahead::bat::ops::select<std::equal_to>(batDY, 1993)); // d_year = 1993
        auto bat8 = bat7->mirror_head(); // prepare joined selection over d_year and d_datekey
        delete bat7;
        MEASURE_OP(bat9, ahead::bat::ops::matchjoin(bat8, batDD)); // only those d_datekey where d_year...
        delete bat8;

        // 3) join lineorder and date
        auto batA = bat9->reverse();
        delete bat9;
        MEASURE_OP(batB, ahead::bat::ops::hashjoin(bat6, batA)); // only those lineorders where lo_quantity... and lo_discount... and d_year...
        delete bat6;
        delete batA;
        // batB has in the Head the positions from lineorder and in the Tail the positions from date
        auto batC = batB->mirror_head(); // only those lineorder-positions where lo_quantity... and lo_discount... and d_year...
        delete batB;
        MEASURE_OP(batD, ahead::bat::ops::matchjoin(batC, batLE));
        MEASURE_OP(batE, ahead::bat::ops::matchjoin(batC, bat4));
        delete batC;
        delete bat4;

        // 4) result
        MEASURE_OP(batF, ahead::bat::ops::aggregate_mul_sum<v2_bigint_t>(batD, batE, 0));
        delete batD;
        delete batE;
        auto iter = batF->begin();
        auto result = iter->tail();
        delete iter;
        delete batF;

        SSBM_AFTER_QUERY(i, result);
    }

    SSBM_AFTER_QUERIES;

    delete batDY;
    delete batDD;
    delete batLQ;
    delete batLD;
    delete batLO;
    delete batLE;

    SSBM_FINALIZE;

    return 0;
}
