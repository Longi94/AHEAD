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
 * File:   ssbm-q21.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 6. November 2016, 22:25
 */

#include "ssbm.hpp"

int main(int argc, char** argv) {
    SSBM_REQUIRED_VARIABLES("SSBM Query 2.1 Normal\n=====================", 34, "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M", "N", "O", "P",
            "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z");

    SSBM_LOAD("date", "lineorder", "part", "supplier", "SSBM Q2.1:\n"
            "select sum(lo_revenue), d_year, p_brand\n"
            "  from lineorder, part, supplier, date\n"
            "  where lo_orderdate = d_datekey\n"
            "    and lo_partkey = p_partkey\n"
            "    and lo_suppkey = s_suppkey\n"
            "    and p_category = 'MFGR#12'\n"
            "    and s_region = 'AMERICA'\n"
            "  group by d_year, p_brand;");

    /* Measure loading ColumnBats */
    MEASURE_OP(batDDcb, new int_colbat_t("date", "datekey"));
    MEASURE_OP(batDYcb, new shortint_colbat_t("date", "year"));
    MEASURE_OP(batLPcb, new int_colbat_t("lineorder", "partkey"));
    MEASURE_OP(batLScb, new int_colbat_t("lineorder", "suppkey"));
    MEASURE_OP(batLOcb, new int_colbat_t("lineorder", "orderdate"));
    MEASURE_OP(batLRcb, new int_colbat_t("lineorder", "revenue"));
    MEASURE_OP(batPPcb, new int_colbat_t("part", "partkey"));
    MEASURE_OP(batPCcb, new str_colbat_t("part", "category"));
    MEASURE_OP(batPBcb, new str_colbat_t("part", "brand"));
    MEASURE_OP(batSScb, new int_colbat_t("supplier", "suppkey"));
    MEASURE_OP(batSRcb, new str_colbat_t("supplier", "region"));

    /* Measure converting (copying) ColumnBats to TempBats */
    MEASURE_OP(batDD, ahead::bat::ops::copy(batDDcb));
    MEASURE_OP(batDY, ahead::bat::ops::copy(batDYcb));
    MEASURE_OP(batLP, ahead::bat::ops::copy(batLPcb));
    MEASURE_OP(batLS, ahead::bat::ops::copy(batLScb));
    MEASURE_OP(batLO, ahead::bat::ops::copy(batLOcb));
    MEASURE_OP(batLR, ahead::bat::ops::copy(batLRcb));
    MEASURE_OP(batPP, ahead::bat::ops::copy(batPPcb));
    MEASURE_OP(batPC, ahead::bat::ops::copy(batPCcb));
    MEASURE_OP(batPB, ahead::bat::ops::copy(batPBcb));
    MEASURE_OP(batSS, ahead::bat::ops::copy(batSScb));
    MEASURE_OP(batSR, ahead::bat::ops::copy(batSRcb));

    delete batDDcb;
    delete batDYcb;
    delete batLPcb;
    delete batLScb;
    delete batLOcb;
    delete batLRcb;
    delete batPPcb;
    delete batPCcb;
    delete batPBcb;
    delete batSScb;
    delete batSRcb;

    SSBM_BEFORE_QUERIES;

    for (size_t i = 0; i < CONFIG.NUM_RUNS; ++i) {
        SSBM_BEFORE_QUERY;

        // s_region = 'AMERICA'
        MEASURE_OP(bat1, ahead::bat::ops::select<std::equal_to>(batSR, const_cast<str_t>("AMERICA"))); // OID supplier | s_region
        auto bat2 = bat1->mirror_head(); // OID supplier | OID supplier
        delete bat1;
        auto bat3 = batSS->reverse(); // s_suppkey | OID supplier
        MEASURE_OP(bat4, ahead::bat::ops::matchjoin(bat3, bat2)); // s_suppkey | OID supplier
        delete bat2;
        delete bat3;
        // lo_suppkey = s_suppkey
        MEASURE_OP(bat5, ahead::bat::ops::hashjoin(batLS, bat4)); // OID lineorder | OID supplier
        delete bat4;
        // join with LO_PARTKEY to already reduce the join partners
        auto bat6 = bat5->mirror_head(); // OID lineorder | OID Lineorder
        delete bat5;
        MEASURE_OP(bat7, ahead::bat::ops::matchjoin(bat6, batLP)); // OID lineorder | lo_partkey (where s_region = 'AMERICA')
        delete bat6;

        // p_category = 'MFGR#12'
        MEASURE_OP(bat8, ahead::bat::ops::select<std::equal_to>(batPC, const_cast<str_t>("MFGR#12"))); // OID part | p_category
        // p_brand = 'MFGR#121'
        // MEASURE_OP(bat8, ahead::bat::ops::select<equal_to>(batPB, "MFGR#121")); // OID part | p_brand
        auto bat9 = bat8->mirror_head(); // OID part | OID part
        delete bat8;
        auto batA = batPP->reverse(); // p_partkey | OID part
        MEASURE_OP(batB, ahead::bat::ops::matchjoin(batA, bat9)); // p_partkey | OID Part where p_category = 'MFGR#12'
        delete batA;
        delete bat9;
        MEASURE_OP(batC, ahead::bat::ops::hashjoin(bat7, batB)); // OID lineorder | OID part (where s_region = 'AMERICA' and p_category = 'MFGR#12')
        delete bat7;
        delete batB;

        // join with date now!
        auto batE = batC->mirror_head(); // OID lineorder | OID lineorder  (where ...)
        delete batC;
        MEASURE_OP(batF, ahead::bat::ops::matchjoin(batE, batLO)); // OID lineorder | lo_orderdate (where ...)
        delete batE;
        auto batH = batDD->reverse(); // d_datekey | OID date
        MEASURE_OP(batI, ahead::bat::ops::hashjoin(batF, batH)); // OID lineorder | OID date (where ..., joined with date)
        delete batF;
        delete batH;

        // now prepare grouped sum
        auto batW = batI->mirror_head(); // OID lineorder | OID lineorder
        MEASURE_OP(batX, ahead::bat::ops::matchjoin(batW, batLP)); // OID lineorder | lo_partkey
        auto batY = batPP->reverse(); // p_partkey | OID part
        MEASURE_OP(batZ, ahead::bat::ops::hashjoin(batX, batY)); // OID lineorder | OID part
        delete batX;
        delete batY;
        MEASURE_OP(batA1, ahead::bat::ops::hashjoin(batZ, batPB)); // OID lineorder | p_brand
        delete batZ;

        MEASURE_OP(batA2, ahead::bat::ops::hashjoin(batI, batDY)); // OID lineorder | d_year
        delete batI;

        MEASURE_OP(batA3, ahead::bat::ops::matchjoin(batW, batLR)); // OID lineorder | lo_revenue (where ...)
        delete batW;

        MEASURE_OP_TUPLE(tupleK, ahead::bat::ops::groupedSum<v2_bigint_t>(batA3, batA2, batA1));
        delete batA1;
        delete batA2;
        delete batA3;

        auto szResult = std::get<0>(tupleK)->size();

        SSBM_AFTER_QUERY(i, szResult);

        if (CONFIG.PRINT_RESULT && i == 0) {
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

    SSBM_AFTER_QUERIES;

    delete batDD;
    delete batDY;
    delete batLP;
    delete batLS;
    delete batLO;
    delete batLR;
    delete batPP;
    delete batPC;
    delete batPB;
    delete batSS;
    delete batSR;

    SSBM_FINALIZE;

    return 0;
}
