using namespace std;

#include "MurmurHash2.h"
#include "../../../../../../../../core/src/java/org/apache/lucene/util/BytesRef.h"

namespace org::apache::lucene::codecs::bloom
{
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<MurmurHash2> MurmurHash2::INSTANCE =
    make_shared<MurmurHash2>();

MurmurHash2::MurmurHash2() {}

int MurmurHash2::hash(std::deque<char> &data, int seed, int offset, int len)
{
  int m = 0x5bd1e995;
  int r = 24;
  int h = seed ^ len;
  int len_4 = len >> 2;
  for (int i = 0; i < len_4; i++) {
    int i_4 = offset + (i << 2);
    int k = data[i_4 + 3];
    k = k << 8;
    k = k | (data[i_4 + 2] & 0xff);
    k = k << 8;
    k = k | (data[i_4 + 1] & 0xff);
    k = k << 8;
    k = k | (data[i_4 + 0] & 0xff);
    k *= m;
    k ^= static_cast<int>(static_cast<unsigned int>(k) >> r);
    k *= m;
    h *= m;
    h ^= k;
  }
  int len_m = len_4 << 2;
  int left = len - len_m;
  if (left != 0) {
    if (left >= 3) {
      h ^= data[offset + len - 3] << 16;
    }
    if (left >= 2) {
      h ^= data[offset + len - 2] << 8;
    }
    if (left >= 1) {
      h ^= data[offset + len - 1];
    }
    h *= m;
  }
  h ^= static_cast<int>(static_cast<unsigned int>(h) >> 13);
  h *= m;
  h ^= static_cast<int>(static_cast<unsigned int>(h) >> 15);
  return h;
}

int MurmurHash2::hash32(std::deque<char> &data, int offset, int len)
{
  return MurmurHash2::hash(data, 0x9747b28c, offset, len);
}

int MurmurHash2::hash(shared_ptr<BytesRef> br)
{
  return hash32(br->bytes, br->offset, br->length);
}

wstring MurmurHash2::toString() { return getClass().getSimpleName(); }
} // namespace org::apache::lucene::codecs::bloom