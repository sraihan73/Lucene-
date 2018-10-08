#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
}

namespace org::apache::lucene::queryparser::xml
{
class CoreParser;
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
namespace org::apache::lucene::queryparser::xml
{

using Analyzer = org::apache::lucene::analysis::Analyzer;

class TestCorePlusQueriesParser : public TestCoreParser
{
  GET_CLASS_NAME(TestCorePlusQueriesParser)

protected:
  std::shared_ptr<CoreParser>
  newCoreParser(const std::wstring &defaultField,
                std::shared_ptr<Analyzer> analyzer) override;

public:
  virtual void testLikeThisQueryXML() ;

  virtual void testBoostingQueryXML() ;

protected:
  std::shared_ptr<TestCorePlusQueriesParser> shared_from_this()
  {
    return std::static_pointer_cast<TestCorePlusQueriesParser>(
        TestCoreParser::shared_from_this());
  }
};

} // namespace org::apache::lucene::queryparser::xml