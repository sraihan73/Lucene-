#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/charfilter/BaseCharFilter.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharacterUtils.h"

#include  "core/src/java/org/apache/lucene/analysis/CharacterBuffer.h"

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
namespace org::apache::lucene::analysis::icu
{

using CharacterUtils = org::apache::lucene::analysis::CharacterUtils;
using BaseCharFilter =
    org::apache::lucene::analysis::charfilter::BaseCharFilter;

using com::ibm::icu::text::Normalizer2;

/**
 * Normalize token text with ICU's {@link Normalizer2}.
 */
class ICUNormalizer2CharFilter final : public BaseCharFilter
{
  GET_CLASS_NAME(ICUNormalizer2CharFilter)

private:
  const std::shared_ptr<Normalizer2> normalizer;
  const std::shared_ptr<StringBuilder> inputBuffer =
      std::make_shared<StringBuilder>();
  const std::shared_ptr<StringBuilder> resultBuffer =
      std::make_shared<StringBuilder>();

  bool inputFinished = false;
  bool afterQuickCheckYes = false;
  int checkedInputBoundary = 0;
  int charCount = 0;

  /**
   * Create a new Normalizer2CharFilter that combines NFKC normalization, Case
   * Folding, and removes Default Ignorables (NFKC_Casefold)
   */
public:
  ICUNormalizer2CharFilter(std::shared_ptr<Reader> in_);

  /**
   * Create a new Normalizer2CharFilter with the specified Normalizer2
   * @param in text
   * @param normalizer normalizer to use
   */
  ICUNormalizer2CharFilter(std::shared_ptr<Reader> in_,
                           std::shared_ptr<Normalizer2> normalizer);

  // for testing ONLY
  ICUNormalizer2CharFilter(std::shared_ptr<Reader> in_,
                           std::shared_ptr<Normalizer2> normalizer,
                           int bufferSize);

  int read(std::deque<wchar_t> &cbuf, int off,
           int len)  override;

private:
  const std::shared_ptr<CharacterUtils::CharacterBuffer> tmpBuffer;

  void readInputToBuffer() ;

  int readAndNormalizeFromInput();

  int readFromInputWhileSpanQuickCheckYes();

  int readFromIoNormalizeUptoBoundary();

  int normalizeInputUpto(int const length);

  void recordOffsetDiff(int inputLength, int outputLength);

  int outputFromResultBuffer(std::deque<wchar_t> &cbuf, int begin, int len);

protected:
  std::shared_ptr<ICUNormalizer2CharFilter> shared_from_this()
  {
    return std::static_pointer_cast<ICUNormalizer2CharFilter>(
        org.apache.lucene.analysis.charfilter
            .BaseCharFilter::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/analysis/icu/
