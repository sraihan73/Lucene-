#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/CharArraySet.h"

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
namespace org::apache::lucene::analysis::pt
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;

using namespace org::apache::lucene::analysis::util;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.analysis.util.StemmerUtil.*;

/**
 * Base class for stemmers that use a set of RSLP-like stemming steps.
 * <p>
 * RSLP (Removedor de Sufixos da Lingua Portuguesa) is an algorithm designed
 * originally for stemming the Portuguese language, described in the paper
 * <i>A Stemming Algorithm for the Portuguese Language</i>, Orengo et. al.
 * <p>
 * Since this time a plural-only modification (RSLP-S) as well as a modification
 * for the Galician language have been implemented. This class parses a
configuration
 * file that describes {@link Step}s, where each Step contains a set of {@link
Rule}s. GET_CLASS_NAME(parses)
 * <p>
 * The general rule format is:
 * <blockquote>{ "suffix", N, "replacement", { "exception1", "exception2",
...}}</blockquote>
 * where:
 * <ul>
 *   <li><code>suffix</code> is the suffix to be removed (such as "inho").
 *   <li><code>N</code> is the min stem size, where stem is defined as the
candidate stem
 *       after removing the suffix (but before appending the replacement!)
 *   <li><code>replacement</code> is an optimal string to append after removing
the suffix.
 *       This can be the empty string.
 *   <li><code>exceptions</code> is an optional deque of exceptions, patterns
that should
 *       not be stemmed. These patterns can be specified as whole word or suffix
(ends-with)
 *       patterns, depending upon the exceptions format flag in the step header.
 * </ul>
 * <p>
 * A step is an ordered deque of rules, with a structure in this format:
 * <blockquote>{ "name", N, B, { "cond1", "cond2", ... }
 *               ... rules ... };
 * </blockquote>
 * where:
 * <ul>
 *   <li><code>name</code> is a name for the step (such as "Plural").
 *   <li><code>N</code> is the min word size. Words that are less than this
length bypass
 *       the step completely, as an optimization. Note: N can be zero, in this
case this
 *       implementation will automatically calculate the appropriate value from
the underlying
 *       rules.
 *   <li><code>B</code> is a "bool" flag specifying how exceptions in the
rules are matched.
 *       A value of 1 indicates whole-word pattern matching, a value of 0
indicates that
 *       exceptions are actually suffixes and should be matched with ends-with.
 *   <li><code>conds</code> are an optional deque of conditions to enter the step
at all. If
 *       the deque is non-empty, then a word must end with one of these
conditions or it will
 *       bypass the step completely as an optimization.
 * </ul>
 * <p>
 * @see <a href="http://www.inf.ufrgs.br/~viviane/rslp/index.htm">RSLP
description</a>
 * @lucene.internal
 */
class RSLPStemmerBase : public std::enable_shared_from_this<RSLPStemmerBase>
{
  GET_CLASS_NAME(RSLPStemmerBase)

  /**
   * A basic rule, with no exceptions.
   */
protected:
  class Rule : public std::enable_shared_from_this<Rule>
  {
    GET_CLASS_NAME(Rule)
  protected:
    std::deque<wchar_t> const suffix;
    std::deque<wchar_t> const replacement;
    const int min;

    /**
     * Create a rule.
     * @param suffix suffix to remove
     * @param min minimum stem length
     * @param replacement replacement string
     */
  public:
    Rule(const std::wstring &suffix, int min, const std::wstring &replacement);

    /**
     * @return true if the word matches this rule.
     */
    virtual bool matches(std::deque<wchar_t> &s, int len);

    /**
     * @return new valid length of the string after firing this rule.
     */
    virtual int replace(std::deque<wchar_t> &s, int len);
  };

  /**
   * A rule with a set of whole-word exceptions.
   */
protected:
  class RuleWithSetExceptions : public Rule
  {
    GET_CLASS_NAME(RuleWithSetExceptions)
  protected:
    const std::shared_ptr<CharArraySet> exceptions;

  public:
    RuleWithSetExceptions(const std::wstring &suffix, int min,
                          const std::wstring &replacement,
                          std::deque<std::wstring> &exceptions);

    bool matches(std::deque<wchar_t> &s, int len) override;

  protected:
    std::shared_ptr<RuleWithSetExceptions> shared_from_this()
    {
      return std::static_pointer_cast<RuleWithSetExceptions>(
          Rule::shared_from_this());
    }
  };

  /**
   * A rule with a set of exceptional suffixes.
   */
protected:
  class RuleWithSuffixExceptions : public Rule
  {
    GET_CLASS_NAME(RuleWithSuffixExceptions)
    // TODO: use a more efficient datastructure: automaton?
  protected:
    std::deque<std::deque<wchar_t>> const exceptions;

  public:
    RuleWithSuffixExceptions(const std::wstring &suffix, int min,
                             const std::wstring &replacement,
                             std::deque<std::wstring> &exceptions);

    bool matches(std::deque<wchar_t> &s, int len) override;

  protected:
    std::shared_ptr<RuleWithSuffixExceptions> shared_from_this()
    {
      return std::static_pointer_cast<RuleWithSuffixExceptions>(
          Rule::shared_from_this());
    }
  };

  /**
   * A step containing a deque of rules.
   */
protected:
  class Step : public std::enable_shared_from_this<Step>
  {
    GET_CLASS_NAME(Step)
  protected:
    const std::wstring name;
    std::deque<std::shared_ptr<Rule>> const rules;
    const int min;
    std::deque<std::deque<wchar_t>> const suffixes;

    /**
     * Create a new step
     * @param name Step's name.
     * @param rules an ordered deque of rules.
     * @param min minimum word size. if this is 0 it is automatically
     * calculated.
     * @param suffixes optional deque of conditional suffixes. may be null.
     */
  public:
    Step(const std::wstring &name, std::deque<std::shared_ptr<Rule>> &rules,
         int min, std::deque<std::wstring> &suffixes);

    /**
     * @return new valid length of the string after applying the entire step.
     */
    virtual int apply(std::deque<wchar_t> &s, int len);
  };

  /**
   * Parse a resource file into an RSLP stemmer description.
   * @return a Map containing the named Steps in this description.
   */
protected:
  static std::unordered_map<std::wstring, std::shared_ptr<Step>>
  parse(std::type_info clazz, const std::wstring &resource);

private:
  static const std::shared_ptr<Pattern> headerPattern;
  static const std::shared_ptr<Pattern> stripPattern;
  static const std::shared_ptr<Pattern> repPattern;
  static const std::shared_ptr<Pattern> excPattern;

  static std::shared_ptr<Step>
  parseStep(std::shared_ptr<LineNumberReader> r,
            const std::wstring &header) ;

  static std::deque<std::shared_ptr<Rule>>
  parseRules(std::shared_ptr<LineNumberReader> r, int type) ;

  static std::deque<std::wstring> parseList(const std::wstring &s);

  static std::wstring parseString(const std::wstring &s);

  static std::wstring
  readLine(std::shared_ptr<LineNumberReader> r) ;
};

} // #include  "core/src/java/org/apache/lucene/analysis/pt/
