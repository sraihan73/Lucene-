#pragma once
#include "../../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::analysis::icu::segmentation
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/** Test tokenizing Myanmar text into syllables */
class TestMyanmarSyllable : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestMyanmarSyllable)

public:
  std::shared_ptr<Analyzer> a;

  void setUp()  override;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestMyanmarSyllable> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestMyanmarSyllable> outerInstance);

  protected:
    std::shared_ptr<Analyzer::TokenStreamComponents>
    createComponents(const std::wstring &fieldName) override;

  protected:
    std::shared_ptr<AnalyzerAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AnalyzerAnonymousInnerClass>(
          org.apache.lucene.analysis.Analyzer::shared_from_this());
    }
  };

public:
  void tearDown()  override;

  /** as opposed to dictionary break of သက်ဝင်|လှုပ်ရှား|စေ|ပြီး */
  virtual void testBasics() ;

  // simple tests from "A Rule-based Syllable Segmentation of Myanmar Text"
  // * http://www.aclweb.org/anthology/I08-3010
  // (see also the presentation:
  // http://gii2.nagaokaut.ac.jp/gii/media/share/20080901-ZMM%20Presentation.pdf)
  // The words are fake, we just test the categories.
  // note that currently our algorithm is not sophisticated enough to handle
  // some of the special cases!

  /** constant */
  virtual void testC() ;

  /** consonant + sign */
  virtual void testCF() ;

  /** consonant + consonant + asat */
  virtual void testCCA() ;

  /** consonant + consonant + asat + sign */
  virtual void testCCAF() ;

  /** consonant + vowel */
  virtual void testCV() ;

  /** consonant + vowel + sign */
  virtual void testCVF() ;

  /** consonant + vowel + vowel + asat */
  virtual void testCVVA() ;

  /** consonant + vowel + vowel + consonant + asat */
  virtual void testCVVCA() ;

  /** consonant + vowel + vowel + consonant + asat + sign */
  virtual void testCVVCAF() ;

  /** consonant + medial */
  virtual void testCM() ;

  /** consonant + medial + sign */
  virtual void testCMF() ;

  /** consonant + medial + consonant + asat */
  virtual void testCMCA() ;

  /** consonant + medial + consonant + asat + sign */
  virtual void testCMCAF() ;

  /** consonant + medial + vowel */
  virtual void testCMV() ;

  /** consonant + medial + vowel + sign */
  virtual void testCMVF() ;

  /** consonant + medial + vowel + vowel + asat */
  virtual void testCMVVA() ;

  /** consonant + medial + vowel + vowel + consonant + asat */
  virtual void testCMVVCA() ;

  /** consonant + medial + vowel + vowel + consonant + asat + sign */
  virtual void testCMVVCAF() ;

  /** independent vowel */
  virtual void testI() ;

  /** independent vowel */
  virtual void testE() ;

protected:
  std::shared_ptr<TestMyanmarSyllable> shared_from_this()
  {
    return std::static_pointer_cast<TestMyanmarSyllable>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/icu/segmentation/
