#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/search/SearcherFactory.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/replicator/nrt/FileMetaData.h"
namespace org::apache::lucene::search
{
template <typename G>
class ReferenceManager;
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

using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using ReferenceManager = org::apache::lucene::search::ReferenceManager;
using SearcherFactory = org::apache::lucene::search::SearcherFactory;
using Directory = org::apache::lucene::store::Directory;

/** Common base class for {@link PrimaryNode} and {@link ReplicaNode}.
 *
 * @lucene.experimental */

class Node : public std::enable_shared_from_this<Node>
{
  GET_CLASS_NAME(Node)

public:
  static bool VERBOSE_FILES;
  static bool VERBOSE_CONNECTIONS;

  // Keys we store into IndexWriter's commit user data:

  /** Key to store the primary gen in the commit data, which increments every
   * time we promote a new primary, so replicas can detect when the
   *  primary they were talking to is changed */
  static std::wstring PRIMARY_GEN_KEY;

  /** Key to store the version in the commit data, which increments every time
   * we open a new NRT reader */
  static std::wstring VERSION_KEY;

  /** Compact ordinal for this node */
protected:
  const int id;

  const std::shared_ptr<Directory> dir;

  const std::shared_ptr<SearcherFactory> searcherFactory;

  // Tracks NRT readers, opened from IW (primary) or opened from replicated
  // SegmentInfos pulled across the wire (replica):
  std::shared_ptr<ReferenceManager<std::shared_ptr<IndexSearcher>>> mgr;

  /** Startup time of original test, carefully propogated to all nodes to
   * produce consistent "seconds since start time" in messages */
public:
  static int64_t globalStartNS;

  /** When this node was started */
  static const int64_t localStartNS = System::nanoTime();

  /** For debug logging */
protected:
  const std::shared_ptr<PrintStream> printStream;

  // public static final long globalStartNS;

  // For debugging:
public:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: volatile std::wstring state = "idle";
  std::wstring state = L"idle";

  /** File metadata for last sync that succeeded; we use this as a cache */
protected:
  // C++ TODO: 'volatile' has a different meaning in C++:
  // ORIGINAL LINE: protected volatile java.util.Map<std::wstring,FileMetaData>
  // lastFileMetaData;
  std::unordered_map<std::wstring, std::shared_ptr<FileMetaData>>
      lastFileMetaData;

public:
  Node(int id, std::shared_ptr<Directory> dir,
       std::shared_ptr<SearcherFactory> searcherFactory,
       std::shared_ptr<PrintStream> printStream);

  /** Returns the {@link ReferenceManager} to use for acquiring and releasing
   * searchers */
  virtual std::shared_ptr<ReferenceManager<std::shared_ptr<IndexSearcher>>>
  getSearcherManager();

  /** Returns the {@link Directory} this node is writing to */
  virtual std::shared_ptr<Directory> getDirectory();

  virtual std::wstring toString();

  virtual void commit() = 0;

  static void nodeMessage(std::shared_ptr<PrintStream> printStream,
                          const std::wstring &message);

  static void nodeMessage(std::shared_ptr<PrintStream> printStream, int id,
                          const std::wstring &message);

  virtual void message(const std::wstring &message);

  virtual std::wstring name();

  virtual bool isClosed() = 0;

  virtual int64_t getCurrentSearchingVersion() ;

  static std::wstring bytesToString(int64_t bytes);

  /** Opens the specified file, reads its identifying information, including
   * file length, full index header (includes the unique segment ID) and the
   * full footer (includes checksum), and returns the resulting {@link
   * FileMetaData}.
   *
   *  <p>This returns null, logging a message, if there are any problems (the
   * file does not exist, is corrupt, truncated, etc.).</p> */
  virtual std::shared_ptr<FileMetaData>
  readLocalFileMetaData(const std::wstring &fileName) ;
};

} // #include  "core/src/java/org/apache/lucene/replicator/nrt/
