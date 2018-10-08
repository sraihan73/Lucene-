#pragma once
#include "BaseUIMATokenizer.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
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

using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * a {@link Tokenizer} which creates tokens from UIMA Annotations
 */
class UIMAAnnotationsTokenizer final : public BaseUIMATokenizer
{
  GET_CLASS_NAME(UIMAAnnotationsTokenizer)

private:
  const std::shared_ptr<CharTermAttribute> termAttr;

  const std::shared_ptr<OffsetAttribute> offsetAttr;

  const std::wstring tokenTypeString;

  int finalOffset = 0;

public:
  UIMAAnnotationsTokenizer(
      const std::wstring &descriptorPath, const std::wstring &tokenType,
      std::unordered_map<std::wstring, std::any> &configurationParameters);

  UIMAAnnotationsTokenizer(
      const std::wstring &descriptorPath, const std::wstring &tokenType,
      std::unordered_map<std::wstring, std::any> &configurationParameters,
      std::shared_ptr<AttributeFactory> factory);

protected:
  void initializeIterator()  override;

public:
  bool incrementToken()  override;

  void end()  override;

protected:
  std::shared_ptr<UIMAAnnotationsTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<UIMAAnnotationsTokenizer>(
        BaseUIMATokenizer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::uima
