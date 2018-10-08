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
namespace org::apache::lucene::analysis::core
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/** An {@link Analyzer} that filters {@link LetterTokenizer}
 *  with {@link LowerCaseFilter}
 **/
class SimpleAnalyzer final : public Analyzer
{
  GET_CLASS_NAME(SimpleAnalyzer)

  /**
   * Creates a new {@link SimpleAnalyzer}
   */
public:
  SimpleAnalyzer();

protected:
  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<SimpleAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<SimpleAnalyzer>(
        org.apache.lucene.analysis.Analyzer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::core
