#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <typeinfo>

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
namespace org::apache::lucene::queryparser::flexible::standard::config
{

/**
 * This class holds the configuration used to parse numeric queries and create
 * {@link PointValues} queries.
GET_CLASS_NAME(holds)
 *
 * @see PointValues
 * @see NumberFormat
 */
class PointsConfig : public std::enable_shared_from_this<PointsConfig>
{
  GET_CLASS_NAME(PointsConfig)

private:
  std::shared_ptr<NumberFormat> format;

  std::type_info type;

  /**
   * Constructs a {@link PointsConfig} object.
   *
   * @param format
   *          the {@link NumberFormat} used to parse a {@link std::wstring} to
   *          {@link Number}
   * @param type
   *          the numeric type used to index the numeric values
   *
   * @see PointsConfig#setNumberFormat(NumberFormat)
   */
public:
  PointsConfig(std::shared_ptr<NumberFormat> format, std::type_info type);

  /**
   * Returns the {@link NumberFormat} used to parse a {@link std::wstring} to
   * {@link Number}
   *
   * @return the {@link NumberFormat} used to parse a {@link std::wstring} to
   *         {@link Number}
   */
  virtual std::shared_ptr<NumberFormat> getNumberFormat();

  /**
   * Returns the numeric type used to index the numeric values
   *
   * @return the numeric type used to index the numeric values
   */
  virtual std::type_info getType();

  /**
   * Sets the numeric type used to index the numeric values
   *
   * @param type the numeric type used to index the numeric values
   */
  virtual void setType(std::type_info type);

  /**
   * Sets the {@link NumberFormat} used to parse a {@link std::wstring} to
   * {@link Number}
   *
   * @param format
   *          the {@link NumberFormat} used to parse a {@link std::wstring} to
   *          {@link Number}, must not be <code>null</code>
   */
  virtual void setNumberFormat(std::shared_ptr<NumberFormat> format);

  virtual int hashCode();

  bool equals(std::any obj) override;
};

} // namespace org::apache::lucene::queryparser::flexible::standard::config
