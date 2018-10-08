#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/TermState.h"

#include  "core/src/java/org/apache/lucene/index/IndexReaderContext.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"

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
namespace org::apache::lucene::index
{

/**
 * Maintains a {@link IndexReader} {@link TermState} view over
 * {@link IndexReader} instances containing a single term. The
 * {@link TermContext} doesn't track if the given {@link TermState}
 * objects are valid, neither if the {@link TermState} instances refer to the
 * same terms in the associated readers.
 *
 * @lucene.experimental
 */
class TermContext final : public std::enable_shared_from_this<TermContext>
{
  GET_CLASS_NAME(TermContext)

  // Important: do NOT keep hard references to index readers
private:
  const std::any topReaderContextIdentity;
  std::deque<std::shared_ptr<TermState>> const states;
  // C++ NOTE: Fields cannot have the same name as methods:
  int docFreq_ = 0;
  // C++ NOTE: Fields cannot have the same name as methods:
  int64_t totalTermFreq_ = 0;

  // public static bool DEBUG = BlockTreeTermsWriter.DEBUG;

  /**
   * Creates an empty {@link TermContext} from a {@link IndexReaderContext}
   */
public:
  TermContext(std::shared_ptr<IndexReaderContext> context);

  /**
   * Expert: Return whether this {@link TermContext} was built for the given
   * {@link IndexReaderContext}. This is typically used for assertions.
   * @lucene.internal
   */
  bool wasBuiltFor(std::shared_ptr<IndexReaderContext> context);

  /**
   * Creates a {@link TermContext} with an initial {@link TermState},
   * {@link IndexReader} pair.
   */
  TermContext(std::shared_ptr<IndexReaderContext> context,
              std::shared_ptr<TermState> state, int ord, int docFreq,
              int64_t totalTermFreq);

  /**
   * Creates a {@link TermContext} from a top-level {@link IndexReaderContext}
   * and the given {@link Term}. This method will lookup the given term in all
   * context's leaf readers and register each of the readers containing the term
   * in the returned {@link TermContext} using the leaf reader's ordinal. <p>
   * Note: the given context must be a top-level context.
   */
  static std::shared_ptr<TermContext>
  build(std::shared_ptr<IndexReaderContext> context,
        std::shared_ptr<Term> term) ;

  /**
   * Clears the {@link TermContext} internal state and removes all
   * registered {@link TermState}s
   */
  void clear();

  /**
   * Registers and associates a {@link TermState} with an leaf ordinal. The leaf
   * ordinal should be derived from a {@link IndexReaderContext}'s leaf ord.
   */
  void register_(std::shared_ptr<TermState> state, int const ord,
                 int const docFreq, int64_t const totalTermFreq);

  /**
   * Expert: Registers and associates a {@link TermState} with an leaf ordinal.
   * The leaf ordinal should be derived from a {@link IndexReaderContext}'s leaf
   * ord. On the contrary to {@link #register(TermState, int, int, long)} this
   * method does NOT update term statistics.
   */
  void register_(std::shared_ptr<TermState> state, int const ord);

  /** Expert: Accumulate term statistics. */
  void accumulateStatistics(int const docFreq, int64_t const totalTermFreq);

  /**
   * Returns the {@link TermState} for an leaf ordinal or <code>null</code> if
   * no
   * {@link TermState} for the ordinal was registered.
   *
   * @param ord
   *          the readers leaf ordinal to get the {@link TermState} for.
   * @return the {@link TermState} for the given readers ord or
   * <code>null</code> if no
   *         {@link TermState} for the reader was registered
   */
  std::shared_ptr<TermState> get(int ord);

  /**
   *  Returns the accumulated document frequency of all {@link TermState}
   *         instances passed to {@link #register(TermState, int, int, long)}.
   * @return the accumulated document frequency of all {@link TermState}
   *         instances passed to {@link #register(TermState, int, int, long)}.
   */
  int docFreq();

  /**
   *  Returns the accumulated term frequency of all {@link TermState}
   *         instances passed to {@link #register(TermState, int, int, long)}.
   * @return the accumulated term frequency of all {@link TermState}
   *         instances passed to {@link #register(TermState, int, int, long)}.
   */
  int64_t totalTermFreq();

  virtual std::wstring toString();
};

} // #include  "core/src/java/org/apache/lucene/index/
