#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/DirectoryReader.h"

#include  "core/src/java/org/apache/lucene/index/IndexWriter.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"

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

using ReferenceManager = org::apache::lucene::search::ReferenceManager;
using Directory = org::apache::lucene::store::Directory;

/**
 * Utility class to safely share {@link DirectoryReader} instances across
 * multiple threads, while periodically reopening. This class ensures each
 * reader is closed only once all threads have finished using it.
 *
 * @see SearcherManager
 *
 * @lucene.experimental
 */
class ReaderManager final
    : public ReferenceManager<std::shared_ptr<DirectoryReader>>
{
  GET_CLASS_NAME(ReaderManager)

  /**
   * Creates and returns a new ReaderManager from the given
   * {@link IndexWriter}.
   *
   * @param writer
   *          the IndexWriter to open the IndexReader from.
   *
   * @throws IOException If there is a low-level I/O error
   */
public:
  ReaderManager(std::shared_ptr<IndexWriter> writer) ;

  /**
   * Expert: creates and returns a new ReaderManager from the given
   * {@link IndexWriter}, controlling whether past deletions should be applied.
   *
   * @param writer
   *          the IndexWriter to open the IndexReader from.
   * @param applyAllDeletes
   *          If <code>true</code>, all buffered deletes will be applied (made
   *          visible) in the {@link IndexSearcher} / {@link DirectoryReader}.
   *          If <code>false</code>, the deletes may or may not be applied, but
   *          remain buffered (in IndexWriter) so that they will be applied in
   *          the future. Applying deletes can be costly, so if your app can
   *          tolerate deleted documents being returned you might gain some
   *          performance by passing <code>false</code>. See
   *          {@link DirectoryReader#openIfChanged(DirectoryReader, IndexWriter,
   * bool)}.
   * @param writeAllDeletes
   *          If <code>true</code>, new deletes will be forcefully written to
   * index files.
   *
   * @throws IOException If there is a low-level I/O error
   */
  ReaderManager(std::shared_ptr<IndexWriter> writer, bool applyAllDeletes,
                bool writeAllDeletes) ;

  /**
   * Creates and returns a new ReaderManager from the given {@link Directory}.
   * @param dir the directory to open the DirectoryReader on.
   *
   * @throws IOException If there is a low-level I/O error
   */
  ReaderManager(std::shared_ptr<Directory> dir) ;

  /**
   * Creates and returns a new ReaderManager from the given
   * already-opened {@link DirectoryReader}, stealing
   * the incoming reference.
   *
   * @param reader the directoryReader to use for future reopens
   *
   * @throws IOException If there is a low-level I/O error
   */
  ReaderManager(std::shared_ptr<DirectoryReader> reader) ;

protected:
  void decRef(std::shared_ptr<DirectoryReader> reference) throw(
      IOException) override;

  std::shared_ptr<DirectoryReader>
  refreshIfNeeded(std::shared_ptr<DirectoryReader> referenceToRefresh) throw(
      IOException) override;

  bool tryIncRef(std::shared_ptr<DirectoryReader> reference) override;

  int getRefCount(std::shared_ptr<DirectoryReader> reference) override;

protected:
  std::shared_ptr<ReaderManager> shared_from_this()
  {
    return std::static_pointer_cast<ReaderManager>(
        org.apache.lucene.search
            .ReferenceManager<DirectoryReader>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
