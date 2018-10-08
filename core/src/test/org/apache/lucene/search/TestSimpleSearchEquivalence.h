#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <stdexcept>
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
namespace org::apache::lucene::search
{

/**
 * Basic equivalence tests for core queries
 */
class TestSimpleSearchEquivalence : public SearchEquivalenceTestBase
{
  GET_CLASS_NAME(TestSimpleSearchEquivalence)

  // TODO: we could go a little crazy for a lot of these,
  // but these are just simple minimal cases in case something
  // goes horribly wrong. Put more intense tests elsewhere.

  /** A ⊆ (A B) */
public:
  virtual void testTermVersusBooleanOr() ;

  /** A ⊆ (+A B) */
  virtual void testTermVersusBooleanReqOpt() ;

  /** (A -B) ⊆ A */
  virtual void testBooleanReqExclVersusTerm() ;

  /** (+A +B) ⊆ (A B) */
  virtual void testBooleanAndVersusBooleanOr() ;

  /** (A B) = (A | B) */
  virtual void
  testDisjunctionSumVersusDisjunctionMax() ;

  /** "A B" ⊆ (+A +B) */
  virtual void testExactPhraseVersusBooleanAnd() ;

  /** same as above, with posincs */
  virtual void
  testExactPhraseVersusBooleanAndWithHoles() ;

  /** "A B" ⊆ "A B"~1 */
  virtual void testPhraseVersusSloppyPhrase() ;

  /** same as above, with posincs */
  virtual void
  testPhraseVersusSloppyPhraseWithHoles() ;

  /** "A B" ⊆ "A (B C)" */
  virtual void testExactPhraseVersusMultiPhrase() ;

  /** same as above, with posincs */
  virtual void
  testExactPhraseVersusMultiPhraseWithHoles() ;

  /** "A B"~∞ = +A +B if A != B */
  virtual void testSloppyPhraseVersusBooleanAnd() ;

  /** Phrase positions are relative. */
  virtual void testPhraseRelativePositions() ;

  /** Sloppy-phrase positions are relative. */
  virtual void testSloppyPhraseRelativePositions() ;

  virtual void testBoostQuerySimplification() ;

  virtual void testBooleanBoostPropagation() ;

  virtual void testBooleanOrVsSynonym() ;

protected:
  std::shared_ptr<TestSimpleSearchEquivalence> shared_from_this()
  {
    return std::static_pointer_cast<TestSimpleSearchEquivalence>(
        SearchEquivalenceTestBase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
