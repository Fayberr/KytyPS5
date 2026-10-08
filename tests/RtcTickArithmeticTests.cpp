#include "libs/libRtc.cpp"

#include <cstdio>
#include <cstdlib>

namespace Loader {
void SymbolDatabase::Add(const SymbolResolve &, uint64_t, const std::string &) {
}
namespace Timer {
double GetTimeMs() { return 0; }
} // namespace Timer
} // namespace Loader

namespace {
using namespace Libs::LibRtc::Rtc;
void Check(bool condition, const char *message) {
  if (!condition) {
    std::fprintf(stderr, "RtcTickArithmeticTests: %s\n", message);
    std::abort();
  }
}
struct Case {
  int64_t delta;
  uint64_t expected;
};

using TickFunction = int(KYTY_SYSV_ABI *)(RtcTick *, const RtcTick *, int64_t);

void Test64(TickFunction function, const Case *cases, size_t count) {
  for (size_t i = 0; i < count; ++i) {
    RtcTick source{0}, result{0};
    Check(function(&result, &source, cases[i].delta) == 0, "return code");
    Check(result.tick == cases[i].expected, "scaled tick bits");
    source.tick = UINT64_MAX;
    Check(function(&result, &source, cases[i].delta) == 0, "wrap return code");
    Check(result.tick == cases[i].expected - uint64_t{1}, "addition wraps");
    source.tick = 123;
    Check(function(&source, &source, cases[i].delta) == 0,
          "in-place return code");
    Check(source.tick == cases[i].expected + uint64_t{123}, "in-place value");
  }
  RtcTick source{42}, result{99};
  Check(function(nullptr, &source, 1) == RTC_ERROR_INVALID_POINTER,
        "null destination");
  Check(function(&result, nullptr, 1) == RTC_ERROR_INVALID_POINTER,
        "null source");
  Check(result.tick == 99, "null source leaves output intact");
}
int KYTY_SYSV_ABI AddWeeks(RtcTick *dst, const RtcTick *src, int64_t delta) {
  return RtcTickAddWeeks(dst, src, static_cast<int32_t>(delta));
}
const Case seconds[] = {
    {INT64_MIN, 0x0ull},
    {-9223372036855ll, 0x7ffffffffffc9440ull},
    {-9223372036854ll, 0x80000000000bd680ull},
    {-2ll, 0xffffffffffe17b80ull},
    {-1ll, 0xfffffffffff0bdc0ull},
    {0ll, 0x0ull},
    {1ll, 0xf4240ull},
    {2ll, 0x1e8480ull},
    {9223372036854ll, 0x7ffffffffff42980ull},
    {9223372036855ll, 0x8000000000036bc0ull},
    {9223372036854775807ll, 0xfffffffffff0bdc0ull},
};
const Case minutes[] = {
    {INT64_MIN, 0x0ull},
    {-153722867281ll, 0x7fffffffffb04900ull},
    {-153722867280ll, 0x800000000343d000ull},
    {-2ll, 0xfffffffff8d8f200ull},
    {-1ll, 0xfffffffffc6c7900ull},
    {0ll, 0x0ull},
    {1ll, 0x3938700ull},
    {2ll, 0x7270e00ull},
    {153722867280ll, 0x7ffffffffcbc3000ull},
    {153722867281ll, 0x80000000004fb700ull},
    {9223372036854775807ll, 0xfffffffffc6c7900ull},
};
const Case weeks[] = {
    {-2147483648ll, 0x978e300000000000ull},
    {-15250285ll, 0x7fffffb2e634e000ull},
    {-15250284ll, 0x8000003fb7188000ull},
    {-2ll, 0xfffffee65e38c000ull},
    {-1ll, 0xffffff732f1c6000ull},
    {0ll, 0x0ull},
    {1ll, 0x8cd0e3a000ull},
    {2ll, 0x119a1c74000ull},
    {15250284ll, 0x7fffffc048e78000ull},
    {15250285ll, 0x8000004d19cb2000ull},
    {2147483647ll, 0x6871cf732f1c6000ull},
};
} // namespace
int main() {
  Test64(RtcTickAddSeconds, seconds, std::size(seconds));
  Test64(RtcTickAddMinutes, minutes, std::size(minutes));
  Test64(AddWeeks, weeks, std::size(weeks));
  std::puts("RTC scaled tick arithmetic passed");
}
