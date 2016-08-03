/* 
 * File:   ssbm-q01.cpp
 * Author: Till Kolditz <till.kolditz@gmail.com>
 *
 * Created on 1. August 2016, 12:20
 */

#include <cstdlib>
#include <algorithm>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cassert>

#include <boost/filesystem.hpp>

#include "column_storage/TempBat.h"
#include "column_storage/ColumnBat.h"
#include "column_storage/ColumnManager.h"
#include "column_storage/TransactionManager.h"
#include "column_operators/operators.h"
#include "column_storage/types.h"
#include "query_executor/executor.h"
#include "query_executor/codeHandler.h"
#include "util/stopwatch.hpp"

using namespace std;

// define
// boost::throw_exception(std::runtime_error("Type name demangling failed"));
namespace boost {

    void throw_exception(std::exception const & e) { // user defined
        throw e;
    }
}

template<typename Head, typename Tail>
void printBat(BatIterator<Head, Tail > *iter, const char* message = nullptr, bool doDelete = true) {
    if (message) {
        cout << message << '\n';
    }
    size_t i = 0;
    while (iter->hasNext()) {
        auto p = iter->next();
        cout << i++ << ": " << p.first << " = " << p.second << '\n';
    }
    cout << flush;
    if (doDelete) {
        delete iter;
    }
}

#if not defined NDEBUG
#define PRINT_BAT(SW, PRINT) \
do {                         \
    SW.stop();               \
    PRINT;                   \
    SW.resume();             \
} while (false)
#else
#define PRINT_BAT(SW, PRINT)
#endif

#define SAVE_TYPE(I, BAT)          \
headTypes[I] = BAT->type_head();   \
tailTypes[I] = BAT->type_tail();   \
hasTwoTypes[I] = true

#define MEASURE_OP(...) VFUNC(MEASURE_OP, __VA_ARGS__)

#define MEASURE_OP7(SW, I, TYPE, VAR, OP, STORE_SIZE_OP, STORE_CONSUMPTION_OP) \
SW.start();                               \
TYPE VAR = OP;                            \
opTimes[I] = SW.stop();                   \
batSizes[I] = STORE_SIZE_OP;              \
batConsumptions[I] = STORE_CONSUMPTION_OP

#define MEASURE_OP5(SW, I, TYPE, VAR, OP)                     \
MEASURE_OP7(SW, I, TYPE, VAR, OP, 1, sizeof(TYPE));           \
headTypes[I] = boost::typeindex::type_id<TYPE>().type_info(); \
hasTwoTypes[I] = false

#define MEASURE_OP4(SW, I, BAT, OP)                                     \
MEASURE_OP7(SW, I, auto, BAT, OP, BAT->size(), BAT->consumption());     \
SAVE_TYPE(I, BAT)

int main(int argc, char** argv) {
    boost::filesystem::path p(argc == 1 ? argv[0] : argv[1]);
    if (boost::filesystem::is_regular(p)) {
        p.remove_filename();
    }
    string baseDir = p.remove_trailing_separator().generic_string();
    MetaRepositoryManager::init(baseDir.c_str());

    TransactionManager* tm = TransactionManager::getInstance();
    TransactionManager::Transaction* t;
    StopWatch sw1, sw2;
    string path;
    size_t num;

    sw1.start();
    t = tm->beginTransaction(true);
    assert(t != nullptr);
    sw2.start();
    path = baseDir;
    path += "/customerAN";
    num = t->load(path.c_str(), "customerAN");
    sw2.stop();
    cout << "File: " << path << "\n\tNumber of BUNs: " << num << "\n\tTime: " << sw2 << " ns." << endl;
    tm->endTransaction(t);

    t = tm->beginTransaction(true);
    assert(t != nullptr);
    sw2.start();
    path = baseDir;
    path += "/dateAN";
    num = t->load(path.c_str(), "dateAN");
    sw2.stop();
    cout << "File: " << path << "\n\tNumber of BUNs: " << num << "\n\tTime: " << sw2 << " ns." << endl;
    tm->endTransaction(t);

    t = tm->beginTransaction(true);
    assert(t != nullptr);
    sw2.start();
    path = baseDir;
    path += "/lineorderAN";
    num = t->load(path.c_str(), "lineorderAN");
    sw2.stop();
    cout << "File: " << path << "\n\tNumber of BUNs: " << num << "\n\tTime: " << sw2 << " ns." << endl;
    tm->endTransaction(t);

    t = tm->beginTransaction(true);
    assert(t != nullptr);
    sw2.start();
    path = baseDir;
    path += "/partAN";
    num = t->load(path.c_str(), "partAN");
    sw2.stop();
    cout << "File: " << path << "\n\tNumber of BUNs: " << num << "\n\tTime: " << sw2 << " ns." << endl;
    tm->endTransaction(t);

    t = tm->beginTransaction(true);
    assert(t != nullptr);
    sw2.start();
    path = baseDir;
    path += "/supplierAN";
    num = t->load(path.c_str(), "supplierAN");
    sw2.stop();
    cout << "File: " << path << "\n\tNumber of BUNs: " << num << "\n\tTime: " << sw2 << " ns." << endl;
    tm->endTransaction(t);
    sw1.stop();
    cout << "Total loading time: " << sw1 << " ns." << endl;

    cout << "\nselect lo_extendedprice\n  from lineorder, date\n  where lo_orderdate = d_datekey\n    and d_year = 1993\n    and lo_discount between 1 and 3\n    and lo_quantity  < 25;" << endl;

    const size_t NUM_OPS = 27;
    nanoseconds::rep opTimes[NUM_OPS];
    size_t batSizes[NUM_OPS];
    size_t batConsumptions[NUM_OPS];
    bool hasTwoTypes[NUM_OPS];
    boost::typeindex::type_index headTypes[NUM_OPS];
    boost::typeindex::type_index tailTypes[NUM_OPS];

    for (size_t i = 0; i < 1; ++i) {
        sw1.start();

        typedef ColumnBat<unsigned, resint_t> resintColType;
        typedef ColumnBat<unsigned, fxd_t> fxdColType;

        MEASURE_OP(sw2, 0, batDYenc, new resintColType("dateAN", "year"));
        MEASURE_OP(sw2, 1, batDDenc, new resintColType("dateAN", "datekey"));
        MEASURE_OP(sw2, 2, batLQenc, new resintColType("lineorderAN", "quantity"));
        MEASURE_OP(sw2, 3, batLDenc, new resintColType("lineorderAN", "discount"));
        MEASURE_OP(sw2, 4, batLOenc, new resintColType("lineorderAN", "orderdate"));
        MEASURE_OP(sw2, 5, batLE, new fxdColType("lineorderAN", "extendedprice"));

        // 0) Eager Check
        MEASURE_OP(sw2, 6, auto, batDYpair, (Bat_Operators::checkAndDecodeA<unsigned, int_t>(batDYenc, ::A_INV, ::AN_UNENC_MAX)), batDYpair.first->size(), batDYpair.first->consumption());
        auto batDY = batDYpair.first;
        SAVE_TYPE(6, batDY);
        MEASURE_OP(sw2, 7, auto, batDDpair, (Bat_Operators::checkAndDecodeA<unsigned, int_t>(batDDenc)), batDDpair.first->size(), batDDpair.first->consumption());
        auto batDD = batDDpair.first;
        SAVE_TYPE(7, batDD);
        MEASURE_OP(sw2, 8, auto, batLQpair, (Bat_Operators::checkAndDecodeA<unsigned, int_t>(batLQenc)), batLQpair.first->size(), batLQpair.first->consumption());
        auto batLQ = batLQpair.first;
        SAVE_TYPE(8, batLQ);
        MEASURE_OP(sw2, 9, auto, batLDpair, (Bat_Operators::checkAndDecodeA<unsigned, int_t>(batLDenc)), batLDpair.first->size(), batLDpair.first->consumption());
        auto batLD = batLDpair.first;
        SAVE_TYPE(9, batLD);
        MEASURE_OP(sw2, 10, auto, batLOpair, (Bat_Operators::checkAndDecodeA<unsigned, int_t>(batLOenc)), batLOpair.first->size(), batLOpair.first->consumption());
        auto batLO = batLOpair.first;
        SAVE_TYPE(10, batLO);

        // 1) selection push-down
        MEASURE_OP(sw2, 11, bat3, Bat_Operators::selection_lt(batLQ, 25)); // lo_quantity < 25
        PRINT_BAT(sw1, printBat(bat3->begin(), "lo_quantity < 25"));
        MEASURE_OP(sw2, 12, bat4, Bat_Operators::selection_bt(batLD, 1, 3)); // lo_discount between 1 and 3
        PRINT_BAT(sw1, printBat(bat4->begin(), "lo_discount between 1 and 3"));
        MEASURE_OP(sw2, 13, batDYsel, Bat_Operators::selection_eq(batDY, 1993)); // d_year = 1993
        PRINT_BAT(sw1, printBat(batDYsel->begin(), "d_year = 1993"));

        // 2) join BATs
        MEASURE_OP(sw2, 14, bat6, Bat_Operators::mirror(bat4)); // prepare joined selection (select from lineorder where lo_quantity... and lo_discount)
        MEASURE_OP(sw2, 15, bat7, Bat_Operators::col_hashjoin(bat6, bat3)); // join selection
        MEASURE_OP(sw2, 16, bat8, Bat_Operators::mirror(bat7)); // prepare joined selection with lo_orderdate (contains positions in tail)
        PRINT_BAT(sw1, printBat(bat8->begin(), "lineorder where lo_quantity < 25 and lo_discoutn between 1 and 3"));
        MEASURE_OP(sw2, 17, batLOsel, Bat_Operators::col_hashjoin(bat8, batLO)); // only those lo_orderdates where lo_quantity... and lo_discount

        // 3) join lineorder and date
        PRINT_BAT(sw1, printBat(bat8->begin(), "lo_orderdates where lo_quantity < 25 and lo_discount between 1 and 3"));
        MEASURE_OP(sw2, 18, batDYsel2, Bat_Operators::mirror(batDYsel)); // prepare joined selection over d_year and d_datekey
        MEASURE_OP(sw2, 19, batDDsel, Bat_Operators::col_hashjoin(batDYsel2, batDD)); // only those d_datekey where d_year...
        PRINT_BAT(sw1, printBat(batDDsel->begin(), "d_datekey where d_year = 1993"));
        MEASURE_OP(sw2, 20, batDDselRev, Bat_Operators::reverse(batDDsel));
        MEASURE_OP(sw2, 21, batE, Bat_Operators::col_hashjoin(batLOsel, batDDselRev)); // only those lineorders where lo_quantity... and lo_discount... and d_year...
        // batE now has in the Head the positions from lineorder and in the Tail the positions from date
        MEASURE_OP(sw2, 22, batF, Bat_Operators::mirror(batE)); // only those lineorder-positions where lo_quantity... and lo_discount... and d_year...
        MEASURE_OP(sw2, 23, unsigned, count, batF->size());
        // BatF only contains the 
        MEASURE_OP(sw2, 24, bat11, Bat_Operators::col_hashjoin(batF, batLE));
        PRINT_BAT(sw1, printBat(bat11->begin(), "lo_extprice where d_year = 1993 and lo_discount between 1 and 3 and lo_quantity < 25"));
        MEASURE_OP(sw2, 25, bat12, Bat_Operators::col_hashjoin(batF, batLD));
        MEASURE_OP(sw2, 26, double, result, Bat_Operators::aggregate_mul_sum(bat11, bat12, 0.0));

        sw2.start();
        delete batDYenc;
        delete batDDenc;
        delete batLQenc;
        delete batLDenc;
        delete batLOenc;
        delete batDY;
        delete batDYpair.second;
        delete batDD;
        delete batDDpair.second;
        delete batLQ;
        delete batLQpair.second;
        delete batLD;
        delete batLDpair.second;
        delete batLO;
        delete batLOpair.second;
        delete batLE;
        delete bat3;
        delete bat4;
        delete batDYsel;
        delete bat6;
        delete bat7;
        delete bat8;
        delete batLOsel;
        delete batDYsel2;
        delete batDDsel;
        delete batDDselRev;
        delete batE;
        delete batF;
        delete bat11;
        delete bat12;
        sw2.stop();

        sw1.stop();

        const size_t LEN_TYPES = 13;
        cout << "SSBM Q1.1 (" << setw(2) << i << ")\n\tresult: " << result << "\n\t count: " << count << "\n\t  time: " << setw(13) << sw1 << " ns.\n\tdelete: " << setw(13) << sw2 << "\n\tq-only: " << hrc_duration(sw1.duration() - sw2.duration());
        cout << "\n\t  name\t" << setw(13) << "time [ns]\t" << setw(10) << "size [#]\t" << setw(10) << "consum [B]\t" << setw(LEN_TYPES) << "type head\t" << setw(LEN_TYPES) << "type tail";
        string emptyString;
        for (size_t i = 0; i < NUM_OPS; ++i) {
            cout << "\n\t  op" << setw(2) << i << "\t" << setw(13) << hrc_duration(opTimes[i]) << "\t" << setw(10) << batSizes[i] << "\t" << setw(10) << batConsumptions[i] << "\t" << setw(LEN_TYPES) << headTypes[i].pretty_name() << "\t" << setw(LEN_TYPES) << (hasTwoTypes[i] ? tailTypes[i].pretty_name() : emptyString);
        }
        cout << endl;
    }

    return 0;
}
