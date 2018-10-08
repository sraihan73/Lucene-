using namespace std;

#include "WordnetSynonymParser.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"

namespace org::apache::lucene::analysis::synonym
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;

WordnetSynonymParser::WordnetSynonymParser(bool dedup, bool expand,
                                           shared_ptr<Analyzer> analyzer)
    : SynonymMap::Parser(dedup, analyzer), expand(expand)
{
}

void WordnetSynonymParser::parse(shared_ptr<Reader> in_) throw(IOException,
                                                               ParseException)
{
  shared_ptr<LineNumberReader> br = make_shared<LineNumberReader>(in_);
  try {
    wstring line = L"";
    wstring lastSynSetID = L"";
    std::deque<std::shared_ptr<CharsRef>> synset(8);
    int synsetSize = 0;

    while ((line = br->readLine()) != L"") {
      wstring synSetID = line.substr(2, 9);

      if (synSetID != lastSynSetID) {
        addInternal(synset, synsetSize);
        synsetSize = 0;
      }

      if (synset.size() <= synsetSize + 1) {
        synset = Arrays::copyOf(synset, synset.size() * 2);
      }

      synset[synsetSize] = parseSynonym(line, make_shared<CharsRefBuilder>());
      synsetSize++;
      lastSynSetID = synSetID;
    }

    // final synset in the file
    addInternal(synset, synsetSize);
  } catch (const invalid_argument &e) {
    shared_ptr<ParseException> ex = make_shared<ParseException>(
        L"Invalid synonym rule at line " + br->getLineNumber(), 0);
    ex->initCause(e);
    throw ex;
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    br->close();
  }
}

shared_ptr<CharsRef> WordnetSynonymParser::parseSynonym(
    const wstring &line, shared_ptr<CharsRefBuilder> reuse) 
{
  if (reuse == nullptr) {
    reuse = make_shared<CharsRefBuilder>();
  }

  int start = (int)line.find(L'\'') + 1;
  int end = (int)line.rfind(L'\'');

  wstring text = line.substr(start, end - start)->replace(L"''", L"'");
  return analyze(text, reuse);
}

void WordnetSynonymParser::addInternal(
    std::deque<std::shared_ptr<CharsRef>> &synset, int size)
{
  if (size <= 1) {
    return; // nothing to do
  }

  if (expand) {
    for (int i = 0; i < size; i++) {
      for (int j = 0; j < size; j++) {
        add(synset[i], synset[j], false);
      }
    }
  } else {
    for (int i = 0; i < size; i++) {
      add(synset[i], synset[0], false);
    }
  }
}
} // namespace org::apache::lucene::analysis::synonym