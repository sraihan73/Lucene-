#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::analysis::ja
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;

/**
 * Simple tests for {@link JapaneseTokenizerFactory}
 */
class TestJapaneseTokenizerFactory : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestJapaneseTokenizerFactory)
public:
  virtual void testSimple() ;

  /**
   * Test that search mode is enabled and working by default
   */
  virtual void testDefaults() ;

  /**
   * Test mode parameter: specifying normal mode
   */
  virtual void testMode() ;

  /**
   * Test user dictionary
   */
  virtual void testUserDict() ;

  /**
   * Test preserving punctuation
   */
  virtual void testPreservePunctuation() ;

  /** Test that bogus arguments result in exception */
  virtual void testBogusArguments() ;

private:
  class HashMapAnonymousInnerClass
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestJapaneseTokenizerFactory> outerInstance;

  public:
    HashMapAnonymousInnerClass(
        std::shared_ptr<TestJapaneseTokenizerFactory> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

private:
  std::shared_ptr<TokenStream>
  makeTokenStream(std::unordered_map<std::wstring, std::wstring> &args,
                  const std::wstring &in_) ;

  /**
   * Test nbestCost parameter
   */
public:
  virtual void testNbestCost() ;

private:
  class HashMapAnonymousInnerClass
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass)
  private:
    std::shared_ptr<TestJapaneseTokenizerFactory> outerInstance;

  public:
    HashMapAnonymousInnerClass(
        std::shared_ptr<TestJapaneseTokenizerFactory> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

  /**
   * Test nbestExamples parameter
   */
public:
  virtual void testNbestExample() ;

private:
  class HashMapAnonymousInnerClass2
      : public std::unordered_map<std::wstring, std::wstring>
  {
    GET_CLASS_NAME(HashMapAnonymousInnerClass2)
  private:
    std::shared_ptr<TestJapaneseTokenizerFactory> outerInstance;

  public:
    HashMapAnonymousInnerClass2(
        std::shared_ptr<TestJapaneseTokenizerFactory> outerInstance);

  protected:
    std::shared_ptr<HashMapAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<HashMapAnonymousInnerClass2>(
          java.util.HashMap<std::wstring, std::wstring>::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestJapaneseTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<TestJapaneseTokenizerFactory>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/ja/
