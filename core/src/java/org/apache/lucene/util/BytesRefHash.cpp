using namespace std;

#include "BytesRefHash.h"

namespace org::apache::lucene::util
{
using DirectAllocator =
    org::apache::lucene::util::ByteBlockPool::DirectAllocator;
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_MASK;
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_SHIFT;
//    import static org.apache.lucene.util.ByteBlockPool.BYTE_BLOCK_SIZE;

BytesRefHash::BytesRefHash()
    : BytesRefHash(new ByteBlockPool(new DirectAllocator()))
{
}

BytesRefHash::BytesRefHash(shared_ptr<ByteBlockPool> pool)
    : BytesRefHash(pool, DEFAULT_CAPACITY,
                   new DirectBytesStartArray(DEFAULT_CAPACITY))
{
}

BytesRefHash::BytesRefHash(shared_ptr<ByteBlockPool> pool, int capacity,
                           shared_ptr<BytesStartArray> bytesStartArray)
    : pool(pool), bytesStartArray(bytesStartArray)
{
  hashSize = capacity;
  hashHalfSize = hashSize >> 1;
  hashMask = hashSize - 1;
  ids = std::deque<int>(hashSize);
  Arrays::fill(ids, -1);
  bytesStart = bytesStartArray->init();
  bytesUsed = bytesStartArray->bytesUsed() == nullptr
                  ? Counter::newCounter()
                  : bytesStartArray->bytesUsed();
  bytesUsed->addAndGet(hashSize * Integer::BYTES);
}

int BytesRefHash::size() { return count; }

shared_ptr<BytesRef> BytesRefHash::get(int bytesID, shared_ptr<BytesRef> ref)
{
  assert((bytesStart.size() > 0, L"bytesStart is null - not initialized"));
  assert((bytesID < bytesStart.size(),
          L"bytesID exceeds byteStart len: " + bytesStart.size()));
  pool->setBytesRef(ref, bytesStart[bytesID]);
  return ref;
}

std::deque<int> BytesRefHash::compact()
{
  assert((bytesStart.size() > 0, L"bytesStart is null - not initialized"));
  int upto = 0;
  for (int i = 0; i < hashSize; i++) {
    if (ids[i] != -1) {
      if (upto < i) {
        ids[upto] = ids[i];
        ids[i] = -1;
      }
      upto++;
    }
  }

  assert(upto == count);
  lastCount = count;
  return ids;
}

std::deque<int> BytesRefHash::sort()
{
  const std::deque<int> compact = this->compact();
  make_shared<StringMSBRadixSorterAnonymousInnerClass>(shared_from_this(),
                                                       compact)
      .sort(0, count);
  return compact;
}

BytesRefHash::StringMSBRadixSorterAnonymousInnerClass::
    StringMSBRadixSorterAnonymousInnerClass(
        shared_ptr<BytesRefHash> outerInstance, deque<int> &compact)
{
  this->outerInstance = outerInstance;
  this->compact = compact;
  scratch = make_shared<BytesRef>();
}

void BytesRefHash::StringMSBRadixSorterAnonymousInnerClass::swap(int i, int j)
{
  int tmp = compact[i];
  compact[i] = compact[j];
  compact[j] = tmp;
}

shared_ptr<BytesRef>
BytesRefHash::StringMSBRadixSorterAnonymousInnerClass::get(int i)
{
  outerInstance->pool->setBytesRef(scratch,
                                   outerInstance->bytesStart[compact[i]]);
  return scratch;
}

bool BytesRefHash::equals(int id, shared_ptr<BytesRef> b)
{
  pool->setBytesRef(scratch1, bytesStart[id]);
  return scratch1->bytesEquals(b);
}

bool BytesRefHash::shrink(int targetSize)
{
  // Cannot use ArrayUtil.shrink because we require power
  // of 2:
  int newSize = hashSize;
  while (newSize >= 8 && newSize / 4 > targetSize) {
    newSize /= 2;
  }
  if (newSize != hashSize) {
    bytesUsed->addAndGet(Integer::BYTES * -(hashSize - newSize));
    hashSize = newSize;
    ids = std::deque<int>(hashSize);
    Arrays::fill(ids, -1);
    hashHalfSize = newSize / 2;
    hashMask = newSize - 1;
    return true;
  } else {
    return false;
  }
}

void BytesRefHash::clear(bool resetPool)
{
  lastCount = count;
  count = 0;
  if (resetPool) {
    pool->reset(false, false); // we don't need to 0-fill the buffers
  }
  bytesStart = bytesStartArray->clear();
  if (lastCount != -1 && shrink(lastCount)) {
    // shrink clears the hash entries
    return;
  }
  Arrays::fill(ids, -1);
}

void BytesRefHash::clear() { clear(true); }

BytesRefHash::~BytesRefHash()
{
  clear(true);
  ids.clear();
  bytesUsed->addAndGet(Integer::BYTES * -hashSize);
}

int BytesRefHash::add(shared_ptr<BytesRef> bytes)
{
  assert((bytesStart.size() > 0, L"Bytesstart is null - not initialized"));
  constexpr int length = bytes->length;
  // final position
  constexpr int hashPos = findHash(bytes);
  int e = ids[hashPos];

  if (e == -1) {
    // new entry
    constexpr int len2 = 2 + bytes->length;
    if (len2 + pool->byteUpto > BYTE_BLOCK_SIZE) {
      if (len2 > BYTE_BLOCK_SIZE) {
        throw make_shared<MaxBytesLengthExceededException>(
            L"bytes can be at most " + (BYTE_BLOCK_SIZE - 2) +
            L" in length; got " + to_wstring(bytes->length));
      }
      pool->nextBuffer();
    }
    const std::deque<char> buffer = pool->buffer;
    constexpr int bufferUpto = pool->byteUpto;
    if (count >= bytesStart.size()) {
      bytesStart = bytesStartArray->grow();
      assert((count < bytesStart.size() + 1,
              L"count: " + to_wstring(count) + L" len: "));
      +bytesStart.size();
    }
    e = count++;

    bytesStart[e] = bufferUpto + pool->byteOffset;

    // We first encode the length, followed by the
    // bytes. Length is encoded as vInt, but will consume
    // 1 or 2 bytes at most (we reject too-long terms,
    // above).
    if (length < 128) {
      // 1 byte to store length
      buffer[bufferUpto] = static_cast<char>(length);
      pool->byteUpto += length + 1;
      assert((length >= 0, L"Length must be positive: " + to_wstring(length)));
      System::arraycopy(bytes->bytes, bytes->offset, buffer, bufferUpto + 1,
                        length);
    } else {
      // 2 byte to store length
      buffer[bufferUpto] = static_cast<char>(0x80 | (length & 0x7f));
      buffer[bufferUpto + 1] = static_cast<char>((length >> 7) & 0xff);
      pool->byteUpto += length + 2;
      System::arraycopy(bytes->bytes, bytes->offset, buffer, bufferUpto + 2,
                        length);
    }
    assert(ids[hashPos] == -1);
    ids[hashPos] = e;

    if (count == hashHalfSize) {
      rehash(2 * hashSize, true);
    }
    return e;
  }
  return -(e + 1);
}

int BytesRefHash::find(shared_ptr<BytesRef> bytes)
{
  return ids[findHash(bytes)];
}

int BytesRefHash::findHash(shared_ptr<BytesRef> bytes)
{
  assert((bytesStart.size() > 0, L"bytesStart is null - not initialized"));

  int code = doHash(bytes->bytes, bytes->offset, bytes->length);

  // final position
  int hashPos = code & hashMask;
  int e = ids[hashPos];
  if (e != -1 && !equals(e, bytes)) {
    // Conflict; use linear probe to find an open slot
    // (see LUCENE-5604):
    do {
      code++;
      hashPos = code & hashMask;
      e = ids[hashPos];
    } while (e != -1 && !equals(e, bytes));
  }

  return hashPos;
}

int BytesRefHash::addByPoolOffset(int offset)
{
  assert((bytesStart.size() > 0, L"Bytesstart is null - not initialized"));
  // final position
  int code = offset;
  int hashPos = offset & hashMask;
  int e = ids[hashPos];
  if (e != -1 && bytesStart[e] != offset) {
    // Conflict; use linear probe to find an open slot
    // (see LUCENE-5604):
    do {
      code++;
      hashPos = code & hashMask;
      e = ids[hashPos];
    } while (e != -1 && bytesStart[e] != offset);
  }
  if (e == -1) {
    // new entry
    if (count >= bytesStart.size()) {
      bytesStart = bytesStartArray->grow();
      assert((count < bytesStart.size() + 1,
              L"count: " + to_wstring(count) + L" len: "));
      +bytesStart.size();
    }
    e = count++;
    bytesStart[e] = offset;
    assert(ids[hashPos] == -1);
    ids[hashPos] = e;

    if (count == hashHalfSize) {
      rehash(2 * hashSize, false);
    }
    return e;
  }
  return -(e + 1);
}

void BytesRefHash::rehash(int const newSize, bool hashOnData)
{
  constexpr int newMask = newSize - 1;
  bytesUsed->addAndGet(Integer::BYTES * (newSize));
  const std::deque<int> newHash = std::deque<int>(newSize);
  Arrays::fill(newHash, -1);
  for (int i = 0; i < hashSize; i++) {
    constexpr int e0 = ids[i];
    if (e0 != -1) {
      int code;
      if (hashOnData) {
        constexpr int off = bytesStart[e0];
        constexpr int start = off & BYTE_BLOCK_MASK;
        const std::deque<char> bytes = pool->buffers[off >> BYTE_BLOCK_SHIFT];
        constexpr int len;
        int pos;
        if ((bytes[start] & 0x80) == 0) {
          // length is 1 byte
          len = bytes[start];
          pos = start + 1;
        } else {
          len = (bytes[start] & 0x7f) + ((bytes[start + 1] & 0xff) << 7);
          pos = start + 2;
        }
        code = doHash(bytes, pos, len);
      } else {
        code = bytesStart[e0];
      }

      int hashPos = code & newMask;
      assert(hashPos >= 0);
      if (newHash[hashPos] != -1) {
        // Conflict; use linear probe to find an open slot
        // (see LUCENE-5604):
        do {
          code++;
          hashPos = code & newMask;
        } while (newHash[hashPos] != -1);
      }
      newHash[hashPos] = e0;
    }
  }

  hashMask = newMask;
  bytesUsed->addAndGet(Integer::BYTES * (-ids.size()));
  ids = newHash;
  hashSize = newSize;
  hashHalfSize = newSize / 2;
}

int BytesRefHash::doHash(std::deque<char> &bytes, int offset, int length)
{
  return StringHelper::murmurhash3_x86_32(bytes, offset, length,
                                          StringHelper::GOOD_FAST_HASH_SEED);
}

void BytesRefHash::reinit()
{
  if (bytesStart.empty()) {
    bytesStart = bytesStartArray->init();
  }

  if (ids.empty()) {
    ids = std::deque<int>(hashSize);
    bytesUsed->addAndGet(Integer::BYTES * hashSize);
  }
}

int BytesRefHash::byteStart(int bytesID)
{
  assert((bytesStart.size() > 0, L"bytesStart is null - not initialized"));
  assert((bytesID >= 0 && bytesID < count, bytesID));
  return bytesStart[bytesID];
}

BytesRefHash::MaxBytesLengthExceededException::MaxBytesLengthExceededException(
    const wstring &message)
    : RuntimeException(message)
{
}

BytesRefHash::DirectBytesStartArray::DirectBytesStartArray(
    int initSize, shared_ptr<Counter> counter)
    : initSize(initSize), bytesUsed(counter)
{
}

BytesRefHash::DirectBytesStartArray::DirectBytesStartArray(int initSize)
    : DirectBytesStartArray(initSize, Counter::newCounter())
{
}

std::deque<int> BytesRefHash::DirectBytesStartArray::clear()
{
  return bytesStart.clear();
}

std::deque<int> BytesRefHash::DirectBytesStartArray::grow()
{
  assert(bytesStart.size() > 0);
  return bytesStart = ArrayUtil::grow(bytesStart, bytesStart.size() + 1);
}

std::deque<int> BytesRefHash::DirectBytesStartArray::init()
{
  return bytesStart =
             std::deque<int>(ArrayUtil::oversize(initSize, Integer::BYTES));
}

shared_ptr<Counter> BytesRefHash::DirectBytesStartArray::bytesUsed()
{
  return bytesUsed_;
}
} // namespace org::apache::lucene::util