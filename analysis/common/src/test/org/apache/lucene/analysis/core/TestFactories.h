#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/util/AbstractAnalysisFactory.h"

#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"
#include  "core/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include  "core/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include  "core/src/java/org/apache/lucene/analysis/util/CharFilterFactory.h"
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
namespace org::apache::lucene::analysis::core
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using DelimitedTermFrequencyTokenFilterFactory = org::apache::lucene::analysis::
    miscellaneous::DelimitedTermFrequencyTokenFilterFactory;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;

/**
 * Sanity check some things about all factories,
 * we do our best to see if we can sanely initialize it with
 * no parameters and smoke test it, etc.
 */
// TODO: move this, TestRandomChains, and TestAllAnalyzersHaveFactories
// to an integration test module that sucks in all analysis modules.
// currently the only way to do this is via eclipse etc (LUCENE-3974)

// TODO: fix this to use CustomAnalyzer instead of its own FactoryAnalyzer
class TestFactories : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestFactories)

  /** Factories that are excluded from testing it with random data */
private:
  static const std::shared_ptr<Set<std::type_info>>
      EXCLUDE_FACTORIES_RANDOM_DATA;

public:
  virtual void test() ;

private:
  void doTestTokenizer(const std::wstring &tokenizer) ;

  void doTestTokenFilter(const std::wstring &tokenfilter) ;

  void doTestCharFilter(const std::wstring &charfilter) ;

  /** tries to initialize a factory with no arguments */
  std::shared_ptr<AbstractAnalysisFactory>
  initialize(std::type_info factoryClazz) ;

  // some silly classes just so we can use checkRandomData
  std::shared_ptr<TokenizerFactory> assertingTokenizer =
      std::make_shared<TokenizerFactoryAnonymousInnerClass>(
          std::unordered_map<std::wstring, std::wstring>());

private:
  class TokenizerFactoryAnonymousInnerClass : public TokenizerFactory
  {
    GET_CLASS_NAME(TokenizerFactoryAnonymousInnerClass)
  public:
    TokenizerFactoryAnonymousInnerClass(
        std::unordered_map<std::wstring, std::wstring> &java);

    std::shared_ptr<MockTokenizer>
    create(std::shared_ptr<AttributeFactory> factory) override;

  protected:
    std::shared_ptr<TokenizerFactoryAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<TokenizerFactoryAnonymousInnerClass>(
          org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
    }
  };

private:
  class FactoryAnalyzer : public Analyzer
  {
    GET_CLASS_NAME(FactoryAnalyzer)
  public:
    const std::shared_ptr<TokenizerFactory> tokenizer;
    const std::shared_ptr<CharFilterFactory> charFilter;
    const std::shared_ptr<TokenFilterFactory> tokenfilter;

    FactoryAnalyzer(std::shared_ptr<TokenizerFactory> tokenizer,
                    std::shared_ptr<TokenFilterFactory> tokenfilter,
                    std::shared_ptr<CharFilterFactory> charFilter);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

    std::shared_ptr<Reader> initReader(const std::wstring &fieldName,
                                       std::shared_ptr<Reader> reader) override;

  protected:
    std::shared_ptr<FactoryAnalyzer> shared_from_this()
    {
      return std::static_pointer_cast<FactoryAnalyzer>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestFactories> shared_from_this()
  {
    return std::static_pointer_cast<TestFactories>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/core/
