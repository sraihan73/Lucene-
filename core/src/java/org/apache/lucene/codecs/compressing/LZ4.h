#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class DataInput;
}

namespace org::apache::lucene::store
{
class DataOutput;
}
namespace org::apache::lucene::util::packed
{
class PackedInts;
}
namespace org::apache::lucene::util::packed
{
class Mutable;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::codecs::compressing
{

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using PackedInts = org::apache::lucene::util::packed::PackedInts;

/**
 * LZ4 compression and decompression routines.
 *
 * http://code.google.com/p/lz4/
 * http://fastcompression.blogspot.fr/p/lz4.html
 */
class LZ4 final : public std::enable_shared_from_this<LZ4>
{
  GET_CLASS_NAME(LZ4)

private:
  LZ4();

public:
  static constexpr int MEMORY_USAGE = 14;
  static constexpr int MIN_MATCH = 4;      // minimum length of a match
  static const int MAX_DISTANCE = 1 << 16; // maximum distance of a reference
  static constexpr int LAST_LITERALS =
      5; // the last 5 bytes must be encoded as literals
  static constexpr int HASH_LOG_HC =
      15; // log size of the dictionary for compressHC
  static const int HASH_TABLE_SIZE_HC = 1 << HASH_LOG_HC;
  static const int OPTIMAL_ML =
      0x0F + 4 - 1; // match length that doesn't require an additional byte

private:
  static int hash(int i, int hashBits);

  static int hashHC(int i);

  static int readInt(std::deque<char> &buf, int i);

  static bool readIntEquals(std::deque<char> &buf, int i, int j);

  static int commonBytes(std::deque<char> &b, int o1, int o2, int limit);

  static int commonBytesBackward(std::deque<char> &b, int o1, int o2, int l1,
                                 int l2);

  /**
   * Decompress at least <code>decompressedLen</code> bytes into
   * <code>dest[dOff:]</code>. Please note that <code>dest</code> must be large
   * enough to be able to hold <b>all</b> decompressed data (meaning that you
   * need to know the total decompressed length).
   */
public:
  static int decompress(std::shared_ptr<DataInput> compressed,
                        int decompressedLen, std::deque<char> &dest,
                        int dOff) ;

private:
  static void encodeLen(int l,
                        std::shared_ptr<DataOutput> out) ;

  static void
  encodeLiterals(std::deque<char> &bytes, int token, int anchor,
                 int literalLen,
                 std::shared_ptr<DataOutput> out) ;

  static void
  encodeLastLiterals(std::deque<char> &bytes, int anchor, int literalLen,
                     std::shared_ptr<DataOutput> out) ;

  static void
  encodeSequence(std::deque<char> &bytes, int anchor, int matchRef,
                 int matchOff, int matchLen,
                 std::shared_ptr<DataOutput> out) ;

public:
  class HashTable final : public std::enable_shared_from_this<HashTable>
  {
    GET_CLASS_NAME(HashTable)
  private:
    int hashLog = 0;
    std::shared_ptr<PackedInts::Mutable> hashTable;

  public:
    void reset(int len);
  };

  /**
   * Compress <code>bytes[off:off+len]</code> into <code>out</code> using
   * at most 16KB of memory. <code>ht</code> shouldn't be shared across threads
   * but can safely be reused.
   */
public:
  static void compress(std::deque<char> &bytes, int off, int len,
                       std::shared_ptr<DataOutput> out,
                       std::shared_ptr<HashTable> ht) ;

private:
  class Match : public std::enable_shared_from_this<Match>
  {
    GET_CLASS_NAME(Match)
  public:
    int start = 0, ref = 0, len = 0;

    virtual void fix(int correction);

    virtual int end();
  };

private:
  static void copyTo(std::shared_ptr<Match> m1, std::shared_ptr<Match> m2);

public:
  class HCHashTable final : public std::enable_shared_from_this<HCHashTable>
  {
    GET_CLASS_NAME(HCHashTable)
  public:
    static constexpr int MAX_ATTEMPTS = 256;
    static const int MASK = MAX_DISTANCE - 1;
    int nextToUpdate = 0;

  private:
    int base = 0;
    std::deque<int> const hashTable;
    std::deque<short> const chainTable;

  public:
    HCHashTable();

  private:
    void reset(int base);

    int hashPointer(std::deque<char> &bytes, int off);

    int next(int off);

    void addHash(std::deque<char> &bytes, int off);

  public:
    void insert(int off, std::deque<char> &bytes);

    bool insertAndFindBestMatch(std::deque<char> &buf, int off, int matchLimit,
                                std::shared_ptr<Match> match);

    bool insertAndFindWiderMatch(std::deque<char> &buf, int off,
                                 int startLimit, int matchLimit, int minLen,
                                 std::shared_ptr<Match> match);
  };

  /**
   * Compress <code>bytes[off:off+len]</code> into <code>out</code>. Compared to
   * {@link LZ4#compress(byte[], int, int, DataOutput, HashTable)}, this method
   * is slower and uses more memory (~ 256KB per thread) but should provide
   * better compression ratios (especially on large inputs) because it chooses
   * the best match among up to 256 candidates and then performs trade-offs to
   * fix overlapping matches. <code>ht</code> shouldn't be shared across threads
   * but can safely be reused.
   */
public:
  static void compressHC(std::deque<char> &src, int srcOff, int srcLen,
                         std::shared_ptr<DataOutput> out,
                         std::shared_ptr<HCHashTable> ht) ;
};

} // namespace org::apache::lucene::codecs::compressing
