#pragma once
#include "UAX29URLEmailTokenizer.h"
#include "exceptionhelper.h"
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

namespace org::apache::lucene::analysis::standard
{

using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;

/**
 * This class implements Word Break rules from the Unicode Text Segmentation
 * algorithm, as specified in
 * <a href="http://unicode.org/reports/tr29/">Unicode Standard Annex #29</a>
 * URLs and email addresses are also tokenized according to the relevant RFCs.
 * <p>
 * Tokens produced are of the following types:
 * <ul>
 *   <li>&lt;ALPHANUM&gt;: A sequence of alphabetic and numeric characters</li>
 *   <li>&lt;NUM&gt;: A number</li>
 *   <li>&lt;URL&gt;: A URL</li>
 *   <li>&lt;EMAIL&gt;: An email address</li>
 *   <li>&lt;SOUTHEAST_ASIAN&gt;: A sequence of characters from South and
 * Southeast Asian languages, including Thai, Lao, Myanmar, and Khmer</li>
 *   <li>&lt;IDEOGRAPHIC&gt;: A single CJKV ideographic character</li>
 *   <li>&lt;HIRAGANA&gt;: A single hiragana character</li>
 *   <li>&lt;KATAKANA&gt;: A sequence of katakana characters</li>
 *   <li>&lt;HANGUL&gt;: A sequence of Hangul characters</li>
 * </ul>
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @SuppressWarnings("fallthrough") public final class
// UAX29URLEmailTokenizerImpl
class UAX29URLEmailTokenizerImpl final
    : public std::enable_shared_from_this<UAX29URLEmailTokenizerImpl>
{

  /** This character denotes the end of file */
public:
  static constexpr int YYEOF = -1;

  /** initial size of the lookahead buffer */
private:
  int ZZ_BUFFERSIZE = 255;

  /** lexical states */
public:
  static constexpr int YYINITIAL = 0;
  static constexpr int AVOID_BAD_URL = 2;

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

  static const std::wstring ZZ_TRANS_PACKED_1;

  static const std::wstring ZZ_TRANS_PACKED_2;

  static const std::wstring ZZ_TRANS_PACKED_3;

  static const std::wstring ZZ_TRANS_PACKED_4;

  static const std::wstring ZZ_TRANS_PACKED_5;

  static const std::wstring ZZ_TRANS_PACKED_6;

  static const std::wstring ZZ_TRANS_PACKED_7;

  static const std::wstring ZZ_TRANS_PACKED_8;

  static const std::wstring ZZ_TRANS_PACKED_9;

  static const std::wstring ZZ_TRANS_PACKED_10;

  static const std::wstring ZZ_TRANS_PACKED_11;

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
  /** Alphanumeric sequences */
public:
  static constexpr int WORD_TYPE = UAX29URLEmailTokenizer::ALPHANUM;

  /** Numbers */
  static constexpr int NUMERIC_TYPE = UAX29URLEmailTokenizer::NUM;

  /**
   * Chars in class \p{Line_Break = Complex_Context} are from South East Asian
   * scripts (Thai, Lao, Myanmar, Khmer, etc.).  Sequences of these are kept
   * together as as a single token rather than broken up, because the logic
   * required to break them at word boundaries is too complex for UAX#29.
   * <p>
   * See Unicode Line Breaking Algorithm:
   * http://www.unicode.org/reports/tr14/#SA
   */
  static constexpr int SOUTH_EAST_ASIAN_TYPE =
      UAX29URLEmailTokenizer::SOUTHEAST_ASIAN;

  static constexpr int IDEOGRAPHIC_TYPE = UAX29URLEmailTokenizer::IDEOGRAPHIC;

  static constexpr int HIRAGANA_TYPE = UAX29URLEmailTokenizer::HIRAGANA;

  static constexpr int KATAKANA_TYPE = UAX29URLEmailTokenizer::KATAKANA;

  static constexpr int HANGUL_TYPE = UAX29URLEmailTokenizer::HANGUL;

  static constexpr int EMAIL_TYPE = UAX29URLEmailTokenizer::EMAIL;

  static constexpr int URL_TYPE = UAX29URLEmailTokenizer::URL;

  int yychar();

  /**
   * Fills CharTermAttribute with the current token text.
   */
  void getText(std::shared_ptr<CharTermAttribute> t);

  /**
   * Sets the scanner buffer size in chars
   */
  void setBufferSize(int numChars);

  /**
   * Creates a new scanner
   *
   * @param   in  the java.io.Reader to read input from.
   */
  UAX29URLEmailTokenizerImpl(std::shared_ptr<java::io::Reader> in_);

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
  void yypushback(int number);

  /**
   * Resumes scanning until the next regular expression is matched,
   * the end of input is encountered or an I/O-Error occurs.
   *
   * @return      the next token
   * @exception   java.io.IOException  if any I/O-Error occurs
   */
  int getNextToken() ;
};

} // namespace org::apache::lucene::analysis::standard