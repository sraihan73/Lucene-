#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::store
{
class Directory;
}
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
namespace org::apache::lucene::index
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class RepeatingTokenizer : public Tokenizer
{
  GET_CLASS_NAME(RepeatingTokenizer)

private:
  const std::shared_ptr<Random> random;
  const float percentDocs;
  const int maxTF;
  int num = 0;

public:
  std::shared_ptr<CharTermAttribute> termAtt;
  std::wstring value;

  RepeatingTokenizer(const std::wstring &val, std::shared_ptr<Random> random,
                     float percentDocs, int maxTF);

  bool incrementToken()  override;

  void reset()  override;

protected:
  std::shared_ptr<RepeatingTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<RepeatingTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};

class TestTermdocPerf : public LuceneTestCase
{
  GET_CLASS_NAME(TestTermdocPerf)

public:
  virtual void addDocs(std::shared_ptr<Random> random,
                       std::shared_ptr<Directory> dir, int const ndocs,
                       const std::wstring &field, const std::wstring &val,
                       int const maxTF,
                       float const percentDocs) ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestTermdocPerf> outerInstance;

    std::shared_ptr<Random> random;
    std::wstring val;
    int maxTF = 0;
    float percentDocs = 0;

  public:
    AnalyzerAnonymousInnerClass(std::shared_ptr<TestTermdocPerf> outerInstance,
                                std::shared_ptr<Random> random,
                                const std::wstring &val, int maxTF,
                                float percentDocs);

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
  virtual int doTest(int iter, int ndocs, int maxTF,
                     float percentDocs) ;

  virtual void testTermDocPerf() ;

protected:
  std::shared_ptr<TestTermdocPerf> shared_from_this()
  {
    return std::static_pointer_cast<TestTermdocPerf>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
