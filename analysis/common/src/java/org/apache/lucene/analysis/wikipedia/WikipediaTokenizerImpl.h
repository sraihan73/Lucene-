#pragma once
#include "WikipediaTokenizer.h"
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis::tokenattributes
{
class CharTermAttribute;
}

/* The following code was generated by JFlex 1.6.0 */

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

using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * JFlex-generated tokenizer that is aware of Wikipedia syntax.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") class WikipediaTokenizerImpl
class WikipediaTokenizerImpl
    : public std::enable_shared_from_this<WikipediaTokenizerImpl>
{

  /** This character denotes the end of file */
public:
  static constexpr int YYEOF = -1;

  /** initial size of the lookahead buffer */
private:
  static constexpr int ZZ_BUFFERSIZE = 4096;

  /** lexical states */
public:
  static constexpr int YYINITIAL = 0;
  static constexpr int CATEGORY_STATE = 2;
  static constexpr int INTERNAL_LINK_STATE = 4;
  static constexpr int EXTERNAL_LINK_STATE = 6;
  static constexpr int TWO_SINGLE_QUOTES_STATE = 8;
  static constexpr int THREE_SINGLE_QUOTES_STATE = 10;
  static constexpr int FIVE_SINGLE_QUOTES_STATE = 12;
  static constexpr int DOUBLE_EQUALS_STATE = 14;
  static constexpr int DOUBLE_BRACE_STATE = 16;
  static constexpr int STRING = 18;

  /**
   * ZZ_LEXSTATE[l] is the state in the DFA for the lexical state l
   * ZZ_LEXSTATE[l+1] is the state in the DFA for the lexical state l
   *                  at the beginning of a line
   * l is of the form l = 2*k, k a non negative integer
   */
private:
  static std::deque<int> const ZZ_LEXSTATE;

  /**
   * Translates characters to character classes
   */
  static const std::wstring ZZ_CMAP_PACKED;

  /**
   * Translates characters to character classes
   */
  static std::deque<wchar_t> const ZZ_CMAP;

  /**
   * Translates DFA states to action switch labels.
   */
  static std::deque<int> const ZZ_ACTION;

  static const std::wstring ZZ_ACTION_PACKED_0;

  static std::deque<int> zzUnpackAction();

  static int zzUnpackAction(const std::wstring &packed, int offset,
                            std::deque<int> &result);

  /**
   * Translates a state to a row index in the transition table
   */
  static std::deque<int> const ZZ_ROWMAP;

  static const std::wstring ZZ_ROWMAP_PACKED_0;

  static std::deque<int> zzUnpackRowMap();

  static int zzUnpackRowMap(const std::wstring &packed, int offset,
                            std::deque<int> &result);

  /**
   * The transition table of the DFA
   */
  static std::deque<int> const ZZ_TRANS;

  static const std::wstring ZZ_TRANS_PACKED_0;

  static std::deque<int> zzUnpackTrans();

  static int zzUnpackTrans(const std::wstring &packed, int offset,
                           std::deque<int> &result);

  /* error codes */
  static constexpr int ZZ_UNKNOWN_ERROR = 0;
  static constexpr int ZZ_NO_MATCH = 1;
  static constexpr int ZZ_PUSHBACK_2BIG = 2;

  /* error messages for the codes above */
  static std::deque<std::wstring> const ZZ_ERROR_MSG;

  /**
   * ZZ_ATTRIBUTE[aState] contains the attributes of state <code>aState</code>
   */
  static std::deque<int> const ZZ_ATTRIBUTE;

  static const std::wstring ZZ_ATTRIBUTE_PACKED_0;

  static std::deque<int> zzUnpackAttribute();

  static int zzUnpackAttribute(const std::wstring &packed, int offset,
                               std::deque<int> &result);

  /** the input device */
  std::shared_ptr<java::io::Reader> zzReader;

  /** the current state of the DFA */
  int zzState = 0;

  /** the current lexical state */
  int zzLexicalState = YYINITIAL;

  /** this buffer contains the current text to be matched and is
      the source of the yytext() string */
  std::deque<wchar_t> zzBuffer = std::deque<wchar_t>(ZZ_BUFFERSIZE);

  /** the textposition at the last accepting state */
  int zzMarkedPos = 0;

  /** the current text position in the buffer */
  int zzCurrentPos = 0;

  /** startRead marks the beginning of the yytext() string in the buffer */
  int zzStartRead = 0;

  /** endRead marks the last character in the buffer, that has been read
      from input */
  int zzEndRead = 0;

  /** number of newlines encountered up to the start of the matched text */
  int yyline = 0;

  /** the number of characters up to the start of the matched text */
  // C++ NOTE: Fields cannot have the same name as methods:
  int yychar_ = 0;

  /**
   * the number of characters from the last newline up to the start of the
   * matched text
   */
  int yycolumn = 0;

  /**
   * zzAtBOL == true <=> the scanner is currently at the beginning of a line
   */
  bool zzAtBOL = true;

  /** zzAtEOF == true <=> the scanner is at the EOF */
  bool zzAtEOF = false;

  /** denotes if the user-EOF-code has already been executed */
  bool zzEOFDone = false;

  /**
   * The number of occupied positions in zzBuffer beyond zzEndRead.
   * When a lead/high surrogate has been read from the input stream
   * into the final zzBuffer position, this will have a value of 1;
   * otherwise, it will have a value of 0.
   */
  int zzFinalHighSurrogate = 0;

  /* user code: */

public:
  static constexpr int ALPHANUM = WikipediaTokenizer::ALPHANUM_ID;
  static constexpr int APOSTROPHE = WikipediaTokenizer::APOSTROPHE_ID;
  static constexpr int ACRONYM = WikipediaTokenizer::ACRONYM_ID;
  static constexpr int COMPANY = WikipediaTokenizer::COMPANY_ID;
  static constexpr int EMAIL = WikipediaTokenizer::EMAIL_ID;
  static constexpr int HOST = WikipediaTokenizer::HOST_ID;
  static constexpr int NUM = WikipediaTokenizer::NUM_ID;
  static constexpr int CJ = WikipediaTokenizer::CJ_ID;
  static constexpr int INTERNAL_LINK = WikipediaTokenizer::INTERNAL_LINK_ID;
  static constexpr int EXTERNAL_LINK = WikipediaTokenizer::EXTERNAL_LINK_ID;
  static constexpr int CITATION = WikipediaTokenizer::CITATION_ID;
  static constexpr int CATEGORY = WikipediaTokenizer::CATEGORY_ID;
  static constexpr int BOLD = WikipediaTokenizer::BOLD_ID;
  static constexpr int ITALICS = WikipediaTokenizer::ITALICS_ID;
  static constexpr int BOLD_ITALICS = WikipediaTokenizer::BOLD_ITALICS_ID;
  static constexpr int HEADING = WikipediaTokenizer::HEADING_ID;
  static constexpr int SUB_HEADING = WikipediaTokenizer::SUB_HEADING_ID;
  static constexpr int EXTERNAL_LINK_URL =
      WikipediaTokenizer::EXTERNAL_LINK_URL_ID;

private:
  int currentTokType = 0;
  int numBalanced = 0;
  int positionInc = 1;
  int numLinkToks = 0;
  // Anytime we start a new on a Wiki reserved token (category, link, etc.) this
  // value will be 0, otherwise it will be the number of tokens seen this can be
  // useful for detecting when a new reserved token is encountered see
  // https://issues.apache.org/jira/browse/LUCENE-1133
  int numWikiTokensSeen = 0;

public:
  static std::deque<std::wstring> const TOKEN_TYPES;

  /**
  Returns the number of tokens seen inside a category or link, etc.
  @return the number of tokens seen inside the context of wiki syntax.
  **/
  int getNumWikiTokensSeen();

  int yychar();

  int getPositionIncrement();

  /**
   * Fills Lucene token with the current token text.
   */
  void getText(std::shared_ptr<CharTermAttribute> t);

  int setText(std::shared_ptr<StringBuilder> buffer);

  void reset();

  /**
   * Creates a new scanner
   *
   * @param   in  the java.io.Reader to read input from.
   */
  WikipediaTokenizerImpl(std::shared_ptr<java::io::Reader> in_);

  /**
   * Unpacks the compressed character translation table.
   *
   * @param packed   the packed character translation table
   * @return         the unpacked character translation table
   */
private:
  static std::deque<wchar_t> zzUnpackCMap(const std::wstring &packed);

  /**
   * Refills the input buffer.
   *
   * @return      <code>false</code>, iff there was new input.
   *
   * @exception   java.io.IOException  if any I/O-Error occurs
   */
  bool zzRefill() ;

  /**
   * Closes the input stream.
   */
public:
  void yyclose() ;

  /**
   * Resets the scanner to read from a new input stream.
   * Does not close the old reader.
   *
   * All internal variables are reset, the old input stream
   * <b>cannot</b> be reused (internal buffer is discarded and lost).
   * Lexical state is set to <tt>ZZ_INITIAL</tt>.
   *
   * Internal scan buffer is resized down to its initial length, if it has
   * grown.
   *
   * @param reader   the new input stream
   */
  void yyreset(std::shared_ptr<java::io::Reader> reader);

  /**
   * Returns the current lexical state.
   */
  int yystate();

  /**
   * Enters a new lexical state
   *
   * @param newState the new lexical state
   */
  void yybegin(int newState);

  /**
   * Returns the text matched by the current regular expression.
   */
  std::wstring yytext();

  /**
   * Returns the character at position <tt>pos</tt> from the
   * matched text.
   *
   * It is equivalent to yytext().charAt(pos), but faster
   *
   * @param pos the position of the character to fetch.
   *            A value from 0 to yylength()-1.
   *
   * @return the character at position pos
   */
  wchar_t yycharat(int pos);

  /**
   * Returns the length of the matched text region.
   */
  int yylength();

  /**
   * Reports an error that occured while scanning.
   *
   * In a wellformed scanner (no or only correct usage of
   * yypushback(int) and a match-all fallback rule) this method
   * will only be called with things that "Can't Possibly Happen".
   * If this method is called, something is seriously wrong
   * (e.g. a JFlex bug producing a faulty scanner etc.).
   *
   * Usual syntax/scanner level error handling should be done
   * in error fallback rules.
   *
   * @param   errorCode  the code of the errormessage to display
   */
private:
  void zzScanError(int errorCode);

  /**
   * Pushes the specified amount of characters back into the input stream.
   *
   * They will be read again by then next call of the scanning method
   *
   * @param number  the number of characters to be read again.
   *                This number must not be greater than yylength()!
   */
public:
  virtual void yypushback(int number);

  /**
   * Resumes scanning until the next regular expression is matched,
   * the end of input is encountered or an I/O-Error occurs.
   *
   * @return      the next token
   * @exception   java.io.IOException  if any I/O-Error occurs
   */
  virtual int getNextToken() ;
};

} // namespace org::apache::lucene::analysis::wikipedia
