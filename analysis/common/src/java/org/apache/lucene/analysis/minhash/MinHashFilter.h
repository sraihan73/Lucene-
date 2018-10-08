#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::minhash
{
class LongPair;
}

namespace org::apache::lucene::analysis::minhash
{
template <typename E>
class FixedSizeTreeSet;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionLengthAttribute;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
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

namespace org::apache::lucene::analysis::minhash
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;

/**
 * Generate min hash tokens from an incoming stream of tokens. The incoming
 * tokens would typically be 5 word shingles.
 *
 * The number of hashes used and the number of minimum values for each hash can
 * be set. You could have 1 hash and keep the 100 lowest values or 100 hashes
 * and keep the lowest one for each. Hashes can also be bucketed in ranges over
 * the 128-bit hash space,
 *
 * A 128-bit hash is used internally. 5 word shingles from 10e5 words generate
 * 10e25 combinations So a 64 bit hash would have collisions (1.8e19)
 *
 * When using different hashes 32 bits are used for the hash position leaving
 * scope for 8e28 unique hashes. A single hash will use all 128 bits.
 *
 */
class MinHashFilter : public TokenFilter
{
  GET_CLASS_NAME(MinHashFilter)
private:
  static constexpr int HASH_CACHE_SIZE = 512;

  static std::deque<std::shared_ptr<LongPair>> const cachedIntHashes;

public:
  static constexpr int DEFAULT_HASH_COUNT = 1;

  static constexpr int DEFAULT_HASH_SET_SIZE = 1;

  static constexpr int DEFAULT_BUCKET_COUNT = 512;

  static const std::wstring MIN_HASH_TYPE;

private:
  const std::deque<std::deque<FixedSizeTreeSet<std::shared_ptr<LongPair>>>>
      minHashSets;

  int hashSetSize = DEFAULT_HASH_SET_SIZE;

  int bucketCount = DEFAULT_BUCKET_COUNT;

  int hashCount = DEFAULT_HASH_COUNT;

  bool requiresInitialisation = true;

  std::shared_ptr<State> endState;

  int hashPosition = -1;

  int bucketPosition = -1;

  int64_t bucketSize = 0;

  const bool withRotation;

  int endOffset = 0;

  bool exhausted = false;

  const std::shared_ptr<CharTermAttribute> termAttribute =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAttribute =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAttribute =
      addAttribute(TypeAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncAttribute =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLenAttribute =
      addAttribute(PositionLengthAttribute::typeid);

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static MinHashFilter::StaticConstructor staticConstructor;

public:
  static std::deque<char> getBytes(int i);

  /**
   * create a MinHash filter
   *
   * @param input the token stream
   * @param hashCount the no. of hashes
   * @param bucketCount the no. of buckets for hashing
   * @param hashSetSize the no. of min hashes to keep
   * @param withRotation whether rotate or not hashes while incrementing tokens
   */
  MinHashFilter(std::shared_ptr<TokenStream> input, int hashCount,
                int bucketCount, int hashSetSize, bool withRotation);

  bool incrementToken()  override final;

private:
  static std::shared_ptr<LongPair> getIntHash(int i);

public:
  void end()  override;

  void reset()  override;

private:
  void doRest();

  static wchar_t long0(int64_t x);

  static wchar_t long1(int64_t x);

  static wchar_t long2(int64_t x);

  static wchar_t long3(int64_t x);

  static wchar_t int0(int x);

  static wchar_t int1(int x);

public:
  static bool isLessThanUnsigned(int64_t n1, int64_t n2);

public:
  template <typename E>
  class FixedSizeTreeSet : public std::set<E>
  {
    GET_CLASS_NAME(FixedSizeTreeSet)
    static_assert(std::is_base_of<Comparable<E>, E>::value,
                  L"E must inherit from Comparable<E>");

    /**
     *
     */
  private:
    static constexpr int64_t serialVersionUID = -8237117170340299630LL;
    const int capacity;

  public:
    FixedSizeTreeSet() : FixedSizeTreeSet(20) {}

    FixedSizeTreeSet(int capacity) : TreeSet<E>(), capacity(capacity) {}

    bool add(E const toAdd) override
    {
      if (capacity <= this->size()) {
        constexpr E lastElm = this->last();
        if (toAdd.compareTo(lastElm) > -1) {
          return false;
        } else {
          pollLast();
        }
      }
      return outerInstance->super->add(toAdd);
    }

  protected:
    std::shared_ptr<FixedSizeTreeSet> shared_from_this()
    {
      return std::static_pointer_cast<FixedSizeTreeSet>(
          java.util.TreeSet<E>::shared_from_this());
    }
  };

private:
  static std::shared_ptr<LongPair>
  combineOrdered(std::deque<LongPair> &hashCodes);

  /** 128 bits of state */
public:
  class LongPair final : public std::enable_shared_from_this<LongPair>,
                         public Comparable<std::shared_ptr<LongPair>>
  {
    GET_CLASS_NAME(LongPair)
  public:
    int64_t val1 = 0;
    int64_t val2 = 0;

    /*
     * (non-Javadoc)
     *
     * @see java.lang.Comparable#compareTo(java.lang.Object)
     */
    int compareTo(std::shared_ptr<LongPair> other) override;

    virtual bool equals(std::any o);

    virtual int hashCode();
  };

  /** Gets a long from a byte buffer in little endian byte order. */
private:
  static int64_t getLongLittleEndian(std::deque<char> &buf, int offset);

  /** Returns the MurmurHash3_x64_128 hash, placing the result in "out". */
public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("fallthrough") static void
  // murmurhash3_x64_128(byte[] key, int offset, int len, int seed, LongPair out)
  static void murmurhash3_x64_128(std::deque<char> &key, int offset, int len,
                                  int seed, std::shared_ptr<LongPair> out);

private:
  static int64_t fmix64(int64_t k);

protected:
  std::shared_ptr<MinHashFilter> shared_from_this()
  {
    return std::static_pointer_cast<MinHashFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::minhash
