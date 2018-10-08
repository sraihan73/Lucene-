using namespace std;

#include "StringHelper.h"

namespace org::apache::lucene::util
{

int StringHelper::bytesDifference(shared_ptr<BytesRef> priorTerm,
                                  shared_ptr<BytesRef> currentTerm)
{
  int mismatch = FutureArrays::mismatch(
      priorTerm->bytes, priorTerm->offset,
      priorTerm->offset + priorTerm->length, currentTerm->bytes,
      currentTerm->offset, currentTerm->offset + currentTerm->length);
  if (mismatch < 0) {
    throw invalid_argument(L"terms out of order: priorTerm=" + priorTerm +
                           L",currentTerm=" + currentTerm);
  }
  return mismatch;
}

int StringHelper::sortKeyLength(shared_ptr<BytesRef> priorTerm,
                                shared_ptr<BytesRef> currentTerm)
{
  return bytesDifference(priorTerm, currentTerm) + 1;
}

StringHelper::StringHelper() {}

bool StringHelper::startsWith(std::deque<char> &ref,
                              shared_ptr<BytesRef> prefix)
{
  // not long enough to start with the prefix
  if (ref.size() < prefix->length) {
    return false;
  }
  return FutureArrays::equals(ref, 0, prefix->length, prefix->bytes,
                              prefix->offset, prefix->offset + prefix->length);
}

bool StringHelper::startsWith(shared_ptr<BytesRef> ref,
                              shared_ptr<BytesRef> prefix)
{
  // not long enough to start with the prefix
  if (ref->length < prefix->length) {
    return false;
  }
  return FutureArrays::equals(ref->bytes, ref->offset,
                              ref->offset + prefix->length, prefix->bytes,
                              prefix->offset, prefix->offset + prefix->length);
}

bool StringHelper::endsWith(shared_ptr<BytesRef> ref,
                            shared_ptr<BytesRef> suffix)
{
  int startAt = ref->length - suffix->length;
  // not long enough to start with the suffix
  if (startAt < 0) {
    return false;
  }
  return FutureArrays::equals(
      ref->bytes, ref->offset + startAt, ref->offset + startAt + suffix->length,
      suffix->bytes, suffix->offset, suffix->offset + suffix->length);
}

StringHelper::StaticConstructor::StaticConstructor()
{
  wstring prop = System::getProperty(L"tests.seed");
  if (prop != L"") {
    // So if there is a test failure that relied on hash
    // order, we remain reproducible based on the test seed:
    GOOD_FAST_HASH_SEED = prop.hashCode();
  } else {
    GOOD_FAST_HASH_SEED = static_cast<int>(System::currentTimeMillis());
  }
  // 128 bit unsigned mask
  std::deque<char> maskBytes128(16);
  Arrays::fill(maskBytes128, static_cast<char>(0xff));
  mask128 = make_shared<int64_t>(1, maskBytes128);

  wstring prop = System::getProperty(L"tests.seed");

  // State for xorshift128:
  int64_t x0;
  int64_t x1;

  if (prop != L"") {
    // So if there is a test failure that somehow relied on this id,
    // we remain reproducible based on the test seed:
    if (prop.length() > 8) {
      prop = prop.substr(prop.length() - 8);
    }
    // C++ TODO: Only single-argument parse and valueOf methods are converted:
    // ORIGINAL LINE: x0 = Long.parseLong(prop, 16);
    x0 = int64_t ::valueOf(prop, 16);
    x1 = x0;
  } else {
    // seed from /dev/urandom, if its available
    // C++ NOTE: The following 'try with resources' block is replaced by its C++
    // equivalent: ORIGINAL LINE: try (java.io.DataInputStream is = new
    // java.io.DataInputStream(java.nio.file.Files.newInputStream(java.nio.file.Paths.get("/dev/urandom"))))
    {
      java::io::DataInputStream is =
          java::io::DataInputStream(java::nio::file::Files::newInputStream(
              java::nio::file::Paths->get(L"/dev/urandom")));
      try {
        x0 = is.readLong();
        x1 = is.readLong();
      } catch (const runtime_error &unavailable) {
        // may not be available on this platform
        // fall back to lower quality randomness from 3 different sources:
        x0 = System::nanoTime();
        x1 = StringHelper::typeid->hashCode() << 32;

        shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
        // Properties can vary across JVM instances:
        try {
          shared_ptr<Properties> p = System::getProperties();
          for (wstring s : p->stringPropertyNames()) {
            sb->append(s);
            sb->append(p->getProperty(s));
          }
          x1 |= sb->toString().hashCode();
        } catch (const SecurityException &notallowed) {
          // getting Properties requires wildcard read-write: may not be allowed
          x1 |= StringBuilder::typeid->hashCode();
        }
      }
    }
  }

  // Use a few iterations of xorshift128 to scatter the seed
  // in case multiple Lucene instances starting up "near" the same
  // nanoTime, since we use ++ (mod 2^128) for full period cycle:
  for (int i = 0; i < 10; i++) {
    int64_t s1 = x0;
    int64_t s0 = x1;
    x0 = s0;
    s1 ^= s1 << 23; // a
    x1 = s1 ^ s0 ^
         (static_cast<int64_t>(static_cast<uint64_t>(s1) >> 17)) ^
         (static_cast<int64_t>(static_cast<uint64_t>(s0) >>
                                 26)); // b, c
  }

  // 64-bit unsigned mask
  std::deque<char> maskBytes64(8);
  Arrays::fill(maskBytes64, static_cast<char>(0xff));
  shared_ptr<int64_t> mask64 = make_shared<int64_t>(1, maskBytes64);

  // First make unsigned versions of x0, x1:
  shared_ptr<int64_t> unsignedX0 = static_cast<int64_t>(x0).and (mask64);
  shared_ptr<int64_t> unsignedX1 = static_cast<int64_t>(x1).and (mask64);

  // Concatentate bits of x0 and x1, as unsigned 128 bit integer:
  nextId = unsignedX0->shiftLeft(64).or (unsignedX1);
}

StringHelper::StaticConstructor StringHelper::staticConstructor;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") public static int
// murmurhash3_x86_32(byte[] data, int offset, int len, int seed)
int StringHelper::murmurhash3_x86_32(std::deque<char> &data, int offset,
                                     int len, int seed)
{

  constexpr int c1 = 0xcc9e2d51;
  constexpr int c2 = 0x1b873593;

  int h1 = seed;
  int roundedEnd = offset + (len & 0xfffffffc); // round down to 4 byte block

  for (int i = offset; i < roundedEnd; i += 4) {
    // little endian load order
    int k1 = (data[i] & 0xff) | ((data[i + 1] & 0xff) << 8) |
             ((data[i + 2] & 0xff) << 16) | (data[i + 3] << 24);
    k1 *= c1;
    k1 = Integer::rotateLeft(k1, 15);
    k1 *= c2;

    h1 ^= k1;
    h1 = Integer::rotateLeft(h1, 13);
    h1 = h1 * 5 + 0xe6546b64;
  }

  // tail
  int k1 = 0;

  switch (len & 0x03) {
  case 3:
    k1 = (data[roundedEnd + 2] & 0xff) << 16;
    // fallthrough
  case 2:
    k1 |= (data[roundedEnd + 1] & 0xff) << 8;
    // fallthrough
  case 1:
    k1 |= (data[roundedEnd] & 0xff);
    k1 *= c1;
    k1 = Integer::rotateLeft(k1, 15);
    k1 *= c2;
    h1 ^= k1;
  }

  // finalization
  h1 ^= len;

  // fmix(h1);
  h1 ^= static_cast<int>(static_cast<unsigned int>(h1) >> 16);
  h1 *= 0x85ebca6b;
  h1 ^= static_cast<int>(static_cast<unsigned int>(h1) >> 13);
  h1 *= 0xc2b2ae35;
  h1 ^= static_cast<int>(static_cast<unsigned int>(h1) >> 16);

  return h1;
}

int StringHelper::murmurhash3_x86_32(shared_ptr<BytesRef> bytes, int seed)
{
  return murmurhash3_x86_32(bytes->bytes, bytes->offset, bytes->length, seed);
}

shared_ptr<java::math::int64_t> StringHelper::nextId;
const shared_ptr<java::math::int64_t> StringHelper::mask128;

std::deque<char> StringHelper::randomId()
{

  // NOTE: we don't use Java's UUID.randomUUID() implementation here because:
  //
  //   * It's overkill for our usage: it tries to be cryptographically
  //     secure, whereas for this use we don't care if someone can
  //     guess the IDs.
  //
  //   * It uses SecureRandom, which on Linux can easily take a long time
  //     (I saw ~ 10 seconds just running a Lucene test) when entropy
  //     harvesting is falling behind.
  //
  //   * It loses a few (6) bits to version and variant and it's not clear
  //     what impact that has on the period, whereas the simple ++ (mod 2^128)
  //     we use here is guaranteed to have the full period.

  std::deque<char> bits;
  {
    lock_guard<mutex> lock(idLock);
    bits = nextId->toByteArray();
    nextId = nextId->add(int64_t::ONE).and (mask128);
  }

  // toByteArray() always returns a sign bit, so it may require an extra byte
  // (always zero)
  if (bits.size() > ID_LENGTH) {
    assert(bits.size() == ID_LENGTH + 1);
    assert(bits[0] == 0);
    return Arrays::copyOfRange(bits, 1, bits.size());
  } else {
    std::deque<char> result(ID_LENGTH);
    System::arraycopy(bits, 0, result, result.size() - bits.size(),
                      bits.size());
    return result;
  }
}

wstring StringHelper::idToString(std::deque<char> &id)
{
  if (id.empty()) {
    return L"(null)";
  } else {
    shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
    // C++ TODO: There is no native C++ equivalent to 'toString':
    sb->append(
        (make_shared<int64_t>(1, id))->toString(Character::MAX_RADIX));
    if (id.size() != ID_LENGTH) {
      sb->append(L" (INVALID FORMAT)");
    }
    return sb->toString();
  }
}

shared_ptr<BytesRef> StringHelper::intsRefToBytesRef(shared_ptr<IntsRef> ints)
{
  std::deque<char> bytes(ints->length);
  for (int i = 0; i < ints->length; i++) {
    int x = ints->ints[ints->offset + i];
    if (x < 0 || x > 255) {
      throw invalid_argument(L"int at pos=" + to_wstring(i) + L" with value=" +
                             to_wstring(x) + L" is out-of-bounds for byte");
    }
    bytes[i] = static_cast<char>(x);
  }

  return make_shared<BytesRef>(bytes);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public static int compare(int count, byte[] a, int
// aOffset, byte[] b, int bOffset)
int StringHelper::compare(int count, std::deque<char> &a, int aOffset,
                          std::deque<char> &b, int bOffset)
{
  return FutureArrays::compareUnsigned(a, aOffset, aOffset + count, b, bOffset,
                                       bOffset + count);
}
} // namespace org::apache::lucene::util