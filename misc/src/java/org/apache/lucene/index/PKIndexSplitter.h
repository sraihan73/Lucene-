#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::index
{
class IndexWriterConfig;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class DirectoryReader;
}
namespace org::apache::lucene::util
{
class Bits;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Weight;
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

using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;
using Directory = org::apache::lucene::store::Directory;
using Bits = org::apache::lucene::util::Bits;

/**
 * Split an index based on a {@link Query}.
 */

class PKIndexSplitter : public std::enable_shared_from_this<PKIndexSplitter>
{
  GET_CLASS_NAME(PKIndexSplitter)
private:
  const std::shared_ptr<Query> docsInFirstIndex;
  const std::shared_ptr<Directory> input;
  const std::shared_ptr<Directory> dir1;
  const std::shared_ptr<Directory> dir2;
  const std::shared_ptr<IndexWriterConfig> config1;
  const std::shared_ptr<IndexWriterConfig> config2;

  /**
   * Split an index based on a {@link Query}. All documents that match the query
   * are sent to dir1, remaining ones to dir2.
   */
public:
  PKIndexSplitter(std::shared_ptr<Directory> input,
                  std::shared_ptr<Directory> dir1,
                  std::shared_ptr<Directory> dir2,
                  std::shared_ptr<Query> docsInFirstIndex);

private:
  static std::shared_ptr<IndexWriterConfig> newDefaultConfig();

public:
  PKIndexSplitter(std::shared_ptr<Directory> input,
                  std::shared_ptr<Directory> dir1,
                  std::shared_ptr<Directory> dir2,
                  std::shared_ptr<Query> docsInFirstIndex,
                  std::shared_ptr<IndexWriterConfig> config1,
                  std::shared_ptr<IndexWriterConfig> config2);

  /**
   * Split an index based on a  given primary key term
   * and a 'middle' term.  If the middle term is present, it's
   * sent to dir2.
   */
  PKIndexSplitter(std::shared_ptr<Directory> input,
                  std::shared_ptr<Directory> dir1,
                  std::shared_ptr<Directory> dir2,
                  std::shared_ptr<Term> midTerm);

  PKIndexSplitter(std::shared_ptr<Directory> input,
                  std::shared_ptr<Directory> dir1,
                  std::shared_ptr<Directory> dir2,
                  std::shared_ptr<Term> midTerm,
                  std::shared_ptr<IndexWriterConfig> config1,
                  std::shared_ptr<IndexWriterConfig> config2);

  virtual void split() ;

private:
  void createIndex(std::shared_ptr<IndexWriterConfig> config,
                   std::shared_ptr<Directory> target,
                   std::shared_ptr<DirectoryReader> reader,
                   std::shared_ptr<Query> preserveFilter,
                   bool negateFilter) ;

private:
  class DocumentFilteredLeafIndexReader : public FilterCodecReader
  {
    GET_CLASS_NAME(DocumentFilteredLeafIndexReader)
  public:
    const std::shared_ptr<Bits> liveDocs;
    // C++ NOTE: Fields cannot have the same name as methods:
    const int numDocs_;

    DocumentFilteredLeafIndexReader(std::shared_ptr<LeafReaderContext> context,
                                    std::shared_ptr<Weight> preserveWeight,
                                    bool negateFilter) ;

    int numDocs() override;

    std::shared_ptr<Bits> getLiveDocs() override;

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

  protected:
    std::shared_ptr<DocumentFilteredLeafIndexReader> shared_from_this()
    {
      return std::static_pointer_cast<DocumentFilteredLeafIndexReader>(
          FilterCodecReader::shared_from_this());
    }
  };
};

} // namespace org::apache::lucene::index
