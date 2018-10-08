#pragma once
#include "SynonymMap.h"
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

namespace org::apache::lucene::util
{
class CharsRef;
}
namespace org::apache::lucene::util
{
class CharsRefBuilder;
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
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;

/**
 * Parser for wordnet prolog format
 * <p>
 * See http://wordnet.princeton.edu/man/prologdb.5WN.html for a description of
 * the format.
 * @lucene.experimental
 */
// TODO: allow you to specify syntactic categories (e.g. just nouns, etc)
class WordnetSynonymParser : public SynonymMap::Parser
{
  GET_CLASS_NAME(WordnetSynonymParser)
private:
  const bool expand;

public:
  WordnetSynonymParser(bool dedup, bool expand,
                       std::shared_ptr<Analyzer> analyzer);

  void parse(std::shared_ptr<Reader> in_) throw(IOException,
                                                ParseException) override;

private:
  std::shared_ptr<CharsRef>
  parseSynonym(const std::wstring &line,
               std::shared_ptr<CharsRefBuilder> reuse) ;

  void addInternal(std::deque<std::shared_ptr<CharsRef>> &synset, int size);

protected:
  std::shared_ptr<WordnetSynonymParser> shared_from_this()
  {
    return std::static_pointer_cast<WordnetSynonymParser>(
        SynonymMap.Parser::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis::synonym