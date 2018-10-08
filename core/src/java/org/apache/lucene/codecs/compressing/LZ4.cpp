using namespace std;

#include "LZ4.h"
#include "../../store/DataInput.h"
#include "../../store/DataOutput.h"
#include "../../util/FutureArrays.h"
#include "../../util/packed/PackedInts.h"

namespace org::apache::lucene::codecs::compressing
{
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using FutureArrays = org::apache::lucene::util::FutureArrays;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

LZ4::LZ4() {}

int LZ4::hash(int i, int hashBits)
{
  return static_cast<int>(static_cast<unsigned int>((i * -1640531535)) >>
                          (32 - hashBits));
}

int LZ4::hashHC(int i) { return hash(i, HASH_LOG_HC); }

int LZ4::readInt(std::deque<char> &buf, int i)
{
  return ((buf[i] & 0xFF) << 24) | ((buf[i + 1] & 0xFF) << 16) |
         ((buf[i + 2] & 0xFF) << 8) | (buf[i + 3] & 0xFF);
}

bool LZ4::readIntEquals(std::deque<char> &buf, int i, int j)
{
  return readInt(buf, i) == readInt(buf, j);
}

int LZ4::commonBytes(std::deque<char> &b, int o1, int o2, int limit)
{
  assert(o1 < o2);
  // never -1 because lengths always differ
  return FutureArrays::mismatch(b, o1, limit, b, o2, limit);
}

int LZ4::commonBytesBackward(std::deque<char> &b, int o1, int o2, int l1,
                             int l2)
{
  int count = 0;
  while (o1 > l1 && o2 > l2 && b[--o1] == b[--o2]) {
    ++count;
  }
  return count;
}

int LZ4::decompress(shared_ptr<DataInput> compressed, int decompressedLen,
                    std::deque<char> &dest, int dOff) 
{
  constexpr int destEnd = dest.size();

  do {
    // literals
    constexpr int token = compressed->readByte() & 0xFF;
    int literalLen = static_cast<int>(static_cast<unsigned int>(token) >> 4);

    if (literalLen != 0) {
      if (literalLen == 0x0F) {
        char len;
        while ((len = compressed->readByte()) == static_cast<char>(0xFF)) {
          literalLen += 0xFF;
        }
        literalLen += len & 0xFF;
      }
      compressed->readBytes(dest, dOff, literalLen);
      dOff += literalLen;
    }

    if (dOff >= decompressedLen) {
      break;
    }

    // matchs
    constexpr int matchDec = (compressed->readByte() & 0xFF) |
                             ((compressed->readByte() & 0xFF) << 8);
    assert(matchDec > 0);

    int matchLen = token & 0x0F;
    if (matchLen == 0x0F) {
      int len;
      while ((len = compressed->readByte()) == static_cast<char>(0xFF)) {
        matchLen += 0xFF;
      }
      matchLen += len & 0xFF;
    }
    matchLen += MIN_MATCH;

    // copying a multiple of 8 bytes can make decompression from 5% to 10%
    // faster
    constexpr int fastLen = (matchLen + 7) & 0xFFFFFFF8;
    if (matchDec < matchLen || dOff + fastLen > destEnd) {
      // overlap -> naive incremental copy
      for (int ref = dOff - matchDec, end = dOff + matchLen; dOff < end;
           ++ref, ++dOff) {
        dest[dOff] = dest[ref];
      }
    } else {
      // no overlap -> arraycopy
      System::arraycopy(dest, dOff - matchDec, dest, dOff, fastLen);
      dOff += matchLen;
    }
  } while (dOff < decompressedLen);

  return dOff;
}

void LZ4::encodeLen(int l, shared_ptr<DataOutput> out) 
{
  while (l >= 0xFF) {
    out->writeByte(static_cast<char>(0xFF));
    l -= 0xFF;
  }
  out->writeByte(static_cast<char>(l));
}

void LZ4::encodeLiterals(std::deque<char> &bytes, int token, int anchor,
                         int literalLen,
                         shared_ptr<DataOutput> out) 
{
  out->writeByte(static_cast<char>(token));

  // encode literal length
  if (literalLen >= 0x0F) {
    encodeLen(literalLen - 0x0F, out);
  }

  // encode literals
  out->writeBytes(bytes, anchor, literalLen);
}

void LZ4::encodeLastLiterals(std::deque<char> &bytes, int anchor,
                             int literalLen,
                             shared_ptr<DataOutput> out) 
{
  constexpr int token = min(literalLen, 0x0F) << 4;
  encodeLiterals(bytes, token, anchor, literalLen, out);
}

void LZ4::encodeSequence(std::deque<char> &bytes, int anchor, int matchRef,
                         int matchOff, int matchLen,
                         shared_ptr<DataOutput> out) 
{
  constexpr int literalLen = matchOff - anchor;
  assert(matchLen >= 4);
  // encode token
  constexpr int token = (min(literalLen, 0x0F) << 4) | min(matchLen - 4, 0x0F);
  encodeLiterals(bytes, token, anchor, literalLen, out);

  // encode match dec
  constexpr int matchDec = matchOff - matchRef;
  assert(matchDec > 0 && matchDec < 1 << 16);
  out->writeByte(static_cast<char>(matchDec));
  out->writeByte(static_cast<char>(
      static_cast<int>(static_cast<unsigned int>(matchDec) >> 8)));

  // encode match len
  if (matchLen >= MIN_MATCH + 0x0F) {
    encodeLen(matchLen - 0x0F - MIN_MATCH, out);
  }
}

void LZ4::HashTable::reset(int len)
{
  constexpr int bitsPerOffset = PackedInts::bitsRequired(len - LAST_LITERALS);
  constexpr int bitsPerOffsetLog =
      32 - Integer::numberOfLeadingZeros(bitsPerOffset - 1);
  hashLog = MEMORY_USAGE + 3 - bitsPerOffsetLog;
  if (hashTable == nullptr || hashTable->size() < 1 << hashLog ||
      hashTable->getBitsPerValue() < bitsPerOffset) {
    hashTable = PackedInts::getMutable(1 << hashLog, bitsPerOffset,
                                       PackedInts::DEFAULT);
  } else {
    hashTable->clear();
  }
}

void LZ4::compress(std::deque<char> &bytes, int off, int len,
                   shared_ptr<DataOutput> out,
                   shared_ptr<HashTable> ht) 
{

  constexpr int base = off;
  constexpr int end = off + len;

  int anchor = off++;

  if (len > LAST_LITERALS + MIN_MATCH) {

    constexpr int limit = end - LAST_LITERALS;
    constexpr int matchLimit = limit - MIN_MATCH;
    ht->reset(len);
    constexpr int hashLog = ht->hashLog;
    shared_ptr<PackedInts::Mutable> *const hashTable = ht->hashTable;

    while (off <= limit) {
      // find a match
      int ref;
      while (true) {
        if (off >= matchLimit) {
          goto mainBreak;
        }
        constexpr int v = readInt(bytes, off);
        constexpr int h = hash(v, hashLog);
        ref = base + static_cast<int>(hashTable->get(h));
        assert(PackedInts::bitsRequired(off - base) <=
               hashTable->getBitsPerValue());
        hashTable->set(h, off - base);
        if (off - ref < MAX_DISTANCE && readInt(bytes, ref) == v) {
          break;
        }
        ++off;
      }

      // compute match length
      constexpr int matchLen = MIN_MATCH + commonBytes(bytes, ref + MIN_MATCH,
                                                       off + MIN_MATCH, limit);

      encodeSequence(bytes, anchor, ref, off, matchLen, out);
      off += matchLen;
      anchor = off;
    mainContinue:;
    }
  mainBreak:;
  }

  // last literals
  constexpr int literalLen = end - anchor;
  assert(literalLen >= LAST_LITERALS || literalLen == len);
  encodeLastLiterals(bytes, anchor, end - anchor, out);
}

void LZ4::Match::fix(int correction)
{
  start += correction;
  ref += correction;
  len -= correction;
}

int LZ4::Match::end() { return start + len; }

void LZ4::copyTo(shared_ptr<Match> m1, shared_ptr<Match> m2)
{
  m2->len = m1->len;
  m2->start = m1->start;
  m2->ref = m1->ref;
}

LZ4::HCHashTable::HCHashTable()
    : hashTable(std::deque<int>(HASH_TABLE_SIZE_HC)),
      chainTable(std::deque<short>(MAX_DISTANCE))
{
}

void LZ4::HCHashTable::reset(int base)
{
  this->base = base;
  nextToUpdate = base;
  Arrays::fill(hashTable, -1);
  Arrays::fill(chainTable, static_cast<short>(0));
}

int LZ4::HCHashTable::hashPointer(std::deque<char> &bytes, int off)
{
  constexpr int v = readInt(bytes, off);
  constexpr int h = hashHC(v);
  return hashTable[h];
}

int LZ4::HCHashTable::next(int off)
{
  return off - (chainTable[off & MASK] & 0xFFFF);
}

void LZ4::HCHashTable::addHash(std::deque<char> &bytes, int off)
{
  constexpr int v = readInt(bytes, off);
  constexpr int h = hashHC(v);
  int delta = off - hashTable[h];
  assert((delta > 0, delta));
  if (delta >= MAX_DISTANCE) {
    delta = MAX_DISTANCE - 1;
  }
  chainTable[off & MASK] = static_cast<short>(delta);
  hashTable[h] = off;
}

void LZ4::HCHashTable::insert(int off, std::deque<char> &bytes)
{
  for (; nextToUpdate < off; ++nextToUpdate) {
    addHash(bytes, nextToUpdate);
  }
}

bool LZ4::HCHashTable::insertAndFindBestMatch(std::deque<char> &buf, int off,
                                              int matchLimit,
                                              shared_ptr<Match> match)
{
  match->start = off;
  match->len = 0;
  int delta = 0;
  int repl = 0;

  insert(off, buf);

  int ref = hashPointer(buf, off);

  if (ref >= off - 4 && ref <= off && ref >= base) { // potential repetition
    if (readIntEquals(buf, ref, off)) {              // confirmed
      delta = off - ref;
      repl = match->len = MIN_MATCH + commonBytes(buf, ref + MIN_MATCH,
                                                  off + MIN_MATCH, matchLimit);
      match->ref = ref;
    }
    ref = next(ref);
  }

  for (int i = 0; i < MAX_ATTEMPTS; ++i) {
    if (ref < max(base, off - MAX_DISTANCE + 1) || ref > off) {
      break;
    }
    if (buf[ref + match->len] == buf[off + match->len] &&
        readIntEquals(buf, ref, off)) {
      constexpr int matchLen =
          MIN_MATCH +
          commonBytes(buf, ref + MIN_MATCH, off + MIN_MATCH, matchLimit);
      if (matchLen > match->len) {
        match->ref = ref;
        match->len = matchLen;
      }
    }
    ref = next(ref);
  }

  if (repl != 0) {
    int ptr = off;
    constexpr int end = off + repl - (MIN_MATCH - 1);
    while (ptr < end - delta) {
      chainTable[ptr & MASK] = static_cast<short>(delta); // pre load
      ++ptr;
    }
    do {
      chainTable[ptr & MASK] = static_cast<short>(delta);
      hashTable[hashHC(readInt(buf, ptr))] = ptr;
      ++ptr;
    } while (ptr < end);
    nextToUpdate = end;
  }

  return match->len != 0;
}

bool LZ4::HCHashTable::insertAndFindWiderMatch(std::deque<char> &buf, int off,
                                               int startLimit, int matchLimit,
                                               int minLen,
                                               shared_ptr<Match> match)
{
  match->len = minLen;

  insert(off, buf);

  constexpr int delta = off - startLimit;
  int ref = hashPointer(buf, off);
  for (int i = 0; i < MAX_ATTEMPTS; ++i) {
    if (ref < max(base, off - MAX_DISTANCE + 1) || ref > off) {
      break;
    }
    if (buf[ref - delta + match->len] == buf[startLimit + match->len] &&
        readIntEquals(buf, ref, off)) {
      constexpr int matchLenForward =
          MIN_MATCH +
          commonBytes(buf, ref + MIN_MATCH, off + MIN_MATCH, matchLimit);
      constexpr int matchLenBackward =
          commonBytesBackward(buf, ref, off, base, startLimit);
      constexpr int matchLen = matchLenBackward + matchLenForward;
      if (matchLen > match->len) {
        match->len = matchLen;
        match->ref = ref - matchLenBackward;
        match->start = off - matchLenBackward;
      }
    }
    ref = next(ref);
  }

  return match->len > minLen;
}

void LZ4::compressHC(std::deque<char> &src, int srcOff, int srcLen,
                     shared_ptr<DataOutput> out,
                     shared_ptr<HCHashTable> ht) 
{

  constexpr int srcEnd = srcOff + srcLen;
  constexpr int matchLimit = srcEnd - LAST_LITERALS;
  constexpr int mfLimit = matchLimit - MIN_MATCH;

  int sOff = srcOff;
  int anchor = sOff++;

  ht->reset(srcOff);
  shared_ptr<Match> *const match0 = make_shared<Match>();
  shared_ptr<Match> *const match1 = make_shared<Match>();
  shared_ptr<Match> *const match2 = make_shared<Match>();
  shared_ptr<Match> *const match3 = make_shared<Match>();

  while (sOff <= mfLimit) {
    if (!ht->insertAndFindBestMatch(src, sOff, matchLimit, match1)) {
      ++sOff;
      continue;
    }

    // saved, in case we would skip too much
    copyTo(match1, match0);

    while (true) {
      assert(match1->start >= anchor);
      if (match1->end() >= mfLimit ||
          !ht->insertAndFindWiderMatch(src, match1->end() - 2,
                                       match1->start + 1, matchLimit,
                                       match1->len, match2)) {
        // no better match
        encodeSequence(src, anchor, match1->ref, match1->start, match1->len,
                       out);
        anchor = sOff = match1->end();
        goto mainContinue;
      }

      if (match0->start < match1->start) {
        if (match2->start < match1->start + match0->len) { // empirical
          copyTo(match0, match1);
        }
      }
      assert(match2->start > match1->start);

      if (match2->start - match1->start <
          3) { // First Match too small : removed
        copyTo(match2, match1);
        goto search2Continue;
      }

      while (true) {
        if (match2->start - match1->start < OPTIMAL_ML) {
          int newMatchLen = match1->len;
          if (newMatchLen > OPTIMAL_ML) {
            newMatchLen = OPTIMAL_ML;
          }
          if (match1->start + newMatchLen > match2->end() - MIN_MATCH) {
            newMatchLen =
                match2->start - match1->start + match2->len - MIN_MATCH;
          }
          constexpr int correction =
              newMatchLen - (match2->start - match1->start);
          if (correction > 0) {
            match2->fix(correction);
          }
        }

        if (match2->start + match2->len >= mfLimit ||
            !ht->insertAndFindWiderMatch(src, match2->end() - 3, match2->start,
                                         matchLimit, match2->len, match3)) {
          // no better match -> 2 sequences to encode
          if (match2->start < match1->end()) {
            match1->len = match2->start - match1->start;
          }
          // encode seq 1
          encodeSequence(src, anchor, match1->ref, match1->start, match1->len,
                         out);
          anchor = sOff = match1->end();
          // encode seq 2
          encodeSequence(src, anchor, match2->ref, match2->start, match2->len,
                         out);
          anchor = sOff = match2->end();
          goto mainContinue;
        }

        if (match3->start <
            match1->end() + 3) { // Not enough space for match 2 : remove it
          if (match3->start >=
              match1->end()) { // // can write Seq1 immediately ==> Seq2 is
                               // removed, so Seq3 becomes Seq1
            if (match2->start < match1->end()) {
              constexpr int correction = match1->end() - match2->start;
              match2->fix(correction);
              if (match2->len < MIN_MATCH) {
                copyTo(match3, match2);
              }
            }

            encodeSequence(src, anchor, match1->ref, match1->start, match1->len,
                           out);
            anchor = sOff = match1->end();

            copyTo(match3, match1);
            copyTo(match2, match0);

            goto search2Continue;
          }

          copyTo(match3, match2);
          goto search3Continue;
        }

        // OK, now we have 3 ascending matches; let's write at least the first
        // one
        if (match2->start < match1->end()) {
          if (match2->start - match1->start < 0x0F) {
            if (match1->len > OPTIMAL_ML) {
              match1->len = OPTIMAL_ML;
            }
            if (match1->end() > match2->end() - MIN_MATCH) {
              match1->len = match2->end() - match1->start - MIN_MATCH;
            }
            constexpr int correction = match1->end() - match2->start;
            match2->fix(correction);
          } else {
            match1->len = match2->start - match1->start;
          }
        }

        encodeSequence(src, anchor, match1->ref, match1->start, match1->len,
                       out);
        anchor = sOff = match1->end();

        copyTo(match2, match1);
        copyTo(match3, match2);

        goto search3Continue;
      search3Continue:;
      }
    search3Break:;

    search2Continue:;
    }
  search2Break:;

  mainContinue:;
  }
mainBreak:

  encodeLastLiterals(src, anchor, srcEnd - anchor, out);
}
} // namespace org::apache::lucene::codecs::compressing