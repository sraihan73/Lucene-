#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"

#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"

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
namespace org::apache::lucene::queries::function::valuesource
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using FunctionValues = org::apache::lucene::queries::function::FunctionValues;

/**
 * Function that returns a constant double value for every document.
 */
class DoubleConstValueSource : public ConstNumberSource
{
  GET_CLASS_NAME(DoubleConstValueSource)
public:
  const double constant;

private:
  const float fv;
  const int64_t lv;

public:
  DoubleConstValueSource(double constant);

  std::wstring description() override;

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class DoubleDocValuesAnonymousInnerClass : public DoubleDocValues
  {
    GET_CLASS_NAME(DoubleDocValuesAnonymousInnerClass)
  private:
    std::shared_ptr<DoubleConstValueSource> outerInstance;

  public:
    DoubleDocValuesAnonymousInnerClass(
        std::shared_ptr<DoubleConstValueSource> outerInstance);

    float floatVal(int doc) override;

    int intVal(int doc) override;

    int64_t longVal(int doc) override;

    double doubleVal(int doc) override;

    std::wstring strVal(int doc) override;

    std::any objectVal(int doc) override;

    std::wstring toString(int doc) override;

  protected:
    std::shared_ptr<DoubleDocValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<DoubleDocValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function.docvalues
              .DoubleDocValues::shared_from_this());
    }
  };

public:
  virtual int hashCode();

  virtual bool equals(std::any o);

  int getInt() override;

  int64_t getLong() override;

  float getFloat() override;

  double getDouble() override;

  std::shared_ptr<Number> getNumber() override;

  bool getBool() override;

protected:
  std::shared_ptr<DoubleConstValueSource> shared_from_this()
  {
    return std::static_pointer_cast<DoubleConstValueSource>(
        ConstNumberSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
