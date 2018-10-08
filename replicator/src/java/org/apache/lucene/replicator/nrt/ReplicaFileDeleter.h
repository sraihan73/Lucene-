#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/Directory.h"

#include  "core/src/java/org/apache/lucene/replicator/nrt/Node.h"

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

using Directory = org::apache::lucene::store::Directory;

// TODO: can we factor/share with IFD: this is doing exactly the same thing, but
// on the replica side

class ReplicaFileDeleter
    : public std::enable_shared_from_this<ReplicaFileDeleter>
{
  GET_CLASS_NAME(ReplicaFileDeleter)
private:
  const std::unordered_map<std::wstring, int> refCounts =
      std::unordered_map<std::wstring, int>();
  const std::shared_ptr<Directory> dir;
  const std::shared_ptr<Node> node;

public:
  ReplicaFileDeleter(std::shared_ptr<Node> node,
                     std::shared_ptr<Directory> dir) ;

  /** Used only by asserts: returns true if the file exists
   *  (can be opened), false if it cannot be opened, and
   *  (unlike Java's File.exists) throws IOException if
   *  there's some unexpected error. */
private:
  static bool slowFileExists(std::shared_ptr<Directory> dir,
                             const std::wstring &fileName) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void incRef(
      std::shared_ptr<std::deque<std::wstring>> fileNames) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void decRef(
      std::shared_ptr<std::deque<std::wstring>> fileNames) ;

private:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void delete_(std::shared_ptr<std::deque<std::wstring>> toDelete) throw(
      IOException);

  // C++ WARNING: The following method was originally marked 'synchronized':
  void delete_(const std::wstring &fileName) ;

public:
  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual std::optional<int> getRefCount(const std::wstring &fileName);

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void deleteIfNoRef(const std::wstring &fileName) ;

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void deleteNewFile(const std::wstring &fileName) ;

  /*
  public synchronized Set<std::wstring> getPending() {
    return new HashSet<std::wstring>(pending);
  }
  */

  // C++ WARNING: The following method was originally marked 'synchronized':
  virtual void
  deleteUnknownFiles(const std::wstring &segmentsFileName) ;
};

} // #include  "core/src/java/org/apache/lucene/replicator/nrt/
