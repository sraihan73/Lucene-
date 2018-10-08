#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/util/CharFilterFactory.h"

#include  "core/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include  "core/src/java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
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
namespace org::apache::lucene::benchmark::byTask::utils
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;

/**
 * A factory to create an analyzer.
 * See {@link org.apache.lucene.benchmark.byTask.tasks.AnalyzerFactoryTask}
 */
class AnalyzerFactory final
    : public std::enable_shared_from_this<AnalyzerFactory>
{
  GET_CLASS_NAME(AnalyzerFactory)
private:
  const std::deque<std::shared_ptr<CharFilterFactory>> charFilterFactories;
  const std::shared_ptr<TokenizerFactory> tokenizerFactory;
  const std::deque<std::shared_ptr<TokenFilterFactory>> tokenFilterFactories;
  std::wstring name = L"";
  std::optional<int> positionIncrementGap = std::nullopt;
  std::optional<int> offsetGap = std::nullopt;

public:
  AnalyzerFactory(
      std::deque<std::shared_ptr<CharFilterFactory>> &charFilterFactories,
      std::shared_ptr<TokenizerFactory> tokenizerFactory,
      std::deque<std::shared_ptr<TokenFilterFactory>> &tokenFilterFactories);

  void setName(const std::wstring &name);

  void setPositionIncrementGap(std::optional<int> &positionIncrementGap);

  void setOffsetGap(std::optional<int> &offsetGap);

  std::shared_ptr<Analyzer> create();

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<AnalyzerFactory> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(std::shared_ptr<AnalyzerFactory> outerInstance);

  private:
    const std::optional<int> outerInstance->positionIncrementGap;
    const std::optional<int> outerInstance->offsetGap;

  public:
    std::shared_ptr<Reader> initReader(const std::wstring &fieldName,
                                       std::shared_ptr<Reader> reader) override;

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  public:
    int getPositionIncrementGap(const std::wstring &fieldName) override;

    int getOffsetGap(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/utils/
