using namespace std;

#include "StopwordAnalyzerBase.h"
#include "../util/IOUtils.h"
#include "CharArraySet.h"
#include "WordlistLoader.h"

namespace org::apache::lucene::analysis
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using IOUtils = org::apache::lucene::util::IOUtils;

shared_ptr<CharArraySet> StopwordAnalyzerBase::getStopwordSet()
{
  return stopwords;
}

StopwordAnalyzerBase::StopwordAnalyzerBase(shared_ptr<CharArraySet> stopwords)
    : stopwords(stopwords == nullptr ? CharArraySet::EMPTY_SET
                                     : CharArraySet::unmodifiableSet(
                                           CharArraySet::copy(stopwords)))
{
  // analyzers should use char array set for stopwords!
}

StopwordAnalyzerBase::StopwordAnalyzerBase() : StopwordAnalyzerBase(nullptr) {}

shared_ptr<CharArraySet> StopwordAnalyzerBase::loadStopwordSet(
    bool const ignoreCase, type_info const aClass, const wstring &resource,
    const wstring &comment) 
{
  shared_ptr<Reader> reader = nullptr;
  try {
    reader = IOUtils::getDecodingReader(aClass.getResourceAsStream(resource),
                                        StandardCharsets::UTF_8);
    return WordlistLoader::getWordSet(
        reader, comment, make_shared<CharArraySet>(16, ignoreCase));
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({reader});
  }
}

shared_ptr<CharArraySet> StopwordAnalyzerBase::loadStopwordSet(
    shared_ptr<Path> stopwords) 
{
  shared_ptr<Reader> reader = nullptr;
  try {
    reader = Files::newBufferedReader(stopwords, StandardCharsets::UTF_8);
    return WordlistLoader::getWordSet(reader);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({reader});
  }
}

shared_ptr<CharArraySet> StopwordAnalyzerBase::loadStopwordSet(
    shared_ptr<Reader> stopwords) 
{
  try {
    return WordlistLoader::getWordSet(stopwords);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({stopwords});
  }
}
} // namespace org::apache::lucene::analysis