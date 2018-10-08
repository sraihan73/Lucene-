#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class FreqProxPostingsArray;
}

namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class FieldInvertState;
}
namespace org::apache::lucene::index
{
class TermsHash;
}
namespace org::apache::lucene::index
{
class TermsHashPerField;
}
namespace org::apache::lucene::index
{
class IndexableField;
}
namespace org::apache::lucene::index
{
class ParallelPostingsArray;
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
namespace org::apache::lucene::index
{

using OffsetAttribute =
    org::apache::lucene::analysis::tokenattributes::OffsetAttribute;
using PayloadAttribute =
    org::apache::lucene::analysis::tokenattributes::PayloadAttribute;

// TODO: break into separate freq and prox writers as
// codecs; make separate container (tii/tis/skip/*) that can
// be configured as any number of files 1..N
class FreqProxTermsWriterPerField final : public TermsHashPerField
{
  GET_CLASS_NAME(FreqProxTermsWriterPerField)

private:
  std::shared_ptr<FreqProxPostingsArray> freqProxPostingsArray;

public:
  const bool hasFreq;
  const bool hasProx;
  const bool hasOffsets;
  std::shared_ptr<PayloadAttribute> payloadAttribute;
  std::shared_ptr<OffsetAttribute> offsetAttribute;
  int64_t sumTotalTermFreq = 0;
  int64_t sumDocFreq = 0;

  // How many docs have this field:
  int docCount = 0;

  /** Set to true if any token had a payload in the current
   *  segment. */
  bool sawPayloads = false;

  FreqProxTermsWriterPerField(std::shared_ptr<FieldInvertState> invertState,
                              std::shared_ptr<TermsHash> termsHash,
                              std::shared_ptr<FieldInfo> fieldInfo,
                              std::shared_ptr<TermsHashPerField> nextPerField);

  void finish()  override;

  bool start(std::shared_ptr<IndexableField> f, bool first) override;

  void writeProx(int termID, int proxCode);

  void writeOffsets(int termID, int offsetAccum);

  void newTerm(int const termID) override;

  void addTerm(int const termID) override;

private:
  int getTermFreq();

public:
  void newPostingsArray() override;

  std::shared_ptr<ParallelPostingsArray> createPostingsArray(int size) override;

public:
  class FreqProxPostingsArray final : public ParallelPostingsArray
  {
    GET_CLASS_NAME(FreqProxPostingsArray)
  public:
    FreqProxPostingsArray(int size, bool writeFreqs, bool writeProx,
                          bool writeOffsets);

    std::deque<int> termFreqs;  // # times this term occurs in the current doc
    std::deque<int> lastDocIDs; // Last docID where this term occurred
    std::deque<int> lastDocCodes;  // Code for prior doc
    std::deque<int> lastPositions; // Last position where this term occurred
    std::deque<int> lastOffsets;   // Last endOffset where this term occurred

    std::shared_ptr<ParallelPostingsArray> newInstance(int size) override;

    void copyTo(std::shared_ptr<ParallelPostingsArray> toArray,
                int numToCopy) override;

    int bytesPerPosting() override;

  protected:
    std::shared_ptr<FreqProxPostingsArray> shared_from_this()
    {
      return std::static_pointer_cast<FreqProxPostingsArray>(
          ParallelPostingsArray::shared_from_this());
    }
  };

protected:
  std::shared_ptr<FreqProxTermsWriterPerField> shared_from_this()
  {
    return std::static_pointer_cast<FreqProxTermsWriterPerField>(
        TermsHashPerField::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
