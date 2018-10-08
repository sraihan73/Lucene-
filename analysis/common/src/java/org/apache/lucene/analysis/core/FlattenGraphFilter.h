#pragma once
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/RollingBuffer.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/AttributeSource.h"

#include  "core/src/java/org/apache/lucene/util/State.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionLengthAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/OffsetAttribute.h"
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

namespace org::apache::lucene::analysis::core
{

using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using PositionLengthAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionLengthAttribute;
using AttributeSource = org::apache::lucene::util::AttributeSource;
using RollingBuffer = org::apache::lucene::util::RollingBuffer;

/**
 * Converts an incoming graph token stream, such as one from
 * {@link SynonymGraphFilter}, into a flat form so that
 * all nodes form a single linear chain with no side paths.  Every
 * path through the graph touches every node.  This is necessary
 * when indexing a graph token stream, because the index does not
 * save {@link PositionLengthAttribute} and so it cannot
 * preserve the graph structure.  However, at search time,
 * query parsers can correctly handle the graph and this token
 * filter should <b>not</b> be used.
 *
 * <p>If the graph was not already flat to start, this
 * is likely a lossy process, i.e. it will often cause the
 * graph to accept token sequences it should not, and to
 * reject token sequences it should not.
 *
 * <p>However, when applying synonyms during indexing, this
 * is necessary because Lucene already does not index a graph
 * and so the indexing process is already lossy
 * (it ignores the {@link PositionLengthAttribute}).
 *
 * @lucene.experimental
 */
class FlattenGraphFilter final : public TokenFilter
{
  GET_CLASS_NAME(FlattenGraphFilter)

  /** Holds all tokens leaving a given input position. */
private:
  class InputNode final : public std::enable_shared_from_this<InputNode>,
                          public RollingBuffer::Resettable
  {
    GET_CLASS_NAME(InputNode)
  private:
    const std::deque<std::shared_ptr<AttributeSource::State>> tokens =
        std::deque<std::shared_ptr<AttributeSource::State>>();

    /** Our input node, or -1 if we haven't been assigned yet */
  public:
    int node = -1;

    /** Maximum to input node for all tokens leaving here; we use this
     *  to know when we can freeze. */
    int maxToNode = -1;

    /** Where we currently map_obj to; this changes (can only
     *  increase as we see more input tokens), until we are finished
     *  with this position. */
    int outputNode = -1;

    /** Which token (index into {@link #tokens}) we will next output. */
    int nextOut = 0;

    void reset() override;
  };

  /** Gathers up merged input positions into a single output position,
   *  only for the current "frontier" of nodes we've seen but can't yet
   *  output because they are not frozen. */
private:
  class OutputNode final : public std::enable_shared_from_this<OutputNode>,
                           public RollingBuffer::Resettable
  {
    GET_CLASS_NAME(OutputNode)
  private:
    const std::deque<int> inputNodes = std::deque<int>();

    /** Node ID for this output, or -1 if we haven't been assigned yet. */
  public:
    int node = -1;

    /** Which input node (index into {@link #inputNodes}) we will next output.
     */
    int nextOut = 0;

    /** Start offset of tokens leaving this node. */
    int startOffset = -1;

    /** End offset of tokens arriving to this node. */
    int endOffset = -1;

    void reset() override;
  };

private:
  const std::shared_ptr<RollingBuffer<std::shared_ptr<InputNode>>> inputNodes =
      std::make_shared<RollingBufferAnonymousInnerClass>();

private:
  class RollingBufferAnonymousInnerClass
      : public RollingBuffer<std::shared_ptr<InputNode>>
  {
    GET_CLASS_NAME(RollingBufferAnonymousInnerClass)
  public:
    RollingBufferAnonymousInnerClass();

  protected:
    std::shared_ptr<InputNode> newInstance() override;

  protected:
    std::shared_ptr<RollingBufferAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<RollingBufferAnonymousInnerClass>(
          org.apache.lucene.util.RollingBuffer<InputNode>::shared_from_this());
    }
  };

private:
  const std::shared_ptr<RollingBuffer<std::shared_ptr<OutputNode>>>
      outputNodes = std::make_shared<RollingBufferAnonymousInnerClass2>();

private:
  class RollingBufferAnonymousInnerClass2
      : public RollingBuffer<std::shared_ptr<OutputNode>>
  {
    GET_CLASS_NAME(RollingBufferAnonymousInnerClass2)
  public:
    RollingBufferAnonymousInnerClass2();

  protected:
    std::shared_ptr<OutputNode> newInstance() override;

  protected:
    std::shared_ptr<RollingBufferAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<RollingBufferAnonymousInnerClass2>(
          org.apache.lucene.util.RollingBuffer<OutputNode>::shared_from_this());
    }
  };

private:
  const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
      addAttribute(PositionIncrementAttribute::typeid);
  const std::shared_ptr<PositionLengthAttribute> posLenAtt =
      addAttribute(PositionLengthAttribute::typeid);
  const std::shared_ptr<OffsetAttribute> offsetAtt =
      addAttribute(OffsetAttribute::typeid);

  /** Which input node the last seen token leaves from */
  int inputFrom = 0;

  /** We are currently releasing tokens leaving from this output node */
  int outputFrom = 0;

  // for debugging:
  // private int retOutputFrom;

  bool done = false;

  int lastOutputFrom = 0;

  int finalOffset = 0;

  int finalPosInc = 0;

  int maxLookaheadUsed = 0;

  int lastStartOffset = 0;

public:
  FlattenGraphFilter(std::shared_ptr<TokenStream> in_);

private:
  bool releaseBufferedToken();

public:
  bool incrementToken()  override;

  // Only for debugging:
  /*
  private void printStates() {
    System.out.println("states:");
    for(int i=outputFrom;i<outputNodes.getMaxPos();i++) {
      OutputNode outputNode = outputNodes.get(i);
      System.out.println("  output " + i + ": inputs " + outputNode.inputNodes);
      for(int inputNodeID : outputNode.inputNodes) {
        InputNode inputNode = inputNodes.get(inputNodeID);
        assert inputNode.outputNode == i;
      }
    }
  }
  */

  void end()  override;

  void reset()  override;

  /** For testing */
  int getMaxLookaheadUsed();

protected:
  std::shared_ptr<FlattenGraphFilter> shared_from_this()
  {
    return std::static_pointer_cast<FlattenGraphFilter>(
        org.apache.lucene.analysis.TokenFilter::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/analysis/core/
