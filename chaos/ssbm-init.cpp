#include <AHEAD.hpp>
#include "macros.hpp"

using namespace std;
using namespace ahead;
using namespace bat::ops;
using namespace simd::sse;

int main(int argc, char **argv)
{
    auto instance = AHEAD::createInstance(argc, argv);

    vector<string> tableNames = {"customer", "date", "lineorder", "part", "supplier", "customerAN", "dateAN", "lineorderAN", "partAN", "supplierAN"};

    for (auto &tableName : tableNames)
    {
        instance->loadTable(tableName);
        if (tableName.compare("customer") == 0)
        {
            // custkey|name|address|city|nation|region|phone|mktsegment
            // INTEGER|STRING:25|STRING:25|STRING:10|STRING:15|STRING:12|STRING:15|STRING:10
            auto batC1 = new int_colbat_t("customer", "custkey");
            auto batC2 = new str_colbat_t("customer", "name");
            auto batC3 = new str_colbat_t("customer", "address");
            auto batC4 = new str_colbat_t("customer", "city");
            auto batC5 = new str_colbat_t("customer", "nation");
            auto batC6 = new str_colbat_t("customer", "region");
            auto batC7 = new str_colbat_t("customer", "phone");
            auto batC8 = new str_colbat_t("customer", "mktsegment");
        }
        else if (tableName.compare("date") == 0)
        {
            // datekey|date|dayofweek|month|year|yearmonthnum|yearmonth|daynuminweek|daynuminmonth|daynuminyear|monthnuminyear|weeknuminyear|sellingseason|lastdayinweekfl|lastdayinmonthfl|holidayfl|weekdayfl
            // INTEGER|STRING:18|STRING:9|STRING:9|SHORTINT|INTEGER|STRING:7|TINYINT|TINYINT|SHORTINT|TINYINT|TINYINT|STRING:12|CHAR|CHAR|CHAR|CHAR
            auto batD1 = new int_colbat_t("date", "datekey");
            auto batD2 = new str_colbat_t("date", "date");
            auto batD3 = new str_colbat_t("date", "dayofweek");
            auto batD4 = new str_colbat_t("date", "month");
            auto batD5 = new shortint_colbat_t("date", "year");
            auto batD6 = new int_colbat_t("date", "yearmonthnum");
            auto batD7 = new str_colbat_t("date", "yearmonth");
            auto batD8 = new tinyint_colbat_t("date", "daynuminweek");
            auto batD9 = new tinyint_colbat_t("date", "daynuminmonth");
            auto batDA = new shortint_colbat_t("date", "daynuminyear");
            auto batDB = new tinyint_colbat_t("date", "monthnuminyear");
            auto batDC = new tinyint_colbat_t("date", "weeknuminyear");
            auto batDD = new str_colbat_t("date", "sellingseason");
            auto batDE = new char_colbat_t("date", "lastdayinweekfl");
            auto batDF = new char_colbat_t("date", "lastdayinmonthfl");
            auto batDG = new char_colbat_t("date", "holidayfl");
            auto batDH = new char_colbat_t("date", "weekdayfl");
        }
        else if (tableName.compare("lineorder") == 0)
        {
            // orderkey|linenumber|custkey|partkey|suppkey|orderdate|orderpriority|shippriority|quantity|extendedprice|ordertotalprice|discount|revenue|supplycost|tax|commitdate|shipmode
            // INTEGER|TINYINT|INTEGER|INTEGER|INTEGER|INTEGER|STRING:15|CHAR|TINYINT|INTEGER|INTEGER|TINYINT|INTEGER|INTEGER|TINYINT|INTEGER|STRING:10
            auto batL1 = new int_colbat_t("lineorder", "orderkey");
            auto batL2 = new tinyint_colbat_t("lineorder", "linenumber");
            auto batL3 = new int_colbat_t("lineorder", "custkey");
            auto batL4 = new int_colbat_t("lineorder", "partkey");
            auto batL5 = new int_colbat_t("lineorder", "suppkey");
            auto batL6 = new int_colbat_t("lineorder", "orderdate");
            auto batL7 = new str_colbat_t("lineorder", "orderpriority");
            auto batL8 = new char_colbat_t("lineorder", "shippriority");
            auto batL9 = new tinyint_colbat_t("lineorder", "quantity");
            auto batLA = new int_colbat_t("lineorder", "extendedprice");
            auto batLB = new int_colbat_t("lineorder", "ordertotalprice");
            auto batLC = new tinyint_colbat_t("lineorder", "discount");
            auto batLD = new int_colbat_t("lineorder", "revenue");
            auto batLE = new int_colbat_t("lineorder", "supplycost");
            auto batLF = new tinyint_colbat_t("lineorder", "tax");
            auto batLG = new int_colbat_t("lineorder", "commitdate");
            auto batLH = new str_colbat_t("lineorder", "shipmode");
        }
        else if (tableName.compare("part") == 0)
        {
            // partkey|name|mfgr|category|brand|color|type|size|container
            // INTEGER|STRING:22|STRING:6|STRING:7|STRING:9|STRING:11|STRING:25|TINYINT|STRING:10
            auto batP1 = new int_colbat_t("part", "partkey");
            auto batP2 = new str_colbat_t("part", "name");
            auto batP3 = new str_colbat_t("part", "mfgr");
            auto batP4 = new str_colbat_t("part", "category");
            auto batP5 = new str_colbat_t("part", "brand");
            auto batP6 = new str_colbat_t("part", "color");
            auto batP7 = new str_colbat_t("part", "type");
            auto batP8 = new tinyint_colbat_t("part", "size");
            auto batP9 = new str_colbat_t("part", "container");
        }
        else if (tableName.compare("supplier") == 0)
        {
            // suppkey|name|address|city|nation|region|phone
            // INTEGER|STRING:25|STRING:25|STRING:10|STRING:15|STRING:12|STRING:15
            auto batS1 = new int_colbat_t("supplier", "suppkey");
            auto batS2 = new str_colbat_t("supplier", "name");
            auto batS3 = new str_colbat_t("supplier", "address");
            auto batS4 = new str_colbat_t("supplier", "city");
            auto batS5 = new str_colbat_t("supplier", "nation");
            auto batS6 = new str_colbat_t("supplier", "region");
            auto batS7 = new str_colbat_t("supplier", "phone");
        }
        else if (tableName.compare("customerAN") == 0)
        {
            // custkey|name|address|city|nation|region|phone|mktsegment
            // RESINT|STRING:25|STRING:25|STRING:10|STRING:15|STRING:12|STRING:15|STRING:10
            auto batCA1 = new resint_colbat_t("customerAN", "custkey");
            auto batCA2 = new str_colbat_t("customerAN", "name");
            auto batCA3 = new str_colbat_t("customerAN", "address");
            auto batCA4 = new str_colbat_t("customerAN", "city");
            auto batCA5 = new str_colbat_t("customerAN", "nation");
            auto batCA6 = new str_colbat_t("customerAN", "region");
            auto batCA7 = new str_colbat_t("customerAN", "phone");
            auto batCA8 = new str_colbat_t("customerAN", "mktsegment");
        }
        else if (tableName.compare("dateAN") == 0)
        {
            // datekey|date|dayofweek|month|year|yearmonthnum|yearmonth|daynuminweek|daynuminmonth|daynuminyear|monthnuminyear|weeknuminyear|sellingseason|lastdayinweekfl|lastdayinmonthfl|holidayfl|weekdayfl
            // RESINT|STRING:18|STRING:9|STRING:9|RESSHORT|RESINT|STRING:7|RESTINY|RESTINY|RESSHORT|RESTINY|RESTINY|STRING:12|CHAR|CHAR|CHAR|CHAR
            auto batDA1 = new resint_colbat_t("dateAN", "datekey");
            auto batDA2 = new str_colbat_t("dateAN", "date");
            auto batDA3 = new str_colbat_t("dateAN", "dayofweek");
            auto batDA4 = new str_colbat_t("dateAN", "month");
            auto batDA5 = new resshort_colbat_t("dateAN", "year");
            auto batDA6 = new resint_colbat_t("dateAN", "yearmonthnum");
            auto batDA7 = new str_colbat_t("dateAN", "yearmonth");
            auto batDA8 = new restiny_colbat_t("dateAN", "daynuminweek");
            auto batDA9 = new restiny_colbat_t("dateAN", "daynuminmonth");
            auto batDAA = new resshort_colbat_t("dateAN", "daynuminyear");
            auto batDAB = new restiny_colbat_t("dateAN", "monthnuminyear");
            auto batDAC = new restiny_colbat_t("dateAN", "weeknuminyear");
            auto batDAD = new str_colbat_t("dateAN", "sellingseason");
            auto batDAE = new char_colbat_t("dateAN", "lastdayinweekfl");
            auto batDAF = new char_colbat_t("dateAN", "lastdayinmonthfl");
            auto batDAG = new char_colbat_t("dateAN", "holidayfl");
            auto batDAH = new char_colbat_t("dateAN", "weekdayfl");
        }
        else if (tableName.compare("lineorderAN") == 0)
        {
            // orderkey|linenumber|custkey|partkey|suppkey|orderdate|orderpriority|shippriority|quantity|extendedprice|ordertotalprice|discount|revenue|supplycost|tax|commitdate|shipmode
            // INTEGER|TINYINT|INTEGER|INTEGER|INTEGER|INTEGER|STRING:15|CHAR|TINYINT|INTEGER|INTEGER|TINYINT|INTEGER|INTEGER|TINYINT|INTEGER|STRING:10
            auto batLA1 = new resint_colbat_t("lineorderAN", "orderkey");
            auto batLA2 = new restiny_colbat_t("lineorderAN", "linenumber");
            auto batLA3 = new resint_colbat_t("lineorderAN", "custkey");
            auto batLA4 = new resint_colbat_t("lineorderAN", "partkey");
            auto batLA5 = new resint_colbat_t("lineorderAN", "suppkey");
            auto batLA6 = new resint_colbat_t("lineorderAN", "orderdate");
            auto batLA7 = new str_colbat_t("lineorderAN", "orderpriority");
            auto batLA8 = new char_colbat_t("lineorderAN", "shippriority");
            auto batLA9 = new restiny_colbat_t("lineorderAN", "quantity");
            auto batLAA = new resint_colbat_t("lineorderAN", "extendedprice");
            auto batLAB = new resint_colbat_t("lineorderAN", "ordertotalprice");
            auto batLAC = new restiny_colbat_t("lineorderAN", "discount");
            auto batLAD = new resint_colbat_t("lineorderAN", "revenue");
            auto batLAE = new resint_colbat_t("lineorderAN", "supplycost");
            auto batLAF = new restiny_colbat_t("lineorderAN", "tax");
            auto batLAG = new resint_colbat_t("lineorderAN", "commitdate");
            auto batLAH = new str_colbat_t("lineorderAN", "shipmode");
        }
        else if (tableName.compare("partAN") == 0)
        {
            // partkey|name|mfgr|category|brand|color|type|size|container
            // RESINT|STRING:22|STRING:6|STRING:7|STRING:9|STRING:11|STRING:25|RESTINY|STRING:10
            auto batPA1 = new resint_colbat_t("partAN", "partkey");
            auto batPA2 = new str_colbat_t("partAN", "name");
            auto batPA3 = new str_colbat_t("partAN", "mfgr");
            auto batPA4 = new str_colbat_t("partAN", "category");
            auto batPA5 = new str_colbat_t("partAN", "brand");
            auto batPA6 = new str_colbat_t("partAN", "color");
            auto batPA7 = new str_colbat_t("partAN", "type");
            auto batPA8 = new restiny_colbat_t("partAN", "size");
            auto batPA9 = new str_colbat_t("partAN", "container");
        }
        else if (tableName.compare("supplierAN") == 0)
        {
            // suppkey|name|address|city|nation|region|phone
            // RESINT|STRING:25|STRING:25|STRING:10|STRING:15|STRING:12|STRING:15
            auto batSA1 = new resint_colbat_t("supplierAN", "suppkey");
            auto batSA2 = new str_colbat_t("supplierAN", "name");
            auto batSA3 = new str_colbat_t("supplierAN", "address");
            auto batSA4 = new str_colbat_t("supplierAN", "city");
            auto batSA5 = new str_colbat_t("supplierAN", "nation");
            auto batSA6 = new str_colbat_t("supplierAN", "region");
            auto batSA7 = new str_colbat_t("supplierAN", "phone");
        }
    }

    return 0;
}
