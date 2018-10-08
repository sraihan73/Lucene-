#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/util/BytesRefIterator.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;

/**
 * Interface for enumerating term,weight,payload triples for suggester
 * consumption; currently only {@link AnalyzingSuggester}, {@link
 * FuzzySuggester} and {@link AnalyzingInfixSuggester} support payloads.
 */
class InputIterator : public BytesRefIterator
{
  GET_CLASS_NAME(InputIterator)

  /** A term's weight, higher numbers mean better suggestions. */
public:
  virtual int64_t weight() = 0;

  /** An arbitrary byte[] to record per suggestion.  See
   *  {@link LookupResult#payload} to retrieve the payload
   *  for each suggestion. */
  virtual std::shared_ptr<BytesRef> payload() = 0;

  /** Returns true if the iterator has payloads */
  virtual bool hasPayloads() = 0;

  /**
   * A term's contexts context can be used to filter suggestions.
   * May return null, if suggest entries do not have any context
   * */
  virtual std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts() = 0;

  /** Returns true if the iterator has contexts */
  virtual bool hasContexts() = 0;

  /** Singleton InputIterator that iterates over 0 BytesRefs. */
  static const std::shared_ptr<InputIterator> EMPTY;

  /**
   * Wraps a BytesRefIterator as a suggester InputIterator, with all weights
   * set to <code>1</code> and carries no payload
   */
public:
  class InputIteratorWrapper
      : public std::enable_shared_from_this<InputIteratorWrapper>,
        public InputIterator
  {
    GET_CLASS_NAME(InputIteratorWrapper)
  private:
    const std::shared_ptr<BytesRefIterator> wrapped;

    /**
     * Creates a new wrapper, wrapping the specified iterator and
     * specifying a weight value of <code>1</code> for all terms
     * and nullifies associated payloads.
     */
  public:
    InputIteratorWrapper(std::shared_ptr<BytesRefIterator> wrapped);

    int64_t weight() override;

    std::shared_ptr<BytesRef> next()  override;

    std::shared_ptr<BytesRef> payload() override;

    bool hasPayloads() override;

    std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts() override;

    bool hasContexts() override;
  };
};

} // #include  "core/src/java/org/apache/lucene/search/suggest/
