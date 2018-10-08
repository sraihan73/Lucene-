#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/util/Accountable.h"
#include "stringhelper.h"
#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/bloom/HashFunction.h"

#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"

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
namespace org::apache::lucene::codecs::bloom
{

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

/**
 * <p>
 * A class used to represent a set of many, potentially large, values (e.g. many
 * long strings such as URLs), using a significantly smaller amount of memory.
 * </p>
 * <p>
 * The set is "lossy" in that it cannot definitively state that is does contain
 * a value but it <em>can</em> definitively say if a value is <em>not</em> in
 * the set. It can therefore be used as a Bloom Filter.
 * </p>
 * Another application of the set is that it can be used to perform fuzzy
 * counting because it can estimate reasonably accurately how many unique values
 * are contained in the set. <p>This class is NOT threadsafe.</p> <p> Internally
 * a Bitset is used to record values and once a client has finished recording a
 * stream of values the {@link #downsize(float)} method can be used to create a
 * suitably smaller set that is sized appropriately for the number of values
 * recorded and desired saturation levels.
 *
 * </p>
 * @lucene.experimental
 */
class FuzzySet : public std::enable_shared_from_this<FuzzySet>,
                 public Accountable
{
  GET_CLASS_NAME(FuzzySet)

public:
  static constexpr int VERSION_SPI =
      1; // HashFunction used to be loaded through a SPI
  static constexpr int VERSION_START = VERSION_SPI;
  static constexpr int VERSION_CURRENT = 2;

  static std::shared_ptr<HashFunction> hashFunctionForVersion(int version);

  /**
   * Result from {@link FuzzySet#contains(BytesRef)}:
   * can never return definitively YES (always MAYBE),
   * but can sometimes definitely return NO.
   */
public:
  enum class ContainsResult { GET_CLASS_NAME(ContainsResult) MAYBE, NO };

private:
  std::shared_ptr<HashFunction> hashFunction;
  std::shared_ptr<FixedBitSet> filter;
  int bloomSize = 0;

  // The sizes of BitSet used are all numbers that, when expressed in binary
  // form, are all ones. This is to enable fast downsizing from one bitset to
  // another by simply ANDing each set index in one bitset with the size of the
  // target bitset
  // - this provides a fast modulo of the number. Values previously accumulated
  // in a large bitset and then mapped to a smaller set can be looked up using a
  // single AND operation of the query term's hash rather than needing to
  // perform a 2-step translation of the query term that mirrors the stored
  // content's reprojections.
public:
  static std::deque<int> const usableBitSetSizes;

private:
  class StaticConstructor
      : public std::enable_shared_from_this<StaticConstructor>
  {
    GET_CLASS_NAME(StaticConstructor)
  public:
    StaticConstructor();
  };

private:
  static FuzzySet::StaticConstructor staticConstructor;

  /**
   * Rounds down required maxNumberOfBits to the nearest number that is made up
   * of all ones as a binary number.
   * Use this method where controlling memory use is paramount.
   */
public:
  static int getNearestSetSize(int maxNumberOfBits);

  /**
   * Use this method to choose a set size where accuracy (low content
   * saturation) is more important than deciding how much memory to throw at the
   * problem.
   * @param desiredSaturation A number between 0 and 1 expressing the % of bits
   * set once all values have been recorded
   * @return The size of the set nearest to the required size
   */
  static int getNearestSetSize(int maxNumberOfValuesExpected,
                               float desiredSaturation);

  static std::shared_ptr<FuzzySet> createSetBasedOnMaxMemory(int maxNumBytes);

  static std::shared_ptr<FuzzySet>
  createSetBasedOnQuality(int maxNumUniqueValues, float desiredMaxSaturation);

private:
  // C++ TODO: No base class can be determined:
  FuzzySet(std::shared_ptr<FixedBitSet> filter, int bloomSize,
           std::shared_ptr<HashFunction> hashFunction); // super();

  /**
   * The main method required for a Bloom filter which, given a value determines
   * set membership. Unlike a conventional set, the fuzzy set returns NO or
   * MAYBE rather than true or false.
   * @return NO or MAYBE
   */
public:
  virtual ContainsResult contains(std::shared_ptr<BytesRef> value);

  /**
   * Serializes the data set to file using the following format:
   * <ul>
   *  <li>FuzzySet --&gt;FuzzySetVersion,HashFunctionName,BloomSize,
   * NumBitSetWords,BitSetWord<sup>NumBitSetWords</sup></li>
   * <li>HashFunctionName --&gt; {@link DataOutput#writeString(std::wstring) std::wstring}
The
   * name of a ServiceProvider registered {@link HashFunction}</li>
   * <li>FuzzySetVersion --&gt; {@link DataOutput#writeInt Uint32} The version
number of the {@link FuzzySet} class</li>
   * <li>BloomSize --&gt; {@link DataOutput#writeInt Uint32} The modulo value
used GET_CLASS_NAME(</li>)
   * to project hashes into the field's Bitset</li>
   * <li>NumBitSetWords --&gt; {@link DataOutput#writeInt Uint32} The number of
   * longs (as returned from {@link FixedBitSet#getBits})</li>
   * <li>BitSetWord --&gt; {@link DataOutput#writeLong Long} A long from the
array
   * returned by {@link FixedBitSet#getBits}</li>
   * </ul>
   * @param out Data output stream
   * @throws IOException If there is a low-level I/O error
   */
  virtual void serialize(std::shared_ptr<DataOutput> out) ;
  static std::shared_ptr<FuzzySet>
  deserialize(std::shared_ptr<DataInput> in_) ;

private:
  ContainsResult mayContainValue(int positiveHash);

  /**
   * Records a value in the set. The referenced bytes are hashed and then modulo
   * n'd where n is the chosen size of the internal bitset.
   * @param value the key value to be hashed
   * @throws IOException If there is a low-level I/O error
   */
public:
  virtual void addValue(std::shared_ptr<BytesRef> value) ;

  /**
   *
   * @param targetMaxSaturation A number between 0 and 1 describing the % of
   * bits that would ideally be set in the result. Lower values have better
   * accuracy but require more space.
   * @return a smaller FuzzySet or null if the current set is already
   * over-saturated
   */
  virtual std::shared_ptr<FuzzySet> downsize(float targetMaxSaturation);

  virtual int getEstimatedUniqueValues();

  // Given a set size and a the number of set bits, produces an estimate of the
  // number of unique values recorded
  static int
  getEstimatedNumberUniqueValuesAllowingForCollisions(int setSize,
                                                      int numRecordedBits);

  virtual float getSaturation();

  int64_t ramBytesUsed() override;

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/codecs/bloom/
