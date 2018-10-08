using namespace std;

#include "WildcardQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using FuzzyQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FuzzyQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QuotedFieldQueryNode = org::apache::lucene::queryparser::flexible::core::
    nodes::QuotedFieldQueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using UnescapedCharSequence = org::apache::lucene::queryparser::flexible::core::
    util::UnescapedCharSequence;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using PrefixWildcardQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::PrefixWildcardQueryNode;
using TermRangeQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::TermRangeQueryNode;
using WildcardQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::WildcardQueryNode;
using StandardSyntaxParser = org::apache::lucene::queryparser::flexible::
    standard::parser::StandardSyntaxParser;
using PrefixQuery = org::apache::lucene::search::PrefixQuery;
using BytesRef = org::apache::lucene::util::BytesRef;
const shared_ptr<java::util::regex::Pattern>
    WildcardQueryNodeProcessor::WILDCARD_PATTERN =
        java::util::regex::Pattern::compile(L"(\\.)|([?*]+)");

wstring WildcardQueryNodeProcessor::analyzeWildcard(shared_ptr<Analyzer> a,
                                                    const wstring &field,
                                                    const wstring &wildcard)
{
  // best effort to not pass the wildcard characters through #normalize
  shared_ptr<Matcher> wildcardMatcher = WILDCARD_PATTERN->matcher(wildcard);
  shared_ptr<StringBuilder> sb = make_shared<StringBuilder>();
  int last = 0;

  while (wildcardMatcher->find()) {
    // continue if escaped char
    if (wildcardMatcher->group(1) != nullptr) {
      continue;
    }

    if (wildcardMatcher->start() > 0) {
      wstring chunk = wildcard.substr(last, wildcardMatcher->start() - last);
      shared_ptr<BytesRef> normalized = a->normalize(field, chunk);
      sb->append(normalized->utf8ToString());
    }
    // append the wildcard character
    sb->append(wildcardMatcher->group(2));

    last = wildcardMatcher->end();
  }
  if (last < wildcard.length()) {
    wstring chunk = wildcard.substr(last);
    shared_ptr<BytesRef> normalized = a->normalize(field, chunk);
    sb->append(normalized->utf8ToString());
  }
  return sb->toString();
}

WildcardQueryNodeProcessor::WildcardQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> WildcardQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  // the old Lucene Parser ignores FuzzyQueryNode that are also
  // PrefixWildcardQueryNode or WildcardQueryNode we do the same here, also
  // ignore empty terms
  if (std::dynamic_pointer_cast<FieldQueryNode>(node) != nullptr ||
      std::dynamic_pointer_cast<FuzzyQueryNode>(node) != nullptr) {
    shared_ptr<FieldQueryNode> fqn =
        std::static_pointer_cast<FieldQueryNode>(node);
    shared_ptr<std::wstring> text = fqn->getText();

    // do not process wildcards for TermRangeQueryNode children and
    // QuotedFieldQueryNode to reproduce the old parser behavior
    if (std::dynamic_pointer_cast<TermRangeQueryNode>(fqn->getParent()) !=
            nullptr ||
        std::dynamic_pointer_cast<QuotedFieldQueryNode>(fqn) != nullptr ||
        text->length() <= 0) {
      // Ignore empty terms
      return node;
    }

    // Code below simulates the old lucene parser behavior for wildcards

    if (isWildcard(text)) {
      shared_ptr<Analyzer> analyzer =
          getQueryConfigHandler()->get(ConfigurationKeys::ANALYZER);
      if (analyzer != nullptr) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        text = analyzeWildcard(analyzer, fqn->getFieldAsString(),
                               text->toString());
      }
      if (isPrefixWildcard(text)) {
        return make_shared<PrefixWildcardQueryNode>(
            fqn->getField(), text, fqn->getBegin(), fqn->getEnd());
      } else {
        return make_shared<WildcardQueryNode>(fqn->getField(), text,
                                              fqn->getBegin(), fqn->getEnd());
      }
    }
  }

  return node;
}

bool WildcardQueryNodeProcessor::isWildcard(shared_ptr<std::wstring> text)
{
  if (text == nullptr || text->length() <= 0) {
    return false;
  }

  // If a un-escaped '*' or '?' if found return true
  // start at the end since it's more common to put wildcards at the end
  for (int i = text->length() - 1; i >= 0; i--) {
    if ((text->charAt(i) == L'*' || text->charAt(i) == L'?') &&
        !UnescapedCharSequence::wasEscaped(text, i)) {
      return true;
    }
  }

  return false;
}

bool WildcardQueryNodeProcessor::isPrefixWildcard(shared_ptr<std::wstring> text)
{
  if (text == nullptr || text->length() <= 0 || !isWildcard(text)) {
    return false;
  }

  // Validate last character is a '*' and was not escaped
  // If single '*' is is a wildcard not prefix to simulate old queryparser
  if (text->charAt(text->length() - 1) != L'*') {
    return false;
  }
  if (UnescapedCharSequence::wasEscaped(text, text->length() - 1)) {
    return false;
  }
  if (text->length() == 1) {
    return false;
  }

  // Only make a prefix if there is only one single star at the end and no '?'
  // or '*' characters If single wildcard return false to mimic old queryparser
  for (int i = 0; i < text->length(); i++) {
    if (text->charAt(i) == L'?') {
      return false;
    }
    if (text->charAt(i) == L'*' &&
        !UnescapedCharSequence::wasEscaped(text, i)) {
      if (i == text->length() - 1) {
        return true;
      } else {
        return false;
      }
    }
  }

  return false;
}

shared_ptr<QueryNode> WildcardQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

deque<std::shared_ptr<QueryNode>> WildcardQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors