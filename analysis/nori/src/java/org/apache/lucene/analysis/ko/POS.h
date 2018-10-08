#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

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

/**
 * Part of speech classification for Korean based on Sejong corpus
 * classification. The deque of tags and their meanings is available here:
 * https://docs.google.com/spreadsheets/d/1-9blXKjtjeKZqsf4NzHeYJCrr49-nXeRF6D80udfcwY
 */
class POS : public std::enable_shared_from_this<POS>
{
  GET_CLASS_NAME(POS)

  /**
   * The type of the token.
   */
public:
  enum class Type {
    GET_CLASS_NAME(Type)
    /**
     * A simple morpheme.
     */
    MORPHEME,

    /**
     * Compound noun.
     */
    COMPOUND,

    /**
     * Inflected token.
     */
    INFLECT,

    /**
     * Pre-analysis token.
     */
    PREANALYSIS,
  };

  /**
   * Part of speech tag for Korean based on Sejong corpus classification.
   */
public:
  class Tag final
  {
    GET_CLASS_NAME(Tag)
    /**
     * Verbal endings
     */
  public:
    static Tag E;

    /**
     * Interjection
     */
    static Tag IC;

    /**
     * Ending Particle
     */
    static Tag J;

    /**
     * General Adverb
     */
    static Tag MAG;

    /**
     * Conjunctive adverb
     */
    static Tag MAJ;

    /**
     * Determiner
     **/
    static Tag MM;

    /**
     * General Noun
     **/
    static Tag NNG;

    /**
     * Proper Noun
     **/
    static Tag NNP;

    /**
     * Dependent noun (following nouns)
     **/
    static Tag NNB;

    /**
     * Dependent noun
     **/
    static Tag NNBC;

    /**
     * Pronoun
     **/
    static Tag NP;

    /**
     * Numeral
     **/
    static Tag NR;

    /**
     * Terminal punctuation (? ! .)
     **/
    static Tag SF;

    /**
     * Chinese character
     **/
    static Tag SH;

    /**
     * Foreign language
     **/
    static Tag SL;

    /**
     * Number
     **/
    static Tag SN;

    /**
     * Space
     **/
    static Tag SP;

    /**
     * Closing brackets
     **/
    static Tag SSC;

    /**
     * Opening brackets
     **/
    static Tag SSO;

    /**
     * Separator (· / :)
     **/
    static Tag SC;

    /**
     * Other symbol
     **/
    static Tag SY;

    /**
     * Ellipsis
     **/
    static Tag SE;

    /**
     * Adjective
     **/
    static Tag VA;

    /**
     * Negative designator
     **/
    static Tag VCN;

    /**
     * Positive designator
     **/
    static Tag VCP;

    /**
     * Verb
     **/
    static Tag VV;

    /**
     * Auxiliary Verb or Adjective
     **/
    static Tag VX;

    /**
     * Prefix
     **/
    static Tag XPN;

    /**
     * Root
     **/
    static Tag XR;

    /**
     * Adjective Suffix
     **/
    static Tag XSA;

    /**
     * Noun Suffix
     **/
    static Tag XSN;

    /**
     * Verb Suffix
     **/
    static Tag XSV;

    /**
     * Unknown
     */
    static Tag UNKNOWN;

    /**
     * Unknown
     */
    static Tag UNA;

    /**
     * Unknown
     */
    static Tag NA;

    /**
     * Unknown
     */
    static Tag VSV;

  private:
    static std::deque<Tag> valueList;

    class StaticConstructor
    {
      GET_CLASS_NAME(StaticConstructor)
    public:
      StaticConstructor();
    };

    static StaticConstructor staticConstructor;

  public:
    enum class InnerEnum {
      GET_CLASS_NAME(InnerEnum) E,
      IC,
      J,
      MAG,
      MAJ,
      MM,
      NNG,
      NNP,
      NNB,
      NNBC,
      NP,
      NR,
      SF,
      SH,
      SL,
      SN,
      SP,
      SSC,
      SSO,
      SC,
      SY,
      SE,
      VA,
      VCN,
      VCP,
      VV,
      VX,
      XPN,
      XR,
      XSA,
      XSN,
      XSV,
      UNKNOWN,
      UNA,
      NA,
      VSV
    };

    const InnerEnum innerEnumValue;

  private:
    const std::wstring nameValue;
    const int ordinalValue;
    static int nextOrdinal;

  private:
    const int code;
    const std::wstring desc;

    /**
     * Returns the code associated with the tag (as defined in pos-id.def).
     */
  public:
    virtual int code();

    /**
     * Returns the description associated with the tag.
     */
    virtual std::wstring description();

    /**
     * Returns a new part of speech tag.
     * @param code The code for the tag.
     * @param desc The description of the tag.
     */
    Tag(const std::wstring &name, InnerEnum innerEnum,
        std::shared_ptr<POS> outerInstance, int code, const std::wstring &desc);

  public:
    bool operator==(const Tag &other);

    bool operator!=(const Tag &other);

    static std::deque<Tag> values();

    int ordinal();

    std::wstring toString();

    static Tag valueOf(const std::wstring &name);
  };

  /**
   * Returns the {@link Tag} of the provided <code>name</code>.
   */
public:
  static Tag resolveTag(const std::wstring &name);

  /**
   * Returns the {@link Tag} of the provided <code>tag</code>.
   */
  static Tag resolveTag(char tag);

  /**
   * Returns the {@link Type} of the provided <code>name</code>.
   */
  static Type resolveType(const std::wstring &name);

  /**
   * Returns the {@link Type} of the provided <code>type</code>.
   */
  static Type resolveType(char type);
};

} // namespace org::apache::lucene::analysis::ko
