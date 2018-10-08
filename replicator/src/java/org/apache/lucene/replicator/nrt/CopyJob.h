#pragma once
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::replicator::nrt
{
class ReplicaNode;
}

namespace org::apache::lucene::replicator::nrt
{
class FileMetaData;
}
namespace org::apache::lucene::replicator::nrt
{
class OnceDone;
}
namespace org::apache::lucene::replicator::nrt
{
class CopyOneFile;
}
namespace org::apache::lucene::replicator::nrt
{
class CopyState;
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

namespace org::apache::lucene::replicator::nrt
{

/** Handles copying one set of files, e.g. all files for a new NRT point, or
 * files for pre-copying a merged segment. This notifies the caller via OnceDone
 * when the job finishes or failed.
 *
 * @lucene.experimental */
class CopyJob : public std::enable_shared_from_this<CopyJob>,
                public Comparable<std::shared_ptr<CopyJob>>
{
  GET_CLASS_NAME(CopyJob)
private:
  static const std::shared_ptr<AtomicLong> counter;

protected:
  const std::shared_ptr<ReplicaNode> dest;

  const std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>> files;

public:
  const int64_t ord = counter->incrementAndGet();

  /** True for an NRT sync, false for pre-copying a newly merged segment */
  const bool highPriority;

  const std::shared_ptr<OnceDone> onceDone;

  const int64_t startNS = System::nanoTime();

  const std::wstring reason;

protected:
  const std::deque<
      std::unordered_map::Entry<std::wstring, std::shared_ptr<FileMetaData>>>
      toCopy;

  int64_t totBytes = 0;

  int64_t totBytesCopied = 0;

  // The file we are currently copying:
  std::shared_ptr<CopyOneFile> current;

  // Set when we are cancelled
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: protected volatile Throwable exc;
  std::runtime_error exc;
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: protected volatile std::wstring cancelReason;
  std::wstring cancelReason;

  // toString may concurrently access this:
  const std::unordered_map<std::wstring, std::wstring> copiedFiles =
      std::make_shared<ConcurrentHashMap<std::wstring, std::wstring>>();

  CopyJob(
      const std::wstring &reason,
      std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>> &files,
      std::shared_ptr<ReplicaNode> dest, bool highPriority,
      std::shared_ptr<OnceDone> onceDone) ;

  /** Callback invoked by CopyJob once all files have (finally) finished copying
   */
public:
  class OnceDone
  {
    GET_CLASS_NAME(OnceDone)
  public:
    virtual void run(std::shared_ptr<CopyJob> job) = 0;
  };

  /** Transfers whatever tmp files were already copied in this previous job and
   * cancels the previous job */
public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void
  transferAndCancel(std::shared_ptr<CopyJob> prevJob) ;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void _transferAndCancel(std::shared_ptr<CopyJob> prevJob) ;

protected:
  virtual std::shared_ptr<CopyOneFile>
  newCopyOneFile(std::shared_ptr<CopyOneFile> current) = 0;

  /** Begin copying files */
public:
  virtual void start() = 0;

  /** Use current thread (blocking) to do all copying and then return once done,
   * or throw exception on failure */
  virtual void runBlocking() = 0;

  virtual void cancel(const std::wstring &reason,
                      std::runtime_error exc) ;

  /** Return true if this job is trying to copy any of the same files as the
   * other job */
  virtual bool conflicts(std::shared_ptr<CopyJob> other) = 0;

  /** Renames all copied (tmp) files to their true file names */
  virtual void finish() = 0;

  virtual bool getFailed() = 0;

  /** Returns only those file names (a subset of {@link #getFileNames}) that
   * need to be copied */
  virtual std::shared_ptr<Set<std::wstring>> getFileNamesToCopy() = 0;

  /** Returns all file names referenced in this copy job */
  virtual std::shared_ptr<Set<std::wstring>> getFileNames() = 0;

  virtual std::shared_ptr<CopyState> getCopyState() = 0;

  virtual int64_t getTotalBytesCopied() = 0;
};

} // namespace org::apache::lucene::replicator::nrt
