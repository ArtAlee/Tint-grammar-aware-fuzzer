
#include "src/tint/fuzzers/random_generator.h"
#include "src/tint/fuzzers/tint_common_fuzzer.h"
#include "src/tint/fuzzers/transform_builder.h"
#include "src/tint/lang/wgsl/reader/reader.h"
#include "src/tint/lang/wgsl/writer/writer.h"
#include "testing/libfuzzer/libfuzzer_exports.h"

#include "syntax.h"

namespace tint::fuzzers::structure_fuzzer {
namespace {

extern "C" int LLVMFuzzerInitialize(int* argc [[maybe_unused]], char*** argv [[maybe_unused]]) {
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size == 0) {
        return 0;
    }
    std::stringstream wgsl_ss;

    WGSLSource(wgsl_ss, data, size);
    std::string wgsl_str = wgsl_ss.str();

    const bool debug = std::getenv("TINT_STRUCTURE_FUZZER_DEBUG");

    if (debug) {
        printf("||| input=%8d | %s\n", int(size), wgsl_str.c_str());
    }

    for (OutputFormat fmt :
         {OutputFormat::kWGSL, OutputFormat::kSpv, OutputFormat::kHLSL, OutputFormat::kMSL}) {
        TransformBuilder tb(reinterpret_cast<const uint8_t*>(wgsl_str.data()), wgsl_str.size());
        tb.AddTransform<tint::ast::transform::Robustness>();

        CommonFuzzer fuzzer(InputFormat::kWGSL, fmt);
        fuzzer.SetTransformManager(tb.manager(), tb.data_map());

        fuzzer.Run(reinterpret_cast<const uint8_t*>(wgsl_str.data()), wgsl_str.size());
        if (debug && fmt == OutputFormat::kSpv) {
            if (!fuzzer.HasErrors()) {
                printf("+++ ACCEPTED\n");
            } else {
                printf("--- %s", fuzzer.Diagnostics().Str().c_str());
            }
        }
    }
    if (debug) {
        fflush(stdout);
    }
    return 0;
}
}  // namespace
}  // namespace tint::fuzzers::structure_fuzzer
