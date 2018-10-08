#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class FakeDeleteLeafIndexReader;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::index
{
class CodecReader;
}
namespace org::apache::lucene::util
{
class Bits;
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
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using Bits = org::apache::lucene::util::Bits;

/**
 * This tool splits input index into multiple equal parts. The method employed
 * here uses {@link IndexWriter#addIndexes(CodecReader[])} where the input data
 * comes from the input index with artificially applied deletes to the document
 * id-s that fall outside the selected partition.
 * <p>Note 1: Deletes are only applied to a buffered deque of deleted docs and
 * don't affect the source index - this tool works also with read-only indexes.
 * <p>Note 2: the disadvantage of this tool is that source index needs to be
 * read as many times as there are parts to be created, hence the name of this
 * tool.
 *
 * <p><b>NOTE</b>: this tool is unaware of documents added
 * atomically via {@link IndexWriter#addDocuments} or {@link
 * IndexWriter#updateDocuments}, which means it can easily
 * break up such document groups.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressForbidden(reason = "System.out required: command line
// tool") public class MultiPassIndexSplitter
class MultiPassIndexSplitter
    : public std::enable_shared_from_this<MultiPassIndexSplitter>
{

  /**
   * Split source index into multiple parts.
   * @param in source index, can have deletions, can have
   * multiple segments (or multiple readers).
   * @param outputs deque of directories where the output parts will be stored.
   * @param seq if true, then the source index will be split into equal
   * increasing ranges of document id-s. If false, source document id-s will be
   * assigned in a deterministic round-robin fashion to one of the output
   * splits.
   * @throws IOException If there is a low-level I/O error
   */
public:
  virtual void split(std::shared_ptr<IndexReader> in_,
                     std::deque<std::shared_ptr<Directory>> &outputs,
                     bool seq) ;

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("deprecation") public static void
  // main(std::wstring[] args) throws Exception
  static void main(std::deque<std::wstring> &args) ;

  /**
   * This class emulates deletions on the underlying index.
   */
private:
  class FakeDeleteIndexReader final
      : public BaseCompositeReader<std::shared_ptr<FakeDeleteLeafIndexReader>>
  {
    GET_CLASS_NAME(FakeDeleteIndexReader)

  public:
    FakeDeleteIndexReader(std::shared_ptr<IndexReader> reader) throw(
        IOException);

  private:
    static std::deque<std::shared_ptr<FakeDeleteLeafIndexReader>>
    initSubReaders(std::shared_ptr<IndexReader> reader) ;

  public:
    void deleteDocument(int docID);

    void undeleteAll();

  protected:
    void doClose() override;

  public:
    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

    // no need to override numDocs/hasDeletions,
    // as we pass the subreaders directly to IW.addIndexes().

  protected:
    std::shared_ptr<FakeDeleteIndexReader> shared_from_this()
    {
      return std::static_pointer_cast<FakeDeleteIndexReader>(
          BaseCompositeReader<FakeDeleteLeafIndexReader>::shared_from_this());
    }
  };

private:
  class FakeDeleteLeafIndexReader final : public FilterCodecReader
  {
    GET_CLASS_NAME(FakeDeleteLeafIndexReader)
  public:
    std::shared_ptr<FixedBitSet> liveDocs;

    FakeDeleteLeafIndexReader(std::shared_ptr<CodecReader> reader);

    int numDocs() override;

    void undeleteAll();

    void deleteDocument(int n);

    std::shared_ptr<Bits> getLiveDocs() override;

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<FakeDeleteLeafIndexReader> shared_from_this()
    {
      return std::static_pointer_cast<FakeDeleteLeafIndexReader>(
          FilterCodecReader::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::index
