#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

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
namespace org::apache::lucene::benchmark::byTask::utils
{

/**
 * Perf run configuration properties.
 * <p>
 * Numeric property containing ":", e.g. "10:100:5" is interpreted
 * as array of numeric values. It is extracted once, on first use, and
 * maintain a round number to return the appropriate value.
 * <p>
 * The config property "work.dir" tells where is the root of
 * docs data dirs and indexes dirs. It is set to either of: <ul>
 * <li>value supplied for it in the alg file;</li>
 * <li>otherwise, value of System property "benchmark.work.dir";</li>
 * <li>otherwise, "work".</li>
 * </ul>
 */
class Config : public std::enable_shared_from_this<Config>
{
  GET_CLASS_NAME(Config)

  // For tests, if verbose is not turned on, don't print the props.
private:
  static const std::wstring DEFAULT_PRINT_PROPS;
  static const std::wstring NEW_LINE;

  int roundNumber = 0;
  std::shared_ptr<Properties> props;
  std::unordered_map<std::wstring, std::any> valByRound =
      std::unordered_map<std::wstring, std::any>();
  std::unordered_map<std::wstring, std::wstring> colForValByRound =
      std::unordered_map<std::wstring, std::wstring>();
  std::wstring algorithmText;

  /**
   * Read both algorithm and config properties.
   *
   * @param algReader from where to read algorithm and config properties.
   * @throws IOException If there is a low-level I/O error.
   */
public:
  Config(std::shared_ptr<Reader> algReader) ;

  /**
   * Create config without algorithm - useful for a programmatic perf test.
   * @param props - configuration properties.
   */
  Config(std::shared_ptr<Properties> props);

private:
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings({"unchecked", "rawtypes"}) private void
  // printProps()
  void printProps();

  /**
   * Return a string property.
   *
   * @param name name of property.
   * @param dflt default value.
   * @return a string property.
   */
public:
  virtual std::wstring get(const std::wstring &name, const std::wstring &dflt);

  /**
   * Set a property.
   * Note: once a multiple values property is set, it can no longer be modified.
   *
   * @param name  name of property.
   * @param value either single or multiple property value (multiple values are
   * separated by ":")
   */
  virtual void set(const std::wstring &name,
                   const std::wstring &value) ;

  /**
   * Return an int property.
   * If the property contain ":", e.g. "10:100:5", it is interpreted
   * as array of ints. It is extracted once, on first call
   * to get() it, and a by-round-value is returned.
   *
   * @param name name of property
   * @param dflt default value
   * @return a int property.
   */
  virtual int get(const std::wstring &name, int dflt);

  /**
   * Return a double property.
   * If the property contain ":", e.g. "10:100:5", it is interpreted
   * as array of doubles. It is extracted once, on first call
   * to get() it, and a by-round-value is returned.
   *
   * @param name name of property
   * @param dflt default value
   * @return a double property.
   */
  virtual double get(const std::wstring &name, double dflt);

  /**
   * Return a bool property.
   * If the property contain ":", e.g. "true.true.false", it is interpreted
   * as array of booleans. It is extracted once, on first call
   * to get() it, and a by-round-value is returned.
   *
   * @param name name of property
   * @param dflt default value
   * @return a int property.
   */
  virtual bool get(const std::wstring &name, bool dflt);

  /**
   * Increment the round number, for config values that are extracted by round
   * number.
   *
   * @return the new round number.
   */
  virtual int newRound();

private:
  std::deque<std::wstring> propToStringArray(const std::wstring &s);

  // extract properties to array, e.g. for "10:100:5" return int[]{10,100,5}.
  std::deque<int> propToIntArray(const std::wstring &s);

  // extract properties to array, e.g. for "10.7:100.4:-2.3" return
  // int[]{10.7,100.4,-2.3}.
  std::deque<double> propToDoubleArray(const std::wstring &s);

  // extract properties to array, e.g. for "true:true:false" return
  // bool[]{true,false,false}.
  std::deque<bool> propToBooleanArray(const std::wstring &s);

  /**
   * @return names of params set by round, for reports title
   */
public:
  virtual std::wstring getColsNamesForValsByRound();

  /**
   * @return values of params set by round, for reports lines.
   */
  virtual std::wstring getColsValuesForValsByRound(int roundNum);

  /**
   * @return the round number.
   */
  virtual int getRoundNumber();

  /**
   * @return Returns the algorithmText.
   */
  virtual std::wstring getAlgorithmText();
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/utils/
