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
 * File:   ssbm-q34_normal.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 14. June 2017, 00:29
 */

#include "ssb.hpp"
#include "macros.hpp"

int main(
        int argc,
        char** argv) {
    ssb::init(argc, argv, "SSBM Query 3.4 Normal");

    ssb::loadTables( {"customer", "lineorder", "supplier", "date"}, "SSBM Q3.4:\n"
            "select c_city, s_city, d_year, sum(lo_revenue) as revenue\n"
            "  from customer, lineorder, supplier, date\n"
            "  where lo_custkey = c_custkey\n"
            "    and lo_suppkey = s_suppkey\n"
            "    and lo_orderdate = d_datekey\n"
            "    and (c_city = 'UNITED KI1' or c_city = 'UNITED KI5')\n"
            "    and (s_city = 'UNITED KI1' or s_city = 'UNITED KI5')\n"
            "    and d_yearmonth = 'Dec1997'\n"
            "  group by c_city, s_city, d_year;");

    /* Measure loading ColumnBats */
    MEASURE_OP(batCKcb, new int_colbat_t("customer", "custkey"));
    MEASURE_OP(batCCcb, new str_colbat_t("customer", "city"));
    MEASURE_OP(batDDcb, new int_colbat_t("date", "datekey"));
    MEASURE_OP(batDYcb, new shortint_colbat_t("date", "year"));
    MEASURE_OP(batDMcb, new str_colbat_t("date", "yearmonth"));
    MEASURE_OP(batLCcb, new int_colbat_t("lineorder", "custkey"));
    MEASURE_OP(batLScb, new int_colbat_t("lineorder", "suppkey"));
    MEASURE_OP(batLOcb, new int_colbat_t("lineorder", "orderdate"));
    MEASURE_OP(batLRcb, new int_colbat_t("lineorder", "revenue"));
    MEASURE_OP(batSScb, new int_colbat_t("supplier", "suppkey"));
    MEASURE_OP(batSCcb, new str_colbat_t("supplier", "city"));

    ssb::after_create_columnbats();

    /* Measure converting (copying) ColumnBats to TempBats */
    MEASURE_OP(batCK, copy(batCKcb));
    MEASURE_OP(batCC, copy(batCCcb));
    MEASURE_OP(batDD, copy(batDDcb));
    MEASURE_OP(batDY, copy(batDYcb));
    MEASURE_OP(batDM, copy(batDMcb));
    MEASURE_OP(batLC, copy(batLCcb));
    MEASURE_OP(batLS, copy(batLScb));
    MEASURE_OP(batLO, copy(batLOcb));
    MEASURE_OP(batLR, copy(batLRcb));
    MEASURE_OP(batSS, copy(batSScb));
    MEASURE_OP(batSC, copy(batSCcb));

    delete batCKcb;
    delete batCCcb;
    delete batDDcb;
    delete batDMcb;
    delete batLCcb;
    delete batLScb;
    delete batLOcb;
    delete batLRcb;
    delete batSScb;
    delete batSCcb;

    ssb::before_queries();

    for (size_t i = 0; i < ssb::ssb_config.NUM_RUNS; ++i) {
        ssb::before_query();

        // s_city = 'UNITED KI1' or s_city = 'UNITED KI5'
        MEASURE_OP(bat1, (select<std::equal_to, std::equal_to, ahead::or_is>(batSC, const_cast<str_t>("UNITED KI1"), const_cast<str_t>("UNITED KI5")))); // OID supplier | s_city
        auto bat2 = bat1->mirror_head(); // OID supplier | OID supplier
        delete bat1;
        auto bat3 = batSS->reverse(); // s_suppkey | VOID supplier
        MEASURE_OP(bat4, matchjoin(bat3, bat2)); // s_suppkey | OID supplier
        delete bat2;
        delete bat3;
        // lo_suppkey = s_suppkey
        MEASURE_OP(bat5, hashjoin(batLS, bat4)); // OID lineorder | OID supplier
        auto bat6 = bat5->mirror_head(); // OID lineorder | OID lineorder
        delete bat5;

        // c_city = 'UNITED KI1' or c_city = 'UNITED KI5'
        MEASURE_OP(bat7, (select<std::equal_to, std::equal_to, ahead::or_is>(batCC, const_cast<str_t>("UNITED KI1"), const_cast<str_t>("UNITED KI5")))); // OID customer | c_city
        auto bat8 = bat7->mirror_head(); // OID customer | OID customer
        delete bat7;
        auto bat9 = batCK->reverse(); // c_custkey | VOID customer
        MEASURE_OP(bat10, matchjoin(bat9, bat8)); // c_custkey | OID customer
        delete bat8;
        delete bat9;
        // reduce number of l_custkey joinpartners
        MEASURE_OP(bat11, matchjoin(bat6, batLC)); // OID lineorder | l_custkey
        delete bat6;
        // lo_custkey = c_custkey
        MEASURE_OP(bat12, hashjoin(bat11, bat10)); // OID lineorder | OID customer
        delete bat11;

        // d_yearmonth = 'Dec1997'
        MEASURE_OP(bat13, select<std::equal_to>(batDM, const_cast<str_t>("Dec1997"))); // OID date | d_yearmonth
        auto bat14 = bat13->mirror_head(); // OID date | OID date
        delete bat13;
        MEASURE_OP(bat15, matchjoin(bat14, batDD)); // OID date | d_datekey
        delete bat14;
        auto bat16 = bat15->reverse(); // d_datekey | OID date
        delete bat15;
        auto bat17 = bat12->mirror_head(); // OID lineorder | OID lineorder
        delete bat12;
        // reduce number of lo_orderdate joinpartners
        MEASURE_OP(bat18, matchjoin(bat17, batLO)); // OID lineorder | lo_orderdate
        delete bat17;
        // lo_orderdate = d_datekey
        MEASURE_OP(bat19, hashjoin(bat18, bat16)); // OID lineorder | OID date
        delete bat18;

        // prepare grouping
        auto bat20 = bat19->mirror_head(); // OID lineorder | OID lineorder
        delete bat19;
        auto bat21 = bat20->clear_head(); // VOID | OID lineorder
        delete bat20;
        MEASURE_OP(batAR, fetchjoin(bat21, batLR)); // VOID | lo_revenue !!!
        MEASURE_OP(bat22, fetchjoin(bat21, batLS)); // VOID | lo_suppkey
        MEASURE_OP(bat23, hashjoin(bat22, bat4)); // OID | OID supplier
        delete bat4;
        delete bat22;
        auto bat24 = bat23->clear_head(); // VOID | OID supplier
        delete bat23;
        MEASURE_OP(batAS, fetchjoin(bat24, batSC)); // VOID | s_city !!!
        delete bat24;
        MEASURE_OP(bat25, fetchjoin(bat21, batLC)); // VOID | lo_custkey
        MEASURE_OP(bat26, hashjoin(bat25, bat10)); // OID | OID customer
        delete bat10;
        delete bat25;
        auto bat27 = bat26->clear_head(); // VOID | OID customer
        delete bat26;
        MEASURE_OP(batAC, fetchjoin(bat27, batCC)); // VOID | c_city !!!
        delete bat27;
        MEASURE_OP(bat28, fetchjoin(bat21, batLO)); // VOID | lo_orderdate
        delete bat21;
        MEASURE_OP(bat29, hashjoin(bat28, bat16)); // OID | OID date
        delete bat16;
        delete bat28;
        auto bat30 = bat29->clear_head(); // VOID | OID date
        delete bat29;
        MEASURE_OP(batAD, fetchjoin(bat30, batDY)); // VOID | d_year !!!
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

        auto szResult = batRR->size();

        ssb::after_query(i, szResult);

        if (ssb::ssb_config.PRINT_RESULT && i == 0) {
            bigint_t sum = 0;
            auto iter1 = batRC->begin();
            auto iter2 = batRS->begin();
            auto iter3 = batRD->begin();
            auto iter4 = batRR->begin();
            std::cerr << "+============+============+========+============+\n";
            std::cerr << "+     c_city |     s_city | d_year |    revenue |\n";
            std::cerr << "+------------+------------+--------+------------+\n";
            for (; iter1->hasNext(); ++*iter1, ++*iter2, ++*iter3, ++*iter4) {
                sum += iter4->tail();
                std::cerr << "| " << std::setw(10) << iter1->tail();
                std::cerr << " | " << std::setw(10) << iter2->tail();
                std::cerr << " | " << std::setw(6) << iter3->tail();
                std::cerr << " | " << std::setw(10) << iter4->tail() << " |\n";
            }
            std::cerr << "+============+============+========+============+\n";
            std::cerr << "\t   sum: " << sum << std::endl;
            delete iter1;
            delete iter2;
            delete iter3;
            delete iter4;
        }

        delete batRC;
        delete batRS;
        delete batRD;
        delete batRR;
    }

    ssb::after_queries();

    delete batCK;
    delete batCC;
    delete batDD;
    delete batDY;
    delete batDM;
    delete batLC;
    delete batLS;
    delete batLO;
    delete batLR;
    delete batSS;
    delete batSC;

    ssb::finalize();

    return 0;
}
