using namespace std;

#include "WordlistLoader.h"
#include "../util/IOUtils.h"
#include "CharArrayMap.h"
#include "CharArraySet.h"

namespace org::apache::lucene::analysis
{
using IOUtils = org::apache::lucene::util::IOUtils;

WordlistLoader::WordlistLoader() {}

shared_ptr<CharArraySet>
WordlistLoader::getWordSet(shared_ptr<Reader> reader,
                           shared_ptr<CharArraySet> result) 
{
  shared_ptr<BufferedReader> br = nullptr;
  try {
    br = getBufferedReader(reader);
    wstring word = L"";
    while ((word = br->readLine()) != L"") {
      result->add(StringHelper::trim(word));
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({br});
  }
  return result;
}

shared_ptr<CharArraySet>
WordlistLoader::getWordSet(shared_ptr<Reader> reader) 
{
  return getWordSet(reader, make_shared<CharArraySet>(INITIAL_CAPACITY, false));
}

shared_ptr<CharArraySet>
WordlistLoader::getWordSet(shared_ptr<Reader> reader,
                           const wstring &comment) 
{
  return getWordSet(reader, comment,
                    make_shared<CharArraySet>(INITIAL_CAPACITY, false));
}

shared_ptr<CharArraySet>
WordlistLoader::getWordSet(shared_ptr<Reader> reader, const wstring &comment,
                           shared_ptr<CharArraySet> result) 
{
  shared_ptr<BufferedReader> br = nullptr;
  try {
    br = getBufferedReader(reader);
    wstring word = L"";
    while ((word = br->readLine()) != L"") {
      if (StringHelper::startsWith(word, comment) == false) {
        result->add(StringHelper::trim(word));
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({br});
  }
  return result;
}

shared_ptr<CharArraySet> WordlistLoader::getSnowballWordSet(
    shared_ptr<Reader> reader,
    shared_ptr<CharArraySet> result) 
{
  shared_ptr<BufferedReader> br = nullptr;
  try {
    br = getBufferedReader(reader);
    wstring line = L"";
    while ((line = br->readLine()) != L"") {
      int comment = (int)line.find(L'|');
      if (comment >= 0) {
        line = line.substr(0, comment);
      }
      std::deque<wstring> words = line.split(L"\\s+");
      for (int i = 0; i < words.size(); i++) {
        if (words[i].length() > 0) {
          result->add(words[i]);
        }
      }
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({br});
  }
  return result;
}

shared_ptr<CharArraySet>
WordlistLoader::getSnowballWordSet(shared_ptr<Reader> reader) 
{
  return getSnowballWordSet(reader,
                            make_shared<CharArraySet>(INITIAL_CAPACITY, false));
}

shared_ptr<CharArrayMap<wstring>> WordlistLoader::getStemDict(
    shared_ptr<Reader> reader,
    shared_ptr<CharArrayMap<wstring>> result) 
{
  shared_ptr<BufferedReader> br = nullptr;
  try {
    br = getBufferedReader(reader);
    wstring line;
    while ((line = br->readLine()) != L"") {
      std::deque<wstring> wordstem = line.split(L"\t", 2);
      result->put(wordstem[0], wordstem[1]);
    }
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::close({br});
  }
  return result;
}

deque<wstring>
WordlistLoader::getLines(shared_ptr<InputStream> stream,
                         shared_ptr<Charset> charset) 
{
  shared_ptr<BufferedReader> input = nullptr;
  deque<wstring> lines;
  bool success = false;
  try {
    input = getBufferedReader(IOUtils::getDecodingReader(stream, charset));

    lines = deque<wstring>();
    for (wstring word = L""; (word = input->readLine()) != nullptr;) {
      // skip initial bom marker
      if (lines.empty() && word.length() > 0 && word[0] == L'\uFEFF') {
        word = word.substr(1);
      }
      // skip comments
      if (StringHelper::startsWith(word, L"#")) {
        continue;
      }
      word = StringHelper::trim(word);
      // skip blank lines
      if (word.length() == 0) {
        continue;
      }
      lines.push_back(word);
    }
    success = true;
    return lines;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    if (success) {
      IOUtils::close({input});
    } else {
      IOUtils::closeWhileHandlingException({input});
    }
  }
}

shared_ptr<BufferedReader>
WordlistLoader::getBufferedReader(shared_ptr<Reader> reader)
{
  return (std::dynamic_pointer_cast<BufferedReader>(reader) != nullptr)
             ? std::static_pointer_cast<BufferedReader>(reader)
             : make_shared<BufferedReader>(reader);
}
} // namespace org::apache::lucene::analysis