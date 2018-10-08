#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class PostingsFormat;
}

namespace org::apache::lucene::codecs
{
class DocValuesFormat;
}
namespace org::apache::lucene::codecs
{
class TermVectorsFormat;
}
namespace org::apache::lucene::codecs
{
class StoredFieldsFormat;
}
namespace org::apache::lucene::codecs
{
class NormsFormat;
}
namespace org::apache::lucene::codecs
{
class LiveDocsFormat;
}
namespace org::apache::lucene::codecs
{
class PointsFormat;
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
namespace org::apache::lucene::codecs::asserting
{

using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using FilterCodec = org::apache::lucene::codecs::FilterCodec;
using LiveDocsFormat = org::apache::lucene::codecs::LiveDocsFormat;
using NormsFormat = org::apache::lucene::codecs::NormsFormat;
using PointsFormat = org::apache::lucene::codecs::PointsFormat;
using PostingsFormat = org::apache::lucene::codecs::PostingsFormat;
using StoredFieldsFormat = org::apache::lucene::codecs::StoredFieldsFormat;
using TermVectorsFormat = org::apache::lucene::codecs::TermVectorsFormat;

/**
 * Acts like the default codec but with additional asserts.
 */
class AssertingCodec : public FilterCodec
{
  GET_CLASS_NAME(AssertingCodec)

private:
  const std::shared_ptr<PostingsFormat> postings =
      std::make_shared<PerFieldPostingsFormatAnonymousInnerClass>();

private:
  class PerFieldPostingsFormatAnonymousInnerClass
      : public PerFieldPostingsFormat
  {
    GET_CLASS_NAME(PerFieldPostingsFormatAnonymousInnerClass)
  public:
    PerFieldPostingsFormatAnonymousInnerClass();

    std::shared_ptr<PostingsFormat>
    getPostingsFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<PerFieldPostingsFormatAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          PerFieldPostingsFormatAnonymousInnerClass>(
          org.apache.lucene.codecs.perfield
              .PerFieldPostingsFormat::shared_from_this());
    }
  };

private:
  const std::shared_ptr<DocValuesFormat> docValues =
      std::make_shared<PerFieldDocValuesFormatAnonymousInnerClass>();

private:
  class PerFieldDocValuesFormatAnonymousInnerClass
      : public PerFieldDocValuesFormat
  {
    GET_CLASS_NAME(PerFieldDocValuesFormatAnonymousInnerClass)
  public:
    PerFieldDocValuesFormatAnonymousInnerClass();

    std::shared_ptr<DocValuesFormat>
    getDocValuesFormatForField(const std::wstring &field) override;

  protected:
    std::shared_ptr<PerFieldDocValuesFormatAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          PerFieldDocValuesFormatAnonymousInnerClass>(
          org.apache.lucene.codecs.perfield
              .PerFieldDocValuesFormat::shared_from_this());
    }
  };

private:
  const std::shared_ptr<TermVectorsFormat> vectors =
      std::make_shared<AssertingTermVectorsFormat>();
  const std::shared_ptr<StoredFieldsFormat> storedFields =
      std::make_shared<AssertingStoredFieldsFormat>();
  const std::shared_ptr<NormsFormat> norms =
      std::make_shared<AssertingNormsFormat>();
  const std::shared_ptr<LiveDocsFormat> liveDocs =
      std::make_shared<AssertingLiveDocsFormat>();
  const std::shared_ptr<PostingsFormat> defaultFormat =
      std::make_shared<AssertingPostingsFormat>();
  const std::shared_ptr<DocValuesFormat> defaultDVFormat =
      std::make_shared<AssertingDocValuesFormat>();
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<PointsFormat> pointsFormat_ =
      std::make_shared<AssertingPointsFormat>();

public:
  AssertingCodec();

  std::shared_ptr<PostingsFormat> postingsFormat() override;

  std::shared_ptr<TermVectorsFormat> termVectorsFormat() override;

  std::shared_ptr<StoredFieldsFormat> storedFieldsFormat() override;

  std::shared_ptr<DocValuesFormat> docValuesFormat() override;

  std::shared_ptr<NormsFormat> normsFormat() override;

  std::shared_ptr<LiveDocsFormat> liveDocsFormat() override;

  std::shared_ptr<PointsFormat> pointsFormat() override;

  virtual std::wstring toString();

  /** Returns the postings format that should be used for writing
   *  new segments of <code>field</code>.
   *
   *  The default implementation always returns "Asserting"
   */
  virtual std::shared_ptr<PostingsFormat>
  getPostingsFormatForField(const std::wstring &field);

  /** Returns the docvalues format that should be used for writing
   *  new segments of <code>field</code>.
   *
   *  The default implementation always returns "Asserting"
   */
  virtual std::shared_ptr<DocValuesFormat>
  getDocValuesFormatForField(const std::wstring &field);

protected:
  std::shared_ptr<AssertingCodec> shared_from_this()
  {
    return std::static_pointer_cast<AssertingCodec>(
        org.apache.lucene.codecs.FilterCodec::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::asserting
