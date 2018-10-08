#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::util::automaton
{
class CharacterRunAutomaton;
}
namespace org::apache::lucene::search::uhighlight
{
class PhraseHelper;
}
namespace org::apache::lucene::search::uhighlight
{
class UnifiedHighlighter;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search::uhighlight
{
class OffsetsEnum;
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
namespace org::apache::lucene::search::uhighlight
{

using IndexReader = org::apache::lucene::index::IndexReader;
using BytesRef = org::apache::lucene::util::BytesRef;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

/**
 * Uses term vectors that contain offsets.
 *
 * @lucene.internal
 */
class TermVectorOffsetStrategy : public FieldOffsetStrategy
{
  GET_CLASS_NAME(TermVectorOffsetStrategy)

public:
  TermVectorOffsetStrategy(
      const std::wstring &field,
      std::deque<std::shared_ptr<BytesRef>> &queryTerms,
      std::shared_ptr<PhraseHelper> phraseHelper,
      std::deque<std::shared_ptr<CharacterRunAutomaton>> &automata);

  UnifiedHighlighter::OffsetSource getOffsetSource() override;

  std::shared_ptr<OffsetsEnum>
  getOffsetsEnum(std::shared_ptr<IndexReader> reader, int docId,
                 const std::wstring &content)  override;

protected:
  std::shared_ptr<TermVectorOffsetStrategy> shared_from_this()
  {
    return std::static_pointer_cast<TermVectorOffsetStrategy>(
        FieldOffsetStrategy::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::uhighlight
