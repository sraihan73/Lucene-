#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"

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
namespace org::apache::lucene::analysis::uima
{

using Analyzer = org::apache::lucene::analysis::Analyzer;

/**
 * {@link Analyzer} which uses the {@link UIMATypeAwareAnnotationsTokenizer} for
 * the tokenization phase
 */
class UIMATypeAwareAnalyzer final : public Analyzer
{
  GET_CLASS_NAME(UIMATypeAwareAnalyzer)
private:
  const std::wstring descriptorPath;
  const std::wstring tokenType;
  const std::wstring featurePath;
  const std::unordered_map<std::wstring, std::any> configurationParameters;

public:
  UIMATypeAwareAnalyzer(
      const std::wstring &descriptorPath, const std::wstring &tokenType,
      const std::wstring &featurePath,
      std::unordered_map<std::wstring, std::any> &configurationParameters);

protected:
  std::shared_ptr<Analyzer::TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

protected:
  std::shared_ptr<UIMATypeAwareAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<UIMATypeAwareAnalyzer>(
        org.apache.lucene.analysis.Analyzer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/uima/
