#pragma once
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

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
namespace org::apache::lucene::analysis::fa
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Test the Persian Analyzer
 *
 */
class TestPersianAnalyzer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestPersianAnalyzer)

  /**
   * This test fails with NPE when the stopwords file is missing in classpath
   */
public:
  virtual void testResourcesAvailable();

  /**
   * This test shows how the combination of tokenization (breaking on zero-width
   * non-joiner), normalization (such as treating arabic YEH and farsi YEH the
   * same), and stopwords creates a light-stemming effect for verbs.
   *
   * These verb forms are from http://en.wikipedia.org/wiki/Persian_grammar
   */
  virtual void testBehaviorVerbs() ;

  /**
   * This test shows how the combination of tokenization and stopwords creates a
   * light-stemming effect for verbs.
   *
   * In this case, these forms are presented with alternative orthography, using
   * arabic yeh and whitespace. This yeh phenomenon is common for legacy text
   * due to some previous bugs in Microsoft Windows.
   *
   * These verb forms are from http://en.wikipedia.org/wiki/Persian_grammar
   */
  virtual void testBehaviorVerbsDefective() ;

  /**
   * This test shows how the combination of tokenization (breaking on zero-width
   * non-joiner or space) and stopwords creates a light-stemming effect for
   * nouns, removing the plural -ha.
   */
  virtual void testBehaviorNouns() ;

  /**
   * Test showing that non-persian text is treated very much like SimpleAnalyzer
   * (lowercased, etc)
   */
  virtual void testBehaviorNonPersian() ;

  /**
   * Basic test ensuring that tokenStream works correctly.
   */
  virtual void testReusableTokenStream() ;

  /**
   * Test that custom stopwords work, and are not case-sensitive.
   */
  virtual void testCustomStopwords() ;

  /**
   * test we fold digits to latin-1
   */
  virtual void testDigits() ;

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

protected:
  std::shared_ptr<TestPersianAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<TestPersianAnalyzer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::fa
