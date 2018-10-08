#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/DelegatingAnalyzerWrapper.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
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
namespace org::apache::lucene::analysis::miscellaneous
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using DelegatingAnalyzerWrapper =
    org::apache::lucene::analysis::DelegatingAnalyzerWrapper;

/**
 * This analyzer is used to facilitate scenarios where different
 * fields require different analysis techniques.  Use the Map
 * argument in {@link #PerFieldAnalyzerWrapper(Analyzer, java.util.Map)}
 * to add non-default analyzers for fields.
 *
 * <p>Example usage:
 *
 * <pre class="prettyprint">
 * {@code
GET_CLASS_NAME(="prettyprint">)
 * Map<std::wstring,Analyzer> analyzerPerField = new HashMap<>();
 * analyzerPerField.put("firstname", new KeywordAnalyzer());
 * analyzerPerField.put("lastname", new KeywordAnalyzer());
 *
 * PerFieldAnalyzerWrapper aWrapper =
 *   new PerFieldAnalyzerWrapper(new StandardAnalyzer(version),
analyzerPerField);
 * }
 * </pre>
 *
 * <p>In this example, StandardAnalyzer will be used for all fields except
"firstname"
 * and "lastname", for which KeywordAnalyzer will be used.
 *
 * <p>A PerFieldAnalyzerWrapper can be used like any other analyzer, for both
indexing
 * and query parsing.
 */
class PerFieldAnalyzerWrapper final : public DelegatingAnalyzerWrapper
{
  GET_CLASS_NAME(PerFieldAnalyzerWrapper)
private:
  const std::shared_ptr<Analyzer> defaultAnalyzer;
  const std::unordered_map<std::wstring, std::shared_ptr<Analyzer>>
      fieldAnalyzers;

  /**
   * Constructs with default analyzer.
   *
   * @param defaultAnalyzer Any fields not specifically
   * defined to use a different analyzer will use the one provided here.
   */
public:
  PerFieldAnalyzerWrapper(std::shared_ptr<Analyzer> defaultAnalyzer);

  /**
   * Constructs with default analyzer and a map_obj of analyzers to use for
   * specific fields.
   *
   * @param defaultAnalyzer Any fields not specifically
   * defined to use a different analyzer will use the one provided here.
   * @param fieldAnalyzers a Map (std::wstring field name to the Analyzer) to be
   * used for those fields
   */
  PerFieldAnalyzerWrapper(
      std::shared_ptr<Analyzer> defaultAnalyzer,
      std::unordered_map<std::wstring, std::shared_ptr<Analyzer>>
          &fieldAnalyzers);

protected:
  std::shared_ptr<Analyzer>
  getWrappedAnalyzer(const std::wstring &fieldName) override;

public:
  virtual std::wstring toString();

protected:
  std::shared_ptr<PerFieldAnalyzerWrapper> shared_from_this()
  {
    return std::static_pointer_cast<PerFieldAnalyzerWrapper>(
        org.apache.lucene.analysis
            .DelegatingAnalyzerWrapper::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::miscellaneous
