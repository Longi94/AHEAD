/* 
 * File:   ssbm-q11_encoded.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 1. August 2016, 12:20
 */

#include "ssbm.hpp"

int main(int argc, char** argv) {
    cout << "ssbm-q11_encoded\n================" << endl;

    boost::filesystem::path p(argc == 1 ? argv[0] : argv[1]);
    if (boost::filesystem::is_regular(p)) {
        p.remove_filename();
    }
    string baseDir = p.remove_trailing_separator().generic_string();
    MetaRepositoryManager::init(baseDir.c_str());

    StopWatch sw1, sw2;

    sw1.start();
    // loadTable(baseDir, "customerAN");
    loadTable(baseDir, "dateAN");
    loadTable(baseDir, "lineorderAN");
    // loadTable(baseDir, "partAN");
    // loadTable(baseDir, "supplierAN");
    sw1.stop();
    cout << "Total loading time: " << sw1 << " ns." << endl;

    cout << "\nSSBM Q1.1:\nselect sum(lo_extendedprice * lo_discount) as revenue\n  from lineorder, date\n  where lo_orderdate = d_datekey\n    and d_year = 1993\n    and lo_discount between 1 and 3\n    and lo_quantity  < 25;" << endl;

    const size_t NUM_RUNS = 10;
    StopWatch::rep totalTimes[NUM_RUNS] = {0};
    const size_t NUM_OPS = 24;
    cstr_t OP_NAMES[NUM_OPS] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "K", "L", "M", "N", "O", "P"};
    StopWatch::rep opTimes[NUM_OPS] = {0};
    size_t batSizes[NUM_OPS] = {0};
    size_t batConsumptions[NUM_OPS] = {0};
    bool hasTwoTypes[NUM_OPS] = {false};
    boost::typeindex::type_index headTypes[NUM_OPS];
    boost::typeindex::type_index tailTypes[NUM_OPS];

    const size_t LEN_TYPES = 16;
    string emptyString;
    size_t x = 0;

    /* Measure loading ColumnBats */
    MEASURE_OP(sw1, x, batDYcb, new resshort_colbat_t("dateAN", "year"));
    MEASURE_OP(sw1, x, batDDcb, new resint_colbat_t("dateAN", "datekey"));
    MEASURE_OP(sw1, x, batLQcb, new restiny_colbat_t("lineorderAN", "quantity"));
    MEASURE_OP(sw1, x, batLDcb, new restiny_colbat_t("lineorderAN", "discount"));
    MEASURE_OP(sw1, x, batLOcb, new resint_colbat_t("lineorderAN", "orderdate"));
    MEASURE_OP(sw1, x, batLEcb, new resint_colbat_t("lineorderAN", "extendedprice"));

    /* Measure converting (copying) ColumnBats to TempBats */
    MEASURE_OP(sw1, x, batDYenc, v2::bat::ops::copy(batDYcb));
    MEASURE_OP(sw1, x, batDDenc, v2::bat::ops::copy(batDDcb));
    MEASURE_OP(sw1, x, batLQenc, v2::bat::ops::copy(batLQcb));
    MEASURE_OP(sw1, x, batLDenc, v2::bat::ops::copy(batLDcb));
    MEASURE_OP(sw1, x, batLOenc, v2::bat::ops::copy(batLOcb));
    MEASURE_OP(sw1, x, batLEenc, v2::bat::ops::copy(batLEcb));
    delete batDYcb;
    delete batDDcb;
    delete batLQcb;
    delete batLDcb;
    delete batLOcb;
    delete batLEcb;

    COUT_HEADLINE;
    COUT_RESULT(0, x);
    cout << endl;

    for (size_t i = 0; i < NUM_RUNS; ++i) {
        sw1.start();
        x = 0;

        // 1) select from lineorder
        MEASURE_OP_PAIR(sw2, x, pair1, (v2::bat::ops::selectAN<less>(batLQenc, static_cast<restiny_t> (25 * v2_restiny_t::A)))); // lo_quantity < 25
        delete pair1.second;
        MEASURE_OP_PAIR(sw2, x, pair2, (v2::bat::ops::selectAN(batLDenc, static_cast<restiny_t> (1 * v2_restiny_t::A), static_cast<restiny_t> (3 * v2_restiny_t::A)))); // lo_discount between 1 and 3
        delete pair2.second;
        MEASURE_OP(sw2, x, bat3, pair1.first->mirror_head()); // prepare joined selection (select from lineorder where lo_quantity... and lo_discount)
        delete pair1.first;
        MEASURE_OP_TUPLE(sw2, x, tuple4, (v2::bat::ops::hashjoinAN(bat3, pair2.first))); // join selection
        delete bat3;
        delete pair2.first;
        if (get<1>(tuple4)) delete get<1>(tuple4);
        if (get<2>(tuple4)) delete get<2>(tuple4);
        if (get<3>(tuple4)) delete get<3>(tuple4);
        if (get<4>(tuple4)) delete get<4>(tuple4);
        MEASURE_OP(sw2, x, bat5, (get<0>(tuple4)->mirror_head())); // prepare joined selection with lo_orderdate (contains positions in tail)
        MEASURE_OP_TUPLE(sw2, x, tuple6, (v2::bat::ops::hashjoinAN(bat5, batLOenc))); // only those lo_orderdates where lo_quantity... and lo_discount
        delete bat5;
        if (get<1>(tuple6)) delete get<1>(tuple6);
        if (get<2>(tuple6)) delete get<2>(tuple6);
        if (get<3>(tuple6)) delete get<3>(tuple6);
        if (get<4>(tuple6)) delete get<4>(tuple6);

        // 1) select from date (join inbetween to reduce the number of lines we touch in total)
        MEASURE_OP_PAIR(sw2, x, pair7, (v2::bat::ops::selectAN<equal_to>(batDYenc, 1993 * v2_resshort_t::A))); // d_year = 1993
        if (pair7.second) delete pair7.second;
        MEASURE_OP(sw2, x, bat8, (pair7.first->mirror_head())); // prepare joined selection over d_year and d_datekey
        delete pair7.first;
        MEASURE_OP_TUPLE(sw2, x, tuple9, (v2::bat::ops::hashjoinAN(bat8, batDDenc))); // only those d_datekey where d_year...
        delete bat8;
        if (get<1>(tuple9)) delete get<1>(tuple9);
        if (get<2>(tuple9)) delete get<2>(tuple9);
        if (get<3>(tuple9)) delete get<3>(tuple9);
        if (get<4>(tuple9)) delete get<4>(tuple9);

        // 3) join lineorder and date
        MEASURE_OP(sw2, x, batA, (get<0>(tuple9)->reverse()));
        delete get<0>(tuple9);
        MEASURE_OP_TUPLE(sw2, x, tupleB, (v2::bat::ops::hashjoinAN(get<0>(tuple6), batA))); // only those lineorders where lo_quantity... and lo_discount... and d_year...
        delete get<0>(tuple6);
        delete batA;
        if (get<1>(tupleB)) delete get<1>(tupleB);
        if (get<2>(tupleB)) delete get<2>(tupleB);
        if (get<3>(tupleB)) delete get<3>(tupleB);
        if (get<4>(tupleB)) delete get<4>(tupleB);
        // batE now has in the Head the positions from lineorder and in the Tail the positions from date
        MEASURE_OP(sw2, x, batC, (get<0>(tupleB)->mirror_head())); // only those lineorder-positions where lo_quantity... and lo_discount... and d_year...
        delete get<0>(tupleB);
        // BatF only contains the 
        MEASURE_OP_TUPLE(sw2, x, tupleD, (v2::bat::ops::hashjoinAN(batC, batLEenc)));
        if (get<1>(tupleD)) delete get<1>(tupleD);
        if (get<2>(tupleD)) delete get<2>(tupleD);
        if (get<3>(tupleD)) delete get<3>(tupleD);
        if (get<4>(tupleD)) delete get<4>(tupleD);
        MEASURE_OP_TUPLE(sw2, x, tupleE, (v2::bat::ops::hashjoinAN(batC, get<0>(tuple4))));
        delete batC;
        delete get<0>(tuple4);
        if (get<1>(tupleE)) delete get<1>(tupleE);
        if (get<2>(tupleE)) delete get<2>(tupleE);
        if (get<3>(tupleE)) delete get<3>(tupleE);
        if (get<4>(tupleE)) delete get<4>(tupleE);
        MEASURE_OP_TUPLE(sw2, x, tupleF, (v2::bat::ops::aggregate_mul_sumAN<v2_resbigint_t>(get<0>(tupleD), get<0>(tupleE))));
        delete get<0>(tupleD);
        delete get<0>(tupleE);
        delete get<1>(tupleF);
        delete get<2>(tupleF);
        auto iter = get<0>(tupleF)->begin();
        auto result = iter->tail();
        delete iter;
        delete get<0>(tupleF);

        totalTimes[i] = sw1.stop();

        cout << "\n(" << setw(2) << i << ")\n\tresult: " << (result * v2_resbigint_t::A_INV) << " (encoded: " << result << ")\n\t  time: " << sw1 << " ns.";
        COUT_HEADLINE;
        COUT_RESULT(0, x, OP_NAMES);
    }

    cout << "\npeak RSS: " << getPeakRSS(size_enum_t::MB) << " MB.\n";
    cout << "TotalTimes:";
    for (size_t i = 0; i < NUM_RUNS; ++i) {
        cout << '\n' << setw(2) << i << '\t' << totalTimes[i];
    }
    cout << endl;

    delete batDYenc;
    delete batDDenc;
    delete batLQenc;
    delete batLDenc;
    delete batLOenc;
    delete batLEenc;

    TransactionManager::destroyInstance();

    return 0;
}