#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/BaseTokenStreamTestCase.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::synonym
{
class SynonymMap;
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
namespace org::apache::lucene::analysis::synonym
{

using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;

class TestWordnetSynonymParser : public BaseTokenStreamTestCase
{
  GET_CLASS_NAME(TestWordnetSynonymParser)

public:
  std::wstring synonymsFile =
      std::wstring(L"s(100000001,1,'woods',n,1,0).\n") +
      L"s(100000001,2,'wood',n,1,0).\n" + L"s(100000001,3,'forest',n,1,0).\n" +
      L"s(100000002,1,'wolfish',n,1,0).\n" +
      L"s(100000002,2,'ravenous',n,1,0).\n" +
      L"s(100000003,1,'king',n,1,1).\n" + L"s(100000003,2,'baron',n,1,1).\n" +
      L"s(100000004,1,'king''s evil',n,1,1).\n" +
      L"s(100000004,2,'king''s meany',n,1,1).\n";

  virtual void testSynonyms() ;

private:
  class AnalyzerAnonymousInnerClass : public Analyzer
  {
    GET_CLASS_NAME(AnalyzerAnonymousInnerClass)
  private:
    std::shared_ptr<TestWordnetSynonymParser> outerInstance;

    std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap> map_obj;

  public:
    AnalyzerAnonymousInnerClass(
        std::shared_ptr<TestWordnetSynonymParser> outerInstance,
        std::shared_ptr<org::apache::lucene::analysis::synonym::SynonymMap>
            map_obj);

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

protected:
  std::shared_ptr<TestWordnetSynonymParser> shared_from_this()
  {
    return std::static_pointer_cast<TestWordnetSynonymParser>(
        org.apache.lucene.analysis.BaseTokenStreamTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::synonym
