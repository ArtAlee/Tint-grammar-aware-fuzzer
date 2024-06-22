
#ifndef SRC_TINT_FUZZERS_TINT_STRUCTURE_FUZZER_SYNTAX_H_
#define SRC_TINT_FUZZERS_TINT_STRUCTURE_FUZZER_SYNTAX_H_

#include <sstream>
#include <string_view>
#include "src/tint/fuzzers/random_generator.h"

namespace tint::fuzzers::structure_fuzzer {

void WGSLSource(std::stringstream& buffer, const uint8_t* data, size_t size);

}

#endif
