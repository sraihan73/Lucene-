#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/AnalyzerWrapper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

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
namespace org::apache::lucene::analysis::miscellaneous
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using AnalyzerWrapper = org::apache::lucene::analysis::AnalyzerWrapper;

/**
 * This Analyzer limits the number of tokens while indexing. It is
 * a replacement for the maximum field length setting inside {@link
 * org.apache.lucene.index.IndexWriter}.
 * @see LimitTokenCountFilter
 */
class LimitTokenCountAnalyzer final : public AnalyzerWrapper
{
  GET_CLASS_NAME(LimitTokenCountAnalyzer)
private:
  const std::shared_ptr<Analyzer> delegate_;
  const int maxTokenCount;
  const bool consumeAllTokens;

  /**
   * Build an analyzer that limits the maximum number of tokens per field.
   * This analyzer will not consume any tokens beyond the maxTokenCount limit
   *
   * @see #LimitTokenCountAnalyzer(Analyzer,int,bool)
   */
public:
  LimitTokenCountAnalyzer(std::shared_ptr<Analyzer> delegate_,
                          int maxTokenCount);
  /**
   * Build an analyzer that limits the maximum number of tokens per field.
   * @param delegate the analyzer to wrap
   * @param maxTokenCount max number of tokens to produce
   * @param consumeAllTokens whether all tokens from the delegate should be
   * consumed even if maxTokenCount is reached.
   */
  LimitTokenCountAnalyzer(std::shared_ptr<Analyzer> delegate_,
                          int maxTokenCount, bool consumeAllTokens);

protected:
  std::shared_ptr<Analyzer>
  getWrappedAnalyzer(const std::wstring &fieldName) override;

  std::shared_ptr<Analyzer::TokenStreamComponents> wrapComponents(
      const std::wstring &fieldName,
      std::shared_ptr<Analyzer::TokenStreamComponents> components) override;

public:
  virtual std::wstring toString();

protected:
  std::shared_ptr<LimitTokenCountAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<LimitTokenCountAnalyzer>(
        org.apache.lucene.analysis.AnalyzerWrapper::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/miscellaneous/
