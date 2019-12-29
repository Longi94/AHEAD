#ifndef TOOLS_SSBM_MACROS_HPP_
#define TOOLS_SSBM_MACROS_HPP_

#include <iostream>
#include <tuple>
#include <column_operators/ANbase.hpp>

///////////////
// SAVE_TYPE //
///////////////
#define SAVE_TYPE(BAT)                                                         \
do {                                                                           \
    ssb::headTypes.push_back(BAT->type_head());                                \
    ssb::tailTypes.push_back(BAT->type_tail());                                \
    ssb::hasTwoTypes.push_back(true);                                          \
} while (false)

/////////////////////
// CLEAR_SELECT_AN //
/////////////////////
#define CLEAR_SELECT_AN(PAIR)                                                  \
do {                                                                           \
    if (std::get<1>(PAIR)) {                                                   \
        delete std::get<1>(PAIR);                                              \
    }                                                                          \
} while(false)

///////////////////
// CLEAR_JOIN_AN //
///////////////////
#define CLEAR_JOIN_AN(TUPLE)                                                   \
do {                                                                           \
    if (std::get<1>(TUPLE)) {                                                  \
        delete std::get<1>(TUPLE);                                             \
    }                                                                          \
    if (std::get<2>(TUPLE)) {                                                  \
        delete std::get<2>(TUPLE);                                             \
    }                                                                          \
    if (std::get<3>(TUPLE)) {                                                  \
        delete std::get<3>(TUPLE);                                             \
    }                                                                          \
    if (std::get<4>(TUPLE)) {                                                  \
        delete std::get<4>(TUPLE);                                             \
    }                                                                          \
} while (false)

#define CLEAR_FETCHJOIN_AN(TUPLE)                                              \
do {                                                                           \
    if (std::get<1>(TUPLE)) {                                                  \
        delete std::get<1>(TUPLE);                                             \
    }                                                                          \
    if (std::get<2>(TUPLE)) {                                                  \
        delete std::get<2>(TUPLE);                                             \
    }                                                                          \
} while (false)

/////////////////////////////
// CLEAR_CHECKANDDECODE_AN //
/////////////////////////////
#define CLEAR_CHECKANDDECODE_AN(TUPLE)                                         \
do {                                                                           \
    if (std::get<1>(TUPLE)) {                                                  \
        delete std::get<1>(TUPLE);                                             \
    }                                                                          \
    if (std::get<2>(TUPLE)) {                                                  \
        delete std::get<2>(TUPLE);                                             \
    }                                                                          \
} while (false)

//////////////////////
// CLEAR_GROUPBY_AN //
//////////////////////
#define CLEAR_GROUPBY_UNARY_AN(TUPLE)                                          \
do {                                                                           \
    if (std::get<2>(TUPLE)) {                                                  \
        delete std::get<2>(TUPLE);                                             \
    }                                                                          \
    if (std::get<3>(TUPLE)) {                                                  \
        delete std::get<3>(TUPLE);                                             \
    }                                                                          \
} while (false)
#define CLEAR_GROUPBY_BINARY_AN(TUPLE)                                         \
do {                                                                           \
    if (std::get<2>(TUPLE)) {                                                  \
        delete std::get<2>(TUPLE);                                             \
    }                                                                          \
    if (std::get<3>(TUPLE)) {                                                  \
        delete std::get<3>(TUPLE);                                             \
    }                                                                          \
    if (std::get<4>(TUPLE)) {                                                  \
        delete std::get<4>(TUPLE);                                             \
    }                                                                          \
} while (false)

/////////////////////////
// CLEAR_GROUPEDSUM_AN //
/////////////////////////
#define CLEAR_GROUPEDSUM_AN(TUPLE)                                             \
do {                                                                           \
    if (std::get<1>(TUPLE)) {                                                  \
        delete std::get<1>(TUPLE);                                             \
    }                                                                          \
    if (std::get<2>(TUPLE)) {                                                  \
        delete std::get<2>(TUPLE);                                             \
    }                                                                          \
    if (std::get<3>(TUPLE)) {                                                  \
        delete std::get<3>(TUPLE);                                             \
    }                                                                          \
} while (false)

/////////////////////////
// CLEAR_GROUPEDSUM_AN //
/////////////////////////
#define CLEAR_ARITHMETIC_AN(TUPLE)                                             \
do {                                                                           \
    if (std::get<1>(TUPLE)) {                                                  \
        delete std::get<1>(TUPLE);                                             \
    }                                                                          \
    if (std::get<2>(TUPLE)) {                                                  \
        delete std::get<2>(TUPLE);                                             \
    }                                                                          \
    if (std::get<3>(TUPLE)) {                                                  \
        delete std::get<3>(TUPLE);                                             \
    }                                                                          \
    if (std::get<4>(TUPLE)) {                                                  \
        delete std::get<4>(TUPLE);                                             \
    }                                                                          \
} while (false)


void check_an_vector(ahead::bat::ops::AN_indicator_vector* vec, bool& detected)
{
    bool first_detect = !detected;
    detected = detected || (vec && vec->size() > 0);

    if (first_detect && detected) {
        exit(111);
    }
}

#define CHECK_PAIR_AN(PAIR, DETECTED)\
do {\
    check_an_vector(std::get<1>(PAIR), DETECTED);\
} while (false)

#define CHECK_JOIN_AN(TUPLE, DETECTED)\
do {\
    if (!DETECTED)\
    {\
        check_an_vector(std::get<1>(TUPLE), DETECTED);\
        check_an_vector(std::get<2>(TUPLE), DETECTED);\
        check_an_vector(std::get<3>(TUPLE), DETECTED);\
        check_an_vector(std::get<4>(TUPLE), DETECTED);\
    }\
} while (false)

#define CHECK_FETCHJOIN_AN(TUPLE, DETECTED)\
do {\
    if (!DETECTED)\
    {\
        check_an_vector(std::get<1>(TUPLE), DETECTED);\
        check_an_vector(std::get<2>(TUPLE), DETECTED);\
    }\
} while (false)

#define CHECK_CHECKANDDECODE_AN(TUPLE, DETECTED)\
do {\
    if (!DETECTED)\
    {\
        check_an_vector(std::get<1>(TUPLE), DETECTED);\
        check_an_vector(std::get<2>(TUPLE), DETECTED);\
    }\
} while (false)

#define CHECK_GROUPBY_UNARY_AN(TUPLE, DETECTED)\
do {\
    if (!DETECTED)\
    {\
        check_an_vector(std::get<2>(TUPLE), DETECTED);\
        check_an_vector(std::get<3>(TUPLE), DETECTED);\
    }\
} while (false)

#define CHECK_GROUPBY_BINARY_AN(TUPLE, DETECTED)\
do {\
    if (!DETECTED)\
    {\
        check_an_vector(std::get<2>(TUPLE), DETECTED);\
        check_an_vector(std::get<3>(TUPLE), DETECTED);\
        check_an_vector(std::get<4>(TUPLE), DETECTED);\
    }\
} while (false)

#define CHECK_GROUPEDSUM_AN(TUPLE, DETECTED)\
do {\
    if (!DETECTED)\
    {\
        check_an_vector(std::get<1>(TUPLE), DETECTED);\
        check_an_vector(std::get<2>(TUPLE), DETECTED);\
        check_an_vector(std::get<3>(TUPLE), DETECTED);\
    }\
} while (false)

#define CHECK_ARITHMETIC_AN(TUPLE, DETECTED)\
do {\
    if (!DETECTED)\
    {\
        check_an_vector(std::get<1>(TUPLE), DETECTED);\
        check_an_vector(std::get<2>(TUPLE), DETECTED);\
        check_an_vector(std::get<3>(TUPLE), DETECTED);\
        check_an_vector(std::get<4>(TUPLE), DETECTED);\
    }\
} while (false)

#endif /* TOOLS_SSBM_MACROS_HPP_ */
