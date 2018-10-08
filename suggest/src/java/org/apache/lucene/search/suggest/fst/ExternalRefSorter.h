#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class OfflineSorter;
}

namespace org::apache::lucene::util
{
class ByteSequencesWriter;
}
namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class BytesRefIterator;
}
namespace org::apache::lucene::util
{
class ByteSequencesReader;
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
namespace org::apache::lucene::search::suggest::fst
{

using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefIterator = org::apache::lucene::util::BytesRefIterator;
using OfflineSorter = org::apache::lucene::util::OfflineSorter;

/**
 * Builds and iterates over sequences stored on disk.
 * @lucene.experimental
 * @lucene.internal
 */
class ExternalRefSorter
    : public std::enable_shared_from_this<ExternalRefSorter>,
      public BytesRefSorter
{
  GET_CLASS_NAME(ExternalRefSorter)
private:
  const std::shared_ptr<OfflineSorter> sorter;
  std::shared_ptr<OfflineSorter::ByteSequencesWriter> writer;
  std::shared_ptr<IndexOutput> input;
  std::wstring sortedFileName;

  /**
   * Will buffer all sequences to a temporary file and then sort (all on-disk).
   */
public:
  ExternalRefSorter(std::shared_ptr<OfflineSorter> sorter) ;

  void add(std::shared_ptr<BytesRef> utf8)  override;

  std::shared_ptr<BytesRefIterator> iterator()  override;

private:
  void closeWriter() ;

  /**
   * Removes any written temporary files.
   */
public:
  virtual ~ExternalRefSorter();

  /**
   * Iterate over byte refs in a file.
   */
  // TODO: this class is a bit silly ... sole purpose is to "remove" Closeable
  // from what #iterator returns:
public:
  class ByteSequenceIterator
      : public std::enable_shared_from_this<ByteSequenceIterator>,
        public BytesRefIterator
  {
    GET_CLASS_NAME(ByteSequenceIterator)
  private:
    const std::shared_ptr<OfflineSorter::ByteSequencesReader> reader;
    std::shared_ptr<BytesRef> scratch;

  public:
    ByteSequenceIterator(
        std::shared_ptr<OfflineSorter::ByteSequencesReader> reader);

    std::shared_ptr<BytesRef> next()  override;
  };

public:
  std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
  getComparator() override;
};

} // namespace org::apache::lucene::search::suggest::fst
