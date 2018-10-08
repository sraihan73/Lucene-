#pragma once
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class ReadersAndUpdates;
}

namespace org::apache::lucene::index
{
class SegmentCommitInfo;
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
class FieldNumbers;
}
namespace org::apache::lucene::util
{
class InfoStream;
}
namespace org::apache::lucene::index
{
class SegmentInfos;
}
namespace org::apache::lucene::index
{
class StandardDirectoryReader;
}
namespace org::apache::lucene::index
{
class PendingDeletes;
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
using InfoStream = org::apache::lucene::util::InfoStream;

/** Holds shared SegmentReader instances. IndexWriter uses
 *  SegmentReaders for 1) applying deletes/DV updates, 2) doing
 *  merges, 3) handing out a real-time reader.  This pool
 *  reuses instances of the SegmentReaders in all these
 *  places if it is in "near real-time mode" (getReader()
 *  has been called on this instance). */
class ReaderPool final : public std::enable_shared_from_this<ReaderPool>
{
  GET_CLASS_NAME(ReaderPool)

private:
  const std::unordered_map<std::shared_ptr<SegmentCommitInfo>,
                           std::shared_ptr<ReadersAndUpdates>>
      readerMap = std::unordered_map<std::shared_ptr<SegmentCommitInfo>,
                                     std::shared_ptr<ReadersAndUpdates>>();
  const std::shared_ptr<Directory> directory;
  const std::shared_ptr<Directory> originalDirectory;
  const std::shared_ptr<FieldInfos::FieldNumbers> fieldNumbers;
  const std::function<int64_t()> completedDelGenSupplier;
  const std::shared_ptr<InfoStream> infoStream;
  const std::shared_ptr<SegmentInfos> segmentInfos;
  const std::wstring softDeletesField;
  // This is a "write once" variable (like the organic dye
  // on a DVD-R that may or may not be heated by a laser and
  // then cooled to permanently record the event): it's
  // false, by default until {@link #enableReaderPooling()}
  // is called for the first time,
  // at which point it's switched to true and never changes
  // back to false.  Once this is true, we hold open and
  // reuse SegmentReader instances internally for applying
  // deletes, doing merges, and reopening near real-time
  // readers.
  // in practice this should be called once the readers are likely
  // to be needed and reused ie if IndexWriter#getReader is called.
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: private volatile bool poolReaders;
  bool poolReaders = false;
  const std::shared_ptr<AtomicBoolean> closed =
      std::make_shared<AtomicBoolean>(false);

public:
  ReaderPool(
      std::shared_ptr<Directory> directory,
      std::shared_ptr<Directory> originalDirectory,
      std::shared_ptr<SegmentInfos> segmentInfos,
      std::shared_ptr<FieldInfos::FieldNumbers> fieldNumbers,
      std::function<int64_t()> &completedDelGenSupplier,
      std::shared_ptr<InfoStream> infoStream,
      const std::wstring &softDeletesField,
      std::shared_ptr<StandardDirectoryReader> reader) ;

  /** Asserts this info still exists in IW's segment infos */
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool assertInfoIsLive(std::shared_ptr<SegmentCommitInfo> info);

  /**
   * Drops reader for the given {@link SegmentCommitInfo} if it's pooled
   * @return <code>true</code> if a reader is pooled
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool drop(std::shared_ptr<SegmentCommitInfo> info) ;

  /**
   * Returns the sum of the ram used by all the buffered readers and updates in
   * MB
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  int64_t ramBytesUsed();

  /**
   * Returns <code>true</code> iff any of the buffered readers and updates has
   * at least one pending delete
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool anyDeletions();

  /**
   * Enables reader pooling for this pool. This should be called once the
   * readers in this pool are shared with an outside resource like an NRT
   * reader. Once reader pooling is enabled a {@link ReadersAndUpdates} will be
   * kept around in the reader pool on calling {@link
   * #release(ReadersAndUpdates, bool)} until the segment get dropped via
   * calls to {@link #drop(SegmentCommitInfo)} or {@link #dropAll()} or {@link
   * #close()}. Reader pooling is disabled upon construction but can't be
   * disabled again once it's enabled.
   */
  void enableReaderPooling();

  bool isReaderPoolingEnabled();

  /**
   * Releases the {@link ReadersAndUpdates}. This should only be called if the
   * {@link #get(SegmentCommitInfo, bool)} is called with the create paramter
   * set to true.
   * @return <code>true</code> if any files were written by this release call.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool release(std::shared_ptr<ReadersAndUpdates> rld,
               bool assertInfoLive) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual ~ReaderPool();

  /**
   * Writes all doc values updates to disk if there are any.
   * @return <code>true</code> iff any files where written
   */
  bool writeAllDocValuesUpdates() ;

  /**
   * Writes all doc values updates to disk if there are any.
   * @return <code>true</code> iff any files where written
   */
  bool
  writeDocValuesUpdatesForMerge(std::deque<std::shared_ptr<SegmentCommitInfo>>
                                    &infos) ;

  /**
   * Returns a deque of all currently maintained ReadersAndUpdates sorted by it's
   * ram consumption largest to smallest. This deque can also contain readers
   * that don't consume any ram at this point ie. don't have any updates
   * buffered.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::deque<std::shared_ptr<ReadersAndUpdates>> getReadersByRam();

  /** Remove all our references to readers, and commits
   *  any pending changes. */
  // C++ WARNING: The following method was originally marked 'synchronized':
  void dropAll() ;

  /**
   * Commit live docs changes for the segment readers for
   * the provided infos.
   *
   * @throws IOException If there is a low-level I/O error
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool commit(std::shared_ptr<SegmentInfos> infos) ;

  /**
   * Returns <code>true</code> iff there are any buffered doc values updates.
   * Otherwise <code>false</code>.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  bool anyDocValuesChanges();

  /**
   * Obtain a ReadersAndLiveDocs instance from the
   * readerPool.  If create is true, you must later call
   * {@link #release(ReadersAndUpdates, bool)}.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<ReadersAndUpdates>
  get(std::shared_ptr<SegmentCommitInfo> info, bool create);

private:
  std::shared_ptr<PendingDeletes>
  newPendingDeletes(std::shared_ptr<SegmentCommitInfo> info);

  std::shared_ptr<PendingDeletes>
  newPendingDeletes(std::shared_ptr<SegmentReader> reader,
                    std::shared_ptr<SegmentCommitInfo> info);

  // Make sure that every segment appears only once in the
  // pool:
  bool noDups();
};
} // namespace org::apache::lucene::index
