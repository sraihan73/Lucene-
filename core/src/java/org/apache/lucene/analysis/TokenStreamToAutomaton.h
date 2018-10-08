#pragma once
#include "../util/RollingBuffer.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
namespace org::apache::lucene::util
{
template <typename T>
class RollingBuffer;
}
#include  "core/src/java/org/apache/lucene/util/automaton/Builder.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;
using RollingBuffer = org::apache::lucene::util::RollingBuffer;
using Automaton = org::apache::lucene::util::automaton::Automaton;

// TODO: maybe also toFST?  then we can translate atts into FST outputs/weights

/** Consumes a TokenStream and creates an {@link Automaton}
 *  where the transition labels are UTF8 bytes (or Unicode
 *  code points if unicodeArcs is true) from the {@link
 *  TermToBytesRefAttribute}.  Between tokens we insert
 *  POS_SEP and for holes we insert HOLE.
 *
 * @lucene.experimental */
class TokenStreamToAutomaton
    : public std::enable_shared_from_this<TokenStreamToAutomaton>
{
  GET_CLASS_NAME(TokenStreamToAutomaton)

private:
  bool preservePositionIncrements = false;
  bool finalOffsetGapAsHole = false;
  bool unicodeArcs = false;

  /** Sole constructor. */
public:
  TokenStreamToAutomaton();

  /** Whether to generate holes in the automaton for missing positions,
   * <code>true</code> by default. */
  virtual void setPreservePositionIncrements(bool enablePositionIncrements);

  /** If true, any final offset gaps will result in adding a position hole. */
  virtual void setFinalOffsetGapAsHole(bool finalOffsetGapAsHole);

  /** Whether to make transition labels Unicode code points instead of UTF8
   * bytes, <code>false</code> by default */
  virtual void setUnicodeArcs(bool unicodeArcs);

private:
  class Position : public std::enable_shared_from_this<Position>,
                   public RollingBuffer::Resettable
  {
    GET_CLASS_NAME(Position)
    // Any tokens that ended at our position arrive to this state:
  public:
    int arriving = -1;

    // Any tokens that start at our position leave from this state:
    int leaving = -1;

    void reset() override;
  };

private:
  class Positions : public RollingBuffer<std::shared_ptr<Position>>
  {
    GET_CLASS_NAME(Positions)
  protected:
    std::shared_ptr<Position> newInstance() override;

  protected:
    std::shared_ptr<Positions> shared_from_this()
    {
      return std::static_pointer_cast<Positions>(
          org.apache.lucene.util.RollingBuffer<Position>::shared_from_this());
    }
  };

  /** Subclass and implement this if you need to change the
   *  token (such as escaping certain bytes) before it's
   *  turned into a graph. */
protected:
  virtual std::shared_ptr<BytesRef> changeToken(std::shared_ptr<BytesRef> in_);

  /** We create transition between two adjacent tokens. */
public:
  static constexpr int POS_SEP = 0x001f;

  /** We add this arc to represent a hole. */
  static constexpr int HOLE = 0x001e;

  /** Pulls the graph (including {@link
   *  PositionLengthAttribute}) from the provided {@link
   *  TokenStream}, and creates the corresponding
   *  automaton where arcs are bytes (or Unicode code points
   *  if unicodeArcs = true) from each term. */
  virtual std::shared_ptr<Automaton>
  toAutomaton(std::shared_ptr<TokenStream> in_) ;

  // for debugging!
  /*
  private static void toDot(Automaton a) throws IOException {
    final std::wstring s = a.toDot();
    Writer w = new OutputStreamWriter(new FileOutputStream("/tmp/out.dot"));
    w.write(s);
    w.close();
    System.out.println("TEST: saved to /tmp/out.dot");
  }
  */

private:
  static void
  addHoles(std::shared_ptr<Automaton::Builder> builder,
           std::shared_ptr<RollingBuffer<std::shared_ptr<Position>>> positions,
           int pos);
};

} // #include  "core/src/java/org/apache/lucene/analysis/
