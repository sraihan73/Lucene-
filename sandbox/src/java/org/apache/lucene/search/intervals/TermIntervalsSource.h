#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/intervals/IntervalIterator.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"

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

namespace org::apache::lucene::search::intervals
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;

class TermIntervalsSource : public IntervalsSource
{
  GET_CLASS_NAME(TermIntervalsSource)

public:
  const std::shared_ptr<BytesRef> term;

  TermIntervalsSource(std::shared_ptr<BytesRef> term);

  std::shared_ptr<IntervalIterator>
  intervals(const std::wstring &field,
            std::shared_ptr<LeafReaderContext> ctx)  override;

private:
  class IntervalIteratorAnonymousInnerClass : public IntervalIterator
  {
    GET_CLASS_NAME(IntervalIteratorAnonymousInnerClass)
  private:
    std::shared_ptr<TermIntervalsSource> outerInstance;

    std::shared_ptr<PostingsEnum> pe;
    float cost = 0;

  public:
    IntervalIteratorAnonymousInnerClass(
        std::shared_ptr<TermIntervalsSource> outerInstance,
        std::shared_ptr<PostingsEnum> pe, float cost);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

    int64_t cost() override;

    int pos = 0;

    int start() override;

    int end() override;

    int nextInterval()  override;

    float matchCost() override;

  private:
    void reset() ;

  public:
    virtual std::wstring toString();

  protected:
    std::shared_ptr<IntervalIteratorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<IntervalIteratorAnonymousInnerClass>(
          IntervalIterator::shared_from_this());
    }
  };

public:
  virtual int hashCode();

  virtual bool equals(std::any o);

  virtual std::wstring toString();

  void extractTerms(const std::wstring &field,
                    std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

  /** A guess of
   * the average number of simple operations for the initial seek and buffer
   * refill per document for the positions of a term. See also {@link
   * Lucene50PostingsReader.BlockPostingsEnum#nextPosition()}. <p> Aside:
   * Instead of being constant this could depend among others on
   * {@link Lucene50PostingsFormat#BLOCK_SIZE},
   * {@link TermsEnum#docFreq()},
   * {@link TermsEnum#totalTermFreq()},
   * {@link DocIdSetIterator#cost()} (expected number of matching docs),
   * {@link LeafReader#maxDoc()} (total number of docs in the segment),
   * and the seek time and block size of the device storing the index.
   */
private:
  static constexpr int TERM_POSNS_SEEK_OPS_PER_DOC = 128;

  /** Number of simple operations in {@link
   * Lucene50PostingsReader.BlockPostingsEnum#nextPosition()} when no seek or
   * buffer refill is done.
   */
  static constexpr int TERM_OPS_PER_POS = 7;

  /** Returns an expected cost in simple operations
   *  of processing the occurrences of a term
   *  in a document that contains the term.
   *  This is for use by {@link TwoPhaseIterator#matchCost} implementations.
   *  @param termsEnum The term is the term at which this TermsEnum is
   * positioned.
   */
public:
  static float
  termPositionsCost(std::shared_ptr<TermsEnum> termsEnum) ;

protected:
  std::shared_ptr<TermIntervalsSource> shared_from_this()
  {
    return std::static_pointer_cast<TermIntervalsSource>(
        IntervalsSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/intervals/
