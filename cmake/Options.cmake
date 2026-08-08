include(eg_misc)

set(EG_OPTIONS
    EG_UNIT_TESTS  "Build Unit Tests"  OFF
    EG_FUZZ_TESTS  "Build Fuzz Tests"  OFF
    EG_HARDEN      "Hardening"         OFF
    EG_PROFILING   "Profiling"         OFF
    EG_COVERAGE    "Code coverage"     OFF
    EG_ASLR_OFF    "Disable ASLR"      OFF
)

eg_options(${EG_OPTIONS})
eg_options_defs(EG_OPTIONS)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
