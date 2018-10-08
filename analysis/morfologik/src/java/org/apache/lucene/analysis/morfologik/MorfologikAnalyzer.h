#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
}

// -*- c-basic-offset: 2 -*-
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
namespace org::apache::lucene::analysis::morfologik
{

using morfologik::stemming::Dictionary;

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * {@link org.apache.lucene.analysis.Analyzer} using Morfologik library.
 * @see <a href="http://morfologik.blogspot.com/">Morfologik project page</a>
 */
class MorfologikAnalyzer : public Analyzer
{
  GET_CLASS_NAME(MorfologikAnalyzer)
private:
  const std::shared_ptr<Dictionary> dictionary;

  /**
   * Builds an analyzer with an explicit {@link Dictionary} resource.
   *
   * @param dictionary A prebuilt automaton with inflected and base word forms.
   * @see <a
   * href="https://github.com/morfologik/">https://github.com/morfologik/</a>
   */
public:
  MorfologikAnalyzer(std::shared_ptr<Dictionary> dictionary);

  /**
   * Builds an analyzer with the default Morfologik's Polish dictionary.
   */
  MorfologikAnalyzer();

  /**
   * Creates a
   * {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   * which tokenizes all the text in the provided {@link Reader}.
   *
   * @param field ignored field name
   * @return A {@link org.apache.lucene.analysis.Analyzer.TokenStreamComponents}
   *         built from an {@link StandardTokenizer} filtered with
   *         {@link StandardFilter} and {@link MorfologikFilter}.
   */
protected:
  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &field) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<MorfologikAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<MorfologikAnalyzer>(
        org.apache.lucene.analysis.Analyzer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::morfologik
