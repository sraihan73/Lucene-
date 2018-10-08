#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Term;
}

namespace org::apache::lucene::store
{
class DataOutput;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::store
{
class DataInput;
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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.RamUsageEstimator.NUM_BYTES_ARRAY_HEADER;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.RamUsageEstimator.NUM_BYTES_OBJECT_HEADER;
// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.RamUsageEstimator.NUM_BYTES_OBJECT_REF;

using DataInput = org::apache::lucene::store::DataInput;
using DataOutput = org::apache::lucene::store::DataOutput;
using BytesRef = org::apache::lucene::util::BytesRef;

/** An in-place update to a DocValues field. */
class DocValuesUpdate : public std::enable_shared_from_this<DocValuesUpdate>
{
  GET_CLASS_NAME(DocValuesUpdate)

  /* Rough logic: OBJ_HEADER + 3*PTR + INT
   * Term: OBJ_HEADER + 2*PTR
   *   Term.field: 2*OBJ_HEADER + 4*INT + PTR + string.length*CHAR
   *   Term.bytes: 2*OBJ_HEADER + 2*INT + PTR + bytes.length
   * std::wstring: 2*OBJ_HEADER + 4*INT + PTR + string.length*CHAR
   * T: OBJ_HEADER
   */
private:
  static const int RAW_SIZE_IN_BYTES = 8 * NUM_BYTES_OBJECT_HEADER +
                                       8 * NUM_BYTES_OBJECT_REF +
                                       8 * Integer::BYTES;

public:
  const DocValuesType type;
  const std::shared_ptr<Term> term;
  const std::wstring field;
  // used in BufferedDeletes to apply this update only to a slice of docs. It's
  // initialized to BufferedUpdates.MAX_INT since it's safe and most often used
  // this way we safe object creations.
  const int docIDUpto;
  // C++ NOTE: Fields cannot have the same name as methods:
  const bool hasValue_;

  /**
   * Constructor.
   *
   * @param term the {@link Term} which determines the documents that will be
   * updated
   * @param field the {@link NumericDocValuesField} to update
   */
protected:
  DocValuesUpdate(DocValuesType type, std::shared_ptr<Term> term,
                  const std::wstring &field, int docIDUpto, bool hasValue);

public:
  virtual int64_t valueSizeInBytes() = 0;

  int sizeInBytes();

protected:
  virtual std::wstring valueToString() = 0;

public:
  virtual void writeTo(std::shared_ptr<DataOutput> output) = 0;

  virtual bool hasValue();

  virtual std::wstring toString();

  /** An in-place update to a binary DocValues field */
public:
  class BinaryDocValuesUpdate;

  /** An in-place update to a numeric DocValues field */
public:
  class NumericDocValuesUpdate;
};

} // namespace org::apache::lucene::index
class DocValuesUpdate::BinaryDocValuesUpdate final : public DocValuesUpdate
{
  GET_CLASS_NAME(DocValuesUpdate::BinaryDocValuesUpdate)
private:
  const std::shared_ptr<BytesRef> value;

  /* Size of BytesRef: 2*INT + ARRAY_HEADER + PTR */
  static const int64_t RAW_VALUE_SIZE_IN_BYTES =
      NUM_BYTES_ARRAY_HEADER + 2 * Integer::BYTES + NUM_BYTES_OBJECT_REF;

public:
  BinaryDocValuesUpdate(std::shared_ptr<Term> term, const std::wstring &field,
                        std::shared_ptr<BytesRef> value);

private:
  BinaryDocValuesUpdate(std::shared_ptr<Term> term, const std::wstring &field,
                        std::shared_ptr<BytesRef> value, int docIDUpTo);

public:
  std::shared_ptr<BinaryDocValuesUpdate> prepareForApply(int docIDUpto);

  int64_t valueSizeInBytes() override;

protected:
  std::wstring valueToString() override;

public:
  std::shared_ptr<BytesRef> getValue();

  void writeTo(std::shared_ptr<DataOutput> out)  override;

  static std::shared_ptr<BytesRef>
  readFrom(std::shared_ptr<DataInput> in_,
           std::shared_ptr<BytesRef> scratch) ;

protected:
  std::shared_ptr<BinaryDocValuesUpdate> shared_from_this()
  {
    return std::static_pointer_cast<BinaryDocValuesUpdate>(
        DocValuesUpdate::shared_from_this());
  }
};
class DocValuesUpdate::NumericDocValuesUpdate final : public DocValuesUpdate
{
  GET_CLASS_NAME(DocValuesUpdate::NumericDocValuesUpdate)
private:
  const int64_t value;

public:
  NumericDocValuesUpdate(std::shared_ptr<Term> term, const std::wstring &field,
                         int64_t value);

  NumericDocValuesUpdate(std::shared_ptr<Term> term, const std::wstring &field,
                         std::optional<int64_t> &value);

private:
  NumericDocValuesUpdate(std::shared_ptr<Term> term, const std::wstring &field,
                         int64_t value, int docIDUpTo, bool hasValue);

public:
  std::shared_ptr<NumericDocValuesUpdate> prepareForApply(int docIDUpto);

  int64_t valueSizeInBytes() override;

protected:
  std::wstring valueToString() override;

public:
  void writeTo(std::shared_ptr<DataOutput> out)  override;

  static int64_t readFrom(std::shared_ptr<DataInput> in_) ;

  int64_t getValue();

protected:
  std::shared_ptr<NumericDocValuesUpdate> shared_from_this()
  {
    return std::static_pointer_cast<NumericDocValuesUpdate>(
        DocValuesUpdate::shared_from_this());
  }
};
