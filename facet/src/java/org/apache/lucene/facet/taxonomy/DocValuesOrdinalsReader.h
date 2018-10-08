#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::facet::taxonomy
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;

/** Decodes ordinals previously indexed into a BinaryDocValues field */
class DocValuesOrdinalsReader : public OrdinalsReader
{
  GET_CLASS_NAME(DocValuesOrdinalsReader)
private:
  const std::wstring field;

  /** Default constructor. */
public:
  DocValuesOrdinalsReader();

  /** Create this, with the specified indexed field name. */
  DocValuesOrdinalsReader(const std::wstring &field);

  std::shared_ptr<OrdinalsSegmentReader> getReader(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class OrdinalsSegmentReaderAnonymousInnerClass : public OrdinalsSegmentReader
  {
    GET_CLASS_NAME(OrdinalsSegmentReaderAnonymousInnerClass)
  private:
    std::shared_ptr<DocValuesOrdinalsReader> outerInstance;

    std::shared_ptr<BinaryDocValues> values;

  public:
    OrdinalsSegmentReaderAnonymousInnerClass(
        std::shared_ptr<DocValuesOrdinalsReader> outerInstance,
        std::shared_ptr<BinaryDocValues> values);

  private:
    int lastDocID = 0;

  public:
    void get(int docID,
             std::shared_ptr<IntsRef> ordinals)  override;

  protected:
    std::shared_ptr<OrdinalsSegmentReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<OrdinalsSegmentReaderAnonymousInnerClass>(
          OrdinalsSegmentReader::shared_from_this());
    }
  };

public:
  std::wstring getIndexFieldName() override;

  /** Subclass and override if you change the encoding. */
protected:
  virtual void decode(std::shared_ptr<BytesRef> buf,
                      std::shared_ptr<IntsRef> ordinals);

protected:
  std::shared_ptr<DocValuesOrdinalsReader> shared_from_this()
  {
    return std::static_pointer_cast<DocValuesOrdinalsReader>(
        OrdinalsReader::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/facet/taxonomy/
