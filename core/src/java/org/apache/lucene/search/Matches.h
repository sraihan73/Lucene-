#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>
#include <functional>
#include <set>
#include "core/src/java/org/apache/lucene/search/MatchesIterator.h"
#include "core/src/java/org/apache/lucene/search/DisjunctionMatchesIterator.h"

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

namespace org::apache::lucene::search
{

/**
 * Reports the positions and optionally offsets of all matching terms in a query
 * for a single document
 *
 * To obtain a {@link MatchesIterator} for a particular field, call {@link
 * #getMatches(std::wstring)}. Note that you can call {@link #getMatches(std::wstring)}
 * multiple times to retrieve new iterators, but it is not thread-safe.
 *
 * @lucene.experimental
 */
class Matches : public std::deque<std::wstring>
{
  GET_CLASS_NAME(Matches)

  /**
   * Returns a {@link MatchesIterator} over the matches for a single field,
   * or {@code null} if there are no matches in that field.
   */
public:
  virtual std::shared_ptr<MatchesIterator>
  getMatches(const std::wstring &field) = 0;

  /**
   * Indicates a match with no term positions, for example on a Point or
   * DocValues field, or a field indexed as docs and freqs only
   */
  static const std::shared_ptr<Matches> MATCH_WITH_NO_TERMS;

  /**
   * Amalgamate a collection of {@link Matches} into a single object
   */
  // C++ TODO: There is no equivalent in C++ to Java static interface methods:
  static std::shared_ptr<Matches> fromSubMatches(std::deque<std::shared_ptr<Matches>> & subMatches)
    {
      class anon : public Matches
      {
      public:
        std::deque<std::shared_ptr<Matches>> anon_sm;
        std::shared_ptr<MatchesIterator> 
        getMatches(const std::wstring &field) override
        {
          std::deque<std::shared_ptr<MatchesIterator>> subIterators;
          for (auto & m : anon_sm)
          {
            auto it = m->getMatches(field);
            if (!it)
            {
              subIterators.push_back(it);
            }
          }
          return DisjunctionMatchesIterator::fromSubIterators(subIterators);
        }

        std::set<std::wstring> 
        iterator()
        {
          // for each sub-match, iterate its fields (it's an Iterable of the
  //        fields), and return the distinct set return sm.stream().flatMap(m ->
          std::set<std::wstring> fields;
          for (auto & m : *this) fields.insert(m);
          return fields;
        }
      };
      auto retValue = std::make_shared<anon>();
      if (subMatches.size() == 0)
      {
        return MATCH_WITH_NO_TERMS;
      }
      
      for (auto &m : subMatches)
      {
        if (m != MATCH_WITH_NO_TERMS)
        {
          retValue->anon_sm.push_back(m);
        }
      }
      if (retValue->anon_sm.size() == 0)
      {
        return MATCH_WITH_NO_TERMS;
      }
      if (retValue->anon_sm.size() == 1)
      {
        return retValue->anon_sm[0];
      }
      return retValue;
    }

  /**
   * A functional interface that supplies a {@link MatchesIterator}
   * Create a Matches for a single field
   */
  static std::shared_ptr<Matches> forField(std::wstring field, std::shared_ptr<MatchesIterator> mis)
   {
  // The indirection here, using a Supplier object rather than a MatchesIterator
  // directly, is to allow for multiple calls to Matches.getMatches() to return
  // new iterators.  We still need to call MatchesIteratorSupplier.get() eagerly
  // to work out if we have a hit or not.
      if (!mis)
      {
        return nullptr;
      }

      class anon : public Matches
      {
        std::shared_ptr<MatchesIterator> const anon_mis;
        std::wstring const anon_field;
        bool cached = true;
      public:
        anon(std::wstring & _f, std::shared_ptr<MatchesIterator> _mis): anon_mis(_mis), anon_field(_f){
        }
        std::shared_ptr<MatchesIterator> getMatches(const std::wstring &f) override
        {
          if (anon_field != f)
          {
            return nullptr;
          }
          if (cached == false)
          {
            return anon_mis;
          }
          cached = false;
          return anon_mis;
        }

        std::deque<std::wstring> iterator()
        {
          return (*this);
        }
      };
      return std::make_shared<anon>(field, mis);
    }
};

class MatchesAnonymousInnerClass final
    : public std::enable_shared_from_this<MatchesAnonymousInnerClass>,
      public Matches
{
  GET_CLASS_NAME(MatchesAnonymousInnerClass)
public:
  MatchesAnonymousInnerClass();

  std::shared_ptr<std::set<std::wstring>> iterator();

  std::shared_ptr<MatchesIterator>
  getMatches(const std::wstring &field) override;
};
} // namespace org::apache::lucene::search
