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
 * File:   ssbm-q23_normal.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 24. May 2017, 13:40
 */

#include "ssb.hpp"

int main(int argc, char** argv) {
    SSBM_REQUIRED_VARIABLES("SSBM Query 2.3 Normal\n=====================", 34, "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M", "N", "O", "P",
            "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z");

    SSBM_LOAD("date", "lineorder", "part", "supplier", "SSBM Q2.3:\n"
            "select sum(lo_revenue), d_year, p_brand\n"
            "  from lineorder, date, part, supplier\n"
            "  where lo_orderdate = d_datekey\n"
            "    and lo_partkey = p_partkey\n"
            "    and lo_suppkey = s_suppkey\n"
            "    and p_brand = 'MFGR#2239'\n"
            "    and s_region = 'EUROPE'\n"
            "  group by d_year, p_brand;");

    /* Measure loading ColumnBats */
    MEASURE_OP(batDDcb, new int_colbat_t("date", "datekey"));
    MEASURE_OP(batDYcb, new shortint_colbat_t("date", "year"));
    MEASURE_OP(batLPcb, new int_colbat_t("lineorder", "partkey"));
    MEASURE_OP(batLScb, new int_colbat_t("lineorder", "suppkey"));
    MEASURE_OP(batLOcb, new int_colbat_t("lineorder", "orderdate"));
    MEASURE_OP(batLRcb, new int_colbat_t("lineorder", "revenue"));
    MEASURE_OP(batPPcb, new int_colbat_t("part", "partkey"));
    MEASURE_OP(batPBcb, new str_colbat_t("part", "brand"));
    MEASURE_OP(batSScb, new int_colbat_t("supplier", "suppkey"));
    MEASURE_OP(batSRcb, new str_colbat_t("supplier", "region"));

    ssb::after_create_columnbats();

    /* Measure converting (copying) ColumnBats to TempBats */
    MEASURE_OP(batDD, copy(batDDcb));
    MEASURE_OP(batDY, copy(batDYcb));
    MEASURE_OP(batLP, copy(batLPcb));
    MEASURE_OP(batLS, copy(batLScb));
    MEASURE_OP(batLO, copy(batLOcb));
    MEASURE_OP(batLR, copy(batLRcb));
    MEASURE_OP(batPP, copy(batPPcb));
    MEASURE_OP(batPB, copy(batPBcb));
    MEASURE_OP(batSS, copy(batSScb));
    MEASURE_OP(batSR, copy(batSRcb));

    delete batDDcb;
    delete batDYcb;
    delete batLPcb;
    delete batLScb;
    delete batLOcb;
    delete batLRcb;
    delete batPPcb;
    delete batPBcb;
    delete batSScb;
    delete batSRcb;

    ssb::before_queries();

    for (size_t i = 0; i < ssb::ssb_config.NUM_RUNS; ++i) {
        ssb::before_query();

        // s_region = 'EUROPE'
        MEASURE_OP(bat1, select<std::equal_to>(batSR, const_cast<str_t>("EUROPE"))); // OID supplier | s_region
        auto bat2 = bat1->mirror_head(); // OID supplier | OID supplier
        delete bat1;
        auto bat3 = batSS->reverse(); // s_suppkey | OID supplier
        MEASURE_OP(bat4, matchjoin(bat3, bat2)); // s_suppkey | OID supplier
        delete bat2;
        delete bat3;
        // lo_suppkey = s_suppkey
        MEASURE_OP(bat5, hashjoin(batLS, bat4)); // OID lineorder | OID supplier
        delete bat4;
        // join with LO_PARTKEY to already reduce the join partners
        auto bat6 = bat5->mirror_head(); // OID lineorder | OID Lineorder
        delete bat5;
        MEASURE_OP(bat7, matchjoin(bat6, batLP)); // OID lineorder | lo_partkey (where s_region = 'AMERICA')
        delete bat6;

        // p_brand = 'MFGR#2239'
        MEASURE_OP(bat8, select<std::equal_to>(batPB, const_cast<str_t>("MFGR#2239"))); // OID part | p_brand
        auto bat9 = bat8->mirror_head(); // OID part | OID part
        auto batA = batPP->reverse(); // p_partkey | OID part
        MEASURE_OP(batB, matchjoin(batA, bat9)); // p_partkey | OID Part where p_category = 'MFGR#12'
        delete batA;
        delete bat9;
        MEASURE_OP(batC, hashjoin(bat7, batB)); // OID lineorder | OID part (where s_region = 'AMERICA' and p_category = 'MFGR#12')
        delete bat7;
        delete batB;

        // join with date now!
        auto batE = batC->mirror_head(); // OID lineorder | OID lineorder  (where ...)
        delete batC;
        MEASURE_OP(batF, matchjoin(batE, batLO)); // OID lineorder | lo_orderdate (where ...)
        delete batE;
        auto batH = batDD->reverse(); // d_datekey | OID date
        MEASURE_OP(batI, hashjoin(batF, batH)); // OID lineorder | OID date (where ..., joined with date)
        delete batF;
        delete batH;

        // now prepare grouped sum
        auto batW = batI->mirror_head(); // OID lineorder | OID lineorder
        MEASURE_OP(batX, matchjoin(batW, batLP)); // OID lineorder | lo_partkey
        auto batY = batPP->reverse(); // p_partkey | OID part
        MEASURE_OP(batZ, hashjoin(batX, batY)); // OID lineorder | OID part
        delete batX;
        delete batY;
        MEASURE_OP(batA1, hashjoin(batZ, bat8)); // OID lineorder | p_brand
        delete batZ;
        delete bat8;

        MEASURE_OP(batA2, hashjoin(batI, batDY)); // OID lineorder | d_year
        delete batI;

        MEASURE_OP(batA3, matchjoin(batW, batLR)); // OID lineorder | lo_revenue (where ...)
        delete batW;

        MEASURE_OP_TUPLE(tupleK, groupedSum<v2_bigint_t>(batA3, batA2, batA1));
        delete batA1;
        delete batA2;
        delete batA3;

        auto szResult = std::get<0>(tupleK)->size();

        ssb::after_query(i, szResult);

        if (ssb::ssb_config.PRINT_RESULT && i == 0) {
            size_t sum = 0;
            auto iter1 = std::get<0>(tupleK)->begin();
            auto iter2 = std::get<1>(tupleK)->begin();
            auto iter3 = std::get<2>(tupleK)->begin();
            auto iter4 = std::get<3>(tupleK)->begin();
            auto iter5 = std::get<4>(tupleK)->begin();
            std::cerr << "+------------+--------+-----------+\n";
            std::cerr << "| lo_revenue | d_year | p_brand   |\n";
            std::cerr << "+============+========+===========+\n";
            for (; iter1->hasNext(); ++*iter1, ++*iter2, ++*iter4) {
                sum += iter1->tail();
                std::cerr << "| " << std::setw(10) << iter1->tail();
                iter3->position(iter2->tail());
                std::cerr << " | " << std::setw(6) << iter3->tail();
                iter5->position(iter4->tail());
                std::cerr << " | " << std::setw(9) << iter5->tail() << " |\n";
            }
            std::cerr << "+============+========+===========+\n";
            std::cerr << "\t   sum: " << sum << std::endl;
            delete iter1;
            delete iter2;
            delete iter3;
            delete iter4;
            delete iter5;
        }
        delete std::get<0>(tupleK);
        delete std::get<1>(tupleK);
        delete std::get<2>(tupleK);
        delete std::get<3>(tupleK);
        delete std::get<4>(tupleK);
    }

    ssb::after_queries();

    delete batDD;
    delete batDY;
    delete batLP;
    delete batLS;
    delete batLO;
    delete batLR;
    delete batPP;
    delete batPB;
    delete batSS;
    delete batSR;

    ssb::finalize();

    return 0;
}