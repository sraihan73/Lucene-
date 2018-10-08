#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "KoreanTokenizer.h"
#include "POS.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/ko/dict/UserDictionary.h"

#include  "core/src/java/org/apache/lucene/analysis/ko/POS.h"
#include  "core/src/java/org/apache/lucene/analysis/ko/KoreanTokenizer.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::analysis::ko
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using DecompoundMode =
    org::apache::lucene::analysis::ko::KoreanTokenizer::DecompoundMode;
using UserDictionary = org::apache::lucene::analysis::ko::dict::UserDictionary;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.TokenStream.DEFAULT_TOKEN_ATTRIBUTE_FACTORY;

/**
 * Analyzer for Korean that uses morphological analysis.
 * @see KoreanTokenizer
 * @lucene.experimental
 */
class KoreanAnalyzer : public Analyzer
{
  GET_CLASS_NAME(KoreanAnalyzer)
private:
  const std::shared_ptr<UserDictionary> userDict;
  const KoreanTokenizer::DecompoundMode mode;
  const std::shared_ptr<Set<POS::Tag>> stopTags;
  const bool outputUnknownUnigrams;

  /**
   * Creates a new KoreanAnalyzer.
   */
public:
  KoreanAnalyzer();

  /**
   * Creates a new KoreanAnalyzer.
   *
   * @param userDict Optional: if non-null, user dictionary.
   * @param mode Decompound mode.
   * @param stopTags The set of part of speech that should be filtered.
   * @param outputUnknownUnigrams If true outputs unigrams for unknown words.
   */
  KoreanAnalyzer(std::shared_ptr<UserDictionary> userDict, DecompoundMode mode,
                 std::shared_ptr<Set<POS::Tag>> stopTags,
                 bool outputUnknownUnigrams);

protected:
  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<KoreanAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<KoreanAnalyzer>(
        org.apache.lucene.analysis.Analyzer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ko/
