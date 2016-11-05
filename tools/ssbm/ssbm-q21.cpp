// Copyright (c) 2016 Till Kolditz
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

/* 
 * File:   ssbm-q21.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 6. November 2016, 22:25
 */

#include "ssbm.hpp"

int main(int argc, char** argv) {
    ssbmconf_t CONFIG = initSSBM(argc, argv);
    StopWatch::rep totalTimes[CONFIG.NUM_RUNS] = {0};
    const size_t NUM_OPS = 24;
    cstr_t OP_NAMES[NUM_OPS] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M", "N", "O", "P"};
    StopWatch::rep opTimes[NUM_OPS] = {0};
    size_t batSizes[NUM_OPS] = {0};
    size_t batConsumptions[NUM_OPS] = {0};
    bool hasTwoTypes[NUM_OPS] = {false};
    boost::typeindex::type_index headTypes[NUM_OPS];
    boost::typeindex::type_index tailTypes[NUM_OPS];
    string emptyString;
    size_t x = 0;

    cout << "SSBM Query 2.1 Normal\n=====================" << endl;

    boost::filesystem::path p(CONFIG.DB_PATH);
    if (boost::filesystem::is_regular(p)) {
        p.remove_filename();
    }
    string baseDir = p.remove_trailing_separator().generic_string();
    MetaRepositoryManager::init(baseDir.c_str());

    StopWatch sw1, sw2;

    sw1.start();
    // loadTable(baseDir, "customer");
    loadTable(baseDir, "date");
    loadTable(baseDir, "lineorder");
    loadTable(baseDir, "part");
    loadTable(baseDir, "supplier");
    sw1.stop();
    cout << "Total loading time: " << sw1 << " ns." << endl;

    cout << "\nSSBM Q2.1:\n";
    cout << "select sum(lo_revenue), d_year, p_brand\n";
    cout << "  from lineorder, part, supplier, date\n";
    cout << "  where lo_orderdate = d_datekey\n";
    cout << "    and lo_partkey = p_partkey\n";
    cout << "    and lo_suppkey = s_suppkey\n";
    cout << "    and p_category = 'MFGR#12'\n";
    cout << "    and s_region = 'AMERICA'\n";
    cout << "  group by d_year, p_brand;" << endl;

    /* Measure loading ColumnBats */
    MEASURE_OP(sw1, x, batDDcb, new int_colbat_t("date", "datekey"));
    MEASURE_OP(sw1, x, batDYcb, new shortint_colbat_t("date", "year"));
    MEASURE_OP(sw1, x, batLPcb, new int_colbat_t("lineorder", "partkey"));
    MEASURE_OP(sw1, x, batLScb, new int_colbat_t("lineorder", "suppkey"));
    MEASURE_OP(sw1, x, batLOcb, new int_colbat_t("lineorder", "orderdate"));
    MEASURE_OP(sw1, x, batLRcb, new int_colbat_t("lineorder", "revenue"));
    MEASURE_OP(sw1, x, batPPcb, new int_colbat_t("part", "partkey"));
    MEASURE_OP(sw1, x, batPCcb, new str_colbat_t("part", "category"));
    MEASURE_OP(sw1, x, batPBcb, new str_colbat_t("part", "brand"));
    MEASURE_OP(sw1, x, batSScb, new int_colbat_t("supplier", "suppkey"));
    MEASURE_OP(sw1, x, batSRcb, new str_colbat_t("supplier", "region"));

    /* Measure converting (copying) ColumnBats to TempBats */
    MEASURE_OP(sw1, x, batDD, v2::bat::ops::copy(batDDcb));
    MEASURE_OP(sw1, x, batDY, v2::bat::ops::copy(batDYcb));
    MEASURE_OP(sw1, x, batLP, v2::bat::ops::copy(batLPcb));
    MEASURE_OP(sw1, x, batLS, v2::bat::ops::copy(batLScb));
    MEASURE_OP(sw1, x, batLO, v2::bat::ops::copy(batLOcb));
    MEASURE_OP(sw1, x, batLR, v2::bat::ops::copy(batLRcb));
    MEASURE_OP(sw1, x, batPP, v2::bat::ops::copy(batPPcb));
    MEASURE_OP(sw1, x, batPC, v2::bat::ops::copy(batPCcb));
    MEASURE_OP(sw1, x, batPB, v2::bat::ops::copy(batPBcb));
    MEASURE_OP(sw1, x, batSS, v2::bat::ops::copy(batSScb));
    MEASURE_OP(sw1, x, batSR, v2::bat::ops::copy(batSRcb));
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

    COUT_HEADLINE;
    COUT_RESULT(0, x);
    cout << endl;

    for (size_t i = 0; i < CONFIG.NUM_RUNS; ++i) {
        sw1.start();
        x = 0;

        // s_region = 'AMERICA'
        MEASURE_OP(sw2, x, bat1, v2::bat::ops::select<equal_to>(batSR, "AMERICA")); // OID supplier | s_region
        MEASURE_OP(sw2, x, bat2, bat1->mirror_head()); // OID supplier | OID supplier
        delete bat1;
        MEASURE_OP(sw2, x, bat3, batSS->reverse()); // s_suppkey | OID supplier
        MEASURE_OP(sw2, x, bat4, v2::bat::ops::hashjoin(bat3, bat2)); // s_suppkey | OID supplier
        delete bat2;
        delete bat3;
        // lo_suppkey = s_suppkey
        MEASURE_OP(sw2, x, bat5, v2::bat::ops::hashjoin(batLS, bat4)); // OID lineorder | OID supplier
        delete bat4;
        // join with LO_PARTKEY to already reduce the join partners
        MEASURE_OP(sw2, x, bat6, bat5->mirror_head()); // OID lineorder | OID Lineorder
        delete bat5;
        MEASURE_OP(sw2, x, bat7, v2::bat::ops::hashjoin(bat6, batLP)); // OID lineorder | lo_partkey (where s_region = 'AMERICA')
        delete bat6;

        // p_category = 'MFGR#12'
        MEASURE_OP(sw2, x, bat8, v2::bat::ops::select<equal_to>(batPC, "MFGR#12")); // OID part | p_category
        MEASURE_OP(sw2, x, bat9, bat8->mirror_head()); // OID part | OID part
        delete bat8;
        MEASURE_OP(sw2, x, batA, batPP->reverse()); // p_partkey | OID part
        MEASURE_OP(sw2, x, batB, v2::bat::ops::hashjoin(batA, bat9)); // p_partkey | OID Part where p_category = 'MFGR#12'
        delete batA;
        delete bat9;
        MEASURE_OP(sw2, x, batC, v2::bat::ops::hashjoin(bat7, batB)); // OID lineorder | OID part (where s_region = 'AMERICA' and p_category = 'MFGR#12')
        delete bat7;
        delete batB;
        // prepare for group-by p_brand
        MEASURE_OP(sw2, x, batD, v2::bat::ops::hashjoin(batC, batPB)); // OID lineorder | p_brand (where ...)

        // lo_orderdate where s_region = 'AMERICA' and p_category = 'MFGR#12')
        MEASURE_OP(sw2, x, batE, batC->mirror_head()); // OID lineorder | OID lineorder  (where ...)
        delete batC;
        MEASURE_OP(sw2, x, batF, v2::bat::ops::hashjoin(batE, batLO)); // OID lineorder | lo_orderdate (where ...)
        MEASURE_OP(sw2, x, batG, v2::bat::ops::hashjoin(batE, batLR)); // OID lineorder | lo_revenue (where ...)
        delete batE;
        // lo_orderdate = d_datekey
        MEASURE_OP(sw2, x, batH, batDD->reverse()); // d_datekey | OID date
        MEASURE_OP(sw2, x, batI, v2::bat::ops::hashjoin(batF, batH)); // OID lineorder | OID date (where ...)
        delete batF;
        delete batH;
        // prepare for group-by d_year
        MEASURE_OP(sw2, x, batJ, v2::bat::ops::hashjoin(batI, batDY)); // OID lineorder | d_year (where ...)
        delete batI;
        MEASURE_OP_TUPLE(sw2, x, tupleK, v2::bat::ops::groupedSum<v2_bigint_t>(batG, batJ, batD));

        totalTimes[i] = sw1.stop();

        cout << "\n(" << setw(2) << i << ")\n\tresult: " << get<0>(tupleK)->size() << "\n\t  time: " << sw1 << " ns.";

        if (i == 0) {
            auto iter1 = get<0>(tupleK)->begin();
            auto iter2 = get<1>(tupleK)->begin();
            auto iter3 = get<2>(tupleK)->begin();
            auto iter4 = get<3>(tupleK)->begin();
            auto iter5 = get<4>(tupleK)->begin();
            cout << '\n';
            for (; iter1->hasNext(); ++*iter1, ++*iter2, ++*iter4) {
                iter3->position(0);
                iter5->position(0);
                cout << setw(15) << iter1->tail() << " | ";
                iter3->position(iter2->tail());
                cout << iter3->tail() << " | ";
                iter5->position(iter4->tail());
                cout << iter5->tail() << '\n';
            }
            delete iter1;
            delete iter2;
            delete iter3;
            delete iter4;
            delete iter5;
        }
        COUT_HEADLINE;
        COUT_RESULT(0, x, OP_NAMES);
    }

    cout << "\npeak RSS: " << getPeakRSS(size_enum_t::MB) << " MB.\n";
    cout << "TotalTimes:";
    for (size_t i = 0; i < CONFIG.NUM_RUNS; ++i) {
        cout << '\n' << setw(2) << i << '\t' << totalTimes[i];
    }
    cout << endl;

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

    TransactionManager::destroyInstance();

    return 0;
}