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
 * File:   ssbm-q13_dmr_seq.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 24. January 2017, 17:10
 */

#include "ssb.hpp"
#include "macros.hpp"
#include <util/ModularRedundant.hpp>

typedef DMRValue<bigint_t> DMR;

int main(
        int argc,
        char** argv) {
    return ssb::run(argc, argv, [] (int argc, char ** argv) -> int {
        ssb::init(argc, argv, "SSBM Query 1.3 DMR Sequential");

        ssb::loadTables( {"date", "lineorder"}, "SSBM Q1.3:\n"
                "select sum(lo_extendedprice * lo_discount) as revenue\n"
                "  from lineorder, date\n"
                "  where lo_orderdate = d_datekey\n"
                "    and d_weeknuminyear = 6\n"
                "    and d_year = 1994\n"
                "    and lo_discount between 5 and 7\n"
                "    and lo_quantity between 26 and 35;");

        /* Measure loading ColumnBats */
        MEASURE_OP(batDYcb, new shortint_colbat_t("date", "year"));
        MEASURE_OP(batDDcb, new int_colbat_t("date", "datekey"));
        MEASURE_OP(batLQcb, new tinyint_colbat_t("lineorder", "quantity"));
        MEASURE_OP(batLDcb, new tinyint_colbat_t("lineorder", "discount"));
        MEASURE_OP(batLOcb, new int_colbat_t("lineorder", "orderdate"));
        MEASURE_OP(batLEcb, new int_colbat_t("lineorder", "extendedprice"));
        MEASURE_OP(batDWcb, new tinyint_colbat_t("date", "weeknuminyear"));

        ssb::after_create_columnbats();

        /* Measure converting (copying) ColumnBats to TempBats */
        shortint_tmpbat_t * batDYs[DMR::modularity];
        int_tmpbat_t * batDDs[DMR::modularity];
        tinyint_tmpbat_t * batLQs[DMR::modularity];
        tinyint_tmpbat_t * batLDs[DMR::modularity];
        int_tmpbat_t * batLOs[DMR::modularity];
        int_tmpbat_t * batLEs[DMR::modularity];
        tinyint_tmpbat_t * batDWs[DMR::modularity];

        for (size_t k = 0; k < DMR::modularity; ++k) {
            MEASURE_OP(batDYs, [k], copy(batDYcb), batDYs[k]);
            MEASURE_OP(batDDs, [k], copy(batDDcb), batDDs[k]);
            MEASURE_OP(batLQs, [k], copy(batLQcb), batLQs[k]);
            MEASURE_OP(batLDs, [k], copy(batLDcb), batLDs[k]);
            MEASURE_OP(batLOs, [k], copy(batLOcb), batLOs[k]);
            MEASURE_OP(batLEs, [k], copy(batLEcb), batLEs[k]);
            MEASURE_OP(batDWs, [k], copy(batDWcb), batDWs[k]);
        }

        delete batDYcb;
        delete batDDcb;
        delete batLQcb;
        delete batLDcb;
        delete batLOcb;
        delete batLEcb;
        delete batDWcb;

        ssb::before_queries();

        for (size_t i = 0; i < ssb::ssb_config.NUM_RUNS; ++i) {
            ssb::before_query();

            DMR results(0);

            for (size_t k = 0; k < DMR::modularity; ++k) {
                // 1) select from lineorder
                MEASURE_OP(bat1, (select<std::greater_equal, std::less_equal, ahead::and_is>(batLQs[k], 26, 35)));// lo_quantity between 26 and 35
                MEASURE_OP(bat2, (select<std::greater_equal, std::less_equal, ahead::and_is>(batLDs[k], 5, 7)));// lo_discount between 5 and 7
                auto bat3 = bat1->mirror_head(); // prepare joined selection (select from lineorder where lo_quantity... and lo_discount)
                delete bat1;
                auto batZ = bat2->mirror_head();
                delete bat2;
                MEASURE_OP(bat4, matchjoin(bat3, batZ));// join selection
                delete bat3;
                delete batZ;
                auto bat5 = bat4->mirror_head();// prepare joined selection with lo_orderdate (contains positions in tail)
                delete bat4;
                MEASURE_OP(bat6, matchjoin(bat5, batLOs[k]));// only those lo_orderdates where lo_quantity... and lo_discount

                // 1) select from date (join inbetween to reduce the number of lines we touch in total)
                MEASURE_OP(bat7, select<std::equal_to>(batDYs[k], 1994));// d_year = 1994
                auto bat8 = bat7->mirror_head();// prepare joined selection over d_year and d_weeknuminyear
                delete bat7;
                MEASURE_OP(bat9, select<std::equal_to>(batDWs[k], 6));// d_weeknuminyear = 6
                auto batY = bat9->mirror_head();
                delete bat9;
                MEASURE_OP(batA, matchjoin(bat8, batY));
                delete bat8;
                delete batY;
                auto batB = batA->mirror_head();
                delete batA;
                MEASURE_OP(batC, matchjoin(batB, batDDs[k]));// only those d_datekey where d_year and d_weeknuminyear...
                delete batB;
                auto batD = batC->reverse();
                delete batC;

                // 3) join lineorder and date
                MEASURE_OP(batE, hashjoin(bat6, batD));// only those lineorders where lo_quantity... and lo_discount... and d_year...
                delete bat6;
                delete batD;
                // batE now has in the Head the positions from lineorder and in the Tail the positions from date
                auto batF = batE->mirror_head();// only those lineorder-positions where lo_quantity... and lo_discount... and d_year...
                delete batE;
                // BatF only contains the
                MEASURE_OP(batG, matchjoin(batF, batLEs[k]));
                MEASURE_OP(batH, matchjoin(bat5, batLDs[k]));
                delete bat5;
                MEASURE_OP(batI, matchjoin(batF, batH));
                delete batF;
                delete batH;

                // 4) result
                MEASURE_OP(batK, aggregate_mul_sum<v2_bigint_t>(batG, batI, 0));
                delete batG;
                delete batI;
                auto iter = batK->begin();
                auto result = iter->tail();
                delete iter;
                delete batK;

                results[k] = result;
            }

            // 5) Voting
            try {
                auto result = vote_majority_value(results);
                ssb::after_query(i, result);
            } catch (std::exception & ex) {
                ssb::after_query(i, ex);
            }
        }

        ssb::after_queries();

        for (size_t k = 0; k < DMR::modularity; ++k) {
            delete batDYs[k];
            delete batDDs[k];
            delete batLQs[k];
            delete batLDs[k];
            delete batLOs[k];
            delete batLEs[k];
            delete batDWs[k];
        }

        ssb::finalize();

        return 0;
    });
}
