#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopwordAnalyzerBase.h"
#include "JapaneseTokenizer.h"
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::ja::dict
{
class UserDictionary;
}

namespace org::apache::lucene::analysis::ja
{
class JapaneseTokenizer;
}
namespace org::apache::lucene::analysis
{
class CharArraySet;
}
namespace org::apache::lucene::analysis
{
class TokenStream;
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
namespace org::apache::lucene::analysis::ja
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using StopwordAnalyzerBase =
    org::apache::lucene::analysis::StopwordAnalyzerBase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Mode = org::apache::lucene::analysis::ja::JapaneseTokenizer::Mode;
using UserDictionary = org::apache::lucene::analysis::ja::dict::UserDictionary;

/**
 * Analyzer for Japanese that uses morphological analysis.
 * @see JapaneseTokenizer
 */
class JapaneseAnalyzer : public StopwordAnalyzerBase
{
  GET_CLASS_NAME(JapaneseAnalyzer)
private:
  const Mode mode;
  const std::shared_ptr<Set<std::wstring>> stoptags;
  const std::shared_ptr<UserDictionary> userDict;

public:
  JapaneseAnalyzer();

  JapaneseAnalyzer(std::shared_ptr<UserDictionary> userDict, Mode mode,
                   std::shared_ptr<CharArraySet> stopwords,
                   std::shared_ptr<Set<std::wstring>> stoptags);

  static std::shared_ptr<CharArraySet> getDefaultStopSet();

  static std::shared_ptr<Set<std::wstring>> getDefaultStopTags();

  /**
   * Atomically loads DEFAULT_STOP_SET, DEFAULT_STOP_TAGS in a lazy fashion once
   * the outer class accesses the static final set the first time.
   */
private:
  class DefaultSetHolder : public std::enable_shared_from_this<DefaultSetHolder>
  {
    GET_CLASS_NAME(DefaultSetHolder)
  public:
    static const std::shared_ptr<CharArraySet> DEFAULT_STOP_SET;
    static const std::shared_ptr<Set<std::wstring>> DEFAULT_STOP_TAGS;

  private:
    class StaticConstructor
        : public std::enable_shared_from_this<StaticConstructor>
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

  private:
    static DefaultSetHolder::StaticConstructor staticConstructor;
  };

protected:
  std::shared_ptr<TokenStreamComponents>
  createComponents(const std::wstring &fieldName) override;

  std::shared_ptr<TokenStream>
  normalize(const std::wstring &fieldName,
            std::shared_ptr<TokenStream> in_) override;

protected:
  std::shared_ptr<JapaneseAnalyzer> shared_from_this()
  {
    return std::static_pointer_cast<JapaneseAnalyzer>(
        org.apache.lucene.analysis.StopwordAnalyzerBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::ja
