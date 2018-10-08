#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Attribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/AttributeImpl.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

#include  "core/src/java/org/apache/lucene/util/AttributeImpl.h"
#include  "core/src/java/org/apache/lucene/util/AttributeReflector.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/analysis/charfilter/NormalizeCharMap.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/analysis/compound/hyphenation/HyphenationTree.h"

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
namespace org::apache::lucene::analysis::compound
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Attribute = org::apache::lucene::util::Attribute;
using AttributeImpl = org::apache::lucene::util::AttributeImpl;
using AttributeReflector = org::apache::lucene::util::AttributeReflector;

class TestCompoundWordTokenFilter : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestCompoundWordTokenFilter)

private:
  static std::shared_ptr<CharArraySet>
  makeDictionary(std::deque<std::wstring> &dictionary);

public:
  virtual void testHyphenationCompoundWordsDA() ;

  virtual void
  testHyphenationCompoundWordsDELongestMatch() ;

  /**
   * With hyphenation-only, you can get a lot of nonsense tokens.
   * This can be controlled with the min/max subword size.
   */
  virtual void testHyphenationOnly() ;

  virtual void testDumbCompoundWordsSE() ;

  virtual void testDumbCompoundWordsSELongestMatch() ;

  virtual void
  testTokenEndingWithWordComponentOfMinimumLength() ;

  virtual void
  testWordComponentWithLessThanMinimumLength() ;

  virtual void testReset() ;

  virtual void testRetainMockAttribute() ;

  virtual void testLucene8124() ;

public:
  class MockRetainAttribute : public Attribute
  {
    GET_CLASS_NAME(MockRetainAttribute)
  public:
    virtual void setRetain(bool attr) = 0;
    virtual bool getRetain() = 0;
  };

public:
  class MockRetainAttributeImpl final : public AttributeImpl,
                                        public MockRetainAttribute
  {
    GET_CLASS_NAME(MockRetainAttributeImpl)
  private:
    bool retain = false;

  public:
    void clear() override;

    bool getRetain() override;

    void setRetain(bool retain) override;

    void copyTo(std::shared_ptr<AttributeImpl> target) override;

    void reflectWith(AttributeReflector reflector) override;

  protected:
    std::shared_ptr<MockRetainAttributeImpl> shared_from_this()
    {
      return std::static_pointer_cast<MockRetainAttributeImpl>(
          org.apache.lucene.util.AttributeImpl::shared_from_this());
    }
  };

private:
  class MockRetainAttributeFilter : public TokenFilter
  {
    GET_CLASS_NAME(MockRetainAttributeFilter)

  public:
    std::shared_ptr<MockRetainAttribute> retainAtt =
        addAttribute(MockRetainAttribute::typeid);

    MockRetainAttributeFilter(std::shared_ptr<TokenStream> input);

    bool incrementToken()  override;

  protected:
    std::shared_ptr<MockRetainAttributeFilter> shared_from_this()
    {
      return std::static_pointer_cast<MockRetainAttributeFilter>(
          org.apache.lucene.analysis.TokenFilter::shared_from_this());
    }
  };

  // SOLR-2891
  // *CompoundWordTokenFilter blindly adds term length to offset, but this can
  // take things out of bounds wrt original text if a previous filter increases
  // the length of the word (in this case ü -> ue) so in this case we behave
  // like WDF, and preserve any modified offsets
public:
  virtual void testInvalidOffsets() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestCompoundWordTokenFilter> outerInstance;

    std::shared_ptr<CharArraySet> dict;
    std::shared_ptr<NormalizeCharMap> normMap;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestCompoundWordTokenFilter> outerInstance,
        std::shared_ptr<CharArraySet> dict,
        std::shared_ptr<NormalizeCharMap> normMap);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

    std::shared_ptr<Reader> initReader(const std::wstring &fieldName,
                                       std::shared_ptr<Reader> reader) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  /** blast some random strings through the analyzer */
public:
  virtual void testRandomStrings() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestCompoundWordTokenFilter> outerInstance;

    std::shared_ptr<CharArraySet> dict;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestCompoundWordTokenFilter> outerInstance,
        std::shared_ptr<CharArraySet> dict);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class AnalyzerAnonymousInnerClass3 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass3)
  private:
    std::shared_ptr<TestCompoundWordTokenFilter> outerInstance;

    std::shared_ptr<HyphenationTree> hyphenator;

  public:
    AnalyzerAnonymousInnerClass3(
        std::shared_ptr<TestCompoundWordTokenFilter> outerInstance,
        std::shared_ptr<HyphenationTree> hyphenator);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass3> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass3>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass4 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass4)
  private:
    std::shared_ptr<TestCompoundWordTokenFilter> outerInstance;

    std::shared_ptr<CharArraySet> dict;

  public:
    AnalyzerAnonymousInnerClass4(
        std::shared_ptr<TestCompoundWordTokenFilter> outerInstance,
        std::shared_ptr<CharArraySet> dict);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass4> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass4>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

private:
  class AnalyzerAnonymousInnerClass5 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass5)
  private:
    std::shared_ptr<TestCompoundWordTokenFilter> outerInstance;

    std::shared_ptr<HyphenationTree> hyphenator;

  public:
    AnalyzerAnonymousInnerClass5(
        std::shared_ptr<TestCompoundWordTokenFilter> outerInstance,
        std::shared_ptr<HyphenationTree> hyphenator);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass5> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass5>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestCompoundWordTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TestCompoundWordTokenFilter>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/compound/
