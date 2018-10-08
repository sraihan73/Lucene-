#pragma once
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/store/IndexInput.h"

#include  "core/src/java/org/apache/lucene/codecs/PostingsReaderBase.h"
#include  "core/src/java/org/apache/lucene/codecs/idversion/VersionFieldReader.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/index/Terms.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"

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
namespace org::apache::lucene::codecs::idversion
{

using FieldsProducer = org::apache::lucene::codecs::FieldsProducer;
using PostingsReaderBase = org::apache::lucene::codecs::PostingsReaderBase;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using Terms = org::apache::lucene::index::Terms;
using IndexInput = org::apache::lucene::store::IndexInput;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * See {@link VersionBlockTreeTermsWriter}.
 *
 * @lucene.experimental
 */

class VersionBlockTreeTermsReader final : public FieldsProducer
{
  GET_CLASS_NAME(VersionBlockTreeTermsReader)

  // Open input to the main terms dict file (_X.tiv)
public:
  const std::shared_ptr<IndexInput> in_;

  // private static final bool DEBUG = BlockTreeTermsWriter.DEBUG;

  // Reads the terms dict entries, to gather state to
  // produce DocsEnum on demand
  const std::shared_ptr<PostingsReaderBase> postingsReader;

private:
  const std::map_obj<std::wstring, std::shared_ptr<VersionFieldReader>> fields =
      std::map_obj<std::wstring, std::shared_ptr<VersionFieldReader>>();

  /** Sole constructor. */
public:
  VersionBlockTreeTermsReader(
      std::shared_ptr<PostingsReaderBase> postingsReader,
      std::shared_ptr<SegmentReadState> state) ;

private:
  static std::shared_ptr<BytesRef>
  readBytesRef(std::shared_ptr<IndexInput> in_) ;

  /** Seek {@code input} to the directory offset. */
  void seekDir(std::shared_ptr<IndexInput> input) ;

  // for debugging
  // private static std::wstring toHex(int v) {
  //   return "0x" + Integer.toHexString(v);
  // }

public:
  virtual ~VersionBlockTreeTermsReader();

  std::shared_ptr<Iterator<std::wstring>> iterator() override;

  std::shared_ptr<Terms>
  terms(const std::wstring &field)  override;

  int size() override;

  // for debugging
  std::wstring brToString(std::shared_ptr<BytesRef> b);

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  void checkIntegrity()  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<VersionBlockTreeTermsReader> shared_from_this()
  {
    return std::static_pointer_cast<VersionBlockTreeTermsReader>(
        org.apache.lucene.codecs.FieldsProducer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/idversion/
