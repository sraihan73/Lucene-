#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRefBuilder;
}

namespace org::apache::lucene::search::suggest
{
class InputIterator;
}
namespace org::apache::lucene::util
{
class BytesRef;
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
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * This wrapper buffers the incoming elements and makes sure they are in
 * random order.
 * @lucene.experimental
 */
class UnsortedInputIterator : public BufferedInputIterator
{
  GET_CLASS_NAME(UnsortedInputIterator)
  // TODO keep this for now
private:
  std::deque<int> const ords;
  int currentOrd = -1;
  const std::shared_ptr<BytesRefBuilder> spare =
      std::make_shared<BytesRefBuilder>();
  const std::shared_ptr<BytesRefBuilder> payloadSpare =
      std::make_shared<BytesRefBuilder>();
  /**
   * Creates a new iterator, wrapping the specified iterator and
   * returning elements in a random order.
   */
public:
  UnsortedInputIterator(std::shared_ptr<InputIterator> source) throw(
      IOException);

  int64_t weight() override;

  std::shared_ptr<BytesRef> next()  override;

  std::shared_ptr<BytesRef> payload() override;

  std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts() override;

protected:
  std::shared_ptr<UnsortedInputIterator> shared_from_this()
  {
    return std::static_pointer_cast<UnsortedInputIterator>(
        BufferedInputIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::suggest
