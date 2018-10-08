#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/LetterTokenizer.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <cctype>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStreamComponents.h"
#include  "core/src/java/org/apache/lucene/util/AttributeFactory.h"

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
namespace org::apache::lucene::analysis::util
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Testcase for {@link CharTokenizer} subclasses
 */
class TestCharTokenizers : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestCharTokenizers)

  /*
   * test to read surrogate pairs without loosing the pairing
   * if the surrogate pair is at the border of the internal IO buffer
   */
public:
  virtual void testReadSupplementaryChars() ;

  /*
   * test to extend the buffer TermAttribute buffer internally. If the internal
   * alg that extends the size of the char array only extends by 1 char and the
   * next char to be filled in is a supplementary codepoint (using 2 chars) an
   * index out of bound exception is triggered.
   */
  virtual void testExtendCharBuffer() ;

  /*
   * tests the max word length of 255 - tokenizer will split at the 255 char no
   * matter what happens
   */
  virtual void testMaxWordLength() ;

  /*
   * tests the max word length passed as parameter - tokenizer will split at the
   * passed position char no matter what happens
   */
  virtual void testCustomMaxTokenLength() ;

  /*
   * tests the max word length of 255 with a surrogate pair at position 255
   */
  virtual void testMaxWordLengthWithSupplementary() ;

  // LUCENE-3642: normalize SMP->BMP and check that offsets are correct
  virtual void testCrossPlaneNormalization() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestCharTokenizers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestCharTokenizers> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class LetterTokenizerAnonymousInnerClass : public LetterTokenizer
    {
      GET_CLASS_NAME(LetterTokenizerAnonymousInnerClass)
    private:
      std::shared_ptr<AnalyzerAnonymousInnerClass> outerInstance;

    public:
      LetterTokenizerAnonymousInnerClass(
          std::shared_ptr<AnalyzerAnonymousInnerClass> outerInstance,
          std::shared_ptr<org::apache::lucene::util::AttributeFactory>
              newAttributeFactory);

    protected:
      int normalize(int c) override;

    protected:
      std::shared_ptr<LetterTokenizerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<LetterTokenizerAnonymousInnerClass>(
            org.apache.lucene.analysis.core
                .LetterTokenizer::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

  // LUCENE-3642: normalize BMP->SMP and check that offsets are correct
public:
  virtual void testCrossPlaneNormalization2() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestCharTokenizers> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(
        std::shared_ptr<TestCharTokenizers> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  private:
    class LetterTokenizerAnonymousInnerClass2 : public LetterTokenizer
    {
      GET_CLASS_NAME(LetterTokenizerAnonymousInnerClass2)
    private:
      std::shared_ptr<AnalyzerAnonymousInnerClass2> outerInstance;

    public:
      LetterTokenizerAnonymousInnerClass2(
          std::shared_ptr<AnalyzerAnonymousInnerClass2> outerInstance,
          std::shared_ptr<org::apache::lucene::util::AttributeFactory>
              newAttributeFactory);

    protected:
      int normalize(int c) override;

    protected:
      std::shared_ptr<LetterTokenizerAnonymousInnerClass2> shared_from_this()
      {
        return std::static_pointer_cast<LetterTokenizerAnonymousInnerClass2>(
            org.apache.lucene.analysis.core
                .LetterTokenizer::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass2>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  virtual void testDefinitionUsingMethodReference1() ;

  virtual void testDefinitionUsingMethodReference2() ;

  virtual void testDefinitionUsingLambda() ;

protected:
  std::shared_ptr<TestCharTokenizers> shared_from_this()
  {
    return std::static_pointer_cast<TestCharTokenizers>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/util/
