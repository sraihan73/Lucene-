using namespace std;

#include "UnicodeProps.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/SparseFixedBitSet.h"

namespace org::apache::lucene::analysis::util
{
using Bits = org::apache::lucene::util::Bits;
using SparseFixedBitSet = org::apache::lucene::util::SparseFixedBitSet;

UnicodeProps::UnicodeProps() {}

const wstring UnicodeProps::UNICODE_VERSION = L"10.0.0.0";
const shared_ptr<org::apache::lucene::util::Bits> UnicodeProps::WHITESPACE =
    createBits({0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x0020, 0x0085,
                0x00A0, 0x1680, 0x2000, 0x2001, 0x2002, 0x2003, 0x2004,
                0x2005, 0x2006, 0x2007, 0x2008, 0x2009, 0x200A, 0x2028,
                0x2029, 0x202F, 0x205F, 0x3000});

shared_ptr<Bits> UnicodeProps::createBits(deque<int> &codepoints)
{
  constexpr int len = codepoints[codepoints->length - 1] + 1;
  shared_ptr<SparseFixedBitSet> *const bitset =
      make_shared<SparseFixedBitSet>(len);
  for (int i : codepoints) {
    bitset->set(i);
  }
  return make_shared<BitsAnonymousInnerClass>(len, bitset);
}

UnicodeProps::BitsAnonymousInnerClass::BitsAnonymousInnerClass(
    int len, shared_ptr<SparseFixedBitSet> bitset)
{
  this->len = len;
  this->bitset = bitset;
}

bool UnicodeProps::BitsAnonymousInnerClass::get(int index)
{
  return index < len && bitset->get(index);
}

int UnicodeProps::BitsAnonymousInnerClass::length() { return 0x10FFFF + 1; }
} // namespace org::apache::lucene::analysis::util