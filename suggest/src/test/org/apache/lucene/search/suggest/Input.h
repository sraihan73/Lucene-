#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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

/** corresponds to {@link InputIterator}'s entries */
class Input final : public std::enable_shared_from_this<Input>
{
  GET_CLASS_NAME(Input)
public:
  const std::shared_ptr<BytesRef> term;
  const int64_t v;
  const std::shared_ptr<BytesRef> payload;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasPayloads_;
  const std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasContexts_;

  Input(std::shared_ptr<BytesRef> term, int64_t v,
        std::shared_ptr<BytesRef> payload);

  Input(const std::wstring &term, int64_t v,
        std::shared_ptr<BytesRef> payload);

  Input(std::shared_ptr<BytesRef> term, int64_t v,
        std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts);

  Input(const std::wstring &term, int64_t v,
        std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts);

  Input(std::shared_ptr<BytesRef> term, int64_t v);

  Input(const std::wstring &term, int64_t v);

  Input(const std::wstring &term, int v, std::shared_ptr<BytesRef> payload,
        std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts);

  Input(std::shared_ptr<BytesRef> term, int64_t v,
        std::shared_ptr<BytesRef> payload,
        std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts);

  Input(std::shared_ptr<BytesRef> term, int64_t v,
        std::shared_ptr<BytesRef> payload, bool hasPayloads,
        std::shared_ptr<Set<std::shared_ptr<BytesRef>>> contexts,
        bool hasContexts);

  bool hasContexts();

  bool hasPayloads();
};
} // #include  "core/src/java/org/apache/lucene/search/suggest/
