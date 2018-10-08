#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/analysis/sinks/States.h"

#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"
#include  "core/src/java/org/apache/lucene/util/State.h"

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
namespace org::apache::lucene::analysis::sinks
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using AttributeSource = org::apache::lucene::util::AttributeSource;

/**
 * This TokenFilter provides the ability to set aside attribute states that have
 * already been analyzed. This is useful in situations where multiple fields
 * share many common analysis steps and then go their separate ways.
 *
 * <p>
 * It is also useful for doing things like entity extraction or proper noun
 * analysis as part of the analysis workflow and saving off those tokens for use
 * in another field.
 * </p>
 *
 * <pre class="prettyprint">
 * TeeSinkTokenFilter source1 = new TeeSinkTokenFilter(new
 * WhitespaceTokenizer()); TeeSinkTokenFilter.SinkTokenStream sink1 =
 * source1.newSinkTokenStream(); TeeSinkTokenFilter.SinkTokenStream sink2 =
 * source1.newSinkTokenStream();
 *
 * TokenStream final1 = new LowerCaseFilter(source1);
 * TokenStream final2 = new EntityDetect(sink1);
 * TokenStream final3 = new URLDetect(sink2);
 *
 * d.add(new TextField("f1", final1));
 * d.add(new TextField("f2", final2));
 * d.add(new TextField("f3", final3));
 * </pre>
 *
 * <p>
 * In this example, {@code sink1} and {@code sink2} will both get tokens from
 * {@code source1} after whitespace tokenization, and will further do additional
 * token filtering, e.g. detect entities and URLs.
 * </p>
 *
 * <p>
 * <b>NOTE</b>: it is important, that tees are consumed before sinks, therefore
 * you should add them to the document before the sinks. In the above example,
 * <i>f1</i> is added before the other fields, and so by the time they are
 * processed, it has already been consumed, which is the correct way to index
 * the three streams. If for some reason you cannot ensure that, you should call
 * {@link #consumeAllTokens()} before adding the sinks to document fields.
 */
class TeeSinkTokenFilter final : public TokenFilter
{
  GET_CLASS_NAME(TeeSinkTokenFilter)

private:
  const std::shared_ptr<States> cachedStates = std::make_shared<States>();

public:
  TeeSinkTokenFilter(std::shared_ptr<TokenStream> input);

  /** Returns a new {@link SinkTokenStream} that receives all tokens consumed by
   * this stream. */
  std::shared_ptr<TokenStream> newSinkTokenStream();

  /**
   * <code>TeeSinkTokenFilter</code> passes all tokens to the added sinks when
   * itself is consumed. To be sure that all tokens from the input stream are
   * passed to the sinks, you can call this methods. This instance is exhausted
   * after this method returns, but all sinks are instant available.
   */
  void consumeAllTokens() ;

  bool incrementToken()  override;

  void end()  override final;

  void reset()  override;

  /** TokenStream output from a tee. */
public:
  class SinkTokenStream final : public TokenStream
  {
    GET_CLASS_NAME(SinkTokenStream)
  private:
    const std::shared_ptr<States> cachedStates;
    std::shared_ptr<Iterator<std::shared_ptr<AttributeSource::State>>> it =
        nullptr;

    SinkTokenStream(std::shared_ptr<AttributeSource> source,
                    std::shared_ptr<States> cachedStates);

  public:
    bool incrementToken() override final;

    void end()  override;

    void reset() override final;

  protected:
    std::shared_ptr<SinkTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<SinkTokenStream>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

  /** A convenience wrapper for storing the cached states as well the final
   * state of the stream. */
private:
  class States final : public std::enable_shared_from_this<States>
  {
    GET_CLASS_NAME(States)

  private:
    const std::deque<std::shared_ptr<AttributeSource::State>> states =
        std::deque<std::shared_ptr<AttributeSource::State>>();
    std::shared_ptr<AttributeSource::State> finalState;

  public:
    States();

    void setFinalState(std::shared_ptr<AttributeSource::State> finalState);

    std::shared_ptr<AttributeSource::State> getFinalState();

    void add(std::shared_ptr<AttributeSource::State> state);

    std::shared_ptr<Iterator<std::shared_ptr<AttributeSource::State>>>
    getStates();

    void reset();
  };

protected:
  std::shared_ptr<TeeSinkTokenFilter> shared_from_this()
  {
    return std::static_pointer_cast<TeeSinkTokenFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/sinks/
