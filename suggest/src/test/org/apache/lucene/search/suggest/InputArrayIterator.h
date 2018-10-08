#pragma once
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::suggest
{
class Input;
}

namespace org::apache::lucene::util
{
class BytesRefBuilder;
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
 * A {@link InputIterator} over a sequence of {@link Input}s.
 */
class InputArrayIterator final
    : public std::enable_shared_from_this<InputArrayIterator>,
      public InputIterator
{
  GET_CLASS_NAME(InputArrayIterator)
private:
  const std::shared_ptr<Iterator<std::shared_ptr<Input>>> i;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasPayloads_;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasContexts_;
  bool first = false;
  std::shared_ptr<Input> current;
  const std::shared_ptr<BytesRefBuilder> spare =
      std::make_shared<BytesRefBuilder>();

public:
  InputArrayIterator(std::shared_ptr<Iterator<std::shared_ptr<Input>>> i);

  InputArrayIterator(std::deque<std::shared_ptr<Input>> &i);
  InputArrayIterator(std::deque<std::shared_ptr<Input>> &i);

  int64_t weight() override;

  std::shared_ptr<BytesRef> next() override;

  std::shared_ptr<BytesRef> payload() override;

  bool hasPayloads() override;

  std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts() override;

  bool hasContexts() override;
};

} // namespace org::apache::lucene::search::suggest
