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

    // 1) select from lineorder
    auto pair1 = selectAN<less>(batLQenc, 25ull * batLQenc->tail.metaData.AN_A, get<6>(*v2_restiny_t::As), get<6>(*v2_restiny_t::Ainvs)); // lo_quantity < 25
    CHECK_PAIR_AN(pair1, detected);
    CLEAR_SELECT_AN(pair1);
    auto pair2 = selectAN<greater_equal, less_equal, and_is>(batLDenc, 1ull * batLDenc->tail.metaData.AN_A, 3ull * batLDenc->tail.metaData.AN_A, get<5>(*v2_restiny_t::As),
                                                             get<5>(*v2_restiny_t::Ainvs)); // lo_discount between 1 and 3
    CHECK_PAIR_AN(pair2, detected);
    CLEAR_SELECT_AN(pair2);
    auto bat3 = pair1.first->mirror_head(); // prepare joined selection (select from lineorder where lo_quantity... and lo_discount)
    delete pair1.first;
    auto batZ = pair2.first->mirror_head();
    delete pair2.first;
    auto tuple4 = matchjoinAN(bat3, batZ, get<14>(*v2_resoid_t::As), get<14>(*v2_resoid_t::Ainvs), get<4>(*v2_restiny_t::As), get<4>(*v2_restiny_t::Ainvs)); // join selection
    delete bat3;
    delete batZ;
    CHECK_JOIN_AN(tuple4, detected);
    CLEAR_JOIN_AN(tuple4);
    auto bat5 = get<0>(tuple4)->mirror_head(); // prepare joined selection with lo_orderdate (contains positions in tail)
    delete get<0>(tuple4);
    auto tuple6 = matchjoinAN(bat5, batLOenc, get<13>(*v2_resoid_t::As), get<13>(*v2_resoid_t::Ainvs), get<14>(*v2_resint_t::As), get<14>(*v2_resint_t::Ainvs)); // only those lo_orderdates where lo_quantity... and lo_discount
    CHECK_JOIN_AN(tuple6, detected);
    CLEAR_JOIN_AN(tuple6);

    // 2) select from date (join inbetween to reduce the number of lines we touch in total)
    auto pair7 = selectAN<equal_to>(batDYenc, 1993ull * batDYenc->tail.metaData.AN_A, get<14>(*v2_resshort_t::As), get<14>(*v2_resshort_t::Ainvs)); // d_year = 1993                               get<5>(*v2_restiny_t::Ainvs)); // lo_discount between 1 and 3
    CHECK_PAIR_AN(pair7, detected);
    CLEAR_SELECT_AN(pair7);
    auto bat8 = pair7.first->mirror_head(); // prepare joined selection over d_year and d_datekey
    delete pair7.first;
    auto tuple9 = matchjoinAN(bat8, batDDenc, get<12>(*v2_resoid_t::As), get<12>(*v2_resoid_t::Ainvs), get<13>(*v2_resint_t::As), get<13>(*v2_resint_t::Ainvs)); // only those d_datekey where d_year...
    delete bat8;
    CHECK_JOIN_AN(tuple9, detected);
    CLEAR_JOIN_AN(tuple9);

    // 3) join lineorder and date
    auto batA = get<0>(tuple9)->reverse();
    delete get<0>(tuple9);
    auto tupleB = hashjoinAN(get<0>(tuple6), batA, get<11>(*v2_resoid_t::As), get<11>(*v2_resoid_t::Ainvs), get<10>(*v2_resoid_t::As), get<10>(*v2_resoid_t::Ainvs)); // only those lineorders where lo_quantity... and lo_discount... and d_year...
    delete get<0>(tuple6);
    delete batA;
    CHECK_JOIN_AN(tupleB, detected);
    CLEAR_JOIN_AN(tupleB);
    // batB has in the Head the positions from lineorder and in the Tail the positions from date
    auto batC = get<0>(tupleB)->mirror_head(); // only those lineorder-positions where lo_quantity... and lo_discount... and d_year...
    delete get<0>(tupleB);
    auto tupleD = matchjoinAN(batC, batLEenc, get<9>(*v2_resoid_t::As), get<9>(*v2_resoid_t::Ainvs), get<12>(*v2_resint_t::As), get<12>(*v2_resint_t::Ainvs));
    CHECK_JOIN_AN(tupleD, detected);
    CLEAR_JOIN_AN(tupleD);
    auto tupleE = matchjoinAN(bat5, batLDenc, get<8>(*v2_resoid_t::As), get<8>(*v2_resoid_t::Ainvs), get<3>(*v2_restiny_t::As), get<3>(*v2_restiny_t::Ainvs));
    delete bat5;
    CHECK_JOIN_AN(tupleE, detected);
    CLEAR_JOIN_AN(tupleE);
    auto tupleF = matchjoinAN(batC, get<0>(tupleE), get<7>(*v2_resoid_t::As), get<7>(*v2_resoid_t::Ainvs), get<2>(*v2_restiny_t::As), get<2>(*v2_restiny_t::Ainvs));
    delete batC;
    delete get<0>(tupleE);
    CHECK_JOIN_AN(tupleF, detected);
    CLEAR_JOIN_AN(tupleF);

    // 4) result
    auto tupleG = aggregate_mul_sumAN<v2_resbigint_t>(get<0>(tupleD), get<0>(tupleF));
    check_an_vector(get<1>(tupleG), detected);
    check_an_vector(get<2>(tupleG), detected);
    delete get<0>(tupleD);
    delete get<0>(tupleF);
    delete get<1>(tupleG);
    delete get<2>(tupleG);
    auto iter = get<0>(tupleG)->begin();
    auto result = iter->tail() * get<0>(tupleG)->tail.metaData.AN_Ainv;
    delete iter;
    delete get<0>(tupleG);

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
