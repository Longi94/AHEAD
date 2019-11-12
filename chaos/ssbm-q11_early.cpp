#include <column_operators/OperatorsAN.hpp>
#include <AHEAD.hpp>
#include "macros.hpp"

using namespace std;
using namespace ahead;
using namespace bat::ops;
using namespace simd::sse;

int main(int argc, char **argv)
{
    auto instance = AHEAD::createInstance(argc, argv);

    instance->loadTable("dateAN");
    instance->loadTable("lineorderAN");

    /* Measure loading ColumnBats */
    auto batDYcb = new resshort_colbat_t("dateAN", "year");
    auto batDDcb = new resint_colbat_t("dateAN", "datekey");
    auto batLQcb = new restiny_colbat_t("lineorderAN", "quantity");
    auto batLDcb = new restiny_colbat_t("lineorderAN", "discount");
    auto batLOcb = new resint_colbat_t("lineorderAN", "orderdate");
    auto batLEcb = new resint_colbat_t("lineorderAN", "extendedprice");

    /* Measure converting (copying) ColumnBats to TempBats */
    auto batDYenc = copy(batDYcb);
    auto batDDenc = copy(batDDcb);
    auto batLQenc = copy(batLQcb);
    auto batLDenc = copy(batLDcb);
    auto batLOenc = copy(batLOcb);
    auto batLEenc = copy(batLEcb);

    delete batDYcb;
    delete batDDcb;
    delete batLQcb;
    delete batLDcb;
    delete batLOcb;
    delete batLEcb;

    bool detected = false;

    // 0) Eager Check
    auto tupleDD = checkAndDecodeAN(batDDenc);
    CHECK_CHECKANDDECODE_AN(tupleDD, detected);
    CLEAR_CHECKANDDECODE_AN(tupleDD);
    auto batDD = get<0>(tupleDD);
    auto tupleDY = checkAndDecodeAN(batDYenc);
    CHECK_CHECKANDDECODE_AN(tupleDY, detected);
    CLEAR_CHECKANDDECODE_AN(tupleDY);
    auto batDY = get<0>(tupleDY);
    auto tupleLQ = checkAndDecodeAN(batLQenc);
    CHECK_CHECKANDDECODE_AN(tupleLQ, detected);
    CLEAR_CHECKANDDECODE_AN(tupleLQ);
    auto batLQ = get<0>(tupleLQ);
    auto tupleLD = checkAndDecodeAN(batLDenc);
    CHECK_CHECKANDDECODE_AN(tupleLD, detected);
    CLEAR_CHECKANDDECODE_AN(tupleLD);
    auto batLD = get<0>(tupleLD);
    auto tupleLO = checkAndDecodeAN(batLOenc);
    CHECK_CHECKANDDECODE_AN(tupleLO, detected);
    CLEAR_CHECKANDDECODE_AN(tupleLO);
    auto batLO = get<0>(tupleLO);
    auto tupleLE = checkAndDecodeAN(batLEenc);
    CHECK_CHECKANDDECODE_AN(tupleLE, detected);
    CLEAR_CHECKANDDECODE_AN(tupleLE);
    auto batLE = get<0>(tupleLE);

    // 1) select from lineorder
    auto bat1 = select<less>(batLQ, 25);                                // lo_quantity < 25
    auto bat2 = select<greater_equal, less_equal, and_is>(batLD, 1, 3); // lo_discount between 1 and 3
    auto bat3 = bat1->mirror_head();                                    // prepare joined selection (select from lineorder where lo_quantity... and lo_discount)
    delete bat1;
    auto batZ = bat2->mirror_head();
    delete bat2;
    auto bat4 = matchjoin(bat3, batZ); // join selection
    delete bat3;
    delete batZ;
    auto bat5 = bat4->mirror_head(); // prepare joined selection with lo_orderdate (contains positions in tail)
    delete bat4;
    auto bat6 = matchjoin(bat5, batLO); // only those lo_orderdates where lo_quantity... and lo_discount

    // 2) select from date (join inbetween to reduce the number of lines we touch in total)
    auto bat7 = select<equal_to>(batDY, 1993); // d_year = 1993
    auto bat8 = bat7->mirror_head();           // prepare joined selection over d_year and d_datekey
    delete bat7;
    auto bat9 = matchjoin(bat8, batDD); // only those d_datekey where d_year...
    delete bat8;

    // 3) join lineorder and date
    auto batA = bat9->reverse();
    delete bat9;
    auto batB = hashjoin(bat6, batA); // only those lineorders where lo_quantity... and lo_discount... and d_year...
    delete bat6;
    delete batA;
    // batB has in the Head the positions from lineorder and in the Tail the positions from date
    auto batC = batB->mirror_head(); // only those lineorder-positions where lo_quantity... and lo_discount... and d_year...
    delete batB;
    auto batD = matchjoin(batC, batLE);
    auto batE = matchjoin(bat5, batLD);
    delete bat5;
    auto batF = matchjoin(batC, batE);
    delete batC;
    delete batE;

    // delete decoded columns
    delete batDD;
    delete batDY;
    delete batLQ;
    delete batLD;
    delete batLO;
    delete batLE;

    // result
    auto batG = aggregate_mul_sum<v2_bigint_t>(batD, batF, 0);
    delete batD;
    delete batF;
    auto iter = batG->begin();
    auto result = iter->tail();
    delete iter;
    delete batG;

    delete batDYenc;
    delete batDDenc;
    delete batLQenc;
    delete batLDenc;
    delete batLOenc;
    delete batLEenc;

    cout << result << endl;

    AHEAD::destroyInstance();

    return 0;
}
