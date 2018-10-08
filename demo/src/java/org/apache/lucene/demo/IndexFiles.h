#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"

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
namespace org::apache::lucene::demo
{

using IndexWriter = org::apache::lucene::index::IndexWriter;

/** Index all text files under a directory.
 * <p>
 * This is a command-line application demonstrating simple Lucene indexing.
 * Run it with no command-line arguments for usage information.
 */
class IndexFiles : public std::enable_shared_from_this<IndexFiles>
{
  GET_CLASS_NAME(IndexFiles)

private:
  IndexFiles();

  /** Index all text files under a directory. */
  static void main(std::deque<std::wstring> &args);

  /**
   * Indexes the given file using the given writer, or if a directory is given,
   * recurses over files and directories found under the given directory.
   *
   * NOTE: This method indexes one document per input file.  This is slow.  For
   * good throughput, put multiple documents into your input file(s).  An
   * example of this is in the benchmark module, which can create "line doc"
   * files, one document per line, using the <a
   * href="../../../../../contrib-benchmark/org/apache/lucene/benchmark/byTask/tasks/WriteLineDocTask.html"
   * >WriteLineDocTask</a>.
   *
   * @param writer Writer to the index where the given file/dir info will be
   * stored
   * @param path The file to index, or the directory to recurse into to find
   * files to index
   * @throws IOException If there is a low-level I/O error
   */
public:
  static void indexDocs(std::shared_ptr<IndexWriter> writer,
                        std::shared_ptr<Path> path) ;

private:
  class SimpleFileVisitorAnonymousInnerClass
      : public SimpleFileVisitor<std::shared_ptr<Path>>
  {
    GET_CLASS_NAME(SimpleFileVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<IndexWriter> writer;

  public:
    SimpleFileVisitorAnonymousInnerClass(std::shared_ptr<IndexWriter> writer);

    std::shared_ptr<FileVisitResult> visitFile(
        std::shared_ptr<Path> file,
        std::shared_ptr<BasicFileAttributes> attrs)  override;

  protected:
    std::shared_ptr<SimpleFileVisitorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<SimpleFileVisitorAnonymousInnerClass>(
          java.nio.file
              .SimpleFileVisitor<java.nio.file.Path>::shared_from_this());
    }
  };

  /** Indexes a single document */
public:
  static void indexDoc(std::shared_ptr<IndexWriter> writer,
                       std::shared_ptr<Path> file,
                       int64_t lastModified) ;
};

} // #include  "core/src/java/org/apache/lucene/demo/
