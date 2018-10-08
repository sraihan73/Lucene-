#pragma once
#include "SynonymMap.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class Analyzer;
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
namespace org::apache::lucene::analysis::synonym
{

using Analyzer = org::apache::lucene::analysis::Analyzer;

/**
 * Parser for the Solr synonyms format.
 * <ol>
 *   <li> Blank lines and lines starting with '#' are comments.
 *   <li> Explicit mappings match any token sequence on the LHS of "=&gt;"
 *        and replace with all alternatives on the RHS.  These types of mappings
 *        ignore the expand parameter in the constructor.
 *        Example:
 *        <blockquote>i-pod, i pod =&gt; ipod</blockquote>
 *   <li> Equivalent synonyms may be separated with commas and give
 *        no explicit mapping.  In this case the mapping behavior will
 *        be taken from the expand parameter in the constructor.  This allows
 *        the same synonym file to be used in different synonym handling
 * strategies. Example: <blockquote>ipod, i-pod, i pod</blockquote>
 *
 *   <li> Multiple synonym mapping entries are merged.
 *        Example:
 *        <blockquote>
 *         foo =&gt; foo bar<br>
 *         foo =&gt; baz<br><br>
 *         is equivalent to<br><br>
 *         foo =&gt; foo bar, baz
 *        </blockquote>
 *  </ol>
 * @lucene.experimental
 */
class SolrSynonymParser : public SynonymMap::Parser
{
  GET_CLASS_NAME(SolrSynonymParser)
private:
  const bool expand;

public:
  SolrSynonymParser(bool dedup, bool expand,
                    std::shared_ptr<Analyzer> analyzer);

  void parse(std::shared_ptr<Reader> in_) throw(IOException,
                                                ParseException) override;

private:
  void addInternal(std::shared_ptr<BufferedReader> in_) ;

  static std::deque<std::wstring> split(const std::wstring &s,
                                         const std::wstring &separator);

  std::wstring unescape(const std::wstring &s);

protected:
  std::shared_ptr<SolrSynonymParser> shared_from_this()
  {
    return std::static_pointer_cast<SolrSynonymParser>(
        SynonymMap.Parser::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::synonym
