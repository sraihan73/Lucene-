#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::uima
{
class UIMAAnnotationsTokenizer;
}

namespace org::apache::lucene::util
{
class AttributeFactory;
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
namespace org::apache::lucene::analysis::uima
{

using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * {@link org.apache.lucene.analysis.util.TokenizerFactory} for {@link
 * UIMAAnnotationsTokenizer}
 */
class UIMAAnnotationsTokenizerFactory : public TokenizerFactory
{
  GET_CLASS_NAME(UIMAAnnotationsTokenizerFactory)

private:
  std::wstring descriptorPath;
  std::wstring tokenType;
  const std::unordered_map<std::wstring, std::any> configurationParameters =
      std::unordered_map<std::wstring, std::any>();

  /** Creates a new UIMAAnnotationsTokenizerFactory */
public:
  UIMAAnnotationsTokenizerFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<UIMAAnnotationsTokenizer>
  create(std::shared_ptr<AttributeFactory> factory) override;

protected:
  std::shared_ptr<UIMAAnnotationsTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<UIMAAnnotationsTokenizerFactory>(
        org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::uima
