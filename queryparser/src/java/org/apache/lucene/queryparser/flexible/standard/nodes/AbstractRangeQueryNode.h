#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <deque>

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
namespace org::apache::lucene::queryparser::flexible::standard::nodes
{

using FieldValuePairQueryNode = org::apache::lucene::queryparser::flexible::
    core::nodes::FieldValuePairQueryNode;
using QueryNodeImpl =
    org::apache::lucene::queryparser::flexible::core::nodes::QueryNodeImpl;
using RangeQueryNode =
    org::apache::lucene::queryparser::flexible::core::nodes::RangeQueryNode;
using EscapeQuerySyntax =
    org::apache::lucene::queryparser::flexible::core::parser::EscapeQuerySyntax;

/**
 * This class should be extended by nodes intending to represent range queries.
 *
 * @param <T>
 *          the type of the range query bounds (lower and upper)
 */
template <typename T>
// C++ TODO: Java wildcard generics are not converted to C++:
// ORIGINAL LINE: public class AbstractRangeQueryNode<T extends
// org.apache.lucene.queryparser.flexible.core.nodes.FieldValuePairQueryNode<?>>
// extends org.apache.lucene.queryparser.flexible.core.nodes.QueryNodeImpl
// implements
// org.apache.lucene.queryparser.flexible.core.nodes.RangeQueryNode<org.apache.lucene.queryparser.flexible.core.nodes.FieldValuePairQueryNode<?>>
class AbstractRangeQueryNode : public QueryNodeImpl,
                               public RangeQueryNode < FieldValuePairQueryNode <
                               ? >>
{
        static_assert(std::is_base_of<org.apache.lucene.queryparser.flexible.core.nodes.FieldValuePairQueryNode<?>, T>::value, L"T must inherit from org.apache.lucene.queryparser.flexible.core.nodes.FieldValuePairQueryNode<?>");

      private:
        bool lowerInclusive = false, upperInclusive = false;

        /**
         * Constructs an {@link AbstractRangeQueryNode}, it should be invoked
         * only by its extenders.
         */
      protected:
        AbstractRangeQueryNode()
        {
          setLeaf(false);
          allocate();
        }

        /**
         * Returns the field associated with this node.
         *
         * @return the field associated with this node
         *
         * @see FieldableNode
         */
      public:
        std::shared_ptr<std::wstring> getField() override
        {
          std::shared_ptr<std::wstring> field = nullptr;
          T lower = getLowerBound();
          T upper = getUpperBound();

          if (lower != nullptr) {
            field = lower->getField();

          } else if (upper != nullptr) {
            field = upper->getField();
          }

          return field;
        }

        /**
         * Sets the field associated with this node.
         *
         * @param fieldName the field associated with this node
         */
        void setField(std::shared_ptr<std::wstring> fieldName) override
        {
          T lower = getLowerBound();
          T upper = getUpperBound();

          if (lower != nullptr) {
            lower->setField(fieldName);
          }

          if (upper != nullptr) {
            upper->setField(fieldName);
          }
        }

        /**
         * Returns the lower bound node.
         *
         * @return the lower bound node.
         */
        // C++ TODO: Most Java annotations will not have direct C++ equivalents:
        // ORIGINAL LINE: @Override @SuppressWarnings("unchecked") public T
        // getLowerBound()
        T getLowerBound() override
        {
          return static_cast<T>(getChildren()->get(0));
        }

        /**
         * Returns the upper bound node.
         *
         * @return the upper bound node.
         */
        // C++ TODO: Most Java annotations will not have direct C++ equivalents:
        // ORIGINAL LINE: @Override @SuppressWarnings("unchecked") public T
        // getUpperBound()
        T getUpperBound() override
        {
          return static_cast<T>(getChildren()->get(1));
        }

        /**
         * Returns whether the lower bound is inclusive or exclusive.
         *
         * @return <code>true</code> if the lower bound is inclusive, otherwise,
         * <code>false</code>
         */
        bool isLowerInclusive() override { return lowerInclusive; }

        /**
         * Returns whether the upper bound is inclusive or exclusive.
         *
         * @return <code>true</code> if the upper bound is inclusive, otherwise,
         * <code>false</code>
         */
        bool isUpperInclusive() override { return upperInclusive; }

        /**
         * Sets the lower and upper bounds.
         *
         * @param lower the lower bound, <code>null</code> if lower bound is
         * open
         * @param upper the upper bound, <code>null</code> if upper bound is
         * open
         * @param lowerInclusive <code>true</code> if the lower bound is
         * inclusive, otherwise, <code>false</code>
         * @param upperInclusive <code>true</code> if the upper bound is
         * inclusive, otherwise, <code>false</code>
         *
         * @see #getLowerBound()
         * @see #getUpperBound()
         * @see #isLowerInclusive()
         * @see #isUpperInclusive()
         */
        virtual void setBounds(T lower, T upper, bool lowerInclusive,
                               bool upperInclusive)
        {

          if (lower != nullptr && upper != nullptr) {
            // C++ TODO: There is no native C++ equivalent to 'toString':
            std::wstring lowerField = StringUtils::toString(lower->getField());
            // C++ TODO: There is no native C++ equivalent to 'toString':
            std::wstring upperField = StringUtils::toString(upper->getField());

            if ((upperField != L"" || lowerField != L"") &&
                ((upperField != L"" && upperField != lowerField) ||
                 lowerField != upperField)) {
              throw std::invalid_argument(
                  "lower and upper bounds should have the same field name!");
            }

            this->lowerInclusive = lowerInclusive;
            this->upperInclusive = upperInclusive;

            std::deque<std::shared_ptr<QueryNode>> children(2);
            children.push_back(lower);
            children.push_back(upper);

            set(children);
          }
        }

        std::shared_ptr<std::wstring> toQueryString(
            std::shared_ptr<EscapeQuerySyntax> escapeSyntaxParser) override
        {
          std::shared_ptr<StringBuilder> sb = std::make_shared<StringBuilder>();

          T lower = getLowerBound();
          T upper = getUpperBound();

          if (lowerInclusive) {
            sb->append(L'[');

          } else {
            sb->append(L'{');
          }

          if (lower != nullptr) {
            sb->append(lower->toQueryString(escapeSyntaxParser));

          } else {
            sb->append(L"...");
          }

          sb->append(L' ');

          if (upper != nullptr) {
            sb->append(upper->toQueryString(escapeSyntaxParser));

          } else {
            sb->append(L"...");
          }

          if (upperInclusive) {
            sb->append(L']');

          } else {
            sb->append(L'}');
          }

          return sb->toString();
        }

        std::wstring toString() override
        {
          std::shared_ptr<StringBuilder> sb =
              (std::make_shared<StringBuilder>(L"<"))
                  ->append(getClass().getCanonicalName());
          sb->append(L" lowerInclusive=")->append(isLowerInclusive());
          sb->append(L" upperInclusive=")->append(isUpperInclusive());
          sb->append(L">\n\t");
          sb->append(getUpperBound())->append(L"\n\t");
          sb->append(getLowerBound())->append(L"\n");
          sb->append(L"</")
              ->append(getClass().getCanonicalName())
              ->append(L">\n");

          return sb->toString();
        }

      protected:
        std::shared_ptr<AbstractRangeQueryNode> shared_from_this()
        {
          return std::static_pointer_cast<AbstractRangeQueryNode>(
              org.apache.lucene.queryparser.flexible.core.nodes
                  .QueryNodeImpl::shared_from_this());
        }
};

} // namespace org::apache::lucene::queryparser::flexible::standard::nodes
