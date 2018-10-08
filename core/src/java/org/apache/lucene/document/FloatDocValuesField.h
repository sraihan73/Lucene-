#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>

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
namespace org::apache::lucene::document
{

/**
 * Syntactic sugar for encoding floats as NumericDocValues
 * via {@link Float#floatToRawIntBits(float)}.
 * <p>
 * Per-document floating point values can be retrieved via
 * {@link org.apache.lucene.index.LeafReader#getNumericDocValues(std::wstring)}.
 * <p>
 * <b>NOTE</b>: In most all cases this will be rather inefficient,
 * requiring four bytes per document. Consider encoding floating
 * point values yourself with only as much precision as you require.
 */
class FloatDocValuesField : public NumericDocValuesField
{
  GET_CLASS_NAME(FloatDocValuesField)

  /**
   * Creates a new DocValues field with the specified 32-bit float value
   * @param name field name
   * @param value 32-bit float value
   * @throws IllegalArgumentException if the field name is null
   */
public:
  FloatDocValuesField(const std::wstring &name, float value);

  void setFloatValue(float value) override;

  void setLongValue(int64_t value) override;

protected:
  std::shared_ptr<FloatDocValuesField> shared_from_this()
  {
    return std::static_pointer_cast<FloatDocValuesField>(
        NumericDocValuesField::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
