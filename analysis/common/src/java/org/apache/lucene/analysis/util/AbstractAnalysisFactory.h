#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class Version;
}

namespace org::apache::lucene::analysis::util
{
class ResourceLoader;
}
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
namespace org::apache::lucene::analysis::util
{

using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using Version = org::apache::lucene::util::Version;

/**
 * Abstract parent class for analysis factories {@link TokenizerFactory},
 * {@link TokenFilterFactory} and {@link CharFilterFactory}.
GET_CLASS_NAME(for)
 * <p>
 * The typical lifecycle for a factory consumer is:
 * <ol>
 *   <li>Create factory via its constructor (or via XXXFactory.forName)
 *   <li>(Optional) If the factory uses resources such as files, {@link
ResourceLoaderAware#inform(ResourceLoader)} is called to initialize those
resources.
 *   <li>Consumer calls create() to obtain instances.
 * </ol>
 */
class AbstractAnalysisFactory
    : public std::enable_shared_from_this<AbstractAnalysisFactory>
{
  GET_CLASS_NAME(AbstractAnalysisFactory)
public:
  static const std::wstring LUCENE_MATCH_VERSION_PARAM;

  /** The original args, before any processing */
private:
  const std::unordered_map<std::wstring, std::wstring> originalArgs;

  /** the luceneVersion arg */
protected:
  const std::shared_ptr<Version> luceneMatchVersion;
  /** whether the luceneMatchVersion arg is explicitly specified in the
   * serialized schema */
private:
  // C++ NOTE: Fields cannot have the same name as methods:
  bool isExplicitLuceneMatchVersion_ = false;

  /**
   * Initialize this factory via a set of key-value pairs.
   */
protected:
  AbstractAnalysisFactory(std::unordered_map<std::wstring, std::wstring> &args);

public:
  std::unordered_map<std::wstring, std::wstring> getOriginalArgs();

  std::shared_ptr<Version> getLuceneMatchVersion();

  virtual std::wstring
  require(std::unordered_map<std::wstring, std::wstring> &args,
          const std::wstring &name);
  virtual std::wstring
  require(std::unordered_map<std::wstring, std::wstring> &args,
          const std::wstring &name,
          std::shared_ptr<std::deque<std::wstring>> allowedValues);
  virtual std::wstring
  require(std::unordered_map<std::wstring, std::wstring> &args,
          const std::wstring &name,
          std::shared_ptr<std::deque<std::wstring>> allowedValues,
          bool caseSensitive);
  virtual std::wstring get(std::unordered_map<std::wstring, std::wstring> &args,
                           const std::wstring &name);
  virtual std::wstring get(std::unordered_map<std::wstring, std::wstring> &args,
                           const std::wstring &name,
                           const std::wstring &defaultVal);
  virtual std::wstring
  get(std::unordered_map<std::wstring, std::wstring> &args,
      const std::wstring &name,
      std::shared_ptr<std::deque<std::wstring>> allowedValues);
  virtual std::wstring
  get(std::unordered_map<std::wstring, std::wstring> &args,
      const std::wstring &name,
      std::shared_ptr<std::deque<std::wstring>> allowedValues,
      const std::wstring &defaultVal);
  virtual std::wstring
  get(std::unordered_map<std::wstring, std::wstring> &args,
      const std::wstring &name,
      std::shared_ptr<std::deque<std::wstring>> allowedValues,
      const std::wstring &defaultVal, bool caseSensitive);

protected:
  int requireInt(std::unordered_map<std::wstring, std::wstring> &args,
                 const std::wstring &name);
  int getInt(std::unordered_map<std::wstring, std::wstring> &args,
             const std::wstring &name, int defaultVal);

  bool requireBoolean(std::unordered_map<std::wstring, std::wstring> &args,
                      const std::wstring &name);
  bool getBoolean(std::unordered_map<std::wstring, std::wstring> &args,
                  const std::wstring &name, bool defaultVal);

  float requireFloat(std::unordered_map<std::wstring, std::wstring> &args,
                     const std::wstring &name);
  float getFloat(std::unordered_map<std::wstring, std::wstring> &args,
                 const std::wstring &name, float defaultVal);

public:
  virtual wchar_t
  requireChar(std::unordered_map<std::wstring, std::wstring> &args,
              const std::wstring &name);
  virtual wchar_t getChar(std::unordered_map<std::wstring, std::wstring> &args,
                          const std::wstring &name, wchar_t defaultValue);

private:
  static const std::shared_ptr<Pattern> ITEM_PATTERN;

  /** Returns whitespace- and/or comma-separated set of values, or null if none
   * are found */
public:
  virtual std::shared_ptr<Set<std::wstring>>
  getSet(std::unordered_map<std::wstring, std::wstring> &args,
         const std::wstring &name);

  /**
   * Compiles a pattern for the value of the specified argument key
   * <code>name</code>
   */
protected:
  std::shared_ptr<Pattern>
  getPattern(std::unordered_map<std::wstring, std::wstring> &args,
             const std::wstring &name);

  /**
   * Returns as {@link CharArraySet} from wordFiles, which
   * can be a comma-separated deque of filenames
   */
  std::shared_ptr<CharArraySet>
  getWordSet(std::shared_ptr<ResourceLoader> loader,
             const std::wstring &wordFiles, bool ignoreCase) ;

  /**
   * Returns the resource's lines (with content treated as UTF-8)
   */
  std::deque<std::wstring>
  getLines(std::shared_ptr<ResourceLoader> loader,
           const std::wstring &resource) ;

  /** same as {@link #getWordSet(ResourceLoader, std::wstring, bool)},
   * except the input is in snowball format. */
  std::shared_ptr<CharArraySet>
  getSnowballWordSet(std::shared_ptr<ResourceLoader> loader,
                     const std::wstring &wordFiles,
                     bool ignoreCase) ;

  /**
   * Splits file names separated by comma character.
   * File names can contain comma characters escaped by backslash '\'
   *
   * @param fileNames the string containing file names
   * @return a deque of file names with the escaping backslashed removed
   */
  std::deque<std::wstring> splitFileNames(const std::wstring &fileNames);

  /**
   * Splits a deque separated by zero or more given separator characters.
   * List items can contain comma characters escaped by backslash '\'.
   * Whitespace is NOT trimmed from the returned deque items.
   *
   * @param deque the string containing the split deque items
   * @return a deque of items with the escaping backslashes removed
   */
  std::deque<std::wstring> splitAt(wchar_t separator,
                                    const std::wstring &deque);

private:
  static const std::wstring CLASS_NAME;

  /**
   * @return the string used to specify the concrete class name in a serialized
representation: the class arg.
   *         If the concrete class name was not specified via a class arg,
returns {@code getClassName()}. GET_CLASS_NAME(name)
   */
public:
  virtual std::wstring getClassArg();

  virtual bool isExplicitLuceneMatchVersion();

  virtual void setExplicitLuceneMatchVersion(bool isExplicitLuceneMatchVersion);
};

} // namespace org::apache::lucene::analysis::util
