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
    instance->loadTable("customerAN");
    instance->loadTable("supplierAN");
    instance->loadTable("partAN");
    instance->loadTable("lineorderAN");

    /* Measure loading ColumnBats */
    auto batCCcb = new resint_colbat_t("customerAN", "custkey");
    auto batCRcb = new str_colbat_t("customerAN", "region");
    auto batCNcb = new str_colbat_t("customerAN", "nation");
    auto batDDcb = new resint_colbat_t("dateAN", "datekey");
    auto batDYcb = new resshort_colbat_t("dateAN", "year");
    auto batLCcb = new resint_colbat_t("lineorderAN", "custkey");
    auto batLScb = new resint_colbat_t("lineorderAN", "suppkey");
    auto batLPcb = new resint_colbat_t("lineorderAN", "partkey");
    auto batLOcb = new resint_colbat_t("lineorderAN", "orderdate");
    auto batLRcb = new resint_colbat_t("lineorderAN", "revenue");
    auto batLSCcb = new resint_colbat_t("lineorderAN", "supplycost");
    auto batPPcb = new resint_colbat_t("partAN", "partkey");
    auto batPMcb = new str_colbat_t("partAN", "mfgr");
    auto batSScb = new resint_colbat_t("supplierAN", "suppkey");
    auto batSRcb = new str_colbat_t("supplierAN", "region");

    /* Measure converting (copying) ColumnBats to TempBats */
    auto batCC = copy(batCCcb);
    auto batCR = copy(batCRcb);
    auto batCN = copy(batCNcb);
    auto batDD = copy(batDDcb);
    auto batDY = copy(batDYcb);
    auto batLC = copy(batLCcb);
    auto batLS = copy(batLScb);
    auto batLP = copy(batLPcb);
    auto batLO = copy(batLOcb);
    auto batLR = copy(batLRcb);
    auto batLSC = copy(batLSCcb);
    auto batPP = copy(batPPcb);
    auto batPM = copy(batPMcb);
    auto batSS = copy(batSScb);
    auto batSR = copy(batSRcb);

    delete batCCcb;
    delete batCRcb;
    delete batCNcb;
    delete batDDcb;
    delete batDYcb;
    delete batLCcb;
    delete batLScb;
    delete batLPcb;
    delete batLOcb;
    delete batLRcb;
    delete batLSCcb;
    delete batPPcb;
    delete batPMcb;
    delete batSScb;
    delete batSRcb;

    // p_mfgr = 'MFGR#1' or p_mfgr = 'MFGR#2'
    auto tuple1 = selectAN<equal_to, equal_to, or_is>(batPM, const_cast<str_t>("MFGR#1"), const_cast<str_t>("MFGR#2")); // OID part | p_mfgr
    CLEAR_SELECT_AN(tuple1);
    auto bat2 = get<0>(tuple1)->mirror_head(); // OID supplier | OID supplier
    delete get<0>(tuple1);
    auto bat3 = batPP->reverse();          // p_partkey | VOID part
    auto tuple4 = matchjoinAN(bat3, bat2); // p_partkey | OID part
    delete bat2;
    delete bat3;
    CLEAR_JOIN_AN(tuple4);
    // lo_partkey = p_partkey
    auto tuple5 = hashjoinAN(batLP, get<0>(tuple4), get<v2_resoid_t::As->size() - 1>(*v2_resoid_t::As), get<v2_resoid_t::Ainvs->size() - 1>(*v2_resoid_t::Ainvs),
                             get<0>(tuple4)->tail.metaData.AN_A, get<0>(tuple4)->tail.metaData.AN_Ainv); // OID lineorder | OID supplier
    delete get<0>(tuple4);
    CLEAR_JOIN_AN(tuple5);
    auto bat6 = get<0>(tuple5)->mirror_head(); // OID lineorder | OID lineorder
    delete get<0>(tuple5);

    // s_region = 'AMERICA'
    auto tuple7 = selectAN<equal_to>(batSR, const_cast<str_t>("AMERICA")); // OID supplier | s_region
    CLEAR_SELECT_AN(tuple7);
    auto bat8 = get<0>(tuple7)->mirror_head(); // OID supplier | OID supplier
    delete get<0>(tuple7);
    auto bat9 = batSS->reverse();           // s_suppkey | VOID supplier
    auto tuple10 = matchjoinAN(bat9, bat8); // s_suppkey | OID supplier
    delete bat8;
    delete bat9;
    CLEAR_JOIN_AN(tuple10);
    // reduce number of l_suppkey joinpartners
    auto tuple11 = matchjoinAN(bat6, batLS); // OID lineorder | lo_suppkey
    delete bat6;
    CLEAR_JOIN_AN(tuple11);
    // lo_suppkey = s_suppkey
    auto tuple12 = hashjoinAN(get<0>(tuple11), get<0>(tuple10)); // OID lineorder | OID suppkey
    delete get<0>(tuple10);
    delete get<0>(tuple11);
    CLEAR_JOIN_AN(tuple12);
    auto bat13 = get<0>(tuple12)->mirror_head(); // OID lineorder | OID lineorder
    delete get<0>(tuple12);

    // c_region = 'AMERICA'
    auto tuple14 = selectAN<equal_to>(batCR, const_cast<str_t>("AMERICA")); // OID customer | c_region
    CLEAR_SELECT_AN(tuple14);
    auto bat15 = get<0>(tuple14)->mirror_head(); // OID customer | OID customer
    delete get<0>(tuple14);
    auto bat16 = batCC->reverse();            // c_custkey | VOID customer
    auto tuple17 = matchjoinAN(bat16, bat15); // c_custkey | OID customer
    delete bat15;
    delete bat16;
    CLEAR_JOIN_AN(tuple17);
    // reduce number of l_custkey joinpartners
    auto tuple18 = matchjoinAN(bat13, batLC); // OID lineorder | lo_custkey
    delete bat13;
    CLEAR_JOIN_AN(tuple18);
    // lo_custkey = c_custkey
    auto tuple19 = hashjoinAN(get<0>(tuple18), get<0>(tuple17)); // OID lineorder | OID customer
    delete get<0>(tuple18);

    // prepare grouping
    auto bat20 = get<0>(tuple19)->mirror_head(); // OID lineorder | OID lineorder
    delete get<0>(tuple19);
    auto bat21 = bat20->clear_head(); // VOID | OID lineorder
    delete bat20;
    auto tupleAR = fetchjoinAN(bat21, batLR); // VOID | lo_revenue
    CLEAR_FETCHJOIN_AN(tupleAR);
    auto tupleAS = fetchjoinAN(bat21, batLSC); // VOID | lo_supplycost
    CLEAR_FETCHJOIN_AN(tupleAS);
    auto tupleAP = arithmeticAN<sub, v2_resint_t>(get<0>(tupleAR), get<0>(tupleAS)); // VOID | lo_revenue - lo_supplycost !!
    delete get<0>(tupleAR);
    delete get<0>(tupleAS);
    CLEAR_ARITHMETIC_AN(tupleAP);
    auto tuple22 = fetchjoinAN(bat21, batLC); // VOID | lo_custkey
    CLEAR_FETCHJOIN_AN(tuple22);
    auto tuple23 = hashjoinAN(get<0>(tuple22), get<0>(tuple17), get<v2_resoid_t::As->size() - 1>(*v2_resoid_t::As), get<v2_resoid_t::Ainvs->size() - 1>(*v2_resoid_t::Ainvs)); // OID | OID customer
    delete get<0>(tuple17);
    delete get<0>(tuple22);
    CLEAR_JOIN_AN(tuple23);
    auto bat24 = get<0>(tuple23)->clear_head(); // VOID | OID customer
    delete get<0>(tuple23);
    auto tupleAN = fetchjoinAN(bat24, batCN); // VOID | c_nation !!!
    delete bat24;
    CLEAR_FETCHJOIN_AN(tupleAN);
    auto tuple25 = fetchjoinAN(bat21, batLO); // VOID | lo_orderdate
    delete bat21;
    CLEAR_FETCHJOIN_AN(tuple25);
    auto bat29 = batDD->reverse(); // d_datekey | VOID date
    auto tuple30 = hashjoinAN(get<0>(tuple25), bat29, get<v2_resoid_t::As->size() - 1>(*v2_resoid_t::As), get<v2_resoid_t::Ainvs->size() - 1>(*v2_resoid_t::Ainvs),
                              get<v2_resoid_t::As->size() - 1>(*v2_resoid_t::As), get<v2_resoid_t::Ainvs->size() - 1>(*v2_resoid_t::Ainvs)); // OID | OID date
    delete get<0>(tuple25);
    delete bat29;
    auto bat31 = get<0>(tuple30)->clear_head(); // VOID | OID date
    delete get<0>(tuple30);
    auto tupleAY = fetchjoinAN(bat31, batDY); // VOID | d_year !!!
    delete bat31;
    CLEAR_FETCHJOIN_AN(tupleAY);

    // grouping
    auto tupleGY = groupbyAN(get<0>(tupleAY));
    CLEAR_GROUPBY_UNARY_AN(tupleGY);
    auto tupleGN = groupbyAN(get<0>(tupleAN), get<0>(tupleGY), get<1>(tupleGY)->size());
    delete get<0>(tupleGY);
    delete get<1>(tupleGY);

    // result
    auto tupleRP = aggregate_sum_groupedAN<v2_resbigint_t>(get<0>(tupleAP), get<0>(tupleGN), get<1>(tupleGN)->size());
    delete get<0>(tupleAP);
    CLEAR_GROUPEDSUM_AN(tupleRP);
    auto tupleRN = fetchjoinAN(get<1>(tupleGN), get<0>(tupleAN));
    delete get<0>(tupleAN);
    CLEAR_FETCHJOIN_AN(tupleRN);
    auto tupleRY = fetchjoinAN(get<1>(tupleGN), get<0>(tupleAY));
    delete get<0>(tupleAY);
    delete get<0>(tupleGN);
    delete get<1>(tupleGN);
    CLEAR_FETCHJOIN_AN(tupleRY);

    auto iter1 = get<0>(tupleRY)->begin();
    auto iter2 = get<0>(tupleRN)->begin();
    auto iter3 = get<0>(tupleRP)->begin();
    typedef typename remove_pointer<typename decay<decltype(get<0>(tupleRP))>::type>::type::tail_t profit_tail_t;
    profit_tail_t batRPAinv = static_cast<profit_tail_t>(get<0>(tupleRP)->tail.metaData.AN_Ainv);
    typedef typename remove_pointer<typename decay<decltype(get<0>(tupleRY))>::type>::type::tail_t year_tail_t;
    year_tail_t batRYAinv = static_cast<year_tail_t>(get<0>(tupleRY)->tail.metaData.AN_Ainv);
    profit_tail_t sum = 0;
    cout << "d_year|c_nation|profit|" << endl;
    for (; iter1->hasNext(); ++*iter1, ++*iter2, ++*iter3)
    {
        sum += iter3->tail();
        cout << static_cast<year_tail_t>(iter1->tail() * batRYAinv) << "|" << iter2->tail() << "|" << static_cast<profit_tail_t>(iter3->tail() * batRPAinv) << endl;
    }
    delete iter1;
    delete iter2;
    delete iter3;

    delete get<0>(tupleRY);
    delete get<0>(tupleRN);
    delete get<0>(tupleRP);

    delete batCC;
    delete batCR;
    delete batCN;
    delete batDD;
    delete batDY;
    delete batLC;
    delete batLS;
    delete batLP;
    delete batLO;
    delete batLR;
    delete batLSC;
    delete batPP;
    delete batPM;
    delete batSS;
    delete batSR;

    AHEAD::destroyInstance();

    return 0;
}
