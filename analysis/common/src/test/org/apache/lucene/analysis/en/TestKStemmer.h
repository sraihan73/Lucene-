#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::analysis
{
class TokenStreamComponents;
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
namespace org::apache::lucene::analysis::en
{

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.analysis.VocabularyAssert.assertVocabulary;

using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

/**
 * Tests for {@link KStemmer}
 */
class TestKStemmer : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestKStemmer)
private:
  std::shared_ptr<Analyzer> a;

public:
  void setUp()  override;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestKStemmer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass(std::shared_ptr<TestKStemmer> outerInstance);

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

  /** blast some random strings through the analyzer */
  virtual void testRandomStrings() ;

  /**
   * test the kstemmer optimizations against a bunch of words
   * that were stemmed with the original java kstemmer (generated from
   * testCreateMap, commented out below).
   */
  virtual void testVocabulary() ;

  virtual void testEmptyTerm() ;

private:
  class AnalyzerAnonymousInnerClass2 : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass2)
  private:
    std::shared_ptr<TestKStemmer> outerInstance;

  public:
    AnalyzerAnonymousInnerClass2(std::shared_ptr<TestKStemmer> outerInstance);

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

  /****** requires original java kstem source code to create map_obj
  public void testCreateMap() throws Exception {
    std::wstring input = getBigDoc();
    Reader r = new StringReader(input);
    TokenFilter tf = new LowerCaseFilter(new LetterTokenizer(r));
    // tf = new KStemFilter(tf);

    KStemmer kstem = new KStemmer();
    Map<std::wstring,std::wstring> map_obj = new TreeMap<>();
    for(;;) {
      Token t = tf.next();
      if (t==null) break;
      std::wstring s = t.termText();
      if (map_obj.containsKey(s)) continue;
      map_obj.put(s, kstem.stem(s));
    }

    Writer out = new BufferedWriter(new FileWriter("kstem_examples.txt"));
    for (std::wstring key : map_obj.keySet()) {
      out.write(key);
      out.write('\t');
      out.write(map_obj.get(key));
      out.write('\n');
    }
    out.close();
  }
  ******/

protected:
  std::shared_ptr<TestKStemmer> shared_from_this()
  {
    return std::static_pointer_cast<TestKStemmer>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::en
