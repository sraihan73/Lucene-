#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>
#include <queue>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/search/spell/Dictionary.h"
#include  "core/src/java/org/apache/lucene/search/suggest/InputIterator.h"
#include  "core/src/java/org/apache/lucene/search/BooleanQuery.h"
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
namespace org::apache::lucene::search::suggest
{

using BooleanQuery = org::apache::lucene::search::BooleanQuery;
using Dictionary = org::apache::lucene::search::spell::Dictionary;
using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using PriorityQueue = org::apache::lucene::util::PriorityQueue;

/**
 * Simple Lookup interface for {@link std::wstring} suggestions.
 * @lucene.experimental
 */
class Lookup : public std::enable_shared_from_this<Lookup>, public Accountable
{
  GET_CLASS_NAME(Lookup)

  /**
   * Result of a lookup.
   * @lucene.experimental
   */
public:
  class LookupResult final : public std::enable_shared_from_this<LookupResult>,
                             public Comparable<std::shared_ptr<LookupResult>>
  {
    GET_CLASS_NAME(LookupResult)
    /** the key's text */
  public:
    const std::shared_ptr<std::wstring> key;

    /** Expert: custom Object to hold the result of a
     *  highlighted suggestion. */
    const std::any highlightKey;

    /** the key's weight */
    const int64_t value;

    /** the key's payload (null if not present) */
    const std::shared_ptr<BytesRef> payload;

    /** the key's contexts (null if not present) */
    const std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts;

    /**
     * Create a new result from a key+weight pair.
     */
    LookupResult(std::shared_ptr<std::wstring> key, int64_t value);

    /**
     * Create a new result from a key+weight+payload triple.
     */
    LookupResult(std::shared_ptr<std::wstring> key, int64_t value,
                 std::shared_ptr<BytesRef> payload);

    /**
     * Create a new result from a key+highlightKey+weight+payload triple.
     */
    LookupResult(std::shared_ptr<std::wstring> key, std::any highlightKey,
                 int64_t value, std::shared_ptr<BytesRef> payload);

    /**
     * Create a new result from a key+weight+payload+contexts triple.
     */
    LookupResult(std::shared_ptr<std::wstring> key, int64_t value,
                 std::shared_ptr<BytesRef> payload,
                 std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts);

    /**
     * Create a new result from a key+weight+contexts triple.
     */
    LookupResult(std::shared_ptr<std::wstring> key, int64_t value,
                 std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts);

    /**
     * Create a new result from a key+highlightKey+weight+payload+contexts
     * triple.
     */
    LookupResult(std::shared_ptr<std::wstring> key, std::any highlightKey,
                 int64_t value, std::shared_ptr<BytesRef> payload,
                 std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts);

    virtual std::wstring toString();

    /** Compare alphabetically. */
    int compareTo(std::shared_ptr<LookupResult> o) override;
  };

  /**
   * A simple char-by-char comparator for {@link std::wstring}
   */
public:
  static const std::shared_ptr<Comparator<std::shared_ptr<std::wstring>>>
      CHARSEQUENCE_COMPARATOR;

private:
  class CharSequenceComparator
      : public std::enable_shared_from_this<CharSequenceComparator>,
        public Comparator<std::shared_ptr<std::wstring>>
  {
    GET_CLASS_NAME(CharSequenceComparator)

  public:
    int compare(std::shared_ptr<std::wstring> o1,
                std::shared_ptr<std::wstring> o2) override;
  };

  /**
   * A {@link PriorityQueue} collecting a fixed size of high priority {@link
   * LookupResult}
   */
public:
  class LookupPriorityQueue final
      : public PriorityQueue<std::shared_ptr<LookupResult>>
  {
    GET_CLASS_NAME(LookupPriorityQueue)
    // TODO: should we move this out of the interface into a utility class?
    /**
     * Creates a new priority queue of the specified size.
     */
  public:
    LookupPriorityQueue(int size);

  protected:
    bool lessThan(std::shared_ptr<LookupResult> a,
                  std::shared_ptr<LookupResult> b) override;

    /**
     * Returns the top N results in descending order.
     * @return the top N results in descending order.
     */
  public:
    std::deque<std::shared_ptr<LookupResult>> getResults();

  protected:
    std::shared_ptr<LookupPriorityQueue> shared_from_this()
    {
      return std::static_pointer_cast<LookupPriorityQueue>(
          org.apache.lucene.util
              .PriorityQueue<LookupResult>::shared_from_this());
    }
  };

  /**
   * Sole constructor. (For invocation by subclass
   * constructors, typically implicit.)
   */
public:
  Lookup();

  /** Build lookup from a dictionary. Some implementations may require sorted
   * or unsorted keys from the dictionary's iterator - use
   * {@link SortedInputIterator} or
   * {@link UnsortedInputIterator} in such case.
   */
  virtual void build(std::shared_ptr<Dictionary> dict) ;

  /**
   * Calls {@link #load(DataInput)} after converting
   * {@link InputStream} to {@link DataInput}
   */
  virtual bool load(std::shared_ptr<InputStream> input) ;

  /**
   * Calls {@link #store(DataOutput)} after converting
   * {@link OutputStream} to {@link DataOutput}
   */
  virtual bool store(std::shared_ptr<OutputStream> output) ;

  /**
   * Get the number of entries the lookup was built with
   * @return total number of suggester entries
   */
  virtual int64_t getCount() = 0;

  /**
   * Builds up a new internal {@link Lookup} representation based on the given
   * {@link InputIterator}. The implementation might re-sort the data
   * internally.
   */
  virtual void build(std::shared_ptr<InputIterator> inputIterator) = 0;

  /**
   * Look up a key and return possible completion for this key.
   * @param key lookup key. Depending on the implementation this may be
   * a prefix, misspelling, or even infix.
   * @param onlyMorePopular return only more popular results
   * @param num maximum number of results to return
   * @return a deque of possible completions, with their relative weight (e.g.
   * popularity)
   */
  virtual std::deque<std::shared_ptr<LookupResult>>
  lookup(std::shared_ptr<std::wstring> key, bool onlyMorePopular,
         int num) ;

  /**
   * Look up a key and return possible completion for this key.
   * @param key lookup key. Depending on the implementation this may be
   * a prefix, misspelling, or even infix.
   * @param contexts contexts to filter the lookup by, or null if all contexts
   * are allowed; if the suggestion contains any of the contexts, it's a match
   * @param onlyMorePopular return only more popular results
   * @param num maximum number of results to return
   * @return a deque of possible completions, with their relative weight (e.g.
   * popularity)
   */
  virtual std::deque<std::shared_ptr<LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
         bool onlyMorePopular, int num) = 0;

  /**
   * Look up a key and return possible completion for this key.
   * This needs to be overridden by all implementing classes as the default
   * implementation just returns null
   *
   * @param key the lookup key
   * @param contextFilerQuery A query for further filtering the result of the
   * key lookup
   * @param num maximum number of results to return
   * @param allTermsRequired true is all terms are required
   * @param doHighlight set to true if key should be highlighted
   * @return a deque of suggestions/completions. The default implementation
   * returns null, meaning each @Lookup implementation should override this and
   * provide their own implementation
   * @throws IOException when IO exception occurs
   */
  virtual std::deque<std::shared_ptr<LookupResult>>
  lookup(std::shared_ptr<std::wstring> key,
         std::shared_ptr<BooleanQuery> contextFilerQuery, int num,
         bool allTermsRequired, bool doHighlight) ;

  /**
   * Persist the constructed lookup data to a directory. Optional operation.
   * @param output {@link DataOutput} to write the data to.
   * @return true if successful, false if unsuccessful or not supported.
   * @throws IOException when fatal IO error occurs.
   */
  virtual bool store(std::shared_ptr<DataOutput> output) = 0;

  /**
   * Discard current lookup data and load it from a previously saved copy.
   * Optional operation.
   * @param input the {@link DataInput} to load the lookup data.
   * @return true if completed successfully, false if unsuccessful or not
   * supported.
   * @throws IOException when fatal IO error occurs.
   */
  virtual bool load(std::shared_ptr<DataInput> input) = 0;
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/
