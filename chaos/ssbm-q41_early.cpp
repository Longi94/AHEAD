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
    auto batCCenc = copy(batCCcb);
    auto batCR = copy(batCRcb);
    auto batCN = copy(batCNcb);
    auto batDDenc = copy(batDDcb);
    auto batDYenc = copy(batDYcb);
    auto batLCenc = copy(batLCcb);
    auto batLSenc = copy(batLScb);
    auto batLPenc = copy(batLPcb);
    auto batLOenc = copy(batLOcb);
    auto batLRenc = copy(batLRcb);
    auto batLSCenc = copy(batLSCcb);
    auto batPPenc = copy(batPPcb);
    auto batPM = copy(batPMcb);
    auto batSSenc = copy(batSScb);
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

    // 0) Eager Check
    auto tupleCC = checkAndDecodeAN(batCCenc);
    CLEAR_CHECKANDDECODE_AN(tupleCC);
    auto batCC = get<0>(tupleCC);
    auto tupleDD = checkAndDecodeAN(batDDenc);
    CLEAR_CHECKANDDECODE_AN(tupleDD);
    auto batDD = get<0>(tupleDD);
    auto tupleDY = checkAndDecodeAN(batDYenc);
    CLEAR_CHECKANDDECODE_AN(tupleDY);
    auto batDY = get<0>(tupleDY);
    auto tupleLC = checkAndDecodeAN(batLCenc);
    CLEAR_CHECKANDDECODE_AN(tupleLC);
    auto batLC = get<0>(tupleLC);
    auto tupleLS = checkAndDecodeAN(batLSenc);
    CLEAR_CHECKANDDECODE_AN(tupleLS);
    auto batLS = get<0>(tupleLS);
    auto tupleLP = checkAndDecodeAN(batLPenc);
    CLEAR_CHECKANDDECODE_AN(tupleLP);
    auto batLP = get<0>(tupleLP);
    auto tupleLO = checkAndDecodeAN(batLOenc);
    CLEAR_CHECKANDDECODE_AN(tupleLO);
    auto batLO = get<0>(tupleLO);
    auto tupleLR = checkAndDecodeAN(batLRenc);
    CLEAR_CHECKANDDECODE_AN(tupleLR);
    auto batLR = get<0>(tupleLR);
    auto tupleLSC = checkAndDecodeAN(batLSCenc);
    CLEAR_CHECKANDDECODE_AN(tupleLSC);
    auto batLSC = get<0>(tupleLSC);
    auto tuplePP = checkAndDecodeAN(batPPenc);
    CLEAR_CHECKANDDECODE_AN(tuplePP);
    auto batPP = get<0>(tuplePP);
    auto tupleSS = checkAndDecodeAN(batSSenc);
    CLEAR_CHECKANDDECODE_AN(tupleSS);
    auto batSS = get<0>(tupleSS);

    // p_mfgr = 'MFGR#1' or p_mfgr = 'MFGR#2'
    auto bat1 = select<equal_to, equal_to, or_is>(batPM, const_cast<str_t>("MFGR#1"), const_cast<str_t>("MFGR#2")); // OID part | p_mfgr
    auto bat2 = bat1->mirror_head();                                                                                // OID supplier | OID supplier
    delete bat1;
    auto bat3 = batPP->reverse();      // p_partkey | VOID part
    auto bat4 = matchjoin(bat3, bat2); // p_partkey | OID part
    delete bat2;
    delete bat3;
    // lo_partkey = p_partkey
    auto bat5 = hashjoin(batLP, bat4); // OID lineorder | OID supplier
    delete bat4;
    auto bat6 = bat5->mirror_head(); // OID lineorder | OID lineorder
    delete bat5;

    // s_region = 'AMERICA'
    auto bat7 = select<equal_to>(batSR, const_cast<str_t>("AMERICA")); // OID supplier | s_region
    auto bat8 = bat7->mirror_head();                                   // OID supplier | OID supplier
    delete bat7;
    auto bat9 = batSS->reverse();       // s_suppkey | VOID supplier
    auto bat10 = matchjoin(bat9, bat8); // s_suppkey | OID supplier
    delete bat8;
    delete bat9;
    // reduce number of l_suppkey joinpartners
    auto bat11 = matchjoin(bat6, batLS); // OID lineorder | lo_suppkey
    delete bat6;
    // lo_suppkey = s_suppkey
    auto bat12 = hashjoin(bat11, bat10); // OID lineorder | OID suppkey
    delete bat10;
    delete bat11;
    auto bat13 = bat12->mirror_head(); // OID lineorder | OID lineorder
    delete bat12;

    // c_region = 'AMERICA'
    auto bat14 = select<equal_to>(batCR, const_cast<str_t>("AMERICA")); // OID customer | c_region
    auto bat15 = bat14->mirror_head();                                  // OID customer | OID customer
    delete bat14;
    auto bat16 = batCC->reverse();        // c_custkey | VOID customer
    auto bat17 = matchjoin(bat16, bat15); // c_custkey | OID customer
    delete bat15;
    delete bat16;
    // reduce number of l_custkey joinpartners
    auto bat18 = matchjoin(bat13, batLC); // OID lineorder | lo_custkey
    delete bat13;
    // lo_custkey = c_custkey
    auto bat19 = hashjoin(bat18, bat17); // OID lineorder | OID customer
    delete bat18;

    // prepare grouping
    auto bat20 = bat19->mirror_head(); // OID lineorder | OID lineorder
    delete bat19;
    auto bat21 = bat20->clear_head(); // VOID | OID lineorder
    delete bat20;
    auto batAR = fetchjoin(bat21, batLR);                 // VOID | lo_revenue
    auto batAS = fetchjoin(bat21, batLSC);                // VOID | lo_supplycost
    auto batAP = arithmetic<sub, v2_int_t>(batAR, batAS); // VOID | lo_revenue - lo_supplycost !!
    delete batAR;
    delete batAS;
    auto bat22 = fetchjoin(bat21, batLC); // VOID | lo_custkey
    auto bat23 = hashjoin(bat22, bat17);  // OID | OID customer
    delete bat17;
    delete bat22;
    auto bat24 = bat23->clear_head(); // VOID | OID customer
    delete bat23;
    auto batAN = fetchjoin(bat24, batCN); // VOID | c_nation !!!
    delete bat24;
    auto bat25 = fetchjoin(bat21, batLO); // VOID | lo_orderdate
    delete bat21;
    auto bat29 = batDD->reverse();       // d_datekey | VOID date
    auto bat30 = hashjoin(bat25, bat29); // OID | OID date
    delete bat25;
    delete bat29;
    auto bat31 = bat30->clear_head(); // VOID | OID date
    delete bat30;
    auto batAY = fetchjoin(bat31, batDY); // VOID | d_year !!!
    delete bat31;

    // delete decoded columns
    delete batCC;
    delete batDD;
    delete batDY;
    delete batLC;
    delete batLS;
    delete batLP;
    delete batLO;
    delete batLR;
    delete batLSC;
    delete batPP;
    delete batSS;

    // grouping
    auto pairGY = groupby(batAY);
    auto pairGN = groupby(batAN, get<0>(pairGY), get<1>(pairGY)->size());
    delete get<0>(pairGY);
    delete get<1>(pairGY);

    // result
    auto batRP = aggregate_sum_grouped<v2_bigint_t>(batAP, get<0>(pairGN), get<1>(pairGN)->size());
    delete batAP;
    auto batRN = fetchjoin(get<1>(pairGN), batAN);
    delete batAN;
    auto batRY = fetchjoin(get<1>(pairGN), batAY);
    delete batAY;
    delete get<0>(pairGN);
    delete get<1>(pairGN);

    auto iter1 = batRY->begin();
    auto iter2 = batRN->begin();
    auto iter3 = batRP->begin();
    cout << "d_year|c_nation|profit" << endl;
    for (; iter1->hasNext(); ++*iter1, ++*iter2, ++*iter3)
    {
        cout << iter1->tail() << "|" << iter2->tail() << "|" << iter3->tail() << endl;
    }
    delete iter1;
    delete iter2;
    delete iter3;

    delete batCCenc;
    delete batCR;
    delete batCN;
    delete batDDenc;
    delete batDYenc;
    delete batLCenc;
    delete batLSenc;
    delete batLPenc;
    delete batLOenc;
    delete batLRenc;
    delete batLSCenc;
    delete batPPenc;
    delete batPM;
    delete batSSenc;
    delete batSR;

    AHEAD::destroyInstance();

    return 0;
}
