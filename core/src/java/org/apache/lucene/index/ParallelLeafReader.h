#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class FieldInfos;
}

namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::index
{
class LeafMetaData;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::index
{
class StoredFieldVisitor;
}
namespace org::apache::lucene::index
{
class Fields;
}
namespace org::apache::lucene::index
{
class NumericDocValues;
}
namespace org::apache::lucene::index
{
class BinaryDocValues;
}
namespace org::apache::lucene::index
{
class SortedDocValues;
}
namespace org::apache::lucene::index
{
class SortedNumericDocValues;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::index
{
class PointValues;
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

/** An {@link LeafReader} which reads multiple, parallel indexes.  Each index
 * added must have the same number of documents, but typically each contains
 * different fields. Deletions are taken from the first reader.
 * Each document contains the union of the fields of all documents
 * with the same document number.  When searching, matches for a
 * query term are from the first index added that has the field.
 *
 * <p>This is useful, e.g., with collections that have large fields which
 * change rarely and small fields that change more frequently.  The smaller
 * fields may be re-indexed in a new index and both indexes may be searched
 * together.
 *
 * <p><strong>Warning:</strong> It is up to you to make sure all indexes
 * are created and modified the same way. For example, if you add
 * documents to one index, you need to add the same documents in the
 * same order to the other indexes. <em>Failure to do so will result in
 * undefined behavior</em>.
 */
class ParallelLeafReader : public LeafReader
{
  GET_CLASS_NAME(ParallelLeafReader)
private:
  const std::shared_ptr<FieldInfos> fieldInfos;
  std::deque<std::shared_ptr<LeafReader>> const parallelReaders,
      storedFieldsReaders;
  const std::shared_ptr<Set<std::shared_ptr<LeafReader>>> completeReaderSet =
      Collections::newSetFromMap(
          std::make_shared<
              IdentityHashMap<std::shared_ptr<LeafReader>, bool>>());
  const bool closeSubReaders;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int maxDoc_, numDocs_;
  const bool hasDeletions;
  const std::shared_ptr<LeafMetaData> metaData;
  const std::shared_ptr<SortedMap<std::wstring, std::shared_ptr<LeafReader>>>
      tvFieldToReader = std::map_obj<std::wstring, std::shared_ptr<LeafReader>>();
  const std::shared_ptr<SortedMap<std::wstring, std::shared_ptr<LeafReader>>>
      fieldToReader =
          std::map_obj<std::wstring,
                   std::shared_ptr<LeafReader>>(); // TODO needn't sort?
  const std::unordered_map<std::wstring, std::shared_ptr<LeafReader>>
      termsFieldToReader =
          std::unordered_map<std::wstring, std::shared_ptr<LeafReader>>();

  /** Create a ParallelLeafReader based on the provided
   *  readers; auto-closes the given readers on {@link #close()}. */
public:
  ParallelLeafReader(std::deque<LeafReader> &readers) ;

  /** Create a ParallelLeafReader based on the provided
   *  readers. */
  ParallelLeafReader(bool closeSubReaders,
                     std::deque<LeafReader> &readers) ;

  /** Expert: create a ParallelLeafReader based on the provided
   *  readers and storedFieldReaders; when a document is
   *  loaded, only storedFieldsReaders will be used. */
  ParallelLeafReader(bool closeSubReaders,
                     std::deque<std::shared_ptr<LeafReader>> &readers,
                     std::deque<std::shared_ptr<LeafReader>>
                         &storedFieldsReaders) ;

  virtual std::wstring toString();

  // Single instance of this, per ParallelReader instance
private:
  class ParallelFields final : public Fields
  {
    GET_CLASS_NAME(ParallelFields)
  public:
    const std::unordered_map<std::wstring, std::shared_ptr<Terms>> fields =
        std::map_obj<std::wstring, std::shared_ptr<Terms>>();

    ParallelFields();

    void addField(const std::wstring &fieldName, std::shared_ptr<Terms> terms);

    std::shared_ptr<Iterator<std::wstring>> iterator() override;

    std::shared_ptr<Terms> terms(const std::wstring &field) override;

    int size() override;

  protected:
    std::shared_ptr<ParallelFields> shared_from_this()
    {
      return std::static_pointer_cast<ParallelFields>(
          Fields::shared_from_this());
    }
  };

  /**
   * {@inheritDoc}
   * <p>
   * NOTE: the returned field numbers will likely not
   * correspond to the actual field numbers in the underlying
   * readers, and codec metadata ({@link FieldInfo#getAttribute(std::wstring)}
   * will be unavailable.
   */
public:
  std::shared_ptr<FieldInfos> getFieldInfos() override;

  std::shared_ptr<Bits> getLiveDocs() override;

  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  int numDocs() override;

  int maxDoc() override;

  void document(int docID, std::shared_ptr<StoredFieldVisitor> visitor) throw(
      IOException) override;

  std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  std::shared_ptr<Fields> getTermVectors(int docID)  override;

protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void doClose()  override;

public:
  std::shared_ptr<NumericDocValues>
  getNumericDocValues(const std::wstring &field)  override;

  std::shared_ptr<BinaryDocValues>
  getBinaryDocValues(const std::wstring &field)  override;

  std::shared_ptr<SortedDocValues>
  getSortedDocValues(const std::wstring &field)  override;

  std::shared_ptr<SortedNumericDocValues> getSortedNumericDocValues(
      const std::wstring &field)  override;

  std::shared_ptr<SortedSetDocValues>
  getSortedSetDocValues(const std::wstring &field)  override;

  std::shared_ptr<NumericDocValues>
  getNormValues(const std::wstring &field)  override;

  std::shared_ptr<PointValues>
  getPointValues(const std::wstring &fieldName)  override;

  void checkIntegrity()  override;

  /** Returns the {@link LeafReader}s that were passed on init. */
  virtual std::deque<std::shared_ptr<LeafReader>> getParallelReaders();

  std::shared_ptr<LeafMetaData> getMetaData() override;

protected:
  std::shared_ptr<ParallelLeafReader> shared_from_this()
  {
    return std::static_pointer_cast<ParallelLeafReader>(
        LeafReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
