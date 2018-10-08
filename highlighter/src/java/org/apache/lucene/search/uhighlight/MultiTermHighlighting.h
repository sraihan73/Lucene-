#pragma once
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/util/automaton/CharacterRunAutomaton.h"
#include  "core/src/java/org/apache/lucene/search/FuzzyQuery.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/search/AutomatonQuery.h"
#include  "core/src/java/org/apache/lucene/util/automaton/ByteRunAutomaton.h"

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
namespace org::apache::lucene::search::uhighlight
{

using Query = org::apache::lucene::search::Query;
using CharacterRunAutomaton =
    org::apache::lucene::util::automaton::CharacterRunAutomaton;

/**
 * Support for highlighting multi-term queries.
 *
 * @lucene.internal
 */
class MultiTermHighlighting
    : public std::enable_shared_from_this<MultiTermHighlighting>
{
  GET_CLASS_NAME(MultiTermHighlighting)
private:
  MultiTermHighlighting();

  /**
   * Extracts MultiTermQueries that match the provided field predicate.
   * Returns equivalent automata that will match terms.
   */
public:
  static std::deque<std::shared_ptr<CharacterRunAutomaton>> extractAutomata(
      std::shared_ptr<Query> query,
      std::function<bool(const std::wstring &)> &fieldMatcher, bool lookInSpan,
      std::function<std::deque<std::shared_ptr<Query>>> &preRewriteFunc);

private:
  class CharacterRunAutomatonAnonymousInnerClass : public CharacterRunAutomaton
  {
    GET_CLASS_NAME(CharacterRunAutomatonAnonymousInnerClass)
  private:
    std::shared_ptr<FuzzyQuery> fq;

  public:
    CharacterRunAutomatonAnonymousInnerClass(
        std::shared_ptr<Automaton> automaton, std::shared_ptr<FuzzyQuery> fq);

    virtual std::wstring toString();

  protected:
    std::shared_ptr<CharacterRunAutomatonAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<CharacterRunAutomatonAnonymousInnerClass>(
          org.apache.lucene.util.automaton
              .CharacterRunAutomaton::shared_from_this());
    }
  };

private:
  class CharacterRunAutomatonAnonymousInnerClass2 : public CharacterRunAutomaton
  {
    GET_CLASS_NAME(CharacterRunAutomatonAnonymousInnerClass2)
  private:
    std::shared_ptr<AutomatonQuery> aq;

  public:
    CharacterRunAutomatonAnonymousInnerClass2(
        std::shared_ptr<Automaton> getAutomaton,
        std::shared_ptr<AutomatonQuery> aq);

    virtual std::wstring toString();

  protected:
    std::shared_ptr<CharacterRunAutomatonAnonymousInnerClass2>
    shared_from_this()
    {
      return std::static_pointer_cast<
          CharacterRunAutomatonAnonymousInnerClass2>(
          org.apache.lucene.util.automaton
              .CharacterRunAutomaton::shared_from_this());
    }
  };

private:
  class CharacterRunAutomatonAnonymousInnerClass3 : public CharacterRunAutomaton
  {
    GET_CLASS_NAME(CharacterRunAutomatonAnonymousInnerClass3)
  private:
    std::shared_ptr<AutomatonQuery> aq;

  public:
    CharacterRunAutomatonAnonymousInnerClass3(
        std::shared_ptr<Automaton> makeEmpty,
        std::shared_ptr<AutomatonQuery> aq);

    //   TODO can we get access to the aq.compiledAutomaton.runAutomaton ?
    std::shared_ptr<ByteRunAutomaton> byteRunAutomaton =
        std::make_shared<ByteRunAutomaton>(
            aq->getAutomaton(), true,
            Operations::DEFAULT_MAX_DETERMINIZED_STATES);

    bool run(std::deque<wchar_t> &chars, int offset, int length) override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<CharacterRunAutomatonAnonymousInnerClass3>
    shared_from_this()
    {
      return std::static_pointer_cast<
          CharacterRunAutomatonAnonymousInnerClass3>(
          org.apache.lucene.util.automaton
              .CharacterRunAutomaton::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/search/uhighlight/
