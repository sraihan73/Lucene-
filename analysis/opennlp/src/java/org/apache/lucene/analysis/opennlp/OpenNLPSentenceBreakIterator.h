#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::opennlp::tools
{
class NLPSentenceDetectorOp;
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

namespace org::apache::lucene::analysis::opennlp
{

using NLPSentenceDetectorOp =
    org::apache::lucene::analysis::opennlp::tools::NLPSentenceDetectorOp;

/**
 * A {@link BreakIterator} that splits sentences using an OpenNLP sentence
 * chunking model.
 */
class OpenNLPSentenceBreakIterator final : public BreakIterator
{
  GET_CLASS_NAME(OpenNLPSentenceBreakIterator)

private:
  std::shared_ptr<CharacterIterator> text;
  int currentSentence = 0;
  std::deque<int> sentenceStarts;
  std::shared_ptr<NLPSentenceDetectorOp> sentenceOp;

public:
  OpenNLPSentenceBreakIterator(
      std::shared_ptr<NLPSentenceDetectorOp> sentenceOp);

  int current() override;

  int first() override;

  int last() override;

  int next() override;

  int following(int pos) override;

  /** Binary search over sentences */
private:
  void moveToSentenceAt(int pos, int minSentence, int maxSentence);

public:
  int previous() override;

  int preceding(int pos) override;

  int next(int n) override;

  std::shared_ptr<CharacterIterator> getText() override;

  void setText(std::shared_ptr<CharacterIterator> newText) override;

private:
  std::wstring characterIteratorToString();

protected:
  std::shared_ptr<OpenNLPSentenceBreakIterator> shared_from_this()
  {
    return std::static_pointer_cast<OpenNLPSentenceBreakIterator>(
        java.text.BreakIterator::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::opennlp
