#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"

#include  "core/src/java/org/apache/lucene/index/SoftDeletesSubReaderWrapper.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"

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
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

/**
 * This reader filters out documents that have a doc values value in the given
 * field and treat these documents as soft deleted. Hard deleted documents will
 * also be filtered out in the life docs of this reader.
 * @see IndexWriterConfig#setSoftDeletesField(std::wstring)
 * @see IndexWriter#softUpdateDocument(Term, Iterable, Field...)
 * @see SoftDeletesRetentionMergePolicy
 */
class SoftDeletesDirectoryReaderWrapper final : public FilterDirectoryReader
{
  GET_CLASS_NAME(SoftDeletesDirectoryReaderWrapper)
private:
  const std::wstring field;
  const std::shared_ptr<CacheHelper> readerCacheHelper;
  /**
   * Creates a new soft deletes wrapper.
   * @param in the incoming directory reader
   * @param field the soft deletes field
   */
public:
  SoftDeletesDirectoryReaderWrapper(
      std::shared_ptr<DirectoryReader> in_,
      const std::wstring &field) ;

private:
  SoftDeletesDirectoryReaderWrapper(
      std::shared_ptr<DirectoryReader> in_,
      std::shared_ptr<SoftDeletesSubReaderWrapper> wrapper) ;

protected:
  std::shared_ptr<DirectoryReader> doWrapDirectoryReader(
      std::shared_ptr<DirectoryReader> in_)  override;

public:
  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

private:
  class SoftDeletesSubReaderWrapper : public SubReaderWrapper
  {
    GET_CLASS_NAME(SoftDeletesSubReaderWrapper)
  private:
    const std::unordered_map<std::shared_ptr<CacheKey>,
                             std::shared_ptr<LeafReader>>
        mapping;
    const std::wstring field;

  public:
    SoftDeletesSubReaderWrapper(
        std::unordered_map<std::shared_ptr<CacheKey>,
                           std::shared_ptr<LeafReader>> &oldReadersCache,
        const std::wstring &field);

    std::shared_ptr<LeafReader>
    wrap(std::shared_ptr<LeafReader> reader) override;

  protected:
    std::shared_ptr<SoftDeletesSubReaderWrapper> shared_from_this()
    {
      return std::static_pointer_cast<SoftDeletesSubReaderWrapper>(
          SubReaderWrapper::shared_from_this());
    }
  };

public:
  static std::shared_ptr<LeafReader>
  wrap(std::shared_ptr<LeafReader> reader,
       const std::wstring &field) ;

private:
  static bool assertDocCounts(int expectedNumDocs, int numSoftDeletes,
                              std::shared_ptr<LeafReader> reader);

public:
  class SoftDeletesFilterLeafReader final : public FilterLeafReader
  {
    GET_CLASS_NAME(SoftDeletesFilterLeafReader)
  private:
    const std::shared_ptr<LeafReader> reader;
    const std::shared_ptr<FixedBitSet> bits;
    // C++ NOTE: Fields cannot have the same name as methods:
    const int numDocs_;
    const std::shared_ptr<CacheHelper> readerCacheHelper;

    SoftDeletesFilterLeafReader(std::shared_ptr<LeafReader> reader,
                                std::shared_ptr<FixedBitSet> bits, int numDocs);

  public:
    std::shared_ptr<Bits> getLiveDocs() override;

    int numDocs() override;

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<SoftDeletesFilterLeafReader> shared_from_this()
    {
      return std::static_pointer_cast<SoftDeletesFilterLeafReader>(
          FilterLeafReader::shared_from_this());
    }
  };

public:
  class SoftDeletesFilterCodecReader final : public FilterCodecReader
  {
    GET_CLASS_NAME(SoftDeletesFilterCodecReader)
  private:
    const std::shared_ptr<LeafReader> reader;
    const std::shared_ptr<FixedBitSet> bits;
    // C++ NOTE: Fields cannot have the same name as methods:
    const int numDocs_;
    const std::shared_ptr<CacheHelper> readerCacheHelper;

    SoftDeletesFilterCodecReader(std::shared_ptr<CodecReader> reader,
                                 std::shared_ptr<FixedBitSet> bits,
                                 int numDocs);

  public:
    std::shared_ptr<Bits> getLiveDocs() override;

    int numDocs() override;

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<SoftDeletesFilterCodecReader> shared_from_this()
    {
      return std::static_pointer_cast<SoftDeletesFilterCodecReader>(
          FilterCodecReader::shared_from_this());
    }
  };

private:
  class DelegatingCacheHelper
      : public std::enable_shared_from_this<DelegatingCacheHelper>,
        public CacheHelper
  {
    GET_CLASS_NAME(DelegatingCacheHelper)
  private:
    const std::shared_ptr<CacheHelper> delegate_;
    const std::shared_ptr<CacheKey> cacheKey = std::make_shared<CacheKey>();

  public:
    DelegatingCacheHelper(std::shared_ptr<CacheHelper> delegate_);

    std::shared_ptr<CacheKey> getKey() override;

    void addClosedListener(std::shared_ptr<ClosedListener> listener) override;
  };

protected:
  std::shared_ptr<SoftDeletesDirectoryReaderWrapper> shared_from_this()
  {
    return std::static_pointer_cast<SoftDeletesDirectoryReaderWrapper>(
        FilterDirectoryReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
