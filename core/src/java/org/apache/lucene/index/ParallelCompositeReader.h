#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class LeafReader;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::index
{
class CompositeReader;
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

/** An {@link CompositeReader} which reads multiple, parallel indexes.  Each
 * index added must have the same number of documents, and exactly the same
 * number of leaves (with equal {@code maxDoc}), but typically each contains
 * different fields. Deletions are taken from the first reader. Each document
 * contains the union of the fields of all documents with the same document
 * number.  When searching, matches for a query term are from the first index
 * added that has the field.
 *
 * <p>This is useful, e.g., with collections that have large fields which
 * change rarely and small fields that change more frequently.  The smaller
 * fields may be re-indexed in a new index and both indexes may be searched
 * together.
 *
 * <p><strong>Warning:</strong> It is up to you to make sure all indexes
 * are created and modified the same way. For example, if you add
 * documents to one index, you need to add the same documents in the
 * same order to the other indexes. <em>Failure to do so will result in
 * undefined behavior</em>.
 * A good strategy to create suitable indexes with {@link IndexWriter} is to use
 * {@link LogDocMergePolicy}, as this one does not reorder documents
 * during merging (like {@code TieredMergePolicy}) and triggers merges
 * by number of documents per segment. If you use different {@link MergePolicy}s
 * it might happen that the segment structure of your index is no longer
 * predictable.
 */
class ParallelCompositeReader
    : public BaseCompositeReader<std::shared_ptr<LeafReader>>
{
  GET_CLASS_NAME(ParallelCompositeReader)
private:
  const bool closeSubReaders;
  const std::shared_ptr<Set<std::shared_ptr<IndexReader>>> completeReaderSet =
      Collections::newSetFromMap(
          std::make_shared<
              IdentityHashMap<std::shared_ptr<IndexReader>, bool>>());
  const std::shared_ptr<CacheHelper> cacheHelper;

  /** Create a ParallelCompositeReader based on the provided
   *  readers; auto-closes the given readers on {@link #close()}. */
public:
  ParallelCompositeReader(std::deque<CompositeReader> &readers) throw(
      IOException);

  /** Create a ParallelCompositeReader based on the provided
   *  readers. */
  ParallelCompositeReader(
      bool closeSubReaders,
      std::deque<CompositeReader> &readers) ;

  /** Expert: create a ParallelCompositeReader based on the provided
   *  readers and storedFieldReaders; when a document is
   *  loaded, only storedFieldsReaders will be used. */
  ParallelCompositeReader(
      bool closeSubReaders,
      std::deque<std::shared_ptr<CompositeReader>> &readers,
      std::deque<std::shared_ptr<CompositeReader>>
          &storedFieldReaders) ;

private:
  static std::deque<std::shared_ptr<LeafReader>>
  prepareLeafReaders(std::deque<std::shared_ptr<CompositeReader>> &readers,
                     std::deque<std::shared_ptr<CompositeReader>>
                         &storedFieldsReaders) ;

private:
  class ParallelLeafReaderAnonymousInnerClass : public ParallelLeafReader
  {
    GET_CLASS_NAME(ParallelLeafReaderAnonymousInnerClass)
  public:
    ParallelLeafReaderAnonymousInnerClass();

  protected:
    void doClose() override;

  protected:
    std::shared_ptr<ParallelLeafReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ParallelLeafReaderAnonymousInnerClass>(
          ParallelLeafReader::shared_from_this());
    }
  };

private:
  static void validate(std::deque<std::shared_ptr<CompositeReader>> &readers,
                       int maxDoc, std::deque<int> &leafMaxDoc);

public:
  std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

protected:
  // C++ WARNING: The following method was originally marked 'synchronized':
  void doClose()  override;

protected:
  std::shared_ptr<ParallelCompositeReader> shared_from_this()
  {
    return std::static_pointer_cast<ParallelCompositeReader>(
        BaseCompositeReader<LeafReader>::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
