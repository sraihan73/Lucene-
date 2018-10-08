using namespace std;

#include "AnalyzerQueryNodeProcessor.h"

namespace org::apache::lucene::queryparser::flexible::standard::processors
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using CachingTokenFilter = org::apache::lucene::analysis::CachingTokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using QueryNodeException =
    org::apache::lucene::queryparser::flexible::core::QueryNodeException;
using QueryConfigHandler = org::apache::lucene::queryparser::flexible::core::
    config::QueryConfigHandler;
using BooleanQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::BooleanQueryNode;
using FieldQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FieldQueryNode;
using FuzzyQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::FuzzyQueryNode;
using GroupQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::GroupQueryNode;
using ModifierQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::ModifierQueryNode;
using Modifier = org::apache::lucene::queryparser::flexible::core::nodes::
    ModifierQueryNode::Modifier;
using NoTokenFoundQueryNode = org::apache::lucene::queryparser::flexible::core::
    nodes::NoTokenFoundQueryNode;
using QueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNode;
using QuotedFieldQueryNode = org::apache::lucene::queryparser::flexible::core::
    nodes::QuotedFieldQueryNode;
using RangeQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::RangeQueryNode;
using TextableQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::TextableQueryNode;
using TokenizedPhraseQueryNode = org::apache::lucene::queryparser::flexible::
    core::nodes::TokenizedPhraseQueryNode;
using QueryNodeProcessorImpl = org::apache::lucene::queryparser::flexible::
    core::processors::QueryNodeProcessorImpl;
using ConfigurationKeys = org::apache::lucene::queryparser::flexible::standard::
    config::StandardQueryConfigHandler::ConfigurationKeys;
using Operator = org::apache::lucene::queryparser::flexible::standard::config::
    StandardQueryConfigHandler::Operator;
using MultiPhraseQueryNode = org::apache::lucene::queryparser::flexible::
    standard::nodes::MultiPhraseQueryNode;
using RegexpQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::RegexpQueryNode;
using SynonymQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::SynonymQueryNode;
using WildcardQueryNode = org::apache::lucene::queryparser::flexible::standard::
    nodes::WildcardQueryNode;

AnalyzerQueryNodeProcessor::AnalyzerQueryNodeProcessor()
{
  // empty constructor
}

shared_ptr<QueryNode> AnalyzerQueryNodeProcessor::process(
    shared_ptr<QueryNode> queryTree) 
{
  shared_ptr<Analyzer> analyzer =
      getQueryConfigHandler()->get(ConfigurationKeys::ANALYZER);

  if (analyzer != nullptr) {
    this->analyzer = analyzer;
    this->positionIncrementsEnabled = false;
    optional<bool> positionIncrementsEnabled = getQueryConfigHandler()->get(
        ConfigurationKeys::ENABLE_POSITION_INCREMENTS);
    Operator defaultOperator =
        getQueryConfigHandler()->get(ConfigurationKeys::DEFAULT_OPERATOR);
    this->defaultOperator =
        defaultOperator != nullptr ? defaultOperator : Operator::OR;

    if (positionIncrementsEnabled) {
      this->positionIncrementsEnabled = positionIncrementsEnabled;
    }

    if (this->analyzer != nullptr) {
      return QueryNodeProcessorImpl::process(queryTree);
    }
  }

  return queryTree;
}

shared_ptr<QueryNode> AnalyzerQueryNodeProcessor::postProcessNode(
    shared_ptr<QueryNode> node) 
{

  if (std::dynamic_pointer_cast<TextableQueryNode>(node) != nullptr &&
      !(std::dynamic_pointer_cast<WildcardQueryNode>(node) != nullptr) &&
      !(std::dynamic_pointer_cast<FuzzyQueryNode>(node) != nullptr) &&
      !(std::dynamic_pointer_cast<RegexpQueryNode>(node) != nullptr) &&
      !(std::dynamic_pointer_cast<RangeQueryNode>(node->getParent()) !=
        nullptr)) {

    shared_ptr<FieldQueryNode> fieldNode =
        (std::static_pointer_cast<FieldQueryNode>(node));
    wstring text = fieldNode->getTextAsString();
    wstring field = fieldNode->getFieldAsString();

    shared_ptr<CachingTokenFilter> buffer = nullptr;
    shared_ptr<PositionIncrementAttribute> posIncrAtt = nullptr;
    int numTokens = 0;
    int positionCount = 0;
    bool severalTokensAtSamePosition = false;

    try {
      // C++ NOTE: The following 'try with resources' block is replaced by its
      // C++ equivalent: ORIGINAL LINE: try
      // (org.apache.lucene.analysis.TokenStream source =
      // this.analyzer.tokenStream(field, text))
      {
        org::apache::lucene::analysis::TokenStream source =
            this->analyzer->tokenStream(field, text);
        try {
          buffer = make_shared<CachingTokenFilter>(source);
          buffer->reset();

          if (buffer->hasAttribute(PositionIncrementAttribute::typeid)) {
            posIncrAtt =
                buffer->getAttribute(PositionIncrementAttribute::typeid);
          }

          try {

            while (buffer->incrementToken()) {
              numTokens++;
              int positionIncrement = (posIncrAtt != nullptr)
                                          ? posIncrAtt->getPositionIncrement()
                                          : 1;
              if (positionIncrement != 0) {
                positionCount += positionIncrement;

              } else {
                severalTokensAtSamePosition = true;
              }
            }

          } catch (const IOException &e) {
            // ignore
          }

          // rewind the buffer stream
          buffer->reset(); // will never through on subsequent reset calls
        } catch (const IOException &e) {
          throw runtime_error(e);
        }
      }

      if (!buffer->hasAttribute(CharTermAttribute::typeid)) {
        return make_shared<NoTokenFoundQueryNode>();
      }

      shared_ptr<CharTermAttribute> termAtt =
          buffer->getAttribute(CharTermAttribute::typeid);

      if (numTokens == 0) {
        return make_shared<NoTokenFoundQueryNode>();

      } else if (numTokens == 1) {
        wstring term = L"";
        try {
          bool hasNext;
          hasNext = buffer->incrementToken();
          assert(hasNext == true);
          // C++ TODO: There is no native C++ equivalent to 'toString':
          term = termAtt->toString();

        } catch (const IOException &e) {
          // safe to ignore, because we know the number of tokens
        }

        fieldNode->setText(term);

        return fieldNode;

      } else if (severalTokensAtSamePosition ||
                 !(std::dynamic_pointer_cast<QuotedFieldQueryNode>(node) !=
                   nullptr)) {
        if (positionCount == 1 ||
            !(std::dynamic_pointer_cast<QuotedFieldQueryNode>(node) !=
              nullptr)) {
          // no phrase query:

          if (positionCount == 1) {
            // simple case: only one position, with synonyms
            deque<std::shared_ptr<QueryNode>> children =
                deque<std::shared_ptr<QueryNode>>();

            for (int i = 0; i < numTokens; i++) {
              wstring term = L"";
              try {
                bool hasNext = buffer->incrementToken();
                assert(hasNext == true);
                // C++ TODO: There is no native C++ equivalent to 'toString':
                term = termAtt->toString();

              } catch (const IOException &e) {
                // safe to ignore, because we know the number of tokens
              }

              children.push_back(
                  make_shared<FieldQueryNode>(field, term, -1, -1));
            }
            return make_shared<GroupQueryNode>(
                make_shared<SynonymQueryNode>(children));
          } else {
            // multiple positions
            shared_ptr<QueryNode> q = make_shared<BooleanQueryNode>(
                Collections::emptyList<std::shared_ptr<QueryNode>>());
            shared_ptr<QueryNode> currentQuery = nullptr;
            for (int i = 0; i < numTokens; i++) {
              wstring term = L"";
              try {
                bool hasNext = buffer->incrementToken();
                assert(hasNext == true);
                // C++ TODO: There is no native C++ equivalent to 'toString':
                term = termAtt->toString();
              } catch (const IOException &e) {
                // safe to ignore, because we know the number of tokens
              }
              if (posIncrAtt != nullptr &&
                  posIncrAtt->getPositionIncrement() == 0) {
                if (!(std::dynamic_pointer_cast<BooleanQueryNode>(
                          currentQuery) != nullptr)) {
                  shared_ptr<QueryNode> t = currentQuery;
                  currentQuery = make_shared<SynonymQueryNode>(
                      Collections::emptyList<std::shared_ptr<QueryNode>>());
                  (std::static_pointer_cast<BooleanQueryNode>(currentQuery))
                      ->add(t);
                }
                (std::static_pointer_cast<BooleanQueryNode>(currentQuery))
                    ->add(make_shared<FieldQueryNode>(field, term, -1, -1));
              } else {
                if (currentQuery != nullptr) {
                  if (this->defaultOperator == Operator::OR) {
                    q->add(currentQuery);
                  } else {
                    q->add(make_shared<ModifierQueryNode>(
                        currentQuery, ModifierQueryNode::Modifier::MOD_REQ));
                  }
                }
                currentQuery = make_shared<FieldQueryNode>(field, term, -1, -1);
              }
            }
            if (this->defaultOperator == Operator::OR) {
              q->add(currentQuery);
            } else {
              q->add(make_shared<ModifierQueryNode>(
                  currentQuery, ModifierQueryNode::Modifier::MOD_REQ));
            }

            if (std::dynamic_pointer_cast<BooleanQueryNode>(q) != nullptr) {
              q = make_shared<GroupQueryNode>(q);
            }
            return q;
          }
        } else {
          // phrase query:
          shared_ptr<MultiPhraseQueryNode> mpq =
              make_shared<MultiPhraseQueryNode>();

          deque<std::shared_ptr<FieldQueryNode>> multiTerms =
              deque<std::shared_ptr<FieldQueryNode>>();
          int position = -1;
          int i = 0;
          int termGroupCount = 0;
          for (; i < numTokens; i++) {
            wstring term = L"";
            int positionIncrement = 1;
            try {
              bool hasNext = buffer->incrementToken();
              assert(hasNext == true);
              // C++ TODO: There is no native C++ equivalent to 'toString':
              term = termAtt->toString();
              if (posIncrAtt != nullptr) {
                positionIncrement = posIncrAtt->getPositionIncrement();
              }

            } catch (const IOException &e) {
              // safe to ignore, because we know the number of tokens
            }

            if (positionIncrement > 0 && multiTerms.size() > 0) {

              for (auto termNode : multiTerms) {

                if (this->positionIncrementsEnabled) {
                  termNode->setPositionIncrement(position);
                } else {
                  termNode->setPositionIncrement(termGroupCount);
                }

                mpq->add(termNode);
              }

              // Only increment once for each "group" of
              // terms that were in the same position:
              termGroupCount++;

              multiTerms.clear();
            }

            position += positionIncrement;
            multiTerms.push_back(
                make_shared<FieldQueryNode>(field, term, -1, -1));
          }

          for (auto termNode : multiTerms) {

            if (this->positionIncrementsEnabled) {
              termNode->setPositionIncrement(position);

            } else {
              termNode->setPositionIncrement(termGroupCount);
            }

            mpq->add(termNode);
          }

          return mpq;
        }

      } else {

        shared_ptr<TokenizedPhraseQueryNode> pq =
            make_shared<TokenizedPhraseQueryNode>();

        int position = -1;

        for (int i = 0; i < numTokens; i++) {
          wstring term = L"";
          int positionIncrement = 1;

          try {
            bool hasNext = buffer->incrementToken();
            assert(hasNext == true);
            // C++ TODO: There is no native C++ equivalent to 'toString':
            term = termAtt->toString();

            if (posIncrAtt != nullptr) {
              positionIncrement = posIncrAtt->getPositionIncrement();
            }

          } catch (const IOException &e) {
            // safe to ignore, because we know the number of tokens
          }

          shared_ptr<FieldQueryNode> newFieldNode =
              make_shared<FieldQueryNode>(field, term, -1, -1);

          if (this->positionIncrementsEnabled) {
            position += positionIncrement;
            newFieldNode->setPositionIncrement(position);

          } else {
            newFieldNode->setPositionIncrement(i);
          }

          pq->add(newFieldNode);
        }

        return pq;
      }
    }
    // C++ TODO: There is no native C++ equivalent to the exception 'finally'
    // clause:
    finally {
      if (buffer != nullptr) {
        try {
          delete buffer;
        } catch (const IOException &e) {
          // safe to ignore
        }
      }
    }
  }

  return node;
}

shared_ptr<QueryNode> AnalyzerQueryNodeProcessor::preProcessNode(
    shared_ptr<QueryNode> node) 
{

  return node;
}

deque<std::shared_ptr<QueryNode>> AnalyzerQueryNodeProcessor::setChildrenOrder(
    deque<std::shared_ptr<QueryNode>> &children) 
{

  return children;
}
} // namespace org::apache::lucene::queryparser::flexible::standard::processors