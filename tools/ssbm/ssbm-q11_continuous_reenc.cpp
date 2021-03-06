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
 * File:   ssbm-q11_continuous_reenc.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 08-12-2016 01:32
 */

#include <column_operators/OperatorsAN.hpp>
#include "ssb.hpp"
#include "macros.hpp"

int main(
        int argc,
        char** argv) {
    return ssb::run(argc, argv, [] (int argc, char ** argv) -> int {
        ssb::init(argc, argv, "SSBM Query 1.1 Continuous Detection With Reencoding");

        ssb::loadTables( {"dateAN", "lineorderAN"}, "SSBM Q1.1:\n"
                "select sum(lo_extendedprice * lo_discount) as revenue\n"
                "  from lineorder, date\n"
                "  where lo_orderdate = d_datekey\n"
                "    and d_year = 1993\n"
                "    and lo_discount between 1 and 3\n"
                "    and lo_quantity < 25;");

        /* Measure loading ColumnBats */
        MEASURE_OP(batDYcb, new resshort_colbat_t("dateAN", "year"));
        MEASURE_OP(batDDcb, new resint_colbat_t("dateAN", "datekey"));
        MEASURE_OP(batLQcb, new restiny_colbat_t("lineorderAN", "quantity"));
        MEASURE_OP(batLDcb, new restiny_colbat_t("lineorderAN", "discount"));
        MEASURE_OP(batLOcb, new resint_colbat_t("lineorderAN", "orderdate"));
        MEASURE_OP(batLEcb, new resint_colbat_t("lineorderAN", "extendedprice"));

        ssb::after_create_columnbats();

        /* Measure converting (copying) ColumnBats to TempBats */
        MEASURE_OP(batDYenc, copy(batDYcb));
        MEASURE_OP(batDDenc, copy(batDDcb));
        MEASURE_OP(batLQenc, copy(batLQcb));
        MEASURE_OP(batLDenc, copy(batLDcb));
        MEASURE_OP(batLOenc, copy(batLOcb));
        MEASURE_OP(batLEenc, copy(batLEcb));

        delete batDYcb;
        delete batDDcb;
        delete batLQcb;
        delete batLDcb;
        delete batLOcb;
        delete batLEcb;

        ssb::before_queries();

        for (size_t i = 0; i < ssb::ssb_config.NUM_RUNS; ++i) {
            ssb::before_query();

            // 1) select from lineorder
            MEASURE_OP_PAIR(pair1, selectAN<std::less>(batLQenc, 25ull * batLQenc->tail.metaData.AN_A, std::get<6>(*v2_restiny_t::As), std::get<6>(*v2_restiny_t::Ainvs)));// lo_quantity < 25
            CLEAR_SELECT_AN(pair1);
            MEASURE_OP_PAIR(pair2,
                    (selectAN<std::greater_equal, std::less_equal, ahead::and_is>(batLDenc, 1ull * batLDenc->tail.metaData.AN_A, 3ull * batLDenc->tail.metaData.AN_A, std::get<5>(*v2_restiny_t::As),
                                    std::get<5>(*v2_restiny_t::Ainvs))));// lo_discount between 1 and 3
            CLEAR_SELECT_AN(pair2);
            auto bat3 = pair1.first->mirror_head();// prepare joined selection (select from lineorder where lo_quantity... and lo_discount)
            delete pair1.first;
            auto batZ = pair2.first->mirror_head();
            delete pair2.first;
            MEASURE_OP_TUPLE(tuple4, matchjoinAN(bat3, batZ, std::get<14>(*v2_resoid_t::As), std::get<14>(*v2_resoid_t::Ainvs), std::get<4>(*v2_restiny_t::As), std::get<4>(*v2_restiny_t::Ainvs)));// join selection
            delete bat3;
            delete batZ;
            CLEAR_JOIN_AN(tuple4);
            auto bat5 = std::get<0>(tuple4)->mirror_head();// prepare joined selection with lo_orderdate (contains positions in tail)
            delete std::get<0>(tuple4);
            MEASURE_OP_TUPLE(tuple6, matchjoinAN(bat5, batLOenc, std::get<13>(*v2_resoid_t::As), std::get<13>(*v2_resoid_t::Ainvs), std::get<14>(*v2_resint_t::As), std::get<14>(*v2_resint_t::Ainvs)));// only those lo_orderdates where lo_quantity... and lo_discount
            CLEAR_JOIN_AN(tuple6);

            // 2) select from date (join inbetween to reduce the number of lines we touch in total)
            MEASURE_OP_PAIR(pair7, (selectAN<std::equal_to>(batDYenc, 1993ull * batDYenc->tail.metaData.AN_A, std::get<14>(*v2_resshort_t::As), std::get<14>(*v2_resshort_t::Ainvs))));// d_year = 1993
            CLEAR_SELECT_AN(pair7);
            auto bat8 = pair7.first->mirror_head();// prepare joined selection over d_year and d_datekey
            delete pair7.first;
            MEASURE_OP_TUPLE(tuple9, (matchjoinAN(bat8, batDDenc, std::get<12>(*v2_resoid_t::As), std::get<12>(*v2_resoid_t::Ainvs), std::get<13>(*v2_resint_t::As), std::get<13>(*v2_resint_t::Ainvs))));// only those d_datekey where d_year...
            delete bat8;
            CLEAR_JOIN_AN(tuple9);

            // 3) join lineorder and date
            auto batA = std::get<0>(tuple9)->reverse();
            delete std::get<0>(tuple9);
            MEASURE_OP_TUPLE(tupleB,
                    (hashjoinAN(std::get<0>(tuple6), batA, std::get<11>(*v2_resoid_t::As), std::get<11>(*v2_resoid_t::Ainvs), std::get<10>(*v2_resoid_t::As), std::get<10>(*v2_resoid_t::Ainvs))));// only those lineorders where lo_quantity... and lo_discount... and d_year...
            delete std::get<0>(tuple6);
            delete batA;
            CLEAR_JOIN_AN(tupleB);
            // batB has in the Head the positions from lineorder and in the Tail the positions from date
            auto batC = std::get<0>(tupleB)->mirror_head();// only those lineorder-positions where lo_quantity... and lo_discount... and d_year...
            delete std::get<0>(tupleB);
            MEASURE_OP_TUPLE(tupleD, (matchjoinAN(batC, batLEenc, std::get<9>(*v2_resoid_t::As), std::get<9>(*v2_resoid_t::Ainvs), std::get<12>(*v2_resint_t::As), std::get<12>(*v2_resint_t::Ainvs))));
            CLEAR_JOIN_AN(tupleD);
            MEASURE_OP_TUPLE(tupleE, (matchjoinAN(bat5, batLDenc, std::get<8>(*v2_resoid_t::As), std::get<8>(*v2_resoid_t::Ainvs), std::get<3>(*v2_restiny_t::As), std::get<3>(*v2_restiny_t::Ainvs))));
            delete bat5;
            CLEAR_JOIN_AN(tupleE);
            MEASURE_OP_TUPLE(tupleF,
                    (matchjoinAN(batC, std::get<0>(tupleE), std::get<7>(*v2_resoid_t::As), std::get<7>(*v2_resoid_t::Ainvs), std::get<2>(*v2_restiny_t::As), std::get<2>(*v2_restiny_t::Ainvs))));
            delete batC;
            delete std::get<0>(tupleE);
            CLEAR_JOIN_AN(tupleF);

            // 4) result
            MEASURE_OP_TUPLE(tupleG, (aggregate_mul_sumAN<v2_resbigint_t>(std::get<0>(tupleD), std::get<0>(tupleF))));
            delete std::get<0>(tupleD);
            delete std::get<0>(tupleF);
            delete std::get<1>(tupleG);
            delete std::get<2>(tupleG);
            auto iter = std::get<0>(tupleG)->begin();
            auto result = iter->tail() * std::get<0>(tupleG)->tail.metaData.AN_Ainv;
            delete iter;
            delete std::get<0>(tupleG);

            ssb::after_query(i, result);
        }

        ssb::after_queries();

        delete batDYenc;
        delete batDDenc;
        delete batLQenc;
        delete batLDenc;
        delete batLOenc;
        delete batLEenc;

        ssb::finalize();

        return 0;
    });
}
