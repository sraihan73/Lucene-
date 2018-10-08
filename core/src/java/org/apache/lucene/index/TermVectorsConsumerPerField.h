#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class TermVectorsPostingsArray;
}

namespace org::apache::lucene::index
{
class TermVectorsConsumer;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class OffsetAttribute;
}
namespace org::apache::lucene::analysis::tokenattributes
{
class PayloadAttribute;
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

class TermVectorsConsumerPerField final : public TermsHashPerField
{
  GET_CLASS_NAME(TermVectorsConsumerPerField)

private:
  std::shared_ptr<TermVectorsPostingsArray> termVectorsPostingsArray;

public:
  const std::shared_ptr<TermVectorsConsumer> termsWriter;

  bool doVectors = false;
  bool doVectorPositions = false;
  bool doVectorOffsets = false;
  bool doVectorPayloads = false;

  std::shared_ptr<OffsetAttribute> offsetAttribute;
  std::shared_ptr<PayloadAttribute> payloadAttribute;
  bool hasPayloads =
      false; // if enabled, and we actually saw any for this field

  TermVectorsConsumerPerField(std::shared_ptr<FieldInvertState> invertState,
                              std::shared_ptr<TermVectorsConsumer> termsWriter,
                              std::shared_ptr<FieldInfo> fieldInfo);

  /** Called once per field per document if term vectors
   *  are enabled, to write the vectors to
   *  RAMOutputStream, which is then quickly flushed to
   *  the real term vectors files in the Directory. */
  void finish() override;

  void finishDocument() ;

  bool start(std::shared_ptr<IndexableField> field, bool first) override;

  void writeProx(std::shared_ptr<TermVectorsPostingsArray> postings,
                 int termID);

  void newTerm(int const termID) override;

  void addTerm(int const termID) override;

private:
  int getTermFreq();

public:
  void newPostingsArray() override;

  std::shared_ptr<ParallelPostingsArray> createPostingsArray(int size) override;

public:
  class TermVectorsPostingsArray final : public ParallelPostingsArray
  {
    GET_CLASS_NAME(TermVectorsPostingsArray)
  public:
    TermVectorsPostingsArray(int size);

    std::deque<int>
        freqs; // How many times this term occurred in the current doc
    std::deque<int> lastOffsets;   // Last offset we saw
    std::deque<int> lastPositions; // Last position where this term occurred

    std::shared_ptr<ParallelPostingsArray> newInstance(int size) override;

    void copyTo(std::shared_ptr<ParallelPostingsArray> toArray,
                int numToCopy) override;

    int bytesPerPosting() override;

  protected:
    std::shared_ptr<TermVectorsPostingsArray> shared_from_this()
    {
      return std::static_pointer_cast<TermVectorsPostingsArray>(
          ParallelPostingsArray::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TermVectorsConsumerPerField> shared_from_this()
  {
    return std::static_pointer_cast<TermVectorsConsumerPerField>(
        TermsHashPerField::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
