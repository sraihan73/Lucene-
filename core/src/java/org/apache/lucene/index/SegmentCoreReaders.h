#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class FieldsProducer;
}

namespace org::apache::lucene::codecs
{
class NormsProducer;
}
namespace org::apache::lucene::codecs
{
class StoredFieldsReader;
}
namespace org::apache::lucene::codecs
{
class TermVectorsReader;
}
namespace org::apache::lucene::codecs
{
class PointsReader;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class FieldInfos;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class ClosedListener;
}
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
}
namespace org::apache::lucene::store
{
class IOContext;
}
namespace org::apache::lucene::index
{
class CacheHelper;
}
namespace org::apache::lucene::index
{
class CacheKey;
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

using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;

/** Holds core readers that are shared (unchanged) when
 * SegmentReader is cloned or reopened */
class SegmentCoreReaders final
    : public std::enable_shared_from_this<SegmentCoreReaders>
{
  GET_CLASS_NAME(SegmentCoreReaders)

  // Counts how many other readers share the core objects
  // (freqStream, proxStream, tis, etc.) of this reader;
  // when coreRef drops to 0, these core objects may be
  // closed.  A given instance of SegmentReader may be
  // closed, even though it shares core objects with other
  // SegmentReaders:
private:
  const std::shared_ptr<std::atomic<uint64_t>> ref = std::make_shared<std::atomic<uint64_t>>(1);

public:
  const std::shared_ptr<FieldsProducer> fields;
  const std::shared_ptr<NormsProducer> normsProducer;

  const std::shared_ptr<StoredFieldsReader> fieldsReaderOrig;
  const std::shared_ptr<TermVectorsReader> termVectorsReaderOrig;
  const std::shared_ptr<PointsReader> pointsReader;
  const std::shared_ptr<Directory> cfsReader;
  const std::wstring segment;
  /**
   * fieldinfos for this core: means gen=-1.
   * this is the exact fieldinfos these codec components saw at write.
   * in the case of DV updates, SR may hold a newer version. */
  const std::shared_ptr<FieldInfos> coreFieldInfos;

  // TODO: make a single thread local w/ a
  // Thingy class holding fieldsReader, termVectorsReader,
  // normsProducer
private:
  class AnonymousInnerClass : public StoredFieldsReader
  {
    GET_CLASS_NAME(CloseableThreadLocalAnonymousInnerClass)
  public:
    AnonymousInnerClass();

  protected:
    std::shared_ptr<StoredFieldsReader> initialValue() override;
  };

private:
  class AnonymousInnerClass2
      : public TermVectorsReader
  {
    GET_CLASS_NAME(AnonymousInnerClass2)
  public:
    AnonymousInnerClass2();

  protected:
    std::shared_ptr<TermVectorsReader> initialValue() override;
  };

public:
  const std::shared_ptr<StoredFieldsReader> fieldsReaderLocal =
    std::make_shared<AnonymousInnerClass>();
  const std::shared_ptr<TermVectorsReader> termVectorsLocal = 
    std::make_shared<AnonymousInnerClass2>();

private:
  // ToDo need mutex
  const std::shared_ptr<std::unordered_set<IndexReader::ClosedListener>> coreClosedListeners =
      std::make_shared<std::unordered_set<IndexReader::ClosedListener>>();

public:
  SegmentCoreReaders(std::shared_ptr<Directory> dir,
                     std::shared_ptr<SegmentCommitInfo> si,
                     std::shared_ptr<IOContext> context) ;

  int getRefCount();

  void incRef();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("try") void decRef() throws
  // java.io.IOException
  void decRef() ;

private:
  class CacheHelperAnonymousInnerClass
      : public std::enable_shared_from_this<CacheHelperAnonymousInnerClass>,
        public IndexReader::CacheHelper
  {
    GET_CLASS_NAME(CacheHelperAnonymousInnerClass)
  public:
    CacheHelperAnonymousInnerClass();

  private:
    const std::shared_ptr<IndexReader::CacheKey> cacheKey =
        std::make_shared<IndexReader::CacheKey>();

  public:
    std::shared_ptr<CacheKey> getKey() override;

    void addClosedListener(ClosedListener listener) override;
  };

  const std::shared_ptr<IndexReader::CacheHelper> cacheHelper =
      std::make_shared<CacheHelperAnonymousInnerClass>();

public:
  std::shared_ptr<IndexReader::CacheHelper> getCacheHelper();

private:
  void notifyCoreClosedListeners() ;

public:
  virtual std::wstring toString();
};

} // namespace org::apache::lucene::index
