#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>

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
 * This abstract class defines methods to iterate over a set of non-decreasing
 * doc ids. Note that this class assumes it iterates on doc Ids, and therefore
 * {@link #NO_MORE_DOCS} is set to {@value #NO_MORE_DOCS} in order to be used as
 * a sentinel object. Implementations of this class are expected to consider
 * {@link Integer#MAX_VALUE} as an invalid value.
GET_CLASS_NAME(are)
 */
class DocIdSetIterator : public std::enable_shared_from_this<DocIdSetIterator>
{
  GET_CLASS_NAME(DocIdSetIterator)

  /** An empty {@code DocIdSetIterator} instance */
public:
  static std::shared_ptr<DocIdSetIterator> empty();

private:
  class DocIdSetIteratorAnonymousInnerClass;

  /** A {@link DocIdSetIterator} that matches all documents up to
   *  {@code maxDoc - 1}. */
public:
  static std::shared_ptr<DocIdSetIterator> all(int maxDoc);

private:
  class DocIdSetIteratorAnonymousInnerClass2;

  /**
   * When returned by {@link #nextDoc()}, {@link #advance(int)} and
   * {@link #docID()} it means there are no more docs in the iterator.
   */
public:
  static const int NO_MORE_DOCS = std::numeric_limits<int>::max();

  /**
   * Returns the following:
   * <ul>
   * <li><code>-1</code> if {@link #nextDoc()} or
   * {@link #advance(int)} were not called yet.
   * <li>{@link #NO_MORE_DOCS} if the iterator has exhausted.
   * <li>Otherwise it should return the doc ID it is currently on.
   * </ul>
   * <p>
   *
   * @since 2.9
   */
  virtual int docID() = 0;

  /**
   * Advances to the next document in the set and returns the doc it is
   * currently on, or {@link #NO_MORE_DOCS} if there are no more docs in the
   * set.<br>
   *
   * <b>NOTE:</b> after the iterator has exhausted you should not call this
   * method, as it may result in unpredicted behavior.
   *
   * @since 2.9
   */
  virtual int nextDoc() = 0;

  /**
    * Advances to the first beyond the current whose document number is greater
    * than or equal to <i>target</i>, and returns the document number itself.
    * Exhausts the iterator and returns {@link #NO_MORE_DOCS} if <i>target</i>
    * is greater than the highest document number in the set.
    * <p>
    * The behavior of this method is <b>undefined</b> when called with
    * <code> target &le; current</code>, or after the iterator has exhausted.
    * Both cases may result in unpredicted behavior.
    * <p>
    * When <code> target &gt; current</code> it behaves as if written:
    *
    * <pre class="prettyprint">
    * int advance(int target) {
GET_CLASS_NAME(="prettyprint">)
    *   int doc;
    *   while ((doc = nextDoc()) &lt; target) {
    *   }
    *   return doc;
    * }
    * </pre>
    *
    * Some implementations are considerably more efficient than that.
    * <p>
    * <b>NOTE:</b> this method may be called with {@link #NO_MORE_DOCS} for
    * efficiency by some Scorers. If your implementation cannot efficiently
    * determine that it should exhaust, it is recommended that you check for
that
    * value in each call to this method.
    * <p>
    *
    * @since 2.9
    */
  virtual int advance(int target) = 0;

  /** Slow (linear) implementation of {@link #advance} relying on
   *  {@link #nextDoc()} to advance beyond the target position. */
protected:
  int slowAdvance(int target) ;

  /**
   * Returns the estimated cost of this {@link DocIdSetIterator}.
   * <p>
   * This is generally an upper bound of the number of documents this iterator
   * might match, but may be a rough heuristic, hardcoded value, or otherwise
   * completely inaccurate.
   */
public:
  virtual int64_t cost() = 0;
};
class DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass
    : public DocIdSetIterator
{
  GET_CLASS_NAME(DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass)
public:
  DocIdSetIteratorAnonymousInnerClass();

  bool exhausted = false;

  int advance(int target) override;

  int docID() override;
  int nextDoc() override;

  int64_t cost() override;

protected:
  std::shared_ptr<DocIdSetIteratorAnonymousInnerClass> shared_from_this()
  {
    return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass>(
        DocIdSetIterator::shared_from_this());
  }
};
class DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass2
    : public DocIdSetIterator
{
  GET_CLASS_NAME(DocIdSetIterator::DocIdSetIteratorAnonymousInnerClass2)
private:
  int maxDoc = 0;

public:
  DocIdSetIteratorAnonymousInnerClass2(int maxDoc);

  int doc = -1;

  int docID() override;

  int nextDoc()  override;

  int advance(int target)  override;

  int64_t cost() override;

protected:
  std::shared_ptr<DocIdSetIteratorAnonymousInnerClass2> shared_from_this()
  {
    return std::static_pointer_cast<DocIdSetIteratorAnonymousInnerClass2>(
        DocIdSetIterator::shared_from_this());
  }
};
} // namespace org::apache::lucene::search

