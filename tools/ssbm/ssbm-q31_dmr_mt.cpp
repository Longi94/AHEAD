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
 * File:   ssbm-q31_dmr_mt.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 12. June 2017, 09:41
 */

#include <omp.h>
#include "ssb.hpp"
#include "macros.hpp"
#include <util/ModularRedundant.hpp>

typedef typename std::tuple<BAT<v2_void_t, v2_str_t> *, BAT<v2_void_t, v2_str_t> *, BAT<v2_void_t, v2_shortint_t> *, BAT<v2_void_t, v2_bigint_t> *> result_tuple_t;
typedef DMRValue<result_tuple_t> DMR;

int main(
        int argc,
        char** argv) {
    ssb::init(argc, argv, "SSBM Query 3.1 DMR Parallel");

    ssb::loadTables( {"customer", "lineorder", "supplier", "date"}, "SSBM Q3.1:\n"
            "select c_nation, s_nation, d_year, sum(lo_revenue) as revenue\n"
            "  from customer, lineorder, supplier, date\n"
            "  where lo_custkey = c_custkey\n"
            "    and lo_suppkey = s_suppkey\n"
            "    and lo_orderdate = d_datekey\n"
            "    and c_region = 'ASIA'\n"
            "    and s_region = 'ASIA'\n"
            "    and d_year >= 1992 and d_year <= 1997\n"
            "  group by c_nation, s_nation, d_year;");

    /* Measure loading ColumnBats */
    MEASURE_OP(batCCcb, new int_colbat_t("customer", "custkey"));
    MEASURE_OP(batCRcb, new str_colbat_t("customer", "region"));
    MEASURE_OP(batCNcb, new str_colbat_t("customer", "nation"));
    MEASURE_OP(batDDcb, new int_colbat_t("date", "datekey"));
    MEASURE_OP(batDYcb, new shortint_colbat_t("date", "year"));
    MEASURE_OP(batLCcb, new int_colbat_t("lineorder", "custkey"));
    MEASURE_OP(batLScb, new int_colbat_t("lineorder", "suppkey"));
    MEASURE_OP(batLOcb, new int_colbat_t("lineorder", "orderdate"));
    MEASURE_OP(batLRcb, new int_colbat_t("lineorder", "revenue"));
    MEASURE_OP(batSScb, new int_colbat_t("supplier", "suppkey"));
    MEASURE_OP(batSRcb, new str_colbat_t("supplier", "region"));
    MEASURE_OP(batSNcb, new str_colbat_t("supplier", "nation"));

    ssb::after_create_columnbats();

    /* Measure converting (copying) ColumnBats to TempBats */
    int_tmpbat_t * batCC[DMR::modularity];
    str_tmpbat_t * batCR[DMR::modularity];
    str_tmpbat_t * batCN[DMR::modularity];
    int_tmpbat_t * batDD[DMR::modularity];
    shortint_tmpbat_t * batDY[DMR::modularity];
    int_tmpbat_t * batLC[DMR::modularity];
    int_tmpbat_t * batLS[DMR::modularity];
    int_tmpbat_t * batLO[DMR::modularity];
    int_tmpbat_t * batLR[DMR::modularity];
    int_tmpbat_t * batSS[DMR::modularity];
    str_tmpbat_t * batSR[DMR::modularity];
    str_tmpbat_t * batSN[DMR::modularity];

    for (size_t k = 0; k < DMR::modularity; ++k) {
        MEASURE_OP(batCC, [k], copy(batCCcb), batCC[k]);
        MEASURE_OP(batCR, [k], copy(batCRcb), batCR[k]);
        MEASURE_OP(batCN, [k], copy(batCNcb), batCN[k]);
        MEASURE_OP(batDD, [k], copy(batDDcb), batDD[k]);
        MEASURE_OP(batDY, [k], copy(batDYcb), batDY[k]);
        MEASURE_OP(batLC, [k], copy(batLCcb), batLC[k]);
        MEASURE_OP(batLS, [k], copy(batLScb), batLS[k]);
        MEASURE_OP(batLO, [k], copy(batLOcb), batLO[k]);
        MEASURE_OP(batLR, [k], copy(batLRcb), batLR[k]);
        MEASURE_OP(batSS, [k], copy(batSScb), batSS[k]);
        MEASURE_OP(batSR, [k], copy(batSRcb), batSR[k]);
        MEASURE_OP(batSN, [k], copy(batSNcb), batSN[k]);
    }

    delete batCCcb;
    delete batCRcb;
    delete batCNcb;
    delete batDDcb;
    delete batDYcb;
    delete batLCcb;
    delete batLScb;
    delete batLOcb;
    delete batLRcb;
    delete batSScb;
    delete batSRcb;
    delete batSNcb;

    ssb::before_queries();

    for (size_t i = 0; i < ssb::ssb_config.NUM_RUNS; ++i) {
        ssb::before_query();

        DMR results( {nullptr, nullptr, nullptr, nullptr});

#pragma omp parallel for
        for (size_t k = 0; k < DMR::modularity; ++k) {
            // s_region = 'ASIA'
            MEASURE_OP(bat1, select<std::equal_to>(batSR[k], const_cast<str_t>("ASIA"))); // OID supplier | s_region
            auto bat2 = bat1->mirror_head(); // OID supplier | OID supplier
            delete bat1;
            auto bat3 = batSS[k]->reverse(); // s_suppkey | VOID supplier
            MEASURE_OP(bat4, matchjoin(bat3, bat2)); // s_suppkey | OID supplier
            delete bat2;
            delete bat3;
            // lo_suppkey = s_suppkey
            MEASURE_OP(bat5, hashjoin(batLS[k], bat4)); // OID lineorder | OID supplier
            auto bat6 = bat5->mirror_head(); // OID lineorder | OID lineorder
            delete bat5;

            // c_region = 'ASIA'
            MEASURE_OP(bat7, select<std::equal_to>(batCR[k], const_cast<str_t>("ASIA"))); // OID customer | c_region
            auto bat8 = bat7->mirror_head(); // OID customer | OID customer
            delete bat7;
            auto bat9 = batCC[k]->reverse(); // c_custkey | VOID customer
            MEASURE_OP(bat10, matchjoin(bat9, bat8)); // c_custkey | OID customer
            delete bat8;
            delete bat9;
            // reduce number of l_custkey joinpartners
            MEASURE_OP(bat11, matchjoin(bat6, batLC[k])); // OID lineorder | l_custkey
            delete bat6;
            // lo_custkey = c_custkey
            MEASURE_OP(bat12, hashjoin(bat11, bat10)); // OID lineorder | OID customer
            delete bat11;

            // d_year >= 1992 and d_year <= 1997
            MEASURE_OP(bat13, (select<std::greater_equal, std::less_equal, ahead::and_is>(batDY[k], 1992, 1997))); // OID date | d_year
            auto bat14 = bat13->mirror_head(); // OID date | OID date
            delete bat13;
            MEASURE_OP(bat15, matchjoin(bat14, batDD[k])); // OID date | d_datekey
            delete bat14;
            auto bat16 = bat15->reverse(); // d_datekey | OID date
            delete bat15;
            auto bat17 = bat12->mirror_head(); // OID lineorder | OID lineorder
            delete bat12;
            MEASURE_OP(bat18, matchjoin(bat17, batLO[k])); // OID lineorder | lo_orderdate
            delete bat17;
            // lo_orderdate = d_datekey
            MEASURE_OP(bat19, hashjoin(bat18, bat16)); // OID lineorder | OID date
            delete bat18;

            // prepare grouping
            auto bat20 = bat19->mirror_head(); // OID lineorder | OID lineorder
            delete bat19;
            auto bat21 = bat20->clear_head(); // VOID | OID lineorder
            delete bat20;
            MEASURE_OP(batAR, fetchjoin(bat21, batLR[k])); // VOID | lo_revenue !!!
            MEASURE_OP(bat22, fetchjoin(bat21, batLS[k])); // VOID | lo_suppkey
            MEASURE_OP(bat23, hashjoin(bat22, bat4)); // OID | OID supplier
            delete bat4;
            delete bat22;
            auto bat24 = bat23->clear_head(); // VOID | OID supplier
            delete bat23;
            MEASURE_OP(batAS, fetchjoin(bat24, batSN[k])); // VOID | s_nation !!!
            delete bat24;
            MEASURE_OP(bat25, fetchjoin(bat21, batLC[k])); // VOID | lo_custkey
            MEASURE_OP(bat26, hashjoin(bat25, bat10)); // OID | OID customer
            delete bat10;
            delete bat25;
            auto bat27 = bat26->clear_head(); // VOID | OID customer
            delete bat26;
            MEASURE_OP(batAC, fetchjoin(bat27, batCN[k])); // VOID | c_nation !!!
            delete bat27;
            MEASURE_OP(bat28, fetchjoin(bat21, batLO[k])); // VOID | lo_orderdate
            delete bat21;
            MEASURE_OP(bat29, hashjoin(bat28, bat16)); // OID | OID date
            delete bat16;
            delete bat28;
            auto bat30 = bat29->clear_head(); // VOID | OID date
            delete bat29;
            MEASURE_OP(batAD, fetchjoin(bat30, batDY[k])); // VOID | d_year !!!
            delete bat30;

            // grouping
            MEASURE_OP_PAIR(pairGD, groupby(batAD));
            MEASURE_OP_PAIR(pairGS, groupby(batAS, std::get<0>(pairGD), std::get<1>(pairGD)->size()));
            delete std::get<0>(pairGD);
            delete std::get<1>(pairGD);
            MEASURE_OP_PAIR(pairGC, groupby(batAC, std::get<0>(pairGS), std::get<1>(pairGS)->size()));
            delete std::get<0>(pairGS);
            delete std::get<1>(pairGS);

            // result
            MEASURE_OP(batRR, aggregate_sum_grouped<v2_bigint_t>(batAR, std::get<0>(pairGC), std::get<1>(pairGC)->size()));
            delete batAR;
            MEASURE_OP(batRD, fetchjoin(std::get<1>(pairGC), batAD));
            delete batAD;
            MEASURE_OP(batRC, fetchjoin(std::get<1>(pairGC), batAC));
            delete batAC;
            MEASURE_OP(batRS, fetchjoin(std::get<1>(pairGC), batAS));
            delete batAS;
            delete std::get<0>(pairGC);
            delete std::get<1>(pairGC);

#pragma omp critical
            {
                results[k] = std::make_tuple(batRC, batRS, batRD, batRR);
            }
        }

        // 5) Voting and Result Printing
        try {
            result_tuple_t content = ahead::vote_majority_tuple(results);
            oid_t szResult = std::get<0>(content)->size();
            ssb::after_query(i, szResult);
            if (ssb::ssb_config.PRINT_RESULT && i == 0) {
                bigint_t sum = 0;
                auto iter1 = std::get<0>(results[0])->begin();
                auto iter2 = std::get<1>(results[0])->begin();
                auto iter3 = std::get<2>(results[0])->begin();
                auto iter4 = std::get<3>(results[0])->begin();
                std::cerr << "+=================+=================+========+============+\n";
                std::cerr << "+        c_nation |        s_nation | d_year |    revenue |\n";
                std::cerr << "+-----------------+-----------------+--------+------------+\n";
                for (; iter1->hasNext(); ++*iter1, ++*iter2, ++*iter3, ++*iter4) {
                    sum += iter4->tail();
                    std::cerr << "| " << std::setw(15) << iter1->tail();
                    std::cerr << " | " << std::setw(15) << iter2->tail();
                    std::cerr << " | " << std::setw(6) << iter3->tail();
                    std::cerr << " | " << std::setw(10) << iter4->tail() << " |\n";
                }
                std::cerr << "+=================+=================+========+============+\n";
                std::cerr << "\t   sum: " << sum << std::endl;
                delete iter1;
                delete iter2;
                delete iter3;
            }
        } catch (std::exception & ex) {
            ssb::after_query(i, ex);
            if (ssb::ssb_config.PRINT_RESULT && i == 0) {
                std::cerr << "+=================+=================+========+============+\n";
                std::cerr << "+        c_nation |        s_nation | d_year |    revenue |\n";
                std::cerr << "+-----------------+-----------------+--------+------------+\n";
                std::cerr << "| " << ex.what() << "|\n";
                std::cerr << "+=================+=================+========+============+\n";
            }
        }

        for (size_t k = 0; k < DMR::modularity; ++k) {
            delete std::get<0>(results[k]);
            delete std::get<1>(results[k]);
            delete std::get<2>(results[k]);
            delete std::get<3>(results[k]);
        }
    }

    ssb::after_queries();

    for (size_t k = 0; k < DMR::modularity; ++k) {
        delete batCC[k];
        delete batCR[k];
        delete batCN[k];
        delete batDD[k];
        delete batDY[k];
        delete batLC[k];
        delete batLS[k];
        delete batLO[k];
        delete batLR[k];
        delete batSS[k];
        delete batSR[k];
        delete batSN[k];
    }

    ssb::finalize();

    return 0;
}
