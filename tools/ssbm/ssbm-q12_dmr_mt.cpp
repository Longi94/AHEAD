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
 * File:   ssbm-q12_dmr_mt.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 24. January 2017, 17:09
 */

#include <omp.h>
#include "ssb.hpp"
#include "macros.hpp"
#include <util/ModularRedundant.hpp>

typedef DMRValue<bigint_t> DMR;

int main(
        int argc,
        char** argv) {
    return ssb::run(argc, argv, [] (int argc, char ** argv) -> int {
        ssb::init(argc, argv, "SSBM Query 1.2 DMR Parallel");

        ssb::loadTables( {"date", "lineorder"}, "SSBM Q1.2:\n"
                "select sum(lo_extendedprice * lo_discount) as revenue\n"
                "  from lineorder, date\n"
                "  where lo_orderdate = d_datekey\n"
                "    and d_yearmonthnum = 199401\n"
                "    and lo_discount between 4 and 6\n"
                "    and lo_quantity between 26 and 35;");

        /* Measure loading ColumnBats */
        MEASURE_OP(batDYcb, new int_colbat_t("date", "yearmonthnum"));
        MEASURE_OP(batDDcb, new int_colbat_t("date", "datekey"));
        MEASURE_OP(batLQcb, new tinyint_colbat_t("lineorder", "quantity"));
        MEASURE_OP(batLDcb, new tinyint_colbat_t("lineorder", "discount"));
        MEASURE_OP(batLOcb, new int_colbat_t("lineorder", "orderdate"));
        MEASURE_OP(batLEcb, new int_colbat_t("lineorder", "extendedprice"));

        ssb::after_create_columnbats();

        /* Measure converting (copying) ColumnBats to TempBats */
        int_tmpbat_t * batDYs[DMR::modularity];
        int_tmpbat_t * batDDs[DMR::modularity];
        tinyint_tmpbat_t * batLQs[DMR::modularity];
        tinyint_tmpbat_t * batLDs[DMR::modularity];
        int_tmpbat_t * batLOs[DMR::modularity];
        int_tmpbat_t * batLEs[DMR::modularity];

        for (size_t k = 0; k < DMR::modularity; ++k) {
            MEASURE_OP(batDYs, [k], copy(batDYcb), batDYs[k]);
            MEASURE_OP(batDDs, [k], copy(batDDcb), batDDs[k]);
            MEASURE_OP(batLQs, [k], copy(batLQcb), batLQs[k]);
            MEASURE_OP(batLDs, [k], copy(batLDcb), batLDs[k]);
            MEASURE_OP(batLOs, [k], copy(batLOcb), batLOs[k]);
            MEASURE_OP(batLEs, [k], copy(batLEcb), batLEs[k]);
        }

        delete batDYcb;
        delete batDDcb;
        delete batLQcb;
        delete batLDcb;
        delete batLOcb;
        delete batLEcb;

        ssb::before_queries();

        for (size_t i = 0; i < ssb::ssb_config.NUM_RUNS; ++i) {
            ssb::before_query();

            DMR results(0);

#pragma omp parallel for
            for (size_t k = 0; k < DMR::modularity; ++k) {
                // 1) select from lineorder
                MEASURE_OP(bat1, (select<std::greater_equal, std::less_equal, ahead::and_is>(batLQs[k], 26, 35)));// lo_quantity between 26 and 35
                MEASURE_OP(bat2, (select<std::greater_equal, std::less_equal, ahead::and_is>(batLDs[k], 4, 6)));// lo_discount between 4 and 6
                auto bat3 = bat1->mirror_head();// prepare joined selection (select from lineorder where lo_quantity... and lo_discount)
                delete bat1;
                auto batZ = bat2->mirror_head();
                delete bat2;
                MEASURE_OP(bat4, matchjoin(bat3, batZ));// join selection
                delete bat3;
                delete batZ;
                auto bat5 = bat4->mirror_head();// prepare joined selection with lo_orderdate (contains positions in tail)
                delete bat4;
                MEASURE_OP(bat6, matchjoin(bat5, batLOs[k]));// only those lo_orderdates where lo_quantity... and lo_discount

                // 2) select from date (join inbetween to reduce the number of lines we touch in total)
                MEASURE_OP(bat7, select<std::equal_to>(batDYs[k], 199401));// d_yearmonthnum = 199401
                auto bat8 = bat7->mirror_head();// prepare joined selection over d_year and d_datekey
                delete bat7;
                MEASURE_OP(bat9, matchjoin(bat8, batDDs[k]));// only those d_datekey where d_year...
                delete bat8;

                // 3) join lineorder and date
                auto batA = bat9->reverse();
                delete bat9;
                MEASURE_OP(batB, hashjoin(bat6, batA));// only those lineorders where lo_quantity... and lo_discount... and d_year...
                delete bat6;
                delete batA;
                // batE now has in the Head the positions from lineorder and in the Tail the positions from date
                auto batC = batB->mirror_head();// only those lineorder-positions where lo_quantity... and lo_discount... and d_year...
                delete batB;
                // BatF only contains the
                MEASURE_OP(batD, matchjoin(batC, batLEs[k]));
                MEASURE_OP(batE, matchjoin(bat5, batLDs[k]));
                delete bat5;
                MEASURE_OP(batF, matchjoin(batC, batE));
                delete batC;
                delete batE;

                // 4) result
                MEASURE_OP(batG, aggregate_mul_sum<v2_bigint_t>(batD, batF, 0));
                delete batD;
                delete batF;
                auto iter = batG->begin();
                uint64_t result = iter->tail();
                delete iter;
                delete batG;

#pragma omp critical
                {
                    results[k] = result;
                }
            }

            // 5) Voting
            auto result = vote_majority_value(results);

            ssb::after_query(i, result);
        }

        ssb::after_queries();

        for (size_t k = 0; k < DMR::modularity; ++k) {
            delete batDYs[k];
            delete batDDs[k];
            delete batLQs[k];
            delete batLDs[k];
            delete batLOs[k];
            delete batLEs[k];
        }

        ssb::finalize();

        return 0;
    });
}
