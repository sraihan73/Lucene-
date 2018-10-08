using namespace std;

#include "SpanOrTermsBuilder.h"

namespace org::apache::lucene::queryparser::xml::builders
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using TermToBytesRefAttribute =
    org::apache::lucene::analysis::tokenattributes::TermToBytesRefAttribute;
using Term = org::apache::lucene::index::Term;
using SpanBoostQuery = org::apache::lucene::search::spans::SpanBoostQuery;
using SpanOrQuery = org::apache::lucene::search::spans::SpanOrQuery;
using SpanQuery = org::apache::lucene::search::spans::SpanQuery;
using SpanTermQuery = org::apache::lucene::search::spans::SpanTermQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
using DOMUtils = org::apache::lucene::queryparser::xml::DOMUtils;
using ParserException = org::apache::lucene::queryparser::xml::ParserException;
using org::w3c::dom::Element;

SpanOrTermsBuilder::SpanOrTermsBuilder(shared_ptr<Analyzer> analyzer)
    : analyzer(analyzer)
{
}

shared_ptr<SpanQuery>
SpanOrTermsBuilder::getSpanQuery(shared_ptr<Element> e) 
{
  wstring fieldName =
      DOMUtils::getAttributeWithInheritanceOrFail(e, L"fieldName");
  wstring value = DOMUtils::getNonBlankTextOrFail(e);

  deque<std::shared_ptr<SpanQuery>> clausesList =
      deque<std::shared_ptr<SpanQuery>>();

  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.TokenStream ts =
  // analyzer.tokenStream(fieldName, value))
  {
    org::apache::lucene::analysis::TokenStream ts =
        analyzer->tokenStream(fieldName, value);
    try {
      shared_ptr<TermToBytesRefAttribute> termAtt =
          ts->addAttribute(TermToBytesRefAttribute::typeid);
      ts->reset();
      while (ts->incrementToken()) {
        shared_ptr<SpanTermQuery> stq =
            make_shared<SpanTermQuery>(make_shared<Term>(
                fieldName, BytesRef::deepCopyOf(termAtt->getBytesRef())));
        clausesList.push_back(stq);
      }
      ts->end();
      shared_ptr<SpanOrQuery> soq =
          make_shared<SpanOrQuery>(clausesList.toArray(
              std::deque<std::shared_ptr<SpanQuery>>(clausesList.size())));
      float boost = DOMUtils::getAttribute(e, L"boost", 1.0f);
      return make_shared<SpanBoostQuery>(soq, boost);
    } catch (const IOException &ioe) {
      throw make_shared<ParserException>(L"IOException parsing value:" + value);
    }
  }
}
} // namespace org::apache::lucene::queryparser::xml::builders