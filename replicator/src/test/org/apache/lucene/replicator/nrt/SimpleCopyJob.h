#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/replicator/nrt/Connection.h"

#include  "core/src/java/org/apache/lucene/replicator/nrt/CopyState.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/FileMetaData.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/SimpleReplicaNode.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/CopyJob.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/CopyOneFile.h"

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

/** Handles one set of files that need copying, either because we have a
 *  new NRT point, or we are pre-copying merged files for merge warming. */
class SimpleCopyJob : public CopyJob
{
  GET_CLASS_NAME(SimpleCopyJob)
public:
  const std::shared_ptr<Connection> c;

  std::deque<char> const copyBuffer = std::deque<char>(65536);
  const std::shared_ptr<CopyState> copyState;

private:
  std::shared_ptr<Iterator<
      std::unordered_map::Entry<std::wstring, std::shared_ptr<FileMetaData>>>>
      iter;

public:
  SimpleCopyJob(
      const std::wstring &reason, std::shared_ptr<Connection> c,
      std::shared_ptr<CopyState> copyState,
      std::shared_ptr<SimpleReplicaNode> dest,
      std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>> &files,
      bool highPriority, std::shared_ptr<OnceDone> onceDone) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void start()  override;

  int64_t getTotalBytesCopied() override;

  std::shared_ptr<Set<std::wstring>> getFileNamesToCopy() override;

  std::shared_ptr<Set<std::wstring>> getFileNames() override;

  /** Higher priority and then "first come first serve" order. */
  int compareTo(std::shared_ptr<CopyJob> _other) override;

  void finish()  override;

  /** Do an iota of work; returns true if all copying is done */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual bool visit() ;

protected:
  std::shared_ptr<CopyOneFile>
  newCopyOneFile(std::shared_ptr<CopyOneFile> prev) override;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void transferAndCancel(std::shared_ptr<CopyJob> prevJob) throw(
      IOException) override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  void cancel(const std::wstring &reason,
              std::runtime_error exc)  override;

  bool getFailed() override;

  virtual std::wstring toString();

  void runBlocking()  override;

  std::shared_ptr<CopyState> getCopyState() override;

  // C++ WARNING: The following method was originally marked 'synchronized':
  bool conflicts(std::shared_ptr<CopyJob> _other) override;

protected:
  std::shared_ptr<SimpleCopyJob> shared_from_this()
  {
    return std::static_pointer_cast<SimpleCopyJob>(CopyJob::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/replicator/nrt/
