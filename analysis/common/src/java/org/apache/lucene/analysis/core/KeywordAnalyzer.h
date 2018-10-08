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

/**
 * "Tokenizes" the entire stream as a single token. This is useful
 * for data like zip codes, ids, and some product names.
 */
class KeywordAnalyzer final : public Analyzer
{
  GET_CLASS_NAME(KeywordAnalyzer)
public:
  KeywordAnalyzer();

protected:
  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

protected:
  std::shared_ptr<KeywordAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<KeywordAnalyzer>(
        org.apache.lucene.analysis.Analyzer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::core
