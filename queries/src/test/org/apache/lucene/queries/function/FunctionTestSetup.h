#pragma once
#include "stringhelper.h"
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/queries/function/ValueSource.h"

#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include  "core/src/java/org/apache/lucene/index/RandomIndexWriter.h"

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
namespace org::apache::lucene::queries::function
{

using Analyzer = org::apache::lucene::analysis::Analyzer;
using RandomIndexWriter = org::apache::lucene::index::RandomIndexWriter;
using FloatFieldSource =
    org::apache::lucene::queries::function::valuesource::FloatFieldSource;
using IntFieldSource =
    org::apache::lucene::queries::function::valuesource::IntFieldSource;
using MultiValuedFloatFieldSource = org::apache::lucene::queries::function::
    valuesource::MultiValuedFloatFieldSource;
using MultiValuedIntFieldSource = org::apache::lucene::queries::function::
    valuesource::MultiValuedIntFieldSource;
using SortedNumericSelector =
    org::apache::lucene::search::SortedNumericSelector;
using Directory = org::apache::lucene::store::Directory;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/**
 * Setup for function tests
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public abstract class FunctionTestSetup extends
// org.apache.lucene.util.LuceneTestCase
class FunctionTestSetup : public LuceneTestCase
{

  /**
   * Actual score computation order is slightly different than assumptios
   * this allows for a small amount of variation
   */
protected:
  static float TEST_SCORE_TOLERANCE_DELTA;

  static constexpr int N_DOCS = 17; // select a primary number > 2

  static const std::wstring ID_FIELD;
  static const std::wstring TEXT_FIELD;
  static const std::wstring INT_FIELD;
  /**
   * This field is multiValued and should give the exact same results as
   * {@link #INT_FIELD} when used with MIN selector
   */
  static const std::wstring INT_FIELD_MV_MIN;
  /**
   * This field is multiValued and should give the exact same results as
   * {@link #INT_FIELD} when used with MAX selector
   */
  static const std::wstring INT_FIELD_MV_MAX;

  static const std::wstring FLOAT_FIELD;
  /**
   * This field is multiValued and should give the exact same results as
   * {@link #FLOAT_FIELD} when used with MIN selector
   */
  static const std::wstring FLOAT_FIELD_MV_MIN;
  /**
   * This field is multiValued and should give the exact same results as
   * {@link #FLOAT_FIELD} when used with MAX selector
   */
  static const std::wstring FLOAT_FIELD_MV_MAX;

  std::shared_ptr<ValueSource> INT_VALUESOURCE =
      std::make_shared<IntFieldSource>(INT_FIELD);
  std::shared_ptr<ValueSource> INT_MV_MIN_VALUESOURCE =
      std::make_shared<MultiValuedIntFieldSource>(
          INT_FIELD_MV_MIN, SortedNumericSelector::Type::MIN);
  std::shared_ptr<ValueSource> INT_MV_MAX_VALUESOURCE =
      std::make_shared<MultiValuedIntFieldSource>(
          INT_FIELD_MV_MAX, SortedNumericSelector::Type::MAX);
  std::shared_ptr<ValueSource> FLOAT_VALUESOURCE =
      std::make_shared<FloatFieldSource>(FLOAT_FIELD);
  std::shared_ptr<ValueSource> FLOAT_MV_MIN_VALUESOURCE =
      std::make_shared<MultiValuedFloatFieldSource>(
          FLOAT_FIELD_MV_MIN, SortedNumericSelector::Type::MIN);
  std::shared_ptr<ValueSource> FLOAT_MV_MAX_VALUESOURCE =
      std::make_shared<MultiValuedFloatFieldSource>(
          FLOAT_FIELD_MV_MAX, SortedNumericSelector::Type::MAX);

private:
  static std::deque<std::wstring> const DOC_TEXT_LINES;

protected:
  static std::shared_ptr<Directory> dir;
  static std::shared_ptr<Analyzer> anlzr;

public:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @AfterClass public static void afterClassFunctionTestSetup()
  // throws Exception
  static void afterClassFunctionTestSetup() ;

protected:
  static void createIndex(bool doMultiSegment) ;

private:
  static void addDoc(std::shared_ptr<RandomIndexWriter> iw,
                     int i) ;

  // 17 --> ID00017
protected:
  static std::wstring id2String(int scoreAndID);

  // some text line for regular search
private:
  static std::wstring textLine(int docNum);

  // extract expected doc score from its ID Field: "ID7" --> 7.0
protected:
  static float expectedFieldScore(const std::wstring &docIDFieldVal);

  // debug messages (change DBG to true for anything to print)
  static void log(std::any o);

protected:
  std::shared_ptr<FunctionTestSetup> shared_from_this()
  {
    return std::static_pointer_cast<FunctionTestSetup>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/queries/function/
