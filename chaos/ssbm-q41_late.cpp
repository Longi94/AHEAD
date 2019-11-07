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

    // p_mfgr = 'MFGR#1' or p_mfgr = 'MFGR#2'
    auto bat1 = select<equal_to, equal_to, or_is>(batPM, const_cast<str_t>("MFGR#1"), const_cast<str_t>("MFGR#2")); // OID part | p_mfgr
    auto bat2 = bat1->mirror_head();                                                                                // OID supplier | OID supplier
    delete bat1;
    auto bat3 = batPPenc->reverse();   // p_partkey | VOID part
    auto bat4 = matchjoin(bat3, bat2); // p_partkey | OID part
    delete bat2;
    delete bat3;
    // lo_partkey = p_partkey
    auto bat5 = hashjoin(batLPenc, bat4); // OID lineorder | OID supplier
    delete bat4;
    auto bat6 = bat5->mirror_head(); // OID lineorder | OID lineorder
    delete bat5;

    // s_region = 'AMERICA'
    auto bat7 = select<equal_to>(batSR, const_cast<str_t>("AMERICA")); // OID supplier | s_region
    auto bat8 = bat7->mirror_head();                                   // OID supplier | OID supplier
    delete bat7;
    auto bat9 = batSSenc->reverse();    // s_suppkey | VOID supplier
    auto bat10 = matchjoin(bat9, bat8); // s_suppkey | OID supplier
    delete bat8;
    delete bat9;
    // reduce number of l_suppkey joinpartners
    auto bat11 = matchjoin(bat6, batLSenc); // OID lineorder | lo_suppkey
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
    auto bat16 = batCCenc->reverse();     // c_custkey | VOID customer
    auto bat17 = matchjoin(bat16, bat15); // c_custkey | OID customer
    delete bat15;
    delete bat16;
    // reduce number of l_custkey joinpartners
    auto bat18 = matchjoin(bat13, batLCenc); // OID lineorder | lo_custkey
    delete bat13;
    // lo_custkey = c_custkey
    auto bat19 = hashjoin(bat18, bat17); // OID lineorder | OID customer
    delete bat18;

    // prepare grouping
    auto bat20 = bat19->mirror_head(); // OID lineorder | OID lineorder
    delete bat19;
    auto bat21 = bat20->clear_head(); // VOID | OID lineorder
    delete bat20;
    auto batARenc = fetchjoin(bat21, batLRenc);                       // VOID | lo_revenue
    auto batASenc = fetchjoin(bat21, batLSCenc);                      // VOID | lo_supplycost
    auto batAPenc = arithmetic<sub, v2_resint_t>(batARenc, batASenc); // VOID | lo_revenue - lo_supplycost !!
    delete batARenc;
    delete batASenc;
    auto bat22 = fetchjoin(bat21, batLCenc); // VOID | lo_custkey
    auto bat23 = hashjoin(bat22, bat17);     // OID | OID customer
    delete bat17;
    delete bat22;
    auto bat24 = bat23->clear_head(); // VOID | OID customer
    delete bat23;
    auto batAN = fetchjoin(bat24, batCN); // VOID | c_nation !!!
    delete bat24;
    auto bat25 = fetchjoin(bat21, batLOenc); // VOID | lo_orderdate
    delete bat21;
    auto bat29 = batDDenc->reverse();    // d_datekey | VOID date
    auto bat30 = hashjoin(bat25, bat29); // OID | OID date
    delete bat25;
    delete bat29;
    auto bat31 = bat30->clear_head(); // VOID | OID date
    delete bat30;
    auto batAYenc = fetchjoin(bat31, batDYenc); // VOID | d_year !!!
    delete bat31;

    // check and decode
    auto tupleAP = checkAndDecodeAN(batAPenc);
    CLEAR_CHECKANDDECODE_AN(tupleAP);
    auto batAP = get<0>(tupleAP);
    delete batAPenc;
    auto tupleAY = checkAndDecodeAN(batAYenc);
    CLEAR_CHECKANDDECODE_AN(tupleAY);
    auto batAY = get<0>(tupleAY);
    delete batAYenc;

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

    delete batRY;
    delete batRN;
    delete batRP;

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
