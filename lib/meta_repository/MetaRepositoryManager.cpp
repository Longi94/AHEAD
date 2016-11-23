#include <cstring>

#include <meta_repository/MetaRepositoryManager.h>
#include <column_operators/Operators.hpp>
#include <util/resilience.hpp>

MetaRepositoryManager* MetaRepositoryManager::instance = nullptr;
char* MetaRepositoryManager::strBaseDir = nullptr;

auto PATH_DATABASE = "/database";
auto PATH_INFORMATION_SCHEMA = "/database/INFORMATION_SCHEMA";

cstr_t NAME_TINYINT = "TINYINT";
cstr_t NAME_SHORTINT = "SHORTINT";
cstr_t NAME_INTEGER = "INTEGER";
cstr_t NAME_LARGEINT = "LARGEINT";
cstr_t NAME_STRING = "STRING";
cstr_t NAME_FIXED = "FIXED";
cstr_t NAME_CHAR = "CHAR";
cstr_t NAME_RESTINY = "RESTINY";
cstr_t NAME_RESSHORT = "RESSHORT";
cstr_t NAME_RESINT = "RESINT";

const size_t MAXLEN_STRING = 64;

MetaRepositoryManager::MetaRepositoryManager() : pk_table_id(nullptr), table_name(nullptr), pk_attribute_id(nullptr), attribute_name(nullptr), fk_table_id(nullptr), fk_type_id(nullptr), BAT_number(nullptr), pk_layout_id(nullptr), layout_name(nullptr), size(nullptr), pk_operator_id(nullptr), operator_name(nullptr), pk_datatype_id(nullptr), datatype_name(nullptr), datatype_length(nullptr), datatype_category(nullptr), META_PATH(nullptr) {
    // creates the whole repository
    this->createRepository();
    this->createDefaultDataTypes();

    // test table
    //    table_name->append(make_pair(0, "tables"));
    // pk_table_id->append(make_pair(0, 1));
}

MetaRepositoryManager::MetaRepositoryManager(const MetaRepositoryManager &copy) : pk_table_id(copy.pk_table_id), table_name(copy.table_name), pk_attribute_id(copy.pk_attribute_id), attribute_name(copy.attribute_name), fk_table_id(copy.fk_table_id), fk_type_id(copy.fk_type_id), BAT_number(copy.BAT_number), pk_layout_id(copy.pk_layout_id), layout_name(copy.layout_name), size(copy.size), pk_operator_id(copy.pk_operator_id), operator_name(copy.operator_name), pk_datatype_id(copy.pk_datatype_id), datatype_name(copy.datatype_name), datatype_length(copy.datatype_length), datatype_category(copy.datatype_category), META_PATH(copy.META_PATH) {
}

MetaRepositoryManager::~MetaRepositoryManager() {
    delete pk_table_id;
    delete table_name;
    delete pk_attribute_id;
    delete attribute_name;
    delete fk_table_id;
    delete fk_type_id;
    delete BAT_number;
    delete pk_layout_id;
    delete layout_name;
    delete size;
    delete pk_operator_id;
    delete operator_name;
    delete pk_datatype_id;
    delete datatype_name;
    delete datatype_length;
    delete datatype_category;
}

MetaRepositoryManager* MetaRepositoryManager::getInstance() {
    if (MetaRepositoryManager::instance == 0) {
        MetaRepositoryManager::instance = new MetaRepositoryManager();
    }

    return MetaRepositoryManager::instance;
}

void MetaRepositoryManager::destroyInstance() {
    if (MetaRepositoryManager::instance) {
        delete MetaRepositoryManager::instance;
        MetaRepositoryManager::instance = nullptr;
    }
}

void MetaRepositoryManager::init(cstr_t strBaseDir) {
    if (MetaRepositoryManager::strBaseDir == nullptr) {
        size_t len = strlen(strBaseDir);
        MetaRepositoryManager::strBaseDir = new char[len + 1];
        memcpy(MetaRepositoryManager::strBaseDir, strBaseDir, len + 1); // includes NULL character

        getInstance(); // make sure that an instance exists

        // size_t len2 = strlen(PATH_DATABASE);
        // instance->TEST_DATABASE_PATH = new char[len + len2 + 1];
        // memcpy(instance->TEST_DATABASE_PATH, strBaseDir, len); // excludes NULL character
        // memcpy(instance->TEST_DATABASE_PATH + len, PATH_DATABASE, len2 + 1); // includes NULL character

        size_t len3 = strlen(PATH_INFORMATION_SCHEMA);
        instance->META_PATH = new char[len + len3 + 1];
        memcpy(instance->META_PATH, strBaseDir, len); // excludes NULL character
        memcpy(instance->META_PATH + len, PATH_INFORMATION_SCHEMA, len3 + 1); // includes NULL character
    }
}

void MetaRepositoryManager::createDefaultDataTypes() {
    pk_datatype_id->append(make_pair(0, 1));
    pk_datatype_id->append(make_pair(1, 2));
    pk_datatype_id->append(make_pair(2, 3));
    pk_datatype_id->append(make_pair(3, 4));
    pk_datatype_id->append(make_pair(4, 5));
    pk_datatype_id->append(make_pair(5, 6));
    pk_datatype_id->append(make_pair(6, 7));
    pk_datatype_id->append(make_pair(7, 8));
    pk_datatype_id->append(make_pair(8, 9));
    pk_datatype_id->append(make_pair(9, 10));

    datatype_name->append(make_pair(0, const_cast<str_t> (NAME_TINYINT)));
    datatype_name->append(make_pair(1, const_cast<str_t> (NAME_SHORTINT)));
    datatype_name->append(make_pair(2, const_cast<str_t> (NAME_INTEGER)));
    datatype_name->append(make_pair(3, const_cast<str_t> (NAME_LARGEINT)));
    datatype_name->append(make_pair(4, const_cast<str_t> (NAME_STRING)));
    datatype_name->append(make_pair(5, const_cast<str_t> (NAME_FIXED)));
    datatype_name->append(make_pair(6, const_cast<str_t> (NAME_CHAR)));
    datatype_name->append(make_pair(7, const_cast<str_t> (NAME_RESTINY)));
    datatype_name->append(make_pair(8, const_cast<str_t> (NAME_RESSHORT)));
    datatype_name->append(make_pair(9, const_cast<str_t> (NAME_RESINT)));

    datatype_length->append(make_pair(0, sizeof (tinyint_t)));
    datatype_length->append(make_pair(1, sizeof (shortint_t)));
    datatype_length->append(make_pair(2, sizeof (int_t)));
    datatype_length->append(make_pair(3, sizeof (bigint_t)));
    datatype_length->append(make_pair(4, sizeof (char_t) * MAXLEN_STRING));
    datatype_length->append(make_pair(5, sizeof (fixed_t)));
    datatype_length->append(make_pair(6, sizeof (char_t)));
    datatype_length->append(make_pair(7, sizeof (restiny_t)));
    datatype_length->append(make_pair(8, sizeof (resshort_t)));
    datatype_length->append(make_pair(9, sizeof (resint_t)));

    datatype_category->append(make_pair(0, 'N'));
    datatype_category->append(make_pair(1, 'N'));
    datatype_category->append(make_pair(2, 'N'));
    datatype_category->append(make_pair(3, 'N'));
    datatype_category->append(make_pair(4, 'S'));
    datatype_category->append(make_pair(5, 'N'));
    datatype_category->append(make_pair(6, 'S'));
    datatype_category->append(make_pair(7, 'N'));
    datatype_category->append(make_pair(8, 'N'));
    datatype_category->append(make_pair(9, 'N'));
}

void MetaRepositoryManager::createRepository() {
    // create bats for table meta
    pk_table_id = new id_tmpbat_t;
    table_name = new str_tmpbat_t;

    // create bats for attribute meta
    pk_attribute_id = new id_tmpbat_t;
    attribute_name = new str_tmpbat_t;
    fk_table_id = new id_tmpbat_t;
    fk_type_id = new id_tmpbat_t;
    BAT_number = new id_tmpbat_t;

    // create bats for layout meta
    pk_layout_id = new id_tmpbat_t;
    layout_name = new str_tmpbat_t;
    size = new size_tmpbat_t;

    // create bats for operator meta
    pk_operator_id = new id_tmpbat_t;
    operator_name = new str_tmpbat_t;

    // create bats for datatype meta
    pk_datatype_id = new id_tmpbat_t;
    datatype_name = new str_tmpbat_t;
    datatype_length = new size_tmpbat_t;
    datatype_category = new char_tmpbat_t;
}

id_t MetaRepositoryManager::createTable(cstr_t name) {
    id_t newTableId = ID_INVALID;

    if (!dataAlreadyExists(table_name, name)) {
        pair<oid_t, id_t> value = getLastValue(pk_table_id);
        // value.first is the last given head id
        id_t newId = value.first + 1;

        // value.second is the last given id for a table
        newTableId = value.second + 1;

        pk_table_id->append(make_pair(newId, newTableId));
        table_name->append(make_pair(newId, const_cast<str_t> (name)));
    } else {
        // table already exists
        cerr << "Table already exist!" << endl;
    }

    return newTableId;
}

id_t MetaRepositoryManager::getBatIdOfAttribute(cstr_t nameOfTable, cstr_t attribute) {
    pair<unsigned, unsigned> batNrPair;

    int batIdForTableName = this->selectBatId(table_name, nameOfTable);
    int tableId = this->selectPKId(pk_table_id, static_cast<unsigned> (batIdForTableName));

    if (tableId != -1) {
        // auto batForTableId = v2::bat::ops::select_eq(fk_table_id, (id_t) tableId);
        auto batForTableId = v2::bat::ops::select<equal_to>(fk_table_id, (id_t) tableId);

        // first make mirror bat, because the joining algorithm will join the tail of the first bat with the head of the second bat
        // reverse will not work here, because we need the bat id, not the table id
        // auto mirrorTableIdBat = v2::bat::ops::mirrorHead(batForTableId);
        auto mirrorTableIdBat = batForTableId->mirror_head();
        delete batForTableId;
        auto attributesForTable = v2::bat::ops::hashjoin(mirrorTableIdBat, attribute_name);
        delete mirrorTableIdBat;
        int batId = this->selectBatId(attributesForTable, attribute);
        delete attributesForTable;
        auto reverseBat = BAT_number->reverse();
        batNrPair = this->unique_selection(reverseBat, (unsigned) batId);
        delete reverseBat;
    }

    return batNrPair.first;
}

void MetaRepositoryManager::createAttribute(char *name, char *datatype, unsigned BATId, unsigned table_id) {
    id_t batIdOfDataType = selectBatId(datatype_name, datatype);
    id_t dataTypeId = selectPKId(pk_datatype_id, batIdOfDataType);
    id_t newAttributeId = getLastValue(pk_attribute_id).second + 1;

    pk_attribute_id->append(newAttributeId);
    attribute_name->append(name);
    fk_table_id->append(table_id);
    fk_type_id->append(dataTypeId);
    BAT_number->append(BATId);
}

char* MetaRepositoryManager::getDataTypeForAttribute(__attribute__((unused)) char *name) {
    return nullptr;
}

MetaRepositoryManager::TablesIterator::TablesIterator() : pKeyIter(MetaRepositoryManager::instance->pk_table_id->begin()), pNameIter(MetaRepositoryManager::instance->table_name->begin()) {
}

MetaRepositoryManager::TablesIterator::TablesIterator(const TablesIterator &iter) : pKeyIter(new typename table_key_iter_t::self_t(*iter.pKeyIter)), pNameIter(new typename table_name_iter_t::self_t(*iter.pNameIter)) {
}

MetaRepositoryManager::TablesIterator::~TablesIterator() {
    delete pKeyIter;
    delete pNameIter;
}

MetaRepositoryManager::TablesIterator& MetaRepositoryManager::TablesIterator::operator=(const TablesIterator &copy) {
    new (this) TablesIterator(copy);
    return *this;
}

void MetaRepositoryManager::TablesIterator::next() {
    pKeyIter->next();
    pNameIter->next();
}

MetaRepositoryManager::TablesIterator& MetaRepositoryManager::TablesIterator::operator++() {
    next();
    return *this;
}

void MetaRepositoryManager::TablesIterator::position(oid_t index) {
    pKeyIter->position(index);
    pNameIter->position(index);
}

bool MetaRepositoryManager::TablesIterator::hasNext() {
    return pKeyIter->hasNext();
}

id_t MetaRepositoryManager::TablesIterator::head() {
    return pKeyIter->tail();
}

str_t MetaRepositoryManager::TablesIterator::tail() {
    return pNameIter->tail();
}

size_t MetaRepositoryManager::TablesIterator::size() {
    return pKeyIter->size();
}

size_t MetaRepositoryManager::TablesIterator::consumption() {
    return pKeyIter->consumption() + pNameIter->consumption();
}

