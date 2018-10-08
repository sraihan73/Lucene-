using namespace std;

#include "SolrSynonymParser.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"

namespace org::apache::lucene::analysis::synonym
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;

SolrSynonymParser::SolrSynonymParser(bool dedup, bool expand,
                                     shared_ptr<Analyzer> analyzer)
    : SynonymMap::Parser(dedup, analyzer), expand(expand)
{
}

void SolrSynonymParser::parse(shared_ptr<Reader> in_) throw(IOException,
                                                            ParseException)
{
  shared_ptr<LineNumberReader> br = make_shared<LineNumberReader>(in_);
  try {
    addInternal(br);
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

void SolrSynonymParser::addInternal(shared_ptr<BufferedReader> in_) throw(
    IOException)
{
  wstring line = L"";
  while ((line = in_->readLine()) != L"") {
    if (line.length() == 0 || line[0] == L'#') {
      continue; // ignore empty lines and comments
    }

    // TODO: we could process this more efficiently.
    std::deque<wstring> sides = split(line, L"=>");
    if (sides.size() > 1) { // explicit mapping
      if (sides.size() != 2) {
        throw invalid_argument(
            L"more than one explicit mapping specified on the same line");
      }
      std::deque<wstring> inputStrings = split(sides[0], L",");
      std::deque<std::shared_ptr<CharsRef>> inputs(inputStrings.size());
      for (int i = 0; i < inputs.size(); i++) {
        inputs[i] = analyze(StringHelper::trim(unescape(inputStrings[i])),
                            make_shared<CharsRefBuilder>());
      }

      std::deque<wstring> outputStrings = split(sides[1], L",");
      std::deque<std::shared_ptr<CharsRef>> outputs(outputStrings.size());
      for (int i = 0; i < outputs.size(); i++) {
        outputs[i] = analyze(StringHelper::trim(unescape(outputStrings[i])),
                             make_shared<CharsRefBuilder>());
      }
      // these mappings are explicit and never preserve original
      for (int i = 0; i < inputs.size(); i++) {
        for (int j = 0; j < outputs.size(); j++) {
          add(inputs[i], outputs[j], false);
        }
      }
    } else {
      std::deque<wstring> inputStrings = split(line, L",");
      std::deque<std::shared_ptr<CharsRef>> inputs(inputStrings.size());
      for (int i = 0; i < inputs.size(); i++) {
        inputs[i] = analyze(StringHelper::trim(unescape(inputStrings[i])),
                            make_shared<CharsRefBuilder>());
      }
      if (expand) {
        // all pairs
        for (int i = 0; i < inputs.size(); i++) {
          for (int j = 0; j < inputs.size(); j++) {
            if (i != j) {
              add(inputs[i], inputs[j], true);
            }
          }
        }
      } else {
        // all subsequent inputs map_obj to first one; we also add inputs[0] here
        // so that we "effectively" (because we remove the original input and
        // add back a synonym with the same text) change that token's type to
        // SYNONYM (matching legacy behavior):
        for (int i = 0; i < inputs.size(); i++) {
          add(inputs[i], inputs[0], false);
        }
      }
    }
  }
}

std::deque<wstring> SolrSynonymParser::split(const wstring &s,
                                              const wstring &separator)
{
  deque<wstring> deque = deque<wstring>(2);
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  int pos = 0, end = s.length();
  while (pos < end) {
    if (s.startsWith(separator, pos)) {
      if (sb->length() > 0) {
        deque.push_back(sb->toString());
        sb = make_shared<StringBuilder>();
      }
      pos += separator.length();
      continue;
    }

    wchar_t ch = s[pos++];
    if (ch == L'\\') {
      sb->append(ch);
      if (pos >= end) {
        break; // ERROR, or let it go?
      }
      ch = s[pos++];
    }

    sb->append(ch);
  }

  if (sb->length() > 0) {
    deque.push_back(sb->toString());
  }

  return deque.toArray(std::deque<wstring>(deque.size()));
}

wstring SolrSynonymParser::unescape(const wstring &s)
{
  if (s.find(L"\\") != wstring::npos) {
    shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
    for (int i = 0; i < s.length(); i++) {
      wchar_t ch = s[i];
      if (ch == L'\\' && i < s.length() - 1) {
        sb->append(s[++i]);
      } else {
        sb->append(ch);
      }
    }
    return sb->toString();
  }
  return s;
}
} // namespace org::apache::lucene::analysis::synonym