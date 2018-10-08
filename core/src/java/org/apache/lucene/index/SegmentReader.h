#pragma once
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"

#include  "core/src/java/org/apache/lucene/index/LeafMetaData.h"
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/index/SegmentCoreReaders.h"
#include  "core/src/java/org/apache/lucene/index/SegmentDocValues.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/store/IOContext.h"
#include  "core/src/java/org/apache/lucene/codecs/TermVectorsReader.h"
#include  "core/src/java/org/apache/lucene/codecs/StoredFieldsReader.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsReader.h"
#include  "core/src/java/org/apache/lucene/codecs/NormsProducer.h"
#include  "core/src/java/org/apache/lucene/codecs/FieldsProducer.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/index/CacheHelper.h"
#include  "core/src/java/org/apache/lucene/index/CacheKey.h"

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

using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using NormsProducer = org::apache::lucene::codecs::NormsProducer;
using PointsReader = org::apache::lucene::codecs::PointsReader;
using StoredFieldsReader = org::apache::lucene::codecs::StoredFieldsReader;
using TermVectorsReader = org::apache::lucene::codecs::TermVectorsReader;
using Directory = org::apache::lucene::store::Directory;
using IOContext = org::apache::lucene::store::IOContext;
using Bits = org::apache::lucene::util::Bits;

/**
 * IndexReader implementation over a single segment.
 * <p>
 * Instances pointing to the same segment (but with different deletes, etc)
 * may share the same core data.
 * @lucene.experimental
 */
class SegmentReader final : public CodecReader
{
  GET_CLASS_NAME(SegmentReader)

private:
  const std::shared_ptr<SegmentCommitInfo> si;
  // this is the original SI that IW uses internally but it's mutated behind the
  // scenes and we don't want this SI to be used for anything. Yet, IW needs
  // this to do maintainance and lookup pooled readers etc.
  const std::shared_ptr<SegmentCommitInfo> originalSi;
  const std::shared_ptr<LeafMetaData> metaData;
  const std::shared_ptr<Bits> liveDocs;

  // Normally set to si.maxDoc - si.delDocCount, unless we
  // were created as an NRT reader from IW, in which case IW
  // tells us the number of live docs:
  // C++ NOTE: Fields cannot have the same name as methods:
  const int numDocs_;

public:
  const std::shared_ptr<SegmentCoreReaders> core;
  const std::shared_ptr<SegmentDocValues> segDocValues;

  /** True if we are holding RAM only liveDocs or DV updates, i.e. the
   * SegmentCommitInfo delGen doesn't match our liveDocs. */
  const bool isNRT;

  const std::shared_ptr<DocValuesProducer> docValuesProducer;
  const std::shared_ptr<FieldInfos> fieldInfos;

  /**
   * Constructs a new SegmentReader with a new core.
   * @throws CorruptIndexException if the index is corrupt
   * @throws IOException if there is a low-level IO error
   */
  SegmentReader(std::shared_ptr<SegmentCommitInfo> si, int createdVersionMajor,
                std::shared_ptr<IOContext> context) ;

  /** Create new SegmentReader sharing core from a previous
   *  SegmentReader and loading new live docs from a new
   *  deletes file.  Used by openIfChanged. */
  SegmentReader(std::shared_ptr<SegmentCommitInfo> si,
                std::shared_ptr<SegmentReader> sr) ;

  /** Create new SegmentReader sharing core from a previous
   *  SegmentReader and using the provided in-memory
   *  liveDocs.  Used by IndexWriter to provide a new NRT
   *  reader */
  SegmentReader(std::shared_ptr<SegmentCommitInfo> si,
                std::shared_ptr<SegmentReader> sr,
                std::shared_ptr<Bits> liveDocs, int numDocs) ;

  /** Create new SegmentReader sharing core from a previous
   *  SegmentReader and using the provided liveDocs, and recording
   *  whether those liveDocs were carried in ram (isNRT=true). */
  SegmentReader(std::shared_ptr<SegmentCommitInfo> si,
                std::shared_ptr<SegmentReader> sr,
                std::shared_ptr<Bits> liveDocs, int numDocs,
                bool isNRT) ;

  /**
   * init most recent DocValues for the current commit
   */
private:
  std::shared_ptr<DocValuesProducer> initDocValuesProducer() ;

  /**
   * init most recent FieldInfos for the current commit
   */
  std::shared_ptr<FieldInfos> initFieldInfos() ;

public:
  std::shared_ptr<Bits> getLiveDocs() override;

protected:
  void doClose()  override;

public:
  std::shared_ptr<FieldInfos> getFieldInfos() override;

  int numDocs() override;

  int maxDoc() override;

  std::shared_ptr<TermVectorsReader> getTermVectorsReader() override;

  std::shared_ptr<StoredFieldsReader> getFieldsReader() override;

  std::shared_ptr<PointsReader> getPointsReader() override;

  std::shared_ptr<NormsProducer> getNormsReader() override;

  std::shared_ptr<DocValuesProducer> getDocValuesReader() override;

  std::shared_ptr<FieldsProducer> getPostingsReader() override;

  virtual std::wstring toString();

  /**
   * Return the name of the segment this reader is reading.
   */
  std::wstring getSegmentName();

  /**
   * Return the SegmentInfoPerCommit of the segment this reader is reading.
   */
  std::shared_ptr<SegmentCommitInfo> getSegmentInfo();

  /** Returns the directory this index resides in. */
  std::shared_ptr<Directory> directory();

private:
  const std::shared_ptr<Set<ClosedListener>> readerClosedListeners =
      std::make_shared<CopyOnWriteArraySet<ClosedListener>>();

public:
  void notifyReaderClosedListeners()  override;

private:
  const std::shared_ptr<IndexReader::CacheHelper> readerCacheHelper =
      std::make_shared<CacheHelperAnonymousInnerClass>();

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

public:
  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  /** Wrap the cache helper of the core to add ensureOpen() calls that make
   *  sure users do not register closed listeners on closed indices. */
private:
  const std::shared_ptr<IndexReader::CacheHelper> coreCacheHelper =
      std::make_shared<CacheHelperAnonymousInnerClass2>();

private:
  class CacheHelperAnonymousInnerClass2
      : public std::enable_shared_from_this<CacheHelperAnonymousInnerClass2>,
        public IndexReader::CacheHelper
  {
    GET_CLASS_NAME(CacheHelperAnonymousInnerClass2)
  public:
    CacheHelperAnonymousInnerClass2();

    std::shared_ptr<CacheKey> getKey() override;

    void addClosedListener(ClosedListener listener) override;
  };

public:
  std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

  std::shared_ptr<LeafMetaData> getMetaData() override;

  /**
   * Returns the original SegmentInfo passed to the segment reader on creation
   * time.
   * {@link #getSegmentInfo()} returns a clone of this instance.
   */
  std::shared_ptr<SegmentCommitInfo> getOriginalSegmentInfo();

protected:
  std::shared_ptr<SegmentReader> shared_from_this()
  {
    return std::static_pointer_cast<SegmentReader>(
        CodecReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
