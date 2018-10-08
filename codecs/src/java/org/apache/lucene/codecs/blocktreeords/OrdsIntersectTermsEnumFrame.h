#pragma once
#include "../../../../../../../../core/src/java/org/apache/lucene/util/fst/FST.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/ByteArrayDataInput.h"

#include  "core/src/java/org/apache/lucene/util/automaton/Transition.h"
#include  "core/src/java/org/apache/lucene/codecs/blocktreeords/FSTOrdsOutputs.h"
#include  "core/src/java/org/apache/lucene/codecs/blocktreeords/Output.h"
namespace org::apache::lucene::util::fst
{
template <typename T>
class FST;
}
#include  "core/src/java/org/apache/lucene/codecs/BlockTermState.h"
#include  "core/src/java/org/apache/lucene/codecs/blocktreeords/OrdsIntersectTermsEnum.h"

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
namespace org::apache::lucene::codecs::blocktreeords
{

using BlockTermState = org::apache::lucene::codecs::BlockTermState;
using Output =
    org::apache::lucene::codecs::blocktreeords::FSTOrdsOutputs::Output;
using ByteArrayDataInput = org::apache::lucene::store::ByteArrayDataInput;
using Transition = org::apache::lucene::util::automaton::Transition;
using FST = org::apache::lucene::util::fst::FST;

// TODO: can we share this with the frame in STE?
class OrdsIntersectTermsEnumFrame final
    : public std::enable_shared_from_this<OrdsIntersectTermsEnumFrame>
{
  GET_CLASS_NAME(OrdsIntersectTermsEnumFrame)
public:
  const int ord;
  int64_t fp = 0;
  int64_t fpOrig = 0;
  int64_t fpEnd = 0;
  int64_t lastSubFP = 0;

  // State in automaton
  int state = 0;

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

  // Starting termOrd for this frame, used to reset termOrd in rewind()
  int64_t termOrdOrig = 0;

  // 1 + ordinal of the current term
  int64_t termOrd = 0;

  // True if this block is either not a floor block,
  // or, it's the last sub-block of a floor block
  bool isLastInFloor = false;

  // True if all entries are terms
  bool isLeafBlock = false;

  int numFollowFloorBlocks = 0;
  int nextFloorLabel = 0;

  std::shared_ptr<Transition> transition = std::make_shared<Transition>();
  int curTransitionMax = 0;
  int transitionIndex = 0;
  int transitionCount = 0;

  std::shared_ptr<FST::Arc<std::shared_ptr<Output>>> arc;

  const std::shared_ptr<BlockTermState> termState;

  // metadata buffer, holding monotonic values
  std::deque<int64_t> longs;
  // metadata buffer, holding general values
  std::deque<char> bytes;
  std::shared_ptr<ByteArrayDataInput> bytesReader;

  // Cumulative output so far
  std::shared_ptr<Output> outputPrefix;

  int startBytePos = 0;
  int suffix = 0;

private:
  const std::shared_ptr<OrdsIntersectTermsEnum> ite;

public:
  OrdsIntersectTermsEnumFrame(std::shared_ptr<OrdsIntersectTermsEnum> ite,
                              int ord) ;

  void loadNextFloorBlock() ;

  void setState(int state);

  void load(std::shared_ptr<Output> output) ;

  // TODO: maybe add scanToLabel; should give perf boost

  bool next();

  // Decodes next entry; returns true if it's a sub-block
  bool nextLeaf();

  bool nextNonLeaf();

  int getTermBlockOrd();

  void decodeMetaData() ;
};

} // #include  "core/src/java/org/apache/lucene/codecs/blocktreeords/
