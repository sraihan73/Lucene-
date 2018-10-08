#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"

#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/queries/function/FunctionValues.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"

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
using ValueSource = org::apache::lucene::queries::function::ValueSource;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/**
 * Converts individual ValueSource instances to leverage the FunctionValues *Val
 * functions that work with multiple values, i.e. {@link
 * org.apache.lucene.queries.function.FunctionValues#doubleVal(int, double[])}
 */
// Not crazy about the name, but...
class VectorValueSource : public MultiValueSource
{
  GET_CLASS_NAME(VectorValueSource)
protected:
  const std::deque<std::shared_ptr<ValueSource>> sources;

public:
  VectorValueSource(std::deque<std::shared_ptr<ValueSource>> &sources);

  virtual std::deque<std::shared_ptr<ValueSource>> getSources();

  int dimension() override;

  virtual std::wstring name();

  std::shared_ptr<FunctionValues>
  getValues(std::unordered_map context,
            std::shared_ptr<LeafReaderContext> readerContext) 
      override;

private:
  class FunctionValuesAnonymousInnerClass : public FunctionValues
  {
    GET_CLASS_NAME(FunctionValuesAnonymousInnerClass)
  private:
    std::shared_ptr<VectorValueSource> outerInstance;

    std::shared_ptr<FunctionValues> x;
    std::shared_ptr<FunctionValues> y;

  public:
    FunctionValuesAnonymousInnerClass(
        std::shared_ptr<VectorValueSource> outerInstance,
        std::shared_ptr<FunctionValues> x, std::shared_ptr<FunctionValues> y);

    void byteVal(int doc, std::deque<char> &vals)  override;
    void shortVal(int doc,
                  std::deque<short> &vals)  override;
    void intVal(int doc, std::deque<int> &vals)  override;
    void longVal(int doc,
                 std::deque<int64_t> &vals)  override;
    void floatVal(int doc,
                  std::deque<float> &vals)  override;
    void doubleVal(int doc,
                   std::deque<double> &vals)  override;
    void strVal(int doc,
                std::deque<std::wstring> &vals)  override;
    std::wstring toString(int doc)  override;

  protected:
    std::shared_ptr<FunctionValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FunctionValuesAnonymousInnerClass>(
          org.apache.lucene.queries.function
              .FunctionValues::shared_from_this());
    }
  };

private:
  class FunctionValuesAnonymousInnerClass2 : public FunctionValues
  {
    GET_CLASS_NAME(FunctionValuesAnonymousInnerClass2)
  private:
    std::shared_ptr<VectorValueSource> outerInstance;

    std::deque<std::shared_ptr<FunctionValues>> valsArr;

  public:
    FunctionValuesAnonymousInnerClass2(
        std::shared_ptr<VectorValueSource> outerInstance,
        std::deque<std::shared_ptr<FunctionValues>> &valsArr);

    void byteVal(int doc, std::deque<char> &vals)  override;

    void shortVal(int doc,
                  std::deque<short> &vals)  override;

    void floatVal(int doc,
                  std::deque<float> &vals)  override;

    void intVal(int doc, std::deque<int> &vals)  override;

    void longVal(int doc,
                 std::deque<int64_t> &vals)  override;

    void doubleVal(int doc,
                   std::deque<double> &vals)  override;

    void strVal(int doc,
                std::deque<std::wstring> &vals)  override;

    std::wstring toString(int doc)  override;

  protected:
    std::shared_ptr<FunctionValuesAnonymousInnerClass2> shared_from_this()
    {
      return std::static_pointer_cast<FunctionValuesAnonymousInnerClass2>(
          org.apache.lucene.queries.function
              .FunctionValues::shared_from_this());
    }
  };

public:
  void createWeight(
      std::unordered_map context,
      std::shared_ptr<IndexSearcher> searcher)  override;

  std::wstring description() override;

  virtual bool equals(std::any o);

  virtual int hashCode();

protected:
  std::shared_ptr<VectorValueSource> shared_from_this()
  {
    return std::static_pointer_cast<VectorValueSource>(
        MultiValueSource::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/valuesource/
