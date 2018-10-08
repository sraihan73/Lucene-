#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class IndexInput;
}

namespace org::apache::lucene::search::suggest::document
{
class NRTSuggester;
}
namespace org::apache::lucene::util
{
class Accountable;
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
namespace org::apache::lucene::search::suggest::document
{

using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;

/**
 * Holder for suggester and field-level info
 * for a suggest field
 *
 * @lucene.experimental
 */
class CompletionsTermsReader final
    : public std::enable_shared_from_this<CompletionsTermsReader>,
      public Accountable
{
  GET_CLASS_NAME(CompletionsTermsReader)
  /** Minimum entry weight for the suggester */
public:
  const int64_t minWeight;
  /** Maximum entry weight for the suggester */
  const int64_t maxWeight;
  /** type of suggester (context-enabled or not) */
  const char type;

private:
  const std::shared_ptr<IndexInput> dictIn;
  const int64_t offset;

  // C++ NOTE: Fields cannot have the same name as methods:
  std::shared_ptr<NRTSuggester> suggester_;

  /**
   * Creates a CompletionTermsReader to load a field-specific suggester
   * from the index <code>dictIn</code> with <code>offset</code>
   */
public:
  CompletionsTermsReader(std::shared_ptr<IndexInput> dictIn, int64_t offset,
                         int64_t minWeight, int64_t maxWeight,
                         char type) ;

  /**
   * Returns the suggester for a field, if not loaded already, loads
   * the appropriate suggester from CompletionDictionary
   */
  // C++ WARNING: The following method was originally marked 'synchronized':
  std::shared_ptr<NRTSuggester> suggester() ;

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;
};

} // namespace org::apache::lucene::search::suggest::document
