#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::suggest
{
class InputIterator;
}

namespace org::apache::lucene::store
{
class IndexOutput;
}
namespace org::apache::lucene::util
{
class OfflineSorter;
}
namespace org::apache::lucene::util
{
class ByteSequencesReader;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::store
{
class Directory;
}
namespace org::apache::lucene::store
{
class ByteArrayDataInput;
}
namespace org::apache::lucene::store
{
class ByteArrayDataOutput;
}
namespace org::apache::lucene::util
{
class ByteSequencesWriter;
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
namespace org::apache::lucene::search::suggest
{

using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using ByteArrayDataOutput = org::apache::lucene::store::ByteArrayDataOutput;
using Directory = org::apache::lucene::store::Directory;
using IndexOutput = org::apache::lucene::store::IndexOutput;
using BytesRef = org::apache::lucene::util::BytesRef;
using ByteSequencesReader =
    org::apache::lucene::util::OfflineSorter::ByteSequencesReader;
using ByteSequencesWriter =
    org::apache::lucene::util::OfflineSorter::ByteSequencesWriter;
using OfflineSorter = org::apache::lucene::util::OfflineSorter;

/**
 * This wrapper buffers incoming elements and makes sure they are sorted based
 * on given comparator.
 * @lucene.experimental
 */
class SortedInputIterator
    : public std::enable_shared_from_this<SortedInputIterator>,
      public InputIterator
{
  GET_CLASS_NAME(SortedInputIterator)

private:
  const std::shared_ptr<InputIterator> source;
  std::shared_ptr<IndexOutput> tempInput;
  std::wstring tempSortedFileName;
  const std::shared_ptr<ByteSequencesReader> reader;
  const std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>> comparator;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasPayloads_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasContexts_;
  const std::shared_ptr<Directory> tempDir;
  const std::wstring tempFileNamePrefix;
  bool done = false;

  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t weight_ = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  std::shared_ptr<BytesRef> payload_ = std::make_shared<BytesRef>();
  // C++ NOTE: Fields cannot have the same name as methods:
  std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts_ = nullptr;

  /**
   * Creates a new sorted wrapper, using {@linkplain Comparator#naturalOrder()
   * natural order} for sorting. */
public:
  SortedInputIterator(std::shared_ptr<Directory> tempDir,
                      const std::wstring &tempFileNamePrefix,
                      std::shared_ptr<InputIterator> source) ;

  /**
   * Creates a new sorted wrapper, sorting by BytesRef
   * (ascending) then cost (ascending).
   */
  SortedInputIterator(std::shared_ptr<Directory> tempDir,
                      const std::wstring &tempFileNamePrefix,
                      std::shared_ptr<InputIterator> source,
                      std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
                          comparator) ;

  std::shared_ptr<BytesRef> next()  override;

  int64_t weight() override;

  std::shared_ptr<BytesRef> payload() override;

  bool hasPayloads() override;

  std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts() override;

  bool hasContexts() override;

  /** Sortes by BytesRef (ascending) then cost (ascending). */
private:
  const std::shared_ptr<Comparator<std::shared_ptr<BytesRef>>>
      tieBreakByCostComparator =
          std::make_shared<ComparatorAnonymousInnerClass>();

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  public:
    ComparatorAnonymousInnerClass();

  private:
    const std::shared_ptr<BytesRef> leftScratch = std::make_shared<BytesRef>();
    const std::shared_ptr<BytesRef> rightScratch = std::make_shared<BytesRef>();
    const std::shared_ptr<ByteArrayDataInput> input =
        std::make_shared<ByteArrayDataInput>();

  public:
    int compare(std::shared_ptr<BytesRef> left,
                std::shared_ptr<BytesRef> right) override;
  };

private:
  std::shared_ptr<ByteSequencesReader> sort() ;

public:
  virtual ~SortedInputIterator();

  /** encodes an entry (bytes+(contexts)+(payload)+weight) to the provided
   * writer */
protected:
  virtual void encode(std::shared_ptr<ByteSequencesWriter> writer,
                      std::shared_ptr<ByteArrayDataOutput> output,
                      std::deque<char> &buffer,
                      std::shared_ptr<BytesRef> spare,
                      std::shared_ptr<BytesRef> payload,
                      std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
                      int64_t weight) ;

  /** decodes the weight at the current position */
  virtual int64_t decode(std::shared_ptr<BytesRef> scratch,
                           std::shared_ptr<ByteArrayDataInput> tmpInput);

  /** decodes the contexts at the current position */
  virtual std::shared_ptr<Set<std::shared_ptr<BytesRef>>>
  decodeContexts(std::shared_ptr<BytesRef> scratch,
                 std::shared_ptr<ByteArrayDataInput> tmpInput);

  /** decodes the payload at the current position */
  virtual std::shared_ptr<BytesRef>
  decodePayload(std::shared_ptr<BytesRef> scratch,
                std::shared_ptr<ByteArrayDataInput> tmpInput);
};

} // namespace org::apache::lucene::search::suggest
