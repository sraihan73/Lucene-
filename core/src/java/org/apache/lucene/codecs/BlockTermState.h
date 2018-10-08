#pragma once
#include "../index/OrdTermState.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/TermState.h"

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
namespace org::apache::lucene::codecs
{

using OrdTermState = org::apache::lucene::index::OrdTermState;
using TermState = org::apache::lucene::index::TermState;

/**
 * Holds all state required for {@link PostingsReaderBase}
 * to produce a {@link org.apache.lucene.index.PostingsEnum} without re-seeking
 * the terms dict.
 *
 * @lucene.internal
 */
class BlockTermState : public OrdTermState
{
  GET_CLASS_NAME(BlockTermState)
  /** how many docs have this term */
public:
  int docFreq = 0;
  /** total number of occurrences of this term */
  int64_t totalTermFreq = 0;

  /** the term's ord in the current block */
  int termBlockOrd = 0;
  /** fp into the terms dict primary file (_X.tim) that holds this term */
  // TODO: update BTR to nuke this
  int64_t blockFilePointer = 0;

  /** Sole constructor. (For invocation by subclass
   *  constructors, typically implicit.) */
protected:
  BlockTermState();

public:
  void copyFrom(std::shared_ptr<TermState> _other) override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<BlockTermState> shared_from_this()
  {
    return std::static_pointer_cast<BlockTermState>(
        org.apache.lucene.index.OrdTermState::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/
