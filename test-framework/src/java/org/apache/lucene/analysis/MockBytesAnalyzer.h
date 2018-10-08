#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
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
namespace org::apache::lucene::analysis
{

using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Analyzer for testing that encodes terms as UTF-16 bytes.
 */
class MockBytesAnalyzer final : public Analyzer
{
  GET_CLASS_NAME(MockBytesAnalyzer)
protected:
  std::shared_ptr<TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<AttributeFactory>
  attributeFactory(const std::wstring &fieldName) override;

protected:
  std::shared_ptr<MockBytesAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<MockBytesAnalyzer>(
        Analyzer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
