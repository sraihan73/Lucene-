#pragma once
#include "../../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Tokenizer;
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
namespace org::apache::lucene::analysis::cn::smart
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;

/**
 * Factory for {@link HMMChineseTokenizer}
 * <p>
 * Note: this class will currently emit tokens for punctuation. So you should
 * either add a WordDelimiterFilter after to remove these (with concatenate
 * off), or use the SmartChinese stoplist with a StopFilterFactory via:
 * <code>words="org/apache/lucene/analysis/cn/smart/stopwords.txt"</code>
 * @lucene.experimental
 */
class HMMChineseTokenizerFactory final : public TokenizerFactory
{
  GET_CLASS_NAME(HMMChineseTokenizerFactory)

  /** Creates a new HMMChineseTokenizerFactory */
public:
  HMMChineseTokenizerFactory(
      std::unordered_map<std::wstring, std::wstring> &args);

  std::shared_ptr<Tokenizer>
  create(std::shared_ptr<AttributeFactory> factory) override;

protected:
  std::shared_ptr<HMMChineseTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<HMMChineseTokenizerFactory>(
        org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::cn::smart
