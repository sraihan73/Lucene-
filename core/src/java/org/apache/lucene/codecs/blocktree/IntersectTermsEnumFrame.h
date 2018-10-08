#pragma once
#include "../../util/fst/FST.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::store
{
class ByteArrayDataInput;
}

namespace org::apache::lucene::util::automaton
{
class Transition;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
namespace org::apache::lucene::codecs
{
class BlockTermState;
}
namespace org::apache::lucene::codecs::blocktree
{
class IntersectTermsEnum;
}

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
namespace org::apache::lucene::codecs::blocktree
{

using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using BytesRef = org::apache::lucene::util::BytesRef;
using Transition = org::apache::lucene::util::automaton::Transition;
using FST = org::apache::lucene::util::fst::FST;

// TODO: can we share this with the frame in STE?
class IntersectTermsEnumFrame final
    : public std::enable_shared_from_this<IntersectTermsEnumFrame>
{
  GET_CLASS_NAME(IntersectTermsEnumFrame)
public:
  const int ord;
  int64_t fp = 0;
  int64_t fpOrig = 0;
  int64_t fpEnd = 0;
  int64_t lastSubFP = 0;

  // private static bool DEBUG = IntersectTermsEnum.DEBUG;

  // State in automaton
  int state = 0;

  // State just before the last label
  int lastState = 0;

  int metaDataUpto = 0;

  std::deque<char> suffixBytes = std::deque<char>(128);
  const std::shared_ptr<ByteArrayDataInput> suffixesReader =
      std::make_shared<ByteArrayDataInput>();

  std::deque<char> statBytes = std::deque<char>(64);
  const std::shared_ptr<ByteArrayDataInput> statsReader =
      std::make_shared<ByteArrayDataInput>();

  std::deque<char> floorData = std::deque<char>(32);
  const std::shared_ptr<ByteArrayDataInput> floorDataReader =
      std::make_shared<ByteArrayDataInput>();

  // Length of prefix shared by all terms in this block
  int prefix = 0;

  // Number of entries (term or sub-block) in this block
  int entCount = 0;

  // Which term we will next read
  int nextEnt = 0;

  // True if this block is either not a floor block,
  // or, it's the last sub-block of a floor block
  bool isLastInFloor = false;

  // True if all entries are terms
  bool isLeafBlock = false;

  int numFollowFloorBlocks = 0;
  int nextFloorLabel = 0;

  const std::shared_ptr<Transition> transition = std::make_shared<Transition>();
  int transitionIndex = 0;
  int transitionCount = 0;

  std::shared_ptr<FST::Arc<std::shared_ptr<BytesRef>>> arc;

  const std::shared_ptr<BlockTermState> termState;

  // metadata buffer, holding monotonic values
  std::deque<int64_t> const longs;

  // metadata buffer, holding general values
  std::deque<char> bytes = std::deque<char>(32);

  const std::shared_ptr<ByteArrayDataInput> bytesReader =
      std::make_shared<ByteArrayDataInput>();

  // Cumulative output so far
  std::shared_ptr<BytesRef> outputPrefix;

  int startBytePos = 0;
  int suffix = 0;

  // When we are on an auto-prefix term this is the starting lead byte
  // of the suffix (e.g. 'a' for the foo[a-m]* case):
  int floorSuffixLeadStart = 0;

  // When we are on an auto-prefix term this is the ending lead byte
  // of the suffix (e.g. 'm' for the foo[a-m]* case):
  int floorSuffixLeadEnd = 0;

  // True if the term we are currently on is an auto-prefix term:
  bool isAutoPrefixTerm = false;

private:
  const std::shared_ptr<IntersectTermsEnum> ite;

public:
  IntersectTermsEnumFrame(std::shared_ptr<IntersectTermsEnum> ite,
                          int ord) ;

  void loadNextFloorBlock() ;

  void setState(int state);

  void load(std::shared_ptr<BytesRef> frameIndexData) ;

  // TODO: maybe add scanToLabel; should give perf boost

  // Decodes next entry; returns true if it's a sub-block
  bool next();

  void nextLeaf();

  bool nextNonLeaf();

  int getTermBlockOrd();

  void decodeMetaData() ;
};

} // namespace org::apache::lucene::codecs::blocktree
