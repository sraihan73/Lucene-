#pragma once
#include "stringhelper.h"
#include <memory>
#include "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"

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

/**
 * Returned by {@link Scorer#twoPhaseIterator()}
 * to expose an approximation of a {@link DocIdSetIterator}.
 * When the {@link #approximation()}'s
 * {@link DocIdSetIterator#nextDoc()} or {@link DocIdSetIterator#advance(int)}
 * return, {@link #matches()} needs to be checked in order to know whether the
 * returned doc ID actually matches.
 * @lucene.experimental
 */
class TwoPhaseIterator : public std::enable_shared_from_this<TwoPhaseIterator>
{
  GET_CLASS_NAME(TwoPhaseIterator)

protected:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<DocIdSetIterator> approximation_;

  /** Takes the approximation to be returned by {@link #approximation}. Not
   * null. */
  TwoPhaseIterator(std::shared_ptr<DocIdSetIterator> approximation);

  /** Return a {@link DocIdSetIterator} view of the provided
   *  {@link TwoPhaseIterator}. */
public:
  static std::shared_ptr<DocIdSetIterator>
  asDocIdSetIterator(std::shared_ptr<TwoPhaseIterator> twoPhaseIterator);

  /**
   * If the given {@link DocIdSetIterator} has been created with
   * {@link #asDocIdSetIterator}, then this will return the wrapped
   * {@link TwoPhaseIterator}. Otherwise this returns {@code null}.
   */
  static std::shared_ptr<TwoPhaseIterator>
  unwrap(std::shared_ptr<DocIdSetIterator> iterator);

private:
  class TwoPhaseIteratorAsDocIdSetIterator : public DocIdSetIterator
  {
    GET_CLASS_NAME(TwoPhaseIteratorAsDocIdSetIterator)

  public:
    const std::shared_ptr<TwoPhaseIterator> twoPhaseIterator;
    const std::shared_ptr<DocIdSetIterator> approximation;

    TwoPhaseIteratorAsDocIdSetIterator(
        std::shared_ptr<TwoPhaseIterator> twoPhaseIterator);

    int docID() override;

    int nextDoc()  override;

    int advance(int target)  override;

  private:
    int doNext(int doc) ;

  public:
    int64_t cost() override;

  protected:
    std::shared_ptr<TwoPhaseIteratorAsDocIdSetIterator> shared_from_this()
    {
      return std::static_pointer_cast<TwoPhaseIteratorAsDocIdSetIterator>(
          DocIdSetIterator::shared_from_this());
    }
  };

  /** Return an approximation. The returned {@link DocIdSetIterator} is a
   *  superset of the matching documents, and each match needs to be confirmed
   *  with {@link #matches()} in order to know whether it matches or not. */
public:
  virtual std::shared_ptr<DocIdSetIterator> approximation();

  /** Return whether the current doc ID that {@link #approximation()} is on
   * matches. This method should only be called when the iterator is positioned
   * -- ie. not when {@link DocIdSetIterator#docID()} is {@code -1} or
   *  {@link DocIdSetIterator#NO_MORE_DOCS} -- and at most once. */
  virtual bool matches() = 0;

  /** An estimate of the expected cost to determine that a single document
   * {@link #matches()}. This can be called before iterating the documents of
   * {@link #approximation()}. Returns an expected cost in number of simple
   * operations like addition, multiplication, comparing two numbers and
   * indexing an array. The returned value must be positive.
   */
  virtual float matchCost() = 0;
};

} // namespace org::apache::lucene::search
