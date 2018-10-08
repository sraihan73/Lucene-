#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/search/suggest/fst/FSTCompletion.h"
#include  "core/src/java/org/apache/lucene/search/suggest/InputIterator.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/suggest/Lookup.h"
#include  "core/src/java/org/apache/lucene/search/suggest/LookupResult.h"
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"

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
namespace org::apache::lucene::search::suggest::fst
{

using InputIterator = org::apache::lucene::search::suggest::InputIterator;
using Lookup = org::apache::lucene::search::suggest::Lookup;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * An adapter from {@link Lookup} API to {@link FSTCompletion}.
 *
 * <p>This adapter differs from {@link FSTCompletion} in that it attempts
 * to discretize any "weights" as passed from in {@link InputIterator#weight()}
 * to match the number of buckets. For the rationale for bucketing, see
 * {@link FSTCompletion}.
 *
 * <p><b>Note:</b>Discretization requires an additional sorting pass.
 *
 * <p>The range of weights for bucketing/ discretization is determined
 * by sorting the input by weight and then dividing into
 * equal ranges. Then, scores within each range are assigned to that bucket.
 *
 * <p>Note that this means that even large differences in weights may be lost
 * during automaton construction, but the overall distinction between "classes"
 * of weights will be preserved regardless of the distribution of weights.
 *
 * <p>For fine-grained control over which weights are assigned to which buckets,
 * use {@link FSTCompletion} directly or {@link TSTLookup}, for example.
 *
 * @see FSTCompletion
 * @lucene.experimental
 */
class FSTCompletionLookup : public Lookup, public Accountable
{
  GET_CLASS_NAME(FSTCompletionLookup)
  /**
   * An invalid bucket count if we're creating an object
   * of this class from an existing FST.
   *
   * @see #FSTCompletionLookup(Directory, std::wstring, FSTCompletion, bool)
   */
private:
  static int INVALID_BUCKETS_COUNT;

  /**
   * Shared tail length for conflating in the created automaton. Setting this
   * to larger values ({@link Integer#MAX_VALUE}) will create smaller (or
   * minimal) automata at the cost of RAM for keeping nodes hash in the {@link
   * FST}.
   *
   * <p>Empirical pick.
   */
  static constexpr int sharedTailLength = 5;

  const std::shared_ptr<Directory> tempDir;
  const std::wstring tempFileNamePrefix;

  int buckets = 0;
  bool exactMatchFirst = false;

  /**
   * Automaton used for completions with higher weights reordering.
   */
  std::shared_ptr<FSTCompletion> higherWeightsCompletion;

  /**
   * Automaton used for normal completions.
   */
  std::shared_ptr<FSTCompletion> normalCompletion;

  /** Number of entries the lookup was built with */
  int64_t count = 0;

  /**
   * This constructor should only be used to read a previously saved suggester.
   */
public:
  FSTCompletionLookup();

  /**
   * This constructor prepares for creating a suggested FST using the
   * {@link #build(InputIterator)} method. The number of weight
   * discretization buckets is set to {@link FSTCompletion#DEFAULT_BUCKETS} and
   * exact matches are promoted to the top of the suggestions deque.
   */
  FSTCompletionLookup(std::shared_ptr<Directory> tempDir,
                      const std::wstring &tempFileNamePrefix);

  /**
   * This constructor prepares for creating a suggested FST using the
   * {@link #build(InputIterator)} method.
   *
   * @param buckets
   *          The number of weight discretization buckets (see
   *          {@link FSTCompletion} for details).
   *
   * @param exactMatchFirst
   *          If <code>true</code> exact matches are promoted to the top of the
   *          suggestions deque. Otherwise they appear in the order of
   *          discretized weight and alphabetical within the bucket.
   */
  FSTCompletionLookup(std::shared_ptr<Directory> tempDir,
                      const std::wstring &tempFileNamePrefix, int buckets,
                      bool exactMatchFirst);

  /**
   * This constructor takes a pre-built automaton.
   *
   *  @param completion
   *          An instance of {@link FSTCompletion}.
   *  @param exactMatchFirst
   *          If <code>true</code> exact matches are promoted to the top of the
   *          suggestions deque. Otherwise they appear in the order of
   *          discretized weight and alphabetical within the bucket.
   */
  FSTCompletionLookup(std::shared_ptr<Directory> tempDir,
                      const std::wstring &tempFileNamePrefix,
                      std::shared_ptr<FSTCompletion> completion,
                      bool exactMatchFirst);

  void
  build(std::shared_ptr<InputIterator> iterator)  override;

  /** weight -&gt; cost */
private:
  static int encodeWeight(int64_t value);

public:
  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
         bool higherWeightsFirst, int num) override;

  /**
   * Returns the bucket (weight) as a Long for the provided key if it exists,
   * otherwise null if it does not.
   */
  virtual std::any get(std::shared_ptr<std::wstring> key);

  // C++ WARNING: The following method was originally marked 'synchronized':
  bool store(std::shared_ptr<DataOutput> output)  override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  bool load(std::shared_ptr<DataInput> input)  override;

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  int64_t getCount() override;

protected:
  std::shared_ptr<FSTCompletionLookup> shared_from_this()
  {
    return std::static_pointer_cast<FSTCompletionLookup>(
        org.apache.lucene.search.suggest.Lookup::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/fst/
