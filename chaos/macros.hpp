#ifndef TOOLS_SSBM_MACROS_HPP_
#define TOOLS_SSBM_MACROS_HPP_

#include <tuple>

#ifdef _OPENMP
#define BEFORE_SAVE_STATS ssb::lock_for_stats();
#define AFTER_SAVE_STATS ssb::unlock_for_stats();
#else
#define BEFORE_SAVE_STATS
#define AFTER_SAVE_STATS
#endif

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

#endif /* TOOLS_SSBM_MACROS_HPP_ */
