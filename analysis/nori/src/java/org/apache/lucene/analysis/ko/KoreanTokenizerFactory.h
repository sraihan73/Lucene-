#pragma once
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/ResourceLoaderAware.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/util/TokenizerFactory.h"
#include "KoreanTokenizer.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ko::dict
{
class UserDictionary;
}

namespace org::apache::lucene::analysis::util
{
class ResourceLoader;
}
namespace org::apache::lucene::analysis::ko
{
class KoreanTokenizer;
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
namespace org::apache::lucene::analysis::ko
{

using UserDictionary = org::apache::lucene::analysis::ko::dict::UserDictionary;
using TokenizerFactory = org::apache::lucene::analysis::util::TokenizerFactory;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;

/**
 * Factory for {@link KoreanTokenizer}.
 * @lucene.experimental
 */
class KoreanTokenizerFactory : public TokenizerFactory,
                               public ResourceLoaderAware
{
  GET_CLASS_NAME(KoreanTokenizerFactory)
private:
  static const std::wstring USER_DICT_PATH;
  static const std::wstring USER_DICT_ENCODING;
  static const std::wstring DECOMPOUND_MODE;
  static const std::wstring OUTPUT_UNKNOWN_UNIGRAMS;

  const std::wstring userDictionaryPath;
  const std::wstring userDictionaryEncoding;
  std::shared_ptr<UserDictionary> userDictionary;

  const KoreanTokenizer::DecompoundMode mode;
  const bool outputUnknownUnigrams;

  /** Creates a new KoreanTokenizerFactory */
public:
  KoreanTokenizerFactory(std::unordered_map<std::wstring, std::wstring> &args);

  void
  inform(std::shared_ptr<ResourceLoader> loader)  override;

  std::shared_ptr<KoreanTokenizer>
  create(std::shared_ptr<AttributeFactory> factory) override;

protected:
  std::shared_ptr<KoreanTokenizerFactory> shared_from_this()
  {
    return std::static_pointer_cast<KoreanTokenizerFactory>(
        org.apache.lucene.analysis.util.TokenizerFactory::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ko
