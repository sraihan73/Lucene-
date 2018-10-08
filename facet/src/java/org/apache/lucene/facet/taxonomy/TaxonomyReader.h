#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class AlreadyClosedException;
}

namespace org::apache::lucene::facet::taxonomy
{
class ParallelTaxonomyArrays;
}
namespace org::apache::lucene::facet::taxonomy
{
class FacetLabel;
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
namespace org::apache::lucene::facet::taxonomy
{

using AlreadyClosedException =
    org::apache::lucene::store::AlreadyClosedException;

/**
 * TaxonomyReader is the read-only interface with which the faceted-search
 * library uses the taxonomy during search time.
 * <P>
 * A TaxonomyReader holds a deque of categories. Each category has a serial
 * number which we call an "ordinal", and a hierarchical "path" name:
 * <UL>
 * <LI>
 * The ordinal is an integer that starts at 0 for the first category (which is
 * always the root category), and grows contiguously as more categories are
 * added; Note that once a category is added, it can never be deleted.
 * <LI>
 * The path is a CategoryPath object specifying the category's position in the
 * hierarchy.
 * </UL>
 * <B>Notes about concurrent access to the taxonomy:</B>
 * <P>
 * An implementation must allow multiple readers to be active concurrently
 * with a single writer. Readers follow so-called "point in time" semantics,
 * i.e., a TaxonomyReader object will only see taxonomy entries which were
 * available at the time it was created. What the writer writes is only
 * available to (new) readers after the writer's commit() is called.
 * <P>
 * In faceted search, two separate indices are used: the main Lucene index,
 * and the taxonomy. Because the main index refers to the categories listed
 * in the taxonomy, it is important to open the taxonomy *after* opening the
 * main index, and it is also necessary to reopen() the taxonomy after
 * reopen()ing the main index.
 * <P>
 * This order is important, otherwise it would be possible for the main index
 * to refer to a category which is not yet visible in the old snapshot of
 * the taxonomy. Note that it is indeed fine for the the taxonomy to be opened
 * after the main index - even a long time after. The reason is that once
 * a category is added to the taxonomy, it can never be changed or deleted,
 * so there is no danger that a "too new" taxonomy not being consistent with
 * an older index.
 *
 * @lucene.experimental
 */
class TaxonomyReader : public std::enable_shared_from_this<TaxonomyReader>
{
  GET_CLASS_NAME(TaxonomyReader)

  /** An iterator over a category's children. */
public:
  class ChildrenIterator : public std::enable_shared_from_this<ChildrenIterator>
  {
    GET_CLASS_NAME(ChildrenIterator)

  private:
    std::deque<int> const siblings;
    int child = 0;

  public:
    ChildrenIterator(int child, std::deque<int> &siblings);

    /**
     * Return the next child ordinal, or {@link TaxonomyReader#INVALID_ORDINAL}
     * if no more children.
     */
    virtual int next();
  };

  /** Sole constructor. */
public:
  TaxonomyReader();

  /**
   * The root category (the category with the empty path) always has the ordinal
   * 0, to which we give a name ROOT_ORDINAL. {@link #getOrdinal(FacetLabel)}
   * of an empty path will always return {@code ROOT_ORDINAL}, and
   * {@link #getPath(int)} with {@code ROOT_ORDINAL} will return the empty path.
   */
  static constexpr int ROOT_ORDINAL = 0;

  /**
   * Ordinals are always non-negative, so a negative ordinal can be used to
   * signify an error. Methods here return INVALID_ORDINAL (-1) in this case.
   */
  static constexpr int INVALID_ORDINAL = -1;

  /**
   * If the taxonomy has changed since the provided reader was opened, open and
   * return a new {@link TaxonomyReader}; else, return {@code null}. The new
   * reader, if not {@code null}, will be the same type of reader as the one
   * given to this method.
   *
   * <p>
   * This method is typically far less costly than opening a fully new
   * {@link TaxonomyReader} as it shares resources with the provided
   * {@link TaxonomyReader}, when possible.
   */
  template <typename T>
  static T openIfChanged(T oldTaxoReader) ;

private:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool closed = false;
  bool closed = false;

  // set refCount to 1 at start
  const std::shared_ptr<AtomicInteger> refCount =
      std::make_shared<AtomicInteger>(1);

  /**
   * performs the actual task of closing the resources that are used by the
   * taxonomy reader.
   */
protected:
  virtual void doClose() = 0;

  /**
   * Implements the actual opening of a new {@link TaxonomyReader} instance if
   * the taxonomy has changed.
   *
   * @see #openIfChanged(TaxonomyReader)
   */
  virtual std::shared_ptr<TaxonomyReader> doOpenIfChanged() = 0;

  /**
   * Throws {@link AlreadyClosedException} if this IndexReader is closed
   */
  void ensureOpen() ;

public:
  ~TaxonomyReader();

  /**
   * Expert: decreases the refCount of this TaxonomyReader instance. If the
   * refCount drops to 0 this taxonomy reader is closed.
   */
  void decRef() ;

  /**
   * Returns a {@link ParallelTaxonomyArrays} object which can be used to
   * efficiently traverse the taxonomy tree.
   */
  virtual std::shared_ptr<ParallelTaxonomyArrays>
  getParallelTaxonomyArrays() = 0;

  /** Returns an iterator over the children of the given ordinal. */
  virtual std::shared_ptr<ChildrenIterator>
  getChildren(int const ordinal) ;

  /**
   * Retrieve user committed data.
   *
   * @see TaxonomyWriter#setLiveCommitData(Iterable)
   */
  virtual std::unordered_map<std::wstring, std::wstring>
  getCommitUserData() = 0;

  /**
   * Returns the ordinal of the category given as a path. The ordinal is the
   * category's serial number, an integer which starts with 0 and grows as more
   * categories are added (note that once a category is added, it can never be
   * deleted).
   *
   * @return the category's ordinal or {@link #INVALID_ORDINAL} if the category
   *         wasn't foun.
   */
  virtual int getOrdinal(std::shared_ptr<FacetLabel> categoryPath) = 0;

  /** Returns ordinal for the dim + path. */
  virtual int getOrdinal(const std::wstring &dim,
                         std::deque<std::wstring> &path) ;

  /** Returns the path name of the category with the given ordinal. */
  virtual std::shared_ptr<FacetLabel> getPath(int ordinal) = 0;

  /** Returns the current refCount for this taxonomy reader. */
  int getRefCount();

  /**
   * Returns the number of categories in the taxonomy. Note that the number of
   * categories returned is often slightly higher than the number of categories
   * inserted into the taxonomy; This is because when a category is added to the
   * taxonomy, its ancestors are also added automatically (including the root,
   * which always get ordinal 0).
   */
  virtual int getSize() = 0;

  /**
   * Expert: increments the refCount of this TaxonomyReader instance. RefCounts
   * can be used to determine when a taxonomy reader can be closed safely, i.e.
   * as soon as there are no more references. Be sure to always call a
   * corresponding decRef(), in a finally clause; otherwise the reader may never
   * be closed.
   */
  void incRef();

  /** Expert: increments the refCount of this TaxonomyReader
   *  instance only if it has not been closed yet.  Returns
   *  true on success. */
  bool tryIncRef();
};

} // namespace org::apache::lucene::facet::taxonomy
