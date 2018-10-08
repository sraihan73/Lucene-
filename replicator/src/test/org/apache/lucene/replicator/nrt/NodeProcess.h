#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::document
{
class Document;
}

namespace org::apache::lucene::replicator::nrt
{
class Connection;
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

using Document = org::apache::lucene::document::Document;

/** Parent JVM hold this "wrapper" to refer to each child JVM.  This is roughly
 * equivalent e.g. to a client-side "sugar" API. */
class NodeProcess : public std::enable_shared_from_this<NodeProcess>
{
  GET_CLASS_NAME(NodeProcess)
public:
  const std::shared_ptr<Process> p;

  // Port sub-process is listening on
  const int tcpPort;

  const int id;

  const std::shared_ptr<Thread> pumper;

  // Acquired when searching or indexing wants to use this node:
  const std::shared_ptr<ReentrantLock> lock;

  const bool isPrimary;

  // Version in the commit point we opened on init:
  const int64_t initCommitVersion;

  // SegmentInfos.version, which can be higher than the initCommitVersion
  const int64_t initInfosVersion;

  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile bool isOpen = true;
  bool isOpen = true;

  const std::shared_ptr<AtomicBoolean> nodeIsClosing;

  NodeProcess(std::shared_ptr<Process> p, int id, int tcpPort,
              std::shared_ptr<Thread> pumper, bool isPrimary,
              int64_t initCommitVersion, int64_t initInfosVersion,
              std::shared_ptr<AtomicBoolean> nodeIsClosing);

  virtual std::wstring toString();

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void crash();

  virtual bool commit() ;

  virtual void commitAsync() ;

  virtual int64_t getSearchingVersion() ;

  /** Ask the primary node process to flush.  We send it all currently up
   * replicas so it can notify them about the new NRT point.  Returns the newly
   *  flushed version, or a negative (current) version if there were no changes.
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual int64_t flush(int atLeastMarkerCount) ;

  virtual ~NodeProcess();

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual bool shutdown();

  virtual void newNRTPoint(int64_t version, int64_t primaryGen,
                           int primaryTCPPort) ;

  virtual void addOrUpdateDocument(std::shared_ptr<Connection> c,
                                   std::shared_ptr<Document> doc,
                                   bool isUpdate) ;

  virtual void deleteDocument(std::shared_ptr<Connection> c,
                              const std::wstring &docid) ;

  virtual void
  deleteAllDocuments(std::shared_ptr<Connection> c) ;

  virtual void forceMerge(std::shared_ptr<Connection> c) ;
};

} // namespace org::apache::lucene::replicator::nrt
