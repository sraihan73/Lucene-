#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/queryparser/surround/query/BasicQueryFactory.h"

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
namespace org::apache::lucene::queryparser::surround::query
{
using Query = org::apache::lucene::search::Query;

/** Lowest level base class for surround queries */
class SrndQuery : public std::enable_shared_from_this<SrndQuery>,
                  public Cloneable
{
public:
  SrndQuery();

private:
  float weight = static_cast<float>(1.0);
  bool weighted = false;

public:
  virtual void setWeight(float w);
  virtual bool isWeighted();
  virtual float getWeight();
  virtual std::wstring getWeightString();

  virtual std::wstring getWeightOperator();

protected:
  virtual void weightToString(std::shared_ptr<StringBuilder> r);

public:
  virtual std::shared_ptr<Query>
  makeLuceneQueryField(const std::wstring &fieldName,
                       std::shared_ptr<BasicQueryFactory> qf);

  virtual std::shared_ptr<Query>
  makeLuceneQueryFieldNoBoost(const std::wstring &fieldName,
                              std::shared_ptr<BasicQueryFactory> qf) = 0;

  /** This method is used by {@link #hashCode()} and {@link #equals(Object)},
   *  see LUCENE-2945.
   */
  std::wstring toString() = 0;
  override

      virtual bool
      isFieldsSubQueryAcceptable();

  std::shared_ptr<SrndQuery> clone() override;

  /** For subclasses of {@link SrndQuery} within the package
   *  {@link org.apache.lucene.queryparser.surround.query}
GET_CLASS_NAME(es)
   *  it is not necessary to override this method,
   *  @see #toString()
   */
  virtual int hashCode();

  /** For subclasses of {@link SrndQuery} within the package
   *  {@link org.apache.lucene.queryparser.surround.query}
GET_CLASS_NAME(es)
   *  it is not necessary to override this method,
   *  @see #toString()
   */
  bool equals(std::any obj) override;
};

} // #include  "core/src/java/org/apache/lucene/queryparser/surround/query/
