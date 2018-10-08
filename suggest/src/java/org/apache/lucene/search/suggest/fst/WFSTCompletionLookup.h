#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/search/suggest/InputIterator.h"
#include  "core/src/java/org/apache/lucene/store/DataOutput.h"
#include  "core/src/java/org/apache/lucene/store/DataInput.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/suggest/Lookup.h"
#include  "core/src/java/org/apache/lucene/search/suggest/LookupResult.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class Arc;
}
#include  "core/src/java/org/apache/lucene/store/ByteArrayDataOutput.h"
#include  "core/src/java/org/apache/lucene/util/OfflineSorter.h"
#include  "core/src/java/org/apache/lucene/util/ByteSequencesWriter.h"
#include  "core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"
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
using SortedInputIterator =
    org::apache::lucene::search::suggest::SortedInputIterator;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Directory = org::apache::lucene::store::Directory;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using ByteSequencesWriter =
    org::apache::lucene::util::OfflineSorter::ByteSequencesWriter;
using FST = org::apache::lucene::util::fst::FST;
using org::apache::lucene::util::fst::FST::Arc;

/**
 * Suggester based on a weighted FST: it first traverses the prefix,
 * then walks the <i>n</i> shortest paths to retrieve top-ranked
 * suggestions.
 * <p>
 * <b>NOTE</b>:
 * Input weights must be between 0 and {@link Integer#MAX_VALUE}, any
 * other values will be rejected.
 *
 * @lucene.experimental
 */
// redundant 'implements Accountable' to workaround javadocs bugs
class WFSTCompletionLookup : public Lookup, public Accountable
{
  GET_CLASS_NAME(WFSTCompletionLookup)

  /**
   * FST<Long>, weights are encoded as costs: (Integer.MAX_VALUE-weight)
   */
  // NOTE: like FSTSuggester, this is really a WFSA, if you want to
  // customize the code to add some output you should use PairOutputs.
private:
  std::shared_ptr<FST<int64_t>> fst = nullptr;

  /**
   * True if exact match suggestions should always be returned first.
   */
  const bool exactFirst;

  /** Number of entries the lookup was built with */
  int64_t count = 0;

  const std::shared_ptr<Directory> tempDir;
  const std::wstring tempFileNamePrefix;

  /**
   * Calls {@link #WFSTCompletionLookup(Directory,std::wstring,bool)
   * WFSTCompletionLookup(null,null,true)}
   */
public:
  WFSTCompletionLookup(std::shared_ptr<Directory> tempDir,
                       const std::wstring &tempFileNamePrefix);

  /**
   * Creates a new suggester.
   *
   * @param exactFirst <code>true</code> if suggestions that match the
   *        prefix exactly should always be returned first, regardless
   *        of score. This has no performance impact, but could result
   *        in low-quality suggestions.
   */
  WFSTCompletionLookup(std::shared_ptr<Directory> tempDir,
                       const std::wstring &tempFileNamePrefix, bool exactFirst);

  void
  build(std::shared_ptr<InputIterator> iterator)  override;

  bool store(std::shared_ptr<DataOutput> output)  override;

  bool load(std::shared_ptr<DataInput> input)  override;

  std::deque<std::shared_ptr<Lookup::LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
         bool onlyMorePopular, int num) override;

private:
  std::optional<int64_t>
  lookupPrefix(std::shared_ptr<BytesRef> scratch,
               std::shared_ptr<FST::Arc<int64_t>> arc) ;

  /**
   * Returns the weight associated with an input string,
   * or null if it does not exist.
   */
public:
  virtual std::any get(std::shared_ptr<std::wstring> key);

  /** cost -&gt; weight */
private:
  static int decodeWeight(int64_t encoded);

  /** weight -&gt; cost */
  static int encodeWeight(int64_t value);

private:
  class WFSTInputIterator final : public SortedInputIterator
  {
    GET_CLASS_NAME(WFSTInputIterator)

  public:
    WFSTInputIterator(std::shared_ptr<Directory> tempDir,
                      const std::wstring &tempFileNamePrefix,
                      std::shared_ptr<InputIterator> source) ;

  protected:
    void encode(std::shared_ptr<ByteSequencesWriter> writer,
                std::shared_ptr<ByteArrayDataOutput> output,
                std::deque<char> &buffer, std::shared_ptr<BytesRef> spare,
                std::shared_ptr<BytesRef> payload,
                std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
                int64_t weight)  override;

    int64_t decode(std::shared_ptr<BytesRef> scratch,
                     std::shared_ptr<ByteArrayDataInput> tmpInput) override;

  protected:
    std::shared_ptr<WFSTInputIterator> shared_from_this()
    {
      return std::static_pointer_cast<WFSTInputIterator>(
          org.apache.lucene.search.suggest
              .SortedInputIterator::shared_from_this());
    }
  };

public:
  static const std::shared_ptr<Comparator<int64_t>> weightComparator;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<int64_t>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

    int compare(std::optional<int64_t> &left,
                std::optional<int64_t> &right) override;
  };

  /** Returns byte size of the underlying FST. */
public:
  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  int64_t getCount() override;

protected:
  std::shared_ptr<WFSTCompletionLookup> shared_from_this()
  {
    return std::static_pointer_cast<WFSTCompletionLookup>(
        org.apache.lucene.search.suggest.Lookup::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/fst/
