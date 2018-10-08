#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>
#include <queue>
#include "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include "core/src/java/org/apache/lucene/index/Term.h"
#include "core/src/java/org/apache/lucene/util/BytesRefIterator.h"
#include "core/src/java/org/apache/lucene/util/BytesRef.h"
#include "core/src/java/org/apache/lucene/util/PriorityQueue.h"

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

namespace org::apache::lucene::search
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using org::apache::lucene::util::PriorityQueue;

/**
 * A {@link MatchesIterator} that combines matches from a set of sub-iterators
 *
 * Matches are sorted by their start positions, and then by their end positions,
 * so that prefixes sort first.  Matches may overlap, or be duplicated if they
 * appear in more than one of the sub-iterators.
 */
class DisjunctionMatchesIterator final
    : public std::enable_shared_from_this<DisjunctionMatchesIterator>,
      public MatchesIterator
{
  GET_CLASS_NAME(DisjunctionMatchesIterator)

  /**
   * Create a {@link DisjunctionMatchesIterator} over a deque of terms
   *
   * Only terms that have at least one match in the given document will be
   * included
   */
public:
  static std::shared_ptr<MatchesIterator>
  fromTerms(std::shared_ptr<LeafReaderContext> context, int doc,
            const std::wstring &field,
            std::deque<std::shared_ptr<Term>> &terms) ;

private:
  static std::shared_ptr<BytesRefIterator>
  asBytesRefIterator(std::deque<std::shared_ptr<Term>> &terms);

private:
  class BytesRefIteratorAnonymousInnerClass
      : public std::enable_shared_from_this<
            BytesRefIteratorAnonymousInnerClass>,
        public BytesRefIterator
  {
    GET_CLASS_NAME(BytesRefIteratorAnonymousInnerClass)
  private:
    std::deque<std::shared_ptr<Term>> terms;

  public:
    BytesRefIteratorAnonymousInnerClass(
        std::deque<std::shared_ptr<Term>> &terms);

    int i = 0;
    std::shared_ptr<BytesRef> next();
  };

  /**
   * Create a {@link DisjunctionMatchesIterator} over a deque of terms extracted
   * from a {@link BytesRefIterator}
   *
   * Only terms that have at least one match in the given document will be
   * included
   */
public:
  static std::shared_ptr<MatchesIterator>
  fromTermsEnum(std::shared_ptr<LeafReaderContext> context, int doc,
                const std::wstring &field,
                std::shared_ptr<BytesRefIterator> terms) ;

  static std::shared_ptr<MatchesIterator> fromSubIterators(
      std::deque<std::shared_ptr<MatchesIterator>> &mis) ;

private:
  const std::shared_ptr<PriorityQueue<std::shared_ptr<MatchesIterator>>> queue;

  bool started = false;

  DisjunctionMatchesIterator(std::deque<std::shared_ptr<MatchesIterator>>
                                 &matches) ;

public:
  bool next()  override;

  int startPosition() override;

  int endPosition() override;

  int startOffset()  override;

  int endOffset()  override;
};

} // #include  "core/src/java/org/apache/lucene/search/
