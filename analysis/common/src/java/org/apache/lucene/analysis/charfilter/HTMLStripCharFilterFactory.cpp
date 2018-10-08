using namespace std;

#include "HTMLStripCharFilterFactory.h"
#include "HTMLStripCharFilter.h"

namespace org::apache::lucene::analysis::charfilter
{
using CharFilterFactory =
    org::apache::lucene::analysis::util::CharFilterFactory;
const shared_ptr<java::util::regex::Pattern>
    HTMLStripCharFilterFactory::TAG_NAME_PATTERN =
        java::util::regex::Pattern::compile(L"[^\\s,]+");

HTMLStripCharFilterFactory::HTMLStripCharFilterFactory(
    unordered_map<wstring, wstring> &args)
    : org::apache::lucene::analysis::util::CharFilterFactory(args),
      escapedTags(getSet(args, L"escapedTags"))
{
  if (!args.empty()) {
    throw invalid_argument(L"Unknown parameters: " + args);
  }
}

shared_ptr<HTMLStripCharFilter>
HTMLStripCharFilterFactory::create(shared_ptr<Reader> input)
{
  shared_ptr<HTMLStripCharFilter> charFilter;
  if (nullptr == escapedTags) {
    charFilter = make_shared<HTMLStripCharFilter>(input);
  } else {
    charFilter = make_shared<HTMLStripCharFilter>(input, escapedTags);
  }
  return charFilter;
}
} // namespace org::apache::lucene::analysis::charfilter