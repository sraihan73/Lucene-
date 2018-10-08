#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include "core/src/java/org/apache/lucene/index/IndexReader.h"

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/index/PostingsEnum.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/index/LeafMetaData.h"

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
namespace org::apache::lucene::index
{

using Bits = org::apache::lucene::util::Bits;

/** {@code LeafReader} is an abstract class, providing an interface for
 accessing an index.  Search of an index is done entirely through this abstract
 interface, so that any subclass which implements it is searchable. IndexReaders
 implemented by this subclass do not consist of several sub-readers, they are
 atomic. They support retrieval of stored fields, doc values, terms, and
 postings.

 <p>For efficiency, in this API documents are often referred to via
 <i>document numbers</i>, non-negative integers which each name a unique
 document in the index.  These document numbers are ephemeral -- they may change
 as documents are added to and deleted from an index.  Clients should thus not
 rely on a given document having the same number between sessions.

 <p>
 <a name="thread-safety"></a><p><b>NOTE</b>: {@link
 IndexReader} instances are completely thread
 safe, meaning multiple threads can call any of its methods,
 concurrently.  If your application requires external
 synchronization, you should <b>not</b> synchronize on the
 <code>IndexReader</code> instance; use your own
 (non-Lucene) objects instead.
*/
class LeafReader : public IndexReader
{
  GET_CLASS_NAME(LeafReader)

private:
  const std::shared_ptr<LeafReaderContext> readerContext =
      std::make_shared<LeafReaderContext>(shared_from_this());

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  LeafReader();

public:
  std::shared_ptr<IndexReaderContext> getContext() override final;

  /**
   * Optional method: Return a {@link IndexReader.CacheHelper} that can be used
   * to cache based on the content of this leaf regardless of deletions. Two
   * readers that have the same data but different sets of deleted documents or
   * doc values updates may be considered equal. Consider using
   * {@link #getReaderCacheHelper} if you need deletions or dv updates to be
   * taken into account.
   * <p>A return value of {@code null} indicates that this reader is not suited
   * for caching, which is typically the case for short-lived wrappers that
   * alter the content of the wrapped leaf reader.
   * @lucene.experimental
   */
  virtual std::shared_ptr<CacheHelper> getCoreCacheHelper() = 0;

  int docFreq(std::shared_ptr<Term> term)  override final;

  /** Returns the number of documents containing the term
   * <code>t</code>.  This method returns 0 if the term or
   * field does not exists.  This method does not take into
   * account deleted documents that have not yet been merged
   * away. */
  int64_t
  totalTermFreq(std::shared_ptr<Term> term)  override final;

  int64_t
  getSumDocFreq(const std::wstring &field)  override final;

  int getDocCount(const std::wstring &field)  override final;

  int64_t getSumTotalTermFreq(const std::wstring &field) override final;

  /** Returns the {@link Terms} index for this field, or null if it has none. */
  virtual std::shared_ptr<Terms> terms(const std::wstring &field) = 0;

  /** Returns {@link PostingsEnum} for the specified term.
   *  This will return null if either the field or
   *  term does not exist.
   *  <p><b>NOTE:</b> The returned {@link PostingsEnum} may contain deleted
   * docs.
   *  @see TermsEnum#postings(PostingsEnum) */
  std::shared_ptr<PostingsEnum> postings(std::shared_ptr<Term> term,
                                         int flags) ;

  /** Returns {@link PostingsEnum} for the specified term
   *  with {@link PostingsEnum#FREQS}.
   *  <p>
   *  Use this method if you only require documents and frequencies,
   *  and do not need any proximity data.
   *  This method is equivalent to
   *  {@link #postings(Term, int) postings(term, PostingsEnum.FREQS)}
   *  <p><b>NOTE:</b> The returned {@link PostingsEnum} may contain deleted
   * docs.
   *  @see #postings(Term, int)
   */
  std::shared_ptr<PostingsEnum>
  postings(std::shared_ptr<Term> term) ;

  /** Returns {@link NumericDocValues} for this field, or
   *  null if no numeric doc values were indexed for
   *  this field.  The returned instance should only be
   *  used by a single thread. */
  virtual std::shared_ptr<NumericDocValues>
  getNumericDocValues(const std::wstring &field) = 0;

  /** Returns {@link BinaryDocValues} for this field, or
   *  null if no binary doc values were indexed for
   *  this field.  The returned instance should only be
   *  used by a single thread. */
  virtual std::shared_ptr<BinaryDocValues>
  getBinaryDocValues(const std::wstring &field) = 0;

  /** Returns {@link SortedDocValues} for this field, or
   *  null if no {@link SortedDocValues} were indexed for
   *  this field.  The returned instance should only be
   *  used by a single thread. */
  virtual std::shared_ptr<SortedDocValues>
  getSortedDocValues(const std::wstring &field) = 0;

  /** Returns {@link SortedNumericDocValues} for this field, or
   *  null if no {@link SortedNumericDocValues} were indexed for
   *  this field.  The returned instance should only be
   *  used by a single thread. */
  virtual std::shared_ptr<SortedNumericDocValues>
  getSortedNumericDocValues(const std::wstring &field) = 0;

  /** Returns {@link SortedSetDocValues} for this field, or
   *  null if no {@link SortedSetDocValues} were indexed for
   *  this field.  The returned instance should only be
   *  used by a single thread. */
  virtual std::shared_ptr<SortedSetDocValues>
  getSortedSetDocValues(const std::wstring &field) = 0;

  /** Returns {@link NumericDocValues} representing norms
   *  for this field, or null if no {@link NumericDocValues}
   *  were indexed. The returned instance should only be
   *  used by a single thread. */
  virtual std::shared_ptr<NumericDocValues>
  getNormValues(const std::wstring &field) = 0;

  /**
   * Get the {@link FieldInfos} describing all fields in
   * this reader.
   * @lucene.experimental
   */
  virtual std::shared_ptr<FieldInfos> getFieldInfos() = 0;

  /** Returns the {@link Bits} representing live (not
   *  deleted) docs.  A set bit indicates the doc ID has not
   *  been deleted.  If this method returns null it means
   *  there are no deleted documents (all documents are
   *  live).
   *
   *  The returned instance has been safely published for
   *  use by multiple threads without additional
   *  synchronization.
   */
  virtual std::shared_ptr<Bits> getLiveDocs() = 0;

  /** Returns the {@link PointValues} used for numeric or
   *  spatial searches for the given field, or null if there
   *  are no point fields. */
  virtual std::shared_ptr<PointValues>
  getPointValues(const std::wstring &field) = 0;

  /**
   * Checks consistency of this reader.
   * <p>
   * Note that this may be costly in terms of I/O, e.g.
   * may involve computing a checksum value against large data files.
   * @lucene.internal
   */
  virtual void checkIntegrity() = 0;

  /**
   * Return metadata about this leaf.
   * @lucene.experimental */
  virtual std::shared_ptr<LeafMetaData> getMetaData() = 0;

protected:
  std::shared_ptr<LeafReader> shared_from_this()
  {
    return std::static_pointer_cast<LeafReader>(
        IndexReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
