#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::wikipedia
{
class WikipediaTokenizerImpl;
}

namespace org::apache::lucene::util
{
class AttributeSource;
}
namespace org::apache::lucene::util
{
class State;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class TypeAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PositionIncrementAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class FlagsAttribute;
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
namespace org::apache::lucene::analysis::wikipedia
{

using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
using AttributeFactory = org::apache::lucene::util::AttributeFactory;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * Extension of StandardTokenizer that is aware of Wikipedia syntax.  It is
 * based off of the Wikipedia tutorial available at
 * http://en.wikipedia.org/wiki/Wikipedia:Tutorial, but it may not be complete.
 * @lucene.experimental
 */
class WikipediaTokenizer final : public Tokenizer
{
  GET_CLASS_NAME(WikipediaTokenizer)
public:
  static const std::wstring INTERNAL_LINK;
  static const std::wstring EXTERNAL_LINK;
  // The URL part of the link, i.e. the first token
  static const std::wstring EXTERNAL_LINK_URL;
  static const std::wstring CITATION;
  static const std::wstring CATEGORY;
  static const std::wstring BOLD;
  static const std::wstring ITALICS;
  static const std::wstring BOLD_ITALICS;
  static const std::wstring HEADING;
  static const std::wstring SUB_HEADING;

  static constexpr int ALPHANUM_ID = 0;
  static constexpr int APOSTROPHE_ID = 1;
  static constexpr int ACRONYM_ID = 2;
  static constexpr int COMPANY_ID = 3;
  static constexpr int EMAIL_ID = 4;
  static constexpr int HOST_ID = 5;
  static constexpr int NUM_ID = 6;
  static constexpr int CJ_ID = 7;
  static constexpr int INTERNAL_LINK_ID = 8;
  static constexpr int EXTERNAL_LINK_ID = 9;
  static constexpr int CITATION_ID = 10;
  static constexpr int CATEGORY_ID = 11;
  static constexpr int BOLD_ID = 12;
  static constexpr int ITALICS_ID = 13;
  static constexpr int BOLD_ITALICS_ID = 14;
  static constexpr int HEADING_ID = 15;
  static constexpr int SUB_HEADING_ID = 16;
  static constexpr int EXTERNAL_LINK_URL_ID = 17;

  /** std::wstring token types that correspond to token type int constants */
  static std::deque<std::wstring> const TOKEN_TYPES;

  /**
   * Only output tokens
   */
  static constexpr int TOKENS_ONLY = 0;
  /**
   * Only output untokenized tokens, which are tokens that would normally be
   * split into several tokens
   */
  static constexpr int UNTOKENIZED_ONLY = 1;
  /**
   * Output the both the untokenized token and the splits
   */
  static constexpr int BOTH = 2;
  /**
   * This flag is used to indicate that the produced "Token" would, if {@link
   * #TOKENS_ONLY} was used, produce multiple tokens.
   */
  static constexpr int UNTOKENIZED_TOKEN_FLAG = 1;
  /**
   * A private instance of the JFlex-constructed scanner
   */
private:
  const std::shared_ptr<WikipediaTokenizerImpl> scanner;

  int tokenOutput = TOKENS_ONLY;
  std::shared_ptr<Set<std::wstring>> untokenizedTypes = Collections::emptySet();
  std::shared_ptr<Iterator<std::shared_ptr<AttributeSource::State>>> tokens =
      nullptr;

  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);
  const std::shared_ptr<TypeAttribute> typeAtt =
      addAttribute(TypeAttribute::typeid);
  const std::shared_ptr<PositionIncrementAttribute> posIncrAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<CharTermAttribute> termAtt =
      addAttribute(CharTermAttribute::typeid);
  const std::shared_ptr<FlagsAttribute> flagsAtt =
      addAttribute(FlagsAttribute::typeid);

  bool first = false;

  /**
   * Creates a new instance of the {@link WikipediaTokenizer}. Attaches the
   * <code>input</code> to a newly created JFlex scanner.
   */
public:
  WikipediaTokenizer();

  /**
   * Creates a new instance of the {@link
   * org.apache.lucene.analysis.wikipedia.WikipediaTokenizer}.  Attaches the
   * <code>input</code> to a the newly created JFlex scanner.
   *
   * @param tokenOutput One of {@link #TOKENS_ONLY}, {@link #UNTOKENIZED_ONLY},
   * {@link #BOTH}
   */
  WikipediaTokenizer(int tokenOutput,
                     std::shared_ptr<Set<std::wstring>> untokenizedTypes);

  /**
   * Creates a new instance of the {@link
   * org.apache.lucene.analysis.wikipedia.WikipediaTokenizer}.  Attaches the
   * <code>input</code> to a the newly created JFlex scanner. Uses the given
   * {@link org.apache.lucene.util.AttributeFactory}.
   *
   * @param tokenOutput One of {@link #TOKENS_ONLY}, {@link #UNTOKENIZED_ONLY},
   * {@link #BOTH}
   */
  WikipediaTokenizer(std::shared_ptr<AttributeFactory> factory, int tokenOutput,
                     std::shared_ptr<Set<std::wstring>> untokenizedTypes);

private:
  void init(int tokenOutput,
            std::shared_ptr<Set<std::wstring>> untokenizedTypes);

  /*
   * (non-Javadoc)
   *
   * @see org.apache.lucene.analysis.TokenStream#next()
   */
public:
  bool incrementToken()  override final;

private:
  void collapseAndSaveTokens(int tokenType,
                             const std::wstring &type) ;

  void setupSavedToken(int positionInc, const std::wstring &type);

  void collapseTokens(int tokenType) ;

  void setupToken();

public:
  virtual ~WikipediaTokenizer();

  /*
   * (non-Javadoc)
   *
   * @see org.apache.lucene.analysis.TokenStream#reset()
   */
  void reset()  override;

  void end()  override;

protected:
  std::shared_ptr<WikipediaTokenizer> shared_from_this()
  {
    return std::static_pointer_cast<WikipediaTokenizer>(
        org.apache.lucene.analysis.Tokenizer::shared_from_this());
  }
};
} // namespace org::apache::lucene::analysis::wikipedia
