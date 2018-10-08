#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Fields;
}

namespace org::apache::lucene::index
{
class ReaderSlice;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class LeafReader;
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
namespace org::apache::lucene::index
{

using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Provides a single {@link Fields} term index view over an
 * {@link IndexReader}.
 * This is useful when you're interacting with an {@link
 * IndexReader} implementation that consists of sequential
 * sub-readers (eg {@link DirectoryReader} or {@link
 * MultiReader}) and you must treat it as a {@link LeafReader}.
 *
 * <p><b>NOTE</b>: for composite readers, you'll get better
 * performance by gathering the sub readers using
 * {@link IndexReader#getContext()} to get the
 * atomic leaves and then operate per-LeafReader,
 * instead of using this class.
 *
 * @lucene.experimental
 */
class MultiFields final : public Fields
{
  GET_CLASS_NAME(MultiFields)
private:
  std::deque<std::shared_ptr<Fields>> const subs;
  std::deque<std::shared_ptr<ReaderSlice>> const subSlices;
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::unordered_map<std::wstring, std::shared_ptr<Terms>> terms_ =
      std::make_shared<
          ConcurrentHashMap<std::wstring, std::shared_ptr<Terms>>>();

  /** Returns a single {@link Fields} instance for this
   *  reader, merging fields/terms/docs/positions on the
   *  fly.  This method will return null if the reader
   *  has no postings.
   *
   *  <p><b>NOTE</b>: this is a slow way to access postings.
   *  It's better to get the sub-readers and iterate through them
   *  yourself. */
public:
  static std::shared_ptr<Fields>
  getFields(std::shared_ptr<IndexReader> reader) ;

  /** Returns a single {@link Bits} instance for this
   *  reader, merging live Documents on the
   *  fly.  This method will return null if the reader
   *  has no deletions.
   *
   *  <p><b>NOTE</b>: this is a very slow way to access live docs.
   *  For example, each Bits access will require a binary search.
   *  It's better to get the sub-readers and iterate through them
   *  yourself. */
  static std::shared_ptr<Bits> getLiveDocs(std::shared_ptr<IndexReader> reader);

  /** This method may return null if the field does not exist or if it has no
   * terms. */
  static std::shared_ptr<Terms>
  getTerms(std::shared_ptr<IndexReader> r,
           const std::wstring &field) ;

  /** Returns {@link PostingsEnum} for the specified field and
   *  term.  This will return null if the field or term does
   *  not exist. */
  static std::shared_ptr<PostingsEnum>
  getTermDocsEnum(std::shared_ptr<IndexReader> r, const std::wstring &field,
                  std::shared_ptr<BytesRef> term) ;

  /** Returns {@link PostingsEnum} for the specified field and
   *  term, with control over whether freqs are required.
   *  Some codecs may be able to optimize their
   *  implementation when freqs are not required.  This will
   *  return null if the field or term does not exist.  See {@link
   *  TermsEnum#postings(PostingsEnum,int)}.*/
  static std::shared_ptr<PostingsEnum>
  getTermDocsEnum(std::shared_ptr<IndexReader> r, const std::wstring &field,
                  std::shared_ptr<BytesRef> term, int flags) ;

  /** Returns {@link PostingsEnum} for the specified
   *  field and term.  This will return null if the field or
   *  term does not exist or positions were not indexed.
   *  @see #getTermPositionsEnum(IndexReader, std::wstring, BytesRef, int) */
  static std::shared_ptr<PostingsEnum>
  getTermPositionsEnum(std::shared_ptr<IndexReader> r,
                       const std::wstring &field,
                       std::shared_ptr<BytesRef> term) ;

  /** Returns {@link PostingsEnum} for the specified
   *  field and term, with control over whether offsets and payloads are
   *  required.  Some codecs may be able to optimize
   *  their implementation when offsets and/or payloads are not
   *  required. This will return null if the field or term does not
   *  exist. See {@link TermsEnum#postings(PostingsEnum,int)}. */
  static std::shared_ptr<PostingsEnum> getTermPositionsEnum(
      std::shared_ptr<IndexReader> r, const std::wstring &field,
      std::shared_ptr<BytesRef> term, int flags) ;

  /**
   * Expert: construct a new MultiFields instance directly.
   * @lucene.internal
   */
  // TODO: why is this public?
  MultiFields(std::deque<std::shared_ptr<Fields>> &subs,
              std::deque<std::shared_ptr<ReaderSlice>> &subSlices);

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked","rawtypes"}) @Override public
  // java.util.Iterator<std::wstring> iterator()
  std::shared_ptr<Iterator<std::wstring>> iterator() override;

  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  int size() override;

  /** Call this to get the (merged) FieldInfos for a
   *  composite reader.
   *  <p>
   *  NOTE: the returned field numbers will likely not
   *  correspond to the actual field numbers in the underlying
   *  readers, and codec metadata ({@link FieldInfo#getAttribute(std::wstring)}
   *  will be unavailable.
   */
  static std::shared_ptr<FieldInfos>
  getMergedFieldInfos(std::shared_ptr<IndexReader> reader);

  /** Call this to get the (merged) FieldInfos representing the
   *  set of indexed fields <b>only</b> for a composite reader.
   *  <p>
   *  NOTE: the returned field numbers will likely not
   *  correspond to the actual field numbers in the underlying
   *  readers, and codec metadata ({@link FieldInfo#getAttribute(std::wstring)}
   *  will be unavailable.
   */
  static std::shared_ptr<std::deque<std::wstring>>
  getIndexedFields(std::shared_ptr<IndexReader> reader);

private:
  class LeafReaderFields : public Fields
  {
    GET_CLASS_NAME(LeafReaderFields)

  private:
    const std::shared_ptr<LeafReader> leafReader;
    const std::deque<std::wstring> indexedFields;

  public:
    LeafReaderFields(std::shared_ptr<LeafReader> leafReader);

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    int size() override;

    std::shared_ptr<Terms>
    terms(const std::wstring &field)  override;

  protected:
    std::shared_ptr<LeafReaderFields> shared_from_this()
    {
      return std::static_pointer_cast<LeafReaderFields>(
          Fields::shared_from_this());
    }
  };

protected:
  std::shared_ptr<MultiFields> shared_from_this()
  {
    return std::static_pointer_cast<MultiFields>(Fields::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
