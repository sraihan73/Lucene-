#pragma once
#include "stringhelper.h"
#include <memory>
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
namespace org::apache::lucene::benchmark::byTask::feeds
{

/** Output of parsing (e.g. HTML parsing) of an input document. */
class DocData : public std::enable_shared_from_this<DocData>
{
  GET_CLASS_NAME(DocData)

private:
  std::wstring name;
  std::wstring body;
  std::wstring title;
  std::wstring date;
  int id = 0;
  std::shared_ptr<Properties> props;

public:
  virtual void clear();

  virtual std::wstring getBody();

  /**
   * @return the date. If the ctor with Date was called, then the std::wstring
   *         returned is the output of
   *         {@link DateTools#dateToString(Date,
   * org.apache.lucene.document.DateTools.Resolution)} . Otherwise it's the
   * std::wstring passed to the other ctor.
   */
  virtual std::wstring getDate();

  virtual std::wstring getName();

  virtual int getID();

  virtual std::shared_ptr<Properties> getProps();

  virtual std::wstring getTitle();

  virtual void setBody(const std::wstring &body);

  virtual void setDate(Date date);

  virtual void setDate(const std::wstring &date);

  virtual void setName(const std::wstring &name);

  virtual void setID(int id);

  virtual void setProps(std::shared_ptr<Properties> props);

  virtual void setTitle(const std::wstring &title);
};

} // #include  "core/src/java/org/apache/lucene/benchmark/byTask/feeds/
