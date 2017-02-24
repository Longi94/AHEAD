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
 * File:   ssbm-q21_dmr_seq.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 24. January 2017, 17:19
 */

#include "ssbm.hpp"

int
main (int argc, char** argv) {
    const size_t MODULARITY = 2;

    SSBM_REQUIRED_VARIABLES("SSBM Query 2.1 DMR Sequential\n=============================", 34, "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z");

    SSBM_LOAD("date", "lineorder", "part", "supplier",
        "SSBM Q2.1:\n"                               \
        "select sum(lo_revenue), d_year, p_brand\n"  \
        "  from lineorder, part, supplier, date\n"   \
        "  where lo_orderdate = d_datekey\n"         \
        "    and lo_partkey = p_partkey\n"           \
        "    and lo_suppkey = s_suppkey\n"           \
        "    and p_category = 'MFGR#12'\n"           \
        "    and s_region = 'AMERICA'\n"             \
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
    int_tmpbat_t * batDDs[2];
    shortint_tmpbat_t * batDYs[2];
    int_tmpbat_t * batLPs[2];
    int_tmpbat_t * batLSs[2];
    int_tmpbat_t * batLOs[2];
    int_tmpbat_t * batLRs[2];
    int_tmpbat_t * batPPs[2];
    str_tmpbat_t * batPCs[2];
    str_tmpbat_t * batPBs[2];
    int_tmpbat_t * batSSs[2];
    str_tmpbat_t * batSRs[2];

    MEASURE_OP(batDDs, [0], v2::bat::ops::copy(batDDcb), batDDs[0]->size(), batDDs[0]->consumption(), batDDs[0]->consumptionProjected());
    MEASURE_OP(batDYs, [0], v2::bat::ops::copy(batDYcb), batDYs[0]->size(), batDYs[0]->consumption(), batDDs[0]->consumptionProjected());
    MEASURE_OP(batLPs, [0], v2::bat::ops::copy(batLPcb), batLPs[0]->size(), batLPs[0]->consumption(), batDDs[0]->consumptionProjected());
    MEASURE_OP(batLSs, [0], v2::bat::ops::copy(batLScb), batLSs[0]->size(), batLSs[0]->consumption(), batDDs[0]->consumptionProjected());
    MEASURE_OP(batLOs, [0], v2::bat::ops::copy(batLOcb), batLOs[0]->size(), batLOs[0]->consumption(), batDDs[0]->consumptionProjected());
    MEASURE_OP(batLRs, [0], v2::bat::ops::copy(batLRcb), batLRs[0]->size(), batLRs[0]->consumption(), batDDs[0]->consumptionProjected());
    MEASURE_OP(batPPs, [0], v2::bat::ops::copy(batPPcb), batPPs[0]->size(), batPPs[0]->consumption(), batDDs[0]->consumptionProjected());
    MEASURE_OP(batPCs, [0], v2::bat::ops::copy(batPCcb), batPCs[0]->size(), batPCs[0]->consumption(), batDDs[0]->consumptionProjected());
    MEASURE_OP(batPBs, [0], v2::bat::ops::copy(batPBcb), batPBs[0]->size(), batPBs[0]->consumption(), batDDs[0]->consumptionProjected());
    MEASURE_OP(batSSs, [0], v2::bat::ops::copy(batSScb), batSSs[0]->size(), batSSs[0]->consumption(), batDDs[0]->consumptionProjected());
    MEASURE_OP(batSRs, [0], v2::bat::ops::copy(batSRcb), batSRs[0]->size(), batSRs[0]->consumption(), batDDs[0]->consumptionProjected());

    MEASURE_OP(batDDs, [1], v2::bat::ops::copy(batDDcb), batDDs[1]->size(), batDDs[1]->consumption(), batDDs[1]->consumptionProjected());
    MEASURE_OP(batDYs, [1], v2::bat::ops::copy(batDYcb), batDYs[1]->size(), batDYs[1]->consumption(), batDDs[1]->consumptionProjected());
    MEASURE_OP(batLPs, [1], v2::bat::ops::copy(batLPcb), batLPs[1]->size(), batLPs[1]->consumption(), batDDs[1]->consumptionProjected());
    MEASURE_OP(batLSs, [1], v2::bat::ops::copy(batLScb), batLSs[1]->size(), batLSs[1]->consumption(), batDDs[1]->consumptionProjected());
    MEASURE_OP(batLOs, [1], v2::bat::ops::copy(batLOcb), batLOs[1]->size(), batLOs[1]->consumption(), batDDs[1]->consumptionProjected());
    MEASURE_OP(batLRs, [1], v2::bat::ops::copy(batLRcb), batLRs[1]->size(), batLRs[1]->consumption(), batDDs[1]->consumptionProjected());
    MEASURE_OP(batPPs, [1], v2::bat::ops::copy(batPPcb), batPPs[1]->size(), batPPs[1]->consumption(), batDDs[1]->consumptionProjected());
    MEASURE_OP(batPCs, [1], v2::bat::ops::copy(batPCcb), batPCs[1]->size(), batPCs[1]->consumption(), batDDs[1]->consumptionProjected());
    MEASURE_OP(batPBs, [1], v2::bat::ops::copy(batPBcb), batPBs[1]->size(), batPBs[1]->consumption(), batDDs[1]->consumptionProjected());
    MEASURE_OP(batSSs, [1], v2::bat::ops::copy(batSScb), batSSs[1]->size(), batSSs[1]->consumption(), batDDs[1]->consumptionProjected());
    MEASURE_OP(batSRs, [1], v2::bat::ops::copy(batSRcb), batSRs[1]->size(), batSRs[1]->consumption(), batDDs[1]->consumptionProjected());

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

        typedef typename std::tuple<TempBAT<v2_void_t, v2_bigint_t>*, TempBAT<v2_void_t, v2_oid_t>*, TempBAT<v2_void_t, v2_shortint_t>*, TempBAT<v2_void_t, v2_oid_t>*, TempBAT<v2_void_t, v2_str_t>*> result_tuple_t;
        result_tuple_t results[MODULARITY] = {
            {nullptr, nullptr, nullptr, nullptr, nullptr},
            {nullptr, nullptr, nullptr, nullptr, nullptr}
        };

        for (size_t k = 0; k < MODULARITY; ++k) {
            // s_region = 'AMERICA'
            MEASURE_OP(bat1, v2::bat::ops::select<std::equal_to>(batSRs[k], const_cast<str_t>("AMERICA"))); // OID supplier | s_region
            auto bat2 = bat1->mirror_head(); // OID supplier | OID supplier
            delete bat1;
            auto bat3 = batSSs[k]->reverse(); // s_suppkey | OID supplier
            MEASURE_OP(bat4, v2::bat::ops::matchjoin(bat3, bat2)); // s_suppkey | OID supplier
            delete bat2;
            delete bat3;
            // lo_suppkey = s_suppkey
            MEASURE_OP(bat5, v2::bat::ops::hashjoin(batLSs[k], bat4)); // OID lineorder | OID supplier
            delete bat4;
            // join with LO_PARTKEY to already reduce the join partners
            auto bat6 = bat5->mirror_head(); // OID lineorder | OID Lineorder
            delete bat5;
            MEASURE_OP(bat7, v2::bat::ops::matchjoin(bat6, batLPs[k])); // OID lineorder | lo_partkey (where s_region = 'AMERICA')
            delete bat6;

            // p_category = 'MFGR#12'
            MEASURE_OP(bat8, v2::bat::ops::select<std::equal_to>(batPCs[k], const_cast<str_t>("MFGR#12"))); // OID part | p_category
            // p_brand = 'MFGR#121'
            // MEASURE_OP(bat8, v2::bat::ops::select<equal_to>(batPB, "MFGR#121")); // OID part | p_brand
            auto bat9 = bat8->mirror_head(); // OID part | OID part
            delete bat8;
            auto batA = batPPs[k]->reverse(); // p_partkey | OID part
            MEASURE_OP(batB, v2::bat::ops::matchjoin(batA, bat9)); // p_partkey | OID Part where p_category = 'MFGR#12'
            delete batA;
            delete bat9;
            MEASURE_OP(batC, v2::bat::ops::hashjoin(bat7, batB)); // OID lineorder | OID part (where s_region = 'AMERICA' and p_category = 'MFGR#12')
            delete bat7;
            delete batB;

            // join with date now!
            auto batE = batC->mirror_head(); // OID lineorder | OID lineorder  (where ...)
            delete batC;
            MEASURE_OP(batF, v2::bat::ops::matchjoin(batE, batLOs[k])); // OID lineorder | lo_orderdate (where ...)
            delete batE;
            auto batH = batDDs[k]->reverse(); // d_datekey | OID date
            MEASURE_OP(batI, v2::bat::ops::hashjoin(batF, batH)); // OID lineorder | OID date (where ..., joined with date)
            delete batF;
            delete batH;

            // now prepare grouped sum
            auto batW = batI->mirror_head(); // OID lineorder | OID lineorder
            MEASURE_OP(batX, v2::bat::ops::matchjoin(batW, batLPs[k])); // OID lineorder | lo_partkey
            auto batY = batPPs[k]->reverse(); // p_partkey | OID part
            MEASURE_OP(batZ, v2::bat::ops::hashjoin(batX, batY)); // OID lineorder | OID part
            delete batX;
            delete batY;
            MEASURE_OP(batA1, v2::bat::ops::hashjoin(batZ, batPBs[k])); // OID lineorder | p_brand
            delete batZ;

            MEASURE_OP(batA2, v2::bat::ops::hashjoin(batI, batDYs[k])); // OID lineorder | d_year
            delete batI;

            MEASURE_OP(batA3, v2::bat::ops::matchjoin(batW, batLRs[k])); // OID lineorder | lo_revenue (where ...)
            delete batW;

            MEASURE_OP_TUPLE(tupleK, v2::bat::ops::groupedSum<v2_bigint_t>(batA3, batA2, batA1));
            delete batA1;
            delete batA2;
            delete batA3;

            results[k] = tupleK;
        }

        // 5) Voting
        bool isSame = true;
        {
            auto iter11 = std::get<0>(results[0])->begin();
            auto iter12 = std::get<0>(results[1])->begin();
            for (; iter11->hasNext() && iter12->hasNext(); ++*iter11, ++*iter12) {
                isSame &= iter11->tail() == iter12->tail();
            }
            delete iter11;
            delete iter12;
            auto iter21 = std::get<1>(results[0])->begin();
            auto iter22 = std::get<1>(results[1])->begin();
            for (; iter21->hasNext() && iter22->hasNext(); ++*iter21, ++*iter22) {
                isSame &= iter21->tail() == iter22->tail();
            }
            delete iter21;
            delete iter22;
            auto iter31 = std::get<2>(results[0])->begin();
            auto iter32 = std::get<2>(results[1])->begin();
            for (; iter31->hasNext() && iter32->hasNext(); ++*iter31, ++*iter32) {
                isSame &= iter31->tail() == iter32->tail();
            }
            delete iter31;
            delete iter32;
            auto iter41 = std::get<3>(results[0])->begin();
            auto iter42 = std::get<3>(results[1])->begin();
            for (; iter41->hasNext() && iter42->hasNext(); ++*iter41, ++*iter42) {
                isSame &= iter41->tail() == iter42->tail();
            }
            delete iter41;
            delete iter42;
            auto iter51 = std::get<4>(results[0])->begin();
            auto iter52 = std::get<4>(results[1])->begin();
            for (; iter51->hasNext() && iter52->hasNext(); ++*iter51, ++*iter52) {
                isSame &= iter51->tail() == iter52->tail();
            }
            delete iter51;
            delete iter52;
        }

        auto szResult = std::get<0>(results[0])->size();

        SSBM_AFTER_QUERY(i, szResult);

        if (CONFIG.PRINT_RESULT && i == 0) {
            size_t sum = 0;
            auto iter1 = std::get<0>(results[0])->begin();
            auto iter2 = std::get<1>(results[0])->begin();
            auto iter3 = std::get<2>(results[0])->begin();
            auto iter4 = std::get<3>(results[0])->begin();
            auto iter5 = std::get<4>(results[0])->begin();
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
        delete std::get<0>(results[0]);
        delete std::get<0>(results[1]);
        delete std::get<1>(results[0]);
        delete std::get<1>(results[1]);
        delete std::get<2>(results[0]);
        delete std::get<2>(results[1]);
        delete std::get<3>(results[0]);
        delete std::get<3>(results[1]);
        delete std::get<4>(results[0]);
        delete std::get<4>(results[1]);
    }

    SSBM_AFTER_QUERIES;

    for (size_t k = 0; k < MODULARITY; ++k) {
        delete batDDs[k];
        delete batDYs[k];
        delete batLPs[k];
        delete batLSs[k];
        delete batLOs[k];
        delete batLRs[k];
        delete batPPs[k];
        delete batPCs[k];
        delete batPBs[k];
        delete batSSs[k];
        delete batSRs[k];
    }

    SSBM_FINALIZE;

    return 0;
}
