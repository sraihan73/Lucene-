using namespace std;

#include "MappingCharFilterFactory.h"
#include "../util/AbstractAnalysisFactory.h"
#include "../util/ResourceLoader.h"
#include "MappingCharFilter.h"
#include "NormalizeCharMap.h"

namespace org::apache::lucene::analysis::charfilter
{
using AbstractAnalysisFactory =
    org::apache::lucene::analysis::util::AbstractAnalysisFactory;
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
using MultiTermAwareComponent =
    org::apache::lucene::analysis::util::MultiTermAwareComponent;
using ResourceLoader = org::apache::lucene::analysis::util::ResourceLoader;
using ResourceLoaderAware =
    org::apache::lucene::analysis::util::ResourceLoaderAware;

MappingCharFilterFactory::MappingCharFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::CharFilterFactory(args),
      mapping(get(args, L"mapping"))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

void MappingCharFilterFactory::inform(shared_ptr<ResourceLoader> loader) throw(
    IOException)
{
  if (mapping != L"") {
    deque<wstring> wlist;
    deque<wstring> files = splitFileNames(mapping);
    wlist = deque<>();
    for (auto file : files) {
      deque<wstring> lines = getLines(loader, file.trim());
      wlist.insert(wlist.end(), lines.begin(), lines.end());
    }
    shared_ptr<NormalizeCharMap::Builder> *const builder =
        make_shared<NormalizeCharMap::Builder>();
    parseRules(wlist, builder);
    normMap = builder->build();
    if (normMap->map_obj == nullptr) {
      // if the inner FST is null, it means it accepts nothing (e.g. the file is
      // empty) so just set the whole map_obj to null
      normMap.reset();
    }
  }
}

shared_ptr<Reader> MappingCharFilterFactory::create(shared_ptr<Reader> input)
{
  // if the map_obj is null, it means there's actually no mappings... just return
  // the original stream as there is nothing to do here.
  return normMap == nullptr ? input
                            : make_shared<MappingCharFilter>(normMap, input);
}

shared_ptr<java::util::regex::Pattern> MappingCharFilterFactory::p =
    java::util::regex::Pattern::compile(L"\"(.*)\"\\s*=>\\s*\"(.*)\"\\s*$");

void MappingCharFilterFactory::parseRules(
    deque<wstring> &rules, shared_ptr<NormalizeCharMap::Builder> builder)
{
  for (auto rule : rules) {
    shared_ptr<Matcher> m = p->matcher(rule);
    if (!m->find()) {
      throw invalid_argument(L"Invalid Mapping Rule : [" + rule +
                             L"], file = " + mapping);
    }
    builder->add(parseString(m->group(1)), parseString(m->group(2)));
  }
}

wstring MappingCharFilterFactory::parseString(const wstring &s)
{
  int readPos = 0;
  int len = s.length();
  int writePos = 0;
  while (readPos < len) {
    wchar_t c = s[readPos++];
    if (c == L'\\') {
      if (readPos >= len) {
        throw invalid_argument(L"Invalid escaped char in [" + s + L"]");
      }
      c = s[readPos++];
      switch (c) {
      case L'\\':
        c = L'\\';
        break;
      case L'"':
        c = L'"';
        break;
      case L'n':
        c = L'\n';
        break;
      case L't':
        c = L'\t';
        break;
      case L'r':
        c = L'\r';
        break;
      case L'b':
        c = L'\b';
        break;
      case L'f':
        c = L'\f';
        break;
      case L'u':
        if (readPos + 3 >= len) {
          throw invalid_argument(L"Invalid escaped char in [" + s + L"]");
        }
        // C++ TODO: Only single-argument parse and valueOf methods are
        // converted: ORIGINAL LINE: c =
        // (char)Integer.parseInt(s.substring(readPos, 4), 16);
        c = static_cast<wchar_t>(Integer::valueOf(s.substr(readPos, 4), 16));
        readPos += 4;
        break;
      }
    }
    out[writePos++] = c;
  }
  return wstring(out, 0, writePos);
}

shared_ptr<AbstractAnalysisFactory>
MappingCharFilterFactory::getMultiTermComponent()
{
  return shared_from_this();
}
} // namespace org::apache::lucene::analysis::charfilter