#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRefArray;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}
namespace org::apache::lucene::search::suggest
{
class InputIterator;
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

using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefArray = org::apache::lucene::util::BytesRefArray;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using Counter = org::apache::lucene::util::Counter;

/**
 * This wrapper buffers incoming elements.
 * @lucene.experimental
 */
class BufferedInputIterator
    : public std::enable_shared_from_this<BufferedInputIterator>,
      public InputIterator
{
  GET_CLASS_NAME(BufferedInputIterator)
  // TODO keep this for now
  /** buffered term entries */
protected:
  std::shared_ptr<BytesRefArray> entries =
      std::make_shared<BytesRefArray>(Counter::newCounter());
  /** buffered payload entries */
  std::shared_ptr<BytesRefArray> payloads =
      std::make_shared<BytesRefArray>(Counter::newCounter());
  /** buffered context set entries */
  std::deque<Set<std::shared_ptr<BytesRef>>> contextSets =
      std::deque<Set<std::shared_ptr<BytesRef>>>();
  /** current buffer position */
  int curPos = -1;
  /** buffered weights, parallel with {@link #entries} */
  std::deque<int64_t> freqs = std::deque<int64_t>(1);

private:
  const std::shared_ptr<BytesRefBuilder> spare =
      std::make_shared<BytesRefBuilder>();
  const std::shared_ptr<BytesRefBuilder> payloadSpare =
      std::make_shared<BytesRefBuilder>();
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasPayloads_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasContexts_;

  /** Creates a new iterator, buffering entries from the specified iterator */
public:
  BufferedInputIterator(std::shared_ptr<InputIterator> source) throw(
      IOException);

  int64_t weight() override;

  std::shared_ptr<BytesRef> next()  override;

  std::shared_ptr<BytesRef> payload() override;

  bool hasPayloads() override;

  std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts() override;

  bool hasContexts() override;
};

} // namespace org::apache::lucene::search::suggest
