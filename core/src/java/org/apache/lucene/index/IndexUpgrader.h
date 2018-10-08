#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class Directory;
}

namespace org::apache::lucene::index
{
class IndexWriterConfig;
}
namespace org::apache::lucene::util
{
class InfoStream;
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

/**
  * This is an easy-to-use tool that upgrades all segments of an index from
previous Lucene versions
  * to the current segment file format. It can be used from command line:
  * <pre>
  *  java -cp lucene-core.jar org.apache.lucene.index.IndexUpgrader
[-delete-prior-commits] [-verbose] indexDir
  * </pre>
  * Alternatively this class can be instantiated and {@link #upgrade} invoked.
It uses {@link UpgradeIndexMergePolicy}
  * and triggers the upgrade via an forceMerge request to {@link IndexWriter}.
GET_CLASS_NAME(can)
  * <p>This tool keeps only the last commit in an index; for this
  * reason, if the incoming index has more than one commit, the tool
  * refuses to run by default. Specify {@code -delete-prior-commits}
  * to override this, allowing the tool to delete all but the last commit.
  * From Java code this can be enabled by passing {@code true} to
  * {@link #IndexUpgrader(Directory,InfoStream,bool)}.
  * <p><b>Warning:</b> This tool may reorder documents if the index was
partially
  * upgraded before execution (e.g., documents were added). If your application
relies
  * on &quot;monotonicity&quot; of doc IDs (which means that the order in which
the documents
  * were added to the index is preserved), do a full forceMerge instead.
  * The {@link MergePolicy} set by {@link IndexWriterConfig} may also reorder
  * documents.
  */
class IndexUpgrader final : public std::enable_shared_from_this<IndexUpgrader>
{
  GET_CLASS_NAME(IndexUpgrader)

private:
  static const std::wstring LOG_PREFIX;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command
  // line tool") private static void printUsage()
  static void printUsage();

  /** Main method to run {code IndexUpgrader} from the
   *  command-line. */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("deprecation") public static void
  // main(std::wstring[] args) throws java.io.IOException
  static void main(std::deque<std::wstring> &args) ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command
  // line tool") static IndexUpgrader parseArgs(std::wstring[] args) throws
  // java.io.IOException
  static std::shared_ptr<IndexUpgrader>
  parseArgs(std::deque<std::wstring> &args) ;

  const std::shared_ptr<Directory> dir;
  const std::shared_ptr<IndexWriterConfig> iwc;
  const bool deletePriorCommits;

  /** Creates index upgrader on the given directory, using an {@link
   * IndexWriter} using the given
   * {@code matchVersion}. The tool refuses to upgrade indexes with multiple
   * commit points. */
public:
  IndexUpgrader(std::shared_ptr<Directory> dir);

  /** Creates index upgrader on the given directory, using an {@link
   * IndexWriter} using the given
   * {@code matchVersion}. You have the possibility to upgrade indexes with
   * multiple commit points by removing
   * all older ones. If {@code infoStream} is not {@code null}, all logging
   * output will be sent to this stream. */
  IndexUpgrader(std::shared_ptr<Directory> dir,
                std::shared_ptr<InfoStream> infoStream,
                bool deletePriorCommits);

  /** Creates index upgrader on the given directory, using an {@link
   * IndexWriter} using the given config. You have the possibility to upgrade
   * indexes with multiple commit points by removing all older ones. */
  IndexUpgrader(std::shared_ptr<Directory> dir,
                std::shared_ptr<IndexWriterConfig> iwc,
                bool deletePriorCommits);

  /** Perform the upgrade. */
  void upgrade() ;
};

} // namespace org::apache::lucene::index
