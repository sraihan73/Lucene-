#pragma once
#include "Analyzer.h"
#include "stringhelper.h"
#include <memory>
#include <string>
#include <typeinfo>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::analysis
{
class CharArraySet;
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
namespace org::apache::lucene::analysis
{

using Analyzer = org::apache::lucene::analysis::Analyzer;

/**
 * Base class for Analyzers that need to make use of stopword sets.
 *
 */
class StopwordAnalyzerBase : public Analyzer
{
  GET_CLASS_NAME(StopwordAnalyzerBase)

  /**
   * An immutable stopword set
   */
protected:
  const std::shared_ptr<CharArraySet> stopwords;

  /**
   * Returns the analyzer's stopword set or an empty set if the analyzer has no
   * stopwords
   *
   * @return the analyzer's stopword set or an empty set if the analyzer has no
   *         stopwords
   */
public:
  virtual std::shared_ptr<CharArraySet> getStopwordSet();

  /**
   * Creates a new instance initialized with the given stopword set
   *
   * @param stopwords
   *          the analyzer's stopword set
   */
protected:
  StopwordAnalyzerBase(std::shared_ptr<CharArraySet> stopwords);

  /**
   * Creates a new Analyzer with an empty stopword set
   */
  StopwordAnalyzerBase();

  /**
   * Creates a CharArraySet from a file resource associated with a class. (See
   * {@link Class#getResourceAsStream(std::wstring)}).
GET_CLASS_NAME(.)
   *
   * @param ignoreCase
   *          <code>true</code> if the set should ignore the case of the
   *          stopwords, otherwise <code>false</code>
   * @param aClass
   *          a class that is associated with the given stopwordResource
   * @param resource
   *          name of the resource file associated with the given class
   * @param comment
   *          comment string to ignore in the stopword file
   * @return a CharArraySet containing the distinct stopwords from the given
   *         file
   * @throws IOException
   *           if loading the stopwords throws an {@link IOException}
   */
  static std::shared_ptr<CharArraySet>
  loadStopwordSet(bool const ignoreCase, std::type_info const aClass,
                  const std::wstring &resource,
                  const std::wstring &comment) ;

  /**
   * Creates a CharArraySet from a path.
   *
   * @param stopwords
   *          the stopwords file to load
   * @return a CharArraySet containing the distinct stopwords from the given
   *         file
   * @throws IOException
   *           if loading the stopwords throws an {@link IOException}
   */
  static std::shared_ptr<CharArraySet>
  loadStopwordSet(std::shared_ptr<Path> stopwords) ;

  /**
   * Creates a CharArraySet from a file.
   *
   * @param stopwords
   *          the stopwords reader to load
   *
   * @return a CharArraySet containing the distinct stopwords from the given
   *         reader
   * @throws IOException
   *           if loading the stopwords throws an {@link IOException}
   */
  static std::shared_ptr<CharArraySet>
  loadStopwordSet(std::shared_ptr<Reader> stopwords) ;

protected:
  std::shared_ptr<StopwordAnalyzerBase> shared_from_this()
  {
    return std::static_pointer_cast<StopwordAnalyzerBase>(
        org.apache.lucene.analysis.Analyzer::shared_from_this());
  }
};

} // namespace org::apache::lucene::analysis
