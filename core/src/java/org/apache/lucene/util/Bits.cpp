using namespace std;

#include "Bits.h"

namespace org::apache::lucene::util
{

std::deque<std::shared_ptr<Bits>> const Bits::EMPTY_ARRAY =
    std::deque<std::shared_ptr<Bits>>(0);

MatchAllBits::MatchAllBits(int len) : len(len) {}

bool MatchAllBits::get(int index) { return true; }

int MatchAllBits::length() { return len; }

MatchNoBits::MatchNoBits(int len) : len(len) {}

bool MatchNoBits::get(int index) { return false; }

int MatchNoBits::length() { return len; }
} // namespace org::apache::lucene::util