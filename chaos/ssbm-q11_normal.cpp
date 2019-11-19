#include <AHEAD.hpp>
#include <malloc.h>

using namespace std;
using namespace ahead;
using namespace bat::ops;
using namespace simd::sse;

int main(int argc, char** argv)
{
    mallopt(M_ARENA_MAX, 1);
    mallopt(M_MMAP_MAX, 0);

    auto instance = AHEAD::createInstance(argc, argv);

    instance->loadTable("date");
    instance->loadTable("lineorder");

    /* Measure loading ColumnBats */
    auto batDYcb = new shortint_colbat_t("date", "year");
    auto batDDcb = new int_colbat_t("date", "datekey");
    auto batLQcb = new tinyint_colbat_t("lineorder", "quantity");
    auto batLDcb = new tinyint_colbat_t("lineorder", "discount");
    auto batLOcb = new int_colbat_t("lineorder", "orderdate");
    auto batLEcb = new int_colbat_t("lineorder", "extendedprice");

    /* Measure converting (copying) ColumnBats to TempBats */
    auto batDY = copy(batDYcb);
    auto batDD = copy(batDDcb);
    auto batLQ = copy(batLQcb);
    auto batLD = copy(batLDcb);
    auto batLO = copy(batLOcb);
    auto batLE = copy(batLEcb);

    delete batDYcb;
    delete batDDcb;
    delete batLQcb;
    delete batLDcb;
    delete batLOcb;
    delete batLEcb;

    // 1) select from lineorder
    auto bat1 = select<less>(batLQ, 25);// lo_quantity < 25
    auto bat2 = select<greater_equal, less_equal, and_is>(batLD, 1, 3);// lo_discount between 1 and 3
    auto bat3 = bat1->mirror_head();// prepare joined selection (select from lineorder where lo_quantity... and lo_discount)
    delete bat1;
    auto batZ = bat2->mirror_head();
    delete bat2;
    auto bat4 = matchjoin(bat3, batZ);// join selection
    delete bat3;
    delete batZ;
    auto bat5 = bat4->mirror_head();// prepare joined selection with lo_orderdate (contains positions in tail)
    delete bat4;
    auto bat6 = matchjoin(bat5, batLO);// only those lo_orderdates where lo_quantity... and lo_discount

    // 2) select from date (join inbetween to reduce the number of lines we touch in total)
    auto bat7 = select<std::equal_to>(batDY, 1993);// d_year = 1993
    auto bat8 = bat7->mirror_head();// prepare joined selection over d_year and d_datekey
    delete bat7;
    auto bat9 = matchjoin(bat8, batDD);// only those d_datekey where d_year...
    delete bat8;

    // 3) join lineorder and date
    auto batA = bat9->reverse();
    delete bat9;
    auto batB = hashjoin(bat6, batA);// only those lineorders where lo_quantity... and lo_discount... and d_year...
    delete bat6;
    delete batA;
    // batB has in the Head the positions from lineorder and in the Tail the positions from date
    auto batC = batB->mirror_head();// only those lineorder-positions where lo_quantity... and lo_discount... and d_year...
    delete batB;
    auto batD = matchjoin(batC, batLE);
    auto batE = matchjoin(bat5, batLD);
    delete bat5;
    auto batF = matchjoin(batC, batE);
    delete batC;
    delete batE;

    // result
    auto batG = aggregate_mul_sum<v2_bigint_t>(batD, batF, 0);
    delete batD;
    delete batF;
    auto iter = batG->begin();
    auto result = iter->tail();
    delete iter;
    delete batG;

    delete batDY;
    delete batDD;
    delete batLQ;
    delete batLD;
    delete batLO;
    delete batLE;

    cout << result << endl;

    AHEAD::destroyInstance();

    return 0;
}
