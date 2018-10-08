#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexWriter;
}

namespace org::apache::lucene::index
{
class SegmentInfos;
}
namespace org::apache::lucene::index
{
class LeafReader;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::index
{
class IndexCommit;
}
namespace org::apache::lucene::index
{
class SegmentReader;
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

using Directory = org::apache::lucene::store::Directory;

/** Default implementation of {@link DirectoryReader}. */
class StandardDirectoryReader final : public DirectoryReader
{
  GET_CLASS_NAME(StandardDirectoryReader)

public:
  const std::shared_ptr<IndexWriter> writer;
  const std::shared_ptr<SegmentInfos> segmentInfos;

private:
  const bool applyAllDeletes;
  const bool writeAllDeletes;

  /** called only from static open() methods */
public:
  StandardDirectoryReader(std::shared_ptr<Directory> directory,
                          std::deque<std::shared_ptr<LeafReader>> &readers,
                          std::shared_ptr<IndexWriter> writer,
                          std::shared_ptr<SegmentInfos> sis,
                          bool applyAllDeletes,
                          bool writeAllDeletes) ;

  /** called from DirectoryReader.open(...) methods */
  static std::shared_ptr<DirectoryReader>
  open(std::shared_ptr<Directory> directory,
       std::shared_ptr<IndexCommit> commit) ;

private:
  class FindSegmentsFileAnonymousInnerClass
      : public SegmentInfos::FindSegmentsFile<std::shared_ptr<DirectoryReader>>
  {
    GET_CLASS_NAME(FindSegmentsFileAnonymousInnerClass)
  private:
    std::shared_ptr<Directory> directory;

  public:
    FindSegmentsFileAnonymousInnerClass(std::shared_ptr<Directory> directory);

  protected:
    std::shared_ptr<DirectoryReader>
    doBody(const std::wstring &segmentFileName)  override;

  protected:
    std::shared_ptr<FindSegmentsFileAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FindSegmentsFileAnonymousInnerClass>(
          SegmentInfos.FindSegmentsFile<DirectoryReader>::shared_from_this());
    }
  };

  /** Used by near real-time search */
public:
  static std::shared_ptr<DirectoryReader>
  open(std::shared_ptr<IndexWriter> writer, std::shared_ptr<SegmentInfos> infos,
       bool applyAllDeletes, bool writeAllDeletes) ;

  /** This constructor is only used for {@link #doOpenIfChanged(SegmentInfos)},
   * as well as NRT replication.
   *
   *  @lucene.internal */
  template <typename T1>
  // C++ TODO: There is no native C++ template equivalent to this generic
  // constraint: ORIGINAL LINE: public static DirectoryReader
  // open(org.apache.lucene.store.Directory directory, SegmentInfos infos,
  // java.util.List<? extends LeafReader> oldReaders) throws java.io.IOException
  static std::shared_ptr<DirectoryReader>
  open(std::shared_ptr<Directory> directory,
       std::shared_ptr<SegmentInfos> infos,
       std::deque<T1> oldReaders) ;

  // TODO: move somewhere shared if it's useful elsewhere
private:
  static void decRefWhileHandlingException(
      std::deque<std::shared_ptr<SegmentReader>> &readers);

public:
  virtual std::wstring toString();

protected:
  std::shared_ptr<DirectoryReader>
  doOpenIfChanged()  override;

  std::shared_ptr<DirectoryReader> doOpenIfChanged(
      std::shared_ptr<IndexCommit> commit)  override;

  std::shared_ptr<DirectoryReader>
  doOpenIfChanged(std::shared_ptr<IndexWriter> writer,
                  bool applyAllDeletes)  override;

private:
  std::shared_ptr<DirectoryReader>
  doOpenFromWriter(std::shared_ptr<IndexCommit> commit) ;

  std::shared_ptr<DirectoryReader>
  doOpenNoWriter(std::shared_ptr<IndexCommit> commit) ;

  std::shared_ptr<DirectoryReader>
  doOpenFromCommit(std::shared_ptr<IndexCommit> commit) ;

private:
  class FindSegmentsFileAnonymousInnerClass2
      : public SegmentInfos::FindSegmentsFile<std::shared_ptr<DirectoryReader>>
  {
    GET_CLASS_NAME(FindSegmentsFileAnonymousInnerClass2)
  private:
    std::shared_ptr<StandardDirectoryReader> outerInstance;

  public:
    FindSegmentsFileAnonymousInnerClass2(
        std::shared_ptr<StandardDirectoryReader> outerInstance,
        std::shared_ptr<Directory> directory);

  protected:
    std::shared_ptr<DirectoryReader>
    doBody(const std::wstring &segmentFileName)  override;

  protected:
    std::shared_ptr<FindSegmentsFileAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<FindSegmentsFileAnonymousInnerClass2>(
          SegmentInfos.FindSegmentsFile<DirectoryReader>::shared_from_this());
    }
  };

public:
  std::shared_ptr<DirectoryReader>
  doOpenIfChanged(std::shared_ptr<SegmentInfos> infos) ;

  int64_t getVersion() override;

  /** Return the {@link SegmentInfos} for this reader.
   *
   * @lucene.internal */
  std::shared_ptr<SegmentInfos> getSegmentInfos();

  bool isCurrent()  override;

protected:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Override @SuppressWarnings("try") protected void doClose()
  // throws java.io.IOException
  void doClose()  override;

public:
  std::shared_ptr<IndexCommit> getIndexCommit()  override;

public:
  class ReaderCommit final : public IndexCommit
  {
    GET_CLASS_NAME(ReaderCommit)
  private:
    std::wstring segmentsFileName;

  public:
    std::shared_ptr<std::deque<std::wstring>> files;
    std::shared_ptr<Directory> dir;
    int64_t generation = 0;
    const std::unordered_map<std::wstring, std::wstring> userData;

  private:
    const int segmentCount;
    const std::shared_ptr<StandardDirectoryReader> reader;

  public:
    ReaderCommit(std::shared_ptr<StandardDirectoryReader> reader,
                 std::shared_ptr<SegmentInfos> infos,
                 std::shared_ptr<Directory> dir) ;

    virtual std::wstring toString();

    int getSegmentCount() override;

    std::wstring getSegmentsFileName() override;

    std::shared_ptr<std::deque<std::wstring>> getFileNames() override;

    std::shared_ptr<Directory> getDirectory() override;

    int64_t getGeneration() override;

    bool isDeleted() override;

    std::unordered_map<std::wstring, std::wstring> getUserData() override;

    void delete_() override;

    std::shared_ptr<StandardDirectoryReader> getReader() override;

  protected:
    std::shared_ptr<ReaderCommit> shared_from_this()
    {
      return std::static_pointer_cast<ReaderCommit>(
          IndexCommit::shared_from_this());
    }
  };

private:
  const std::shared_ptr<Set<std::shared_ptr<ClosedListener>>>
      readerClosedListeners = std::make_shared<
          CopyOnWriteArraySet<std::shared_ptr<ClosedListener>>>();

  const std::shared_ptr<CacheHelper> cacheHelper =
      std::make_shared<CacheHelperAnonymousInnerClass>();

private:
  class CacheHelperAnonymousInnerClass : public CacheHelper
  {
    GET_CLASS_NAME(CacheHelperAnonymousInnerClass)
  public:
    CacheHelperAnonymousInnerClass();

  private:
    const std::shared_ptr<CacheKey> cacheKey = std::make_shared<CacheKey>();

  public:
    std::shared_ptr<CacheKey> getKey() override;

    void addClosedListener(std::shared_ptr<ClosedListener> listener) override;

  protected:
    std::shared_ptr<CacheHelperAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CacheHelperAnonymousInnerClass>(
          CacheHelper::shared_from_this());
    }
  };

public:
  void notifyReaderClosedListeners()  override;

  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

protected:
  std::shared_ptr<StandardDirectoryReader> shared_from_this()
  {
    return std::static_pointer_cast<StandardDirectoryReader>(
        DirectoryReader::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
