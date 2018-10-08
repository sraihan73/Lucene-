#pragma once
#include "TokenFilter.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"

#include  "core/src/java/org/apache/lucene/util/State.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"

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
namespace org::apache::lucene::analysis
{

using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * This class can be used if the token attributes of a TokenStream
 * are intended to be consumed more than once. It caches
 * all token attribute states locally in a List when the first call to
 * {@link #incrementToken()} is called. Subsequent calls will used the cache.
 * <p>
 * <em>Important:</em> Like any proper TokenFilter, {@link #reset()} propagates
 * to the input, although only before {@link #incrementToken()} is called the
 * first time. Prior to  Lucene 5, it was never propagated.
 */
class CachingTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(CachingTokenFilter)
private:
  std::deque<std::shared_ptr<AttributeSource::State>> cache;
  std::shared_ptr<Iterator<std::shared_ptr<AttributeSource::State>>> iterator =
      nullptr;
  std::shared_ptr<AttributeSource::State> finalState;

  /**
   * Create a new CachingTokenFilter around <code>input</code>. As with
   * any normal TokenFilter, do <em>not</em> call reset on the input; this
   * filter will do it normally.
   */
public:
  CachingTokenFilter(std::shared_ptr<TokenStream> input);

  /**
   * Propagates reset if incrementToken has not yet been called. Otherwise
   * it rewinds the iterator to the beginning of the cached deque.
   */
  void reset()  override;

  /** The first time called, it'll read and cache all tokens from the input. */
  bool incrementToken()  override final;

  void end() override final;

private:
  void fillCache() ;

  /** If the underlying token stream was consumed and cached. */
public:
  bool isCached();

protected:
  std::shared_ptr<CachingTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<CachingTokenFilter>(
        TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/
