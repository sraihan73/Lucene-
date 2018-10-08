#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/CharFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/MultiTermAwareComponent.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::util
{
class AbstractAnalysisFactory;
}

namespace org::apache::lucene::analysis
{
class Tokenizer;
}
namespace org::apache::lucene::util
{
class AttributeFactory;
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
namespace org::apache::lucene::analysis::custom
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using TokenFilterFactory =
    org::apache::lucene::analysis::util::TokenFilterFactory;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

class TestCustomAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestCustomAnalyzer)

  // Test some examples (TODO: we only check behavior, we may need something
  // like TestRandomChains...)

public:
  virtual void testWhitespaceFactoryWithFolding() ;

  virtual void testWhitespaceWithFolding() ;

  virtual void testFactoryHtmlStripClassicFolding() ;

  virtual void testHtmlStripClassicFolding() ;

  virtual void testStopWordsFromClasspath() ;

  virtual void testStopWordsFromClasspathWithMap() ;

  virtual void testStopWordsFromFile() ;

  virtual void testStopWordsFromFileAbsolute() ;

  // Now test misconfigurations:

  virtual void testIncorrectOrder() ;

  virtual void testMissingSPI() ;

  virtual void testSetTokenizerTwice() ;

  virtual void testSetMatchVersionTwice() ;

  virtual void testSetPosIncTwice() ;

  virtual void testSetOfsGapTwice() ;

  virtual void testNoTokenizer() ;

  virtual void testNullTokenizer() ;

  virtual void testNullTokenizerFactory() ;

  virtual void testNullParamKey() ;

  virtual void testNullMatchVersion() ;

private:
  class DummyCharFilter : public CharFilter
  {
    GET_CLASS_NAME(DummyCharFilter)

  private:
    const wchar_t match, repl;

  public:
    DummyCharFilter(std::shared_ptr<Reader> input, wchar_t match, wchar_t repl);

  protected:
    int correct(int currentOff) override;

  public:
    int read(std::deque<wchar_t> &cbuf, int off,
             int len)  override;

  protected:
    std::shared_ptr<DummyCharFilter> shared_from_this()
    {
      return std::static_pointer_cast<DummyCharFilter>(
          org.apache.lucene.analysis.CharFilter::shared_from_this());
    }
  };

public:
  class DummyCharFilterFactory : public CharFilterFactory
  {
    GET_CLASS_NAME(DummyCharFilterFactory)

  private:
    const wchar_t match, repl;

  public:
    DummyCharFilterFactory(
        std::unordered_map<std::wstring, std::wstring> &args);

    DummyCharFilterFactory(std::unordered_map<std::wstring, std::wstring> &args,
                           wchar_t match, wchar_t repl);

    std::shared_ptr<Reader> create(std::shared_ptr<Reader> input) override;

  protected:
    std::shared_ptr<DummyCharFilterFactory> shared_from_this()
    {
      return std::static_pointer_cast<DummyCharFilterFactory>(
          org.apache.lucene.analysis.util
              .CharFilterFactory::shared_from_this());
    }
  };

public:
  class DummyMultiTermAwareCharFilterFactory : public DummyCharFilterFactory,
                                               public MultiTermAwareComponent
  {
    GET_CLASS_NAME(DummyMultiTermAwareCharFilterFactory)

  public:
    DummyMultiTermAwareCharFilterFactory(
        std::unordered_map<std::wstring, std::wstring> &args);

    std::shared_ptr<AbstractAnalysisFactory> getMultiTermComponent() override;

  protected:
    std::shared_ptr<DummyMultiTermAwareCharFilterFactory> shared_from_this()
    {
      return std::static_pointer_cast<DummyMultiTermAwareCharFilterFactory>(
          DummyCharFilterFactory::shared_from_this());
    }
  };

public:
  class DummyTokenizerFactory : public TokenizerFactory
  {
    GET_CLASS_NAME(DummyTokenizerFactory)

  public:
    DummyTokenizerFactory(std::unordered_map<std::wstring, std::wstring> &args);

    std::shared_ptr<Tokenizer>
    create(std::shared_ptr<AttributeFactory> factory) override;

  protected:
    std::shared_ptr<DummyTokenizerFactory> shared_from_this()
    {
      return std::static_pointer_cast<DummyTokenizerFactory>(
          org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
    }
  };

public:
  class DummyMultiTermAwareTokenizerFactory : public DummyTokenizerFactory,
                                              public MultiTermAwareComponent
  {
    GET_CLASS_NAME(DummyMultiTermAwareTokenizerFactory)

  public:
    DummyMultiTermAwareTokenizerFactory(
        std::unordered_map<std::wstring, std::wstring> &args);

    std::shared_ptr<AbstractAnalysisFactory> getMultiTermComponent() override;

  protected:
    std::shared_ptr<DummyMultiTermAwareTokenizerFactory> shared_from_this()
    {
      return std::static_pointer_cast<DummyMultiTermAwareTokenizerFactory>(
          DummyTokenizerFactory::shared_from_this());
    }
  };

public:
  class DummyTokenFilterFactory : public TokenFilterFactory
  {
    GET_CLASS_NAME(DummyTokenFilterFactory)

  public:
    DummyTokenFilterFactory(
        std::unordered_map<std::wstring, std::wstring> &args);

    std::shared_ptr<TokenStream>
    create(std::shared_ptr<TokenStream> input) override;

  protected:
    std::shared_ptr<DummyTokenFilterFactory> shared_from_this()
    {
      return std::static_pointer_cast<DummyTokenFilterFactory>(
          org.apache.lucene.analysis.util
              .TokenFilterFactory::shared_from_this());
    }
  };

public:
  class DummyMultiTermAwareTokenFilterFactory : public DummyTokenFilterFactory,
                                                public MultiTermAwareComponent
  {
    GET_CLASS_NAME(DummyMultiTermAwareTokenFilterFactory)

  public:
    DummyMultiTermAwareTokenFilterFactory(
        std::unordered_map<std::wstring, std::wstring> &args);

    std::shared_ptr<AbstractAnalysisFactory> getMultiTermComponent() override;

  protected:
    std::shared_ptr<DummyMultiTermAwareTokenFilterFactory> shared_from_this()
    {
      return std::static_pointer_cast<DummyMultiTermAwareTokenFilterFactory>(
          DummyTokenFilterFactory::shared_from_this());
    }
  };

public:
  virtual void testNormalization() ;

  virtual void testNormalizationWithMultipleTokenFilters() ;

  virtual void testNormalizationWithMultiplCharFilters() ;

  /** test normalize where the TokenizerFactory returns a filter to normalize
   * the text */
  virtual void testNormalizationWithLowerCaseTokenizer() ;

  virtual void testConditions() ;

  virtual void testConditionsWithResourceLoader() ;

  virtual void testConditionsWithWrappedResourceLoader() ;

protected:
  std::shared_ptr<TestCustomAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestCustomAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::custom
