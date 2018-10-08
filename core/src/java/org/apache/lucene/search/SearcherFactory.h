#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

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
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;

/**
 * Factory class used by {@link SearcherManager} to
 * create new IndexSearchers. The default implementation just creates
 * an IndexSearcher with no custom behavior:
 *
 * <pre class="prettyprint">
 *   public IndexSearcher newSearcher(IndexReader r) throws IOException {
GET_CLASS_NAME(="prettyprint">)
 *     return new IndexSearcher(r);
 *   }
 * </pre>
 *
 * You can pass your own factory instead if you want custom behavior, such as:
 * <ul>
 *   <li>Setting a custom scoring model: {@link
IndexSearcher#setSimilarity(Similarity)}
 *   <li>Parallel per-segment search: {@link
IndexSearcher#IndexSearcher(IndexReader, ExecutorService)}
 *   <li>Return custom subclasses of IndexSearcher (for example that implement
distributed scoring)
 *   <li>Run queries to warm your IndexSearcher before it is used. Note: when
using near-realtime search
 *       you may want to also {@link
IndexWriterConfig#setMergedSegmentWarmer(IndexWriter.IndexReaderWarmer)} to warm
 *       newly merged segments in the background, outside of the reopen path.
 * </ul>
 * @lucene.experimental
 */
class SearcherFactory : public std::enable_shared_from_this<SearcherFactory>
{
  GET_CLASS_NAME(SearcherFactory)
  /**
   * Returns a new IndexSearcher over the given reader.
   * @param reader the reader to create a new searcher for
   * @param previousReader the reader previously used to create a new searcher.
   *                       This can be <code>null</code> if unknown or if the
   * given reader is the initially opened reader. If this reader is non-null it
   * can be used to find newly opened segments compared to the new reader to
   * warm the searcher up before returning.
   */
public:
  virtual std::shared_ptr<IndexSearcher>
  newSearcher(std::shared_ptr<IndexReader> reader,
              std::shared_ptr<IndexReader> previousReader) ;
};

} // #include  "core/src/java/org/apache/lucene/search/
