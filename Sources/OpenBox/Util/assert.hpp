//
//  assert.hpp
//  OpenBox

#ifndef assert_hpp
#define assert_hpp

#include "OBBase.h"

namespace OB {
void precondition_failure(const char *format, ...) __cold __dead2;
void non_fatal_precondition_failure(const char *format, ...);
} /* OB */

#endif /* assert_hpp */
