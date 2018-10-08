#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class SortField;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
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
namespace org::apache::lucene::search
{

/**
 * Encapsulates sort criteria for returned hits.
 *
 * <p>The fields used to determine sort order must be carefully chosen.
 * Documents must contain a single term in such a field,
 * and the value of the term should indicate the document's relative position in
 * a given sort order.  The field must be indexed, but should not be tokenized,
 * and does not need to be stored (unless you happen to want it back with the
 * rest of your document data).  In other words:
 *
 * <p><code>document.add (new Field ("byNumber", Integer.toString(x),
 * Field.Store.NO, Field.Index.NOT_ANALYZED));</code></p>
 *
 *
 * <h3>Valid Types of Values</h3>
 *
 * <p>There are four possible kinds of term values which may be put into
 * sorting fields: Integers, Longs, Floats, or Strings.  Unless
 * {@link SortField SortField} objects are specified, the type of value
 * in the field is determined by parsing the first term in the field.
 *
 * <p>Integer term values should contain only digits and an optional
 * preceding negative sign.  Values must be base 10 and in the range
 * <code>Integer.MIN_VALUE</code> and <code>Integer.MAX_VALUE</code> inclusive.
 * Documents which should appear first in the sort
 * should have low value integers, later documents high values
 * (i.e. the documents should be numbered <code>1..n</code> where
 * <code>1</code> is the first and <code>n</code> the last).
 *
 * <p>Long term values should contain only digits and an optional
 * preceding negative sign.  Values must be base 10 and in the range
 * <code>Long.MIN_VALUE</code> and <code>Long.MAX_VALUE</code> inclusive.
 * Documents which should appear first in the sort
 * should have low value integers, later documents high values.
 *
 * <p>Float term values should conform to values accepted by
 * {@link Float Float.valueOf(std::wstring)} (except that <code>NaN</code>
 * and <code>Infinity</code> are not supported).
 * Documents which should appear first in the sort
 * should have low values, later documents high values.
 *
 * <p>std::wstring term values can contain any valid std::wstring, but should
 * not be tokenized.  The values are sorted according to their
 * {@link Comparable natural order}.  Note that using this type
 * of term value has higher memory requirements than the other
 * two types.
 *
 * <h3>Object Reuse</h3>
 *
 * <p>One of these objects can be
 * used multiple times and the sort order changed between usages.
 *
 * <p>This class is thread safe.
 *
 * <h3>Memory Usage</h3>
 *
 * <p>Sorting uses of caches of term values maintained by the
 * internal HitQueue(s).  The cache is static and contains an integer
 * or float array of length <code>IndexReader.maxDoc()</code> for each field
 * name for which a sort is performed.  In other words, the size of the
 * cache in bytes is:
 *
 * <p><code>4 * IndexReader.maxDoc() * (# of different fields actually used to
 * sort)</code>
 *
 * <p>For std::wstring fields, the cache is larger: in addition to the
 * above array, the value of every term in the field is kept in memory.
 * If there are many unique terms in the field, this could
 * be quite large.
 *
 * <p>Note that the size of the cache is not affected by how many
 * fields are in the index and <i>might</i> be used to sort - only by
 * the ones actually used to sort a result set.
 *
 * <p>Created: Feb 12, 2004 10:53:57 AM
 *
 * @since   lucene 1.4
 */
class Sort : public std::enable_shared_from_this<Sort>
{
  GET_CLASS_NAME(Sort)

  /**
   * Represents sorting by computed relevance. Using this sort criteria returns
   * the same results as calling
   * {@link IndexSearcher#search(Query,int) IndexSearcher#search()}without a
   * sort criteria, only with slightly more overhead.
   */
public:
  static const std::shared_ptr<Sort> RELEVANCE;

  /** Represents sorting by index order. */
  static const std::shared_ptr<Sort> INDEXORDER;

  // internal representation of the sort criteria
  std::deque<std::shared_ptr<SortField>> fields;

  /**
   * Sorts by computed relevance. This is the same sort criteria as calling
   * {@link IndexSearcher#search(Query,int) IndexSearcher#search()}without a
   * sort criteria, only with slightly more overhead.
   */
  Sort();

  /** Sorts by the criteria in the given SortField. */
  Sort(std::shared_ptr<SortField> field);

  /** Sets the sort to the given criteria in succession: the
   *  first SortField is checked first, but if it produces a
   *  tie, then the second SortField is used to break the tie,
   *  etc.  Finally, if there is still a tie after all SortFields
   *  are checked, the internal Lucene docid is used to break it. */
  Sort(std::deque<SortField> &fields);

  /** Sets the sort to the given criteria. */
  virtual void setSort(std::shared_ptr<SortField> field);

  /** Sets the sort to the given criteria in succession: the
   *  first SortField is checked first, but if it produces a
   *  tie, then the second SortField is used to break the tie,
   *  etc.  Finally, if there is still a tie after all SortFields
   *  are checked, the internal Lucene docid is used to break it. */
  virtual void setSort(std::deque<SortField> &fields);

  /**
   * Representation of the sort criteria.
   * @return Array of SortField objects used in this sort criteria
   */
  virtual std::deque<std::shared_ptr<SortField>> getSort();

  /**
   * Rewrites the SortFields in this Sort, returning a new Sort if any of the
   * fields changes during their rewriting.
   *
   * @param searcher IndexSearcher to use in the rewriting
   * @return {@code this} if the Sort/Fields have not changed, or a new Sort if
   * there is a change
   * @throws IOException Can be thrown by the rewriting
   * @lucene.experimental
   */
  virtual std::shared_ptr<Sort>
  rewrite(std::shared_ptr<IndexSearcher> searcher) ;

  virtual std::wstring toString();

  /** Returns true if <code>o</code> is equal to this. */
  virtual bool equals(std::any o);

  /** Returns a hash code value for this object. */
  virtual int hashCode();

  /** Returns true if the relevance score is needed to sort documents. */
  virtual bool needsScores();
};

} // namespace org::apache::lucene::search
