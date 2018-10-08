using namespace std;

#include "ShingleFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/TypeAttribute.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CannedTokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/WhitespaceTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/shingle/ShingleFilter.h"

namespace org::apache::lucene::analysis::shingle
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using WhitespaceTokenizer =
    org::apache::lucene::analysis::core::WhitespaceTokenizer;
using TypeAttribute =
    org::apache::lucene::analysis::tokenattributes::TypeAttribute;
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TEST_TOKEN =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6), createToken(L"divide", 7, 13),
            createToken(L"this", 14, 18), createToken(L"sentence", 19, 27),
            createToken(L"into", 28, 32), createToken(L"shingles", 33, 39)};
std::deque<int> const ShingleFilterTest::UNIGRAM_ONLY_POSITION_INCREMENTS =
    std::deque<int>{1, 1, 1, 1, 1, 1};
std::deque<wstring> const ShingleFilterTest::UNIGRAM_ONLY_TYPES =
    std::deque<wstring>{L"word", L"word", L"word", L"word", L"word", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>
    ShingleFilterTest::testTokenWithHoles;
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::BI_GRAM_TOKENS =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"please divide", 0, 13),
            createToken(L"divide", 7, 13),
            createToken(L"divide this", 7, 18),
            createToken(L"this", 14, 18),
            createToken(L"this sentence", 14, 27),
            createToken(L"sentence", 19, 27),
            createToken(L"sentence into", 19, 32),
            createToken(L"into", 28, 32),
            createToken(L"into shingles", 28, 39),
            createToken(L"shingles", 33, 39)};
std::deque<int> const ShingleFilterTest::BI_GRAM_POSITION_INCREMENTS =
    std::deque<int>{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
std::deque<wstring> const ShingleFilterTest::BI_GRAM_TYPES =
    std::deque<wstring>{L"word", L"shingle", L"word", L"shingle",
                         L"word", L"shingle", L"word", L"shingle",
                         L"word", L"shingle", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::BI_GRAM_TOKENS_WITH_HOLES =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"please divide", 0, 13),
            createToken(L"divide", 7, 13),
            createToken(L"divide _", 7, 19),
            createToken(L"_ sentence", 19, 27),
            createToken(L"sentence", 19, 27),
            createToken(L"sentence _", 19, 33),
            createToken(L"_ shingles", 33, 39),
            createToken(L"shingles", 33, 39)};
std::deque<int> const
    ShingleFilterTest::BI_GRAM_POSITION_INCREMENTS_WITH_HOLES =
        std::deque<int>{1, 0, 1, 0, 1, 1, 0, 1, 1};
std::deque<wstring> const ShingleFilterTest::BI_GRAM_TYPES_WITH_HOLES = {
    L"word", L"shingle", L"word",    L"shingle", L"shingle",
    L"word", L"shingle", L"shingle", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::BI_GRAM_TOKENS_WITHOUT_UNIGRAMS =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please divide", 0, 13),
            createToken(L"divide this", 7, 18),
            createToken(L"this sentence", 14, 27),
            createToken(L"sentence into", 19, 32),
            createToken(L"into shingles", 28, 39)};
std::deque<int> const
    ShingleFilterTest::BI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS =
        std::deque<int>{1, 1, 1, 1, 1};
std::deque<wstring> const ShingleFilterTest::BI_GRAM_TYPES_WITHOUT_UNIGRAMS =
    std::deque<wstring>{L"shingle", L"shingle", L"shingle", L"shingle",
                         L"shingle"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::BI_GRAM_TOKENS_WITH_HOLES_WITHOUT_UNIGRAMS =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please divide", 0, 13),
            createToken(L"divide _", 7, 19), createToken(L"_ sentence", 19, 27),
            createToken(L"sentence _", 19, 33),
            createToken(L"_ shingles", 33, 39)};
std::deque<int> const
    ShingleFilterTest::BI_GRAM_POSITION_INCREMENTS_WITH_HOLES_WITHOUT_UNIGRAMS =
        std::deque<int>{1, 1, 1, 1, 1, 1};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TEST_SINGLE_TOKEN =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6)};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::SINGLE_TOKEN =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6)};
std::deque<int> const ShingleFilterTest::SINGLE_TOKEN_INCREMENTS =
    std::deque<int>{1};
std::deque<wstring> const ShingleFilterTest::SINGLE_TOKEN_TYPES =
    std::deque<wstring>{L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::EMPTY_TOKEN_ARRAY =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>();
std::deque<int> const ShingleFilterTest::EMPTY_TOKEN_INCREMENTS_ARRAY =
    std::deque<int>();
std::deque<wstring> const ShingleFilterTest::EMPTY_TOKEN_TYPES_ARRAY =
    std::deque<wstring>();
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TRI_GRAM_TOKENS =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"please divide", 0, 13),
            createToken(L"please divide this", 0, 18),
            createToken(L"divide", 7, 13),
            createToken(L"divide this", 7, 18),
            createToken(L"divide this sentence", 7, 27),
            createToken(L"this", 14, 18),
            createToken(L"this sentence", 14, 27),
            createToken(L"this sentence into", 14, 32),
            createToken(L"sentence", 19, 27),
            createToken(L"sentence into", 19, 32),
            createToken(L"sentence into shingles", 19, 39),
            createToken(L"into", 28, 32),
            createToken(L"into shingles", 28, 39),
            createToken(L"shingles", 33, 39)};
std::deque<int> const ShingleFilterTest::TRI_GRAM_POSITION_INCREMENTS =
    std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1};
std::deque<wstring> const ShingleFilterTest::TRI_GRAM_TYPES =
    std::deque<wstring>{L"word",    L"shingle", L"shingle", L"word",
                         L"shingle", L"shingle", L"word",    L"shingle",
                         L"shingle", L"word",    L"shingle", L"shingle",
                         L"word",    L"shingle", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TRI_GRAM_TOKENS_WITHOUT_UNIGRAMS =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please divide", 0, 13),
            createToken(L"please divide this", 0, 18),
            createToken(L"divide this", 7, 18),
            createToken(L"divide this sentence", 7, 27),
            createToken(L"this sentence", 14, 27),
            createToken(L"this sentence into", 14, 32),
            createToken(L"sentence into", 19, 32),
            createToken(L"sentence into shingles", 19, 39),
            createToken(L"into shingles", 28, 39)};
std::deque<int> const
    ShingleFilterTest::TRI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS =
        std::deque<int>{1, 0, 1, 0, 1, 0, 1, 0, 1};
std::deque<wstring> const ShingleFilterTest::TRI_GRAM_TYPES_WITHOUT_UNIGRAMS =
    std::deque<wstring>{L"shingle", L"shingle", L"shingle",
                         L"shingle", L"shingle", L"shingle",
                         L"shingle", L"shingle", L"shingle"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::FOUR_GRAM_TOKENS =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"please divide", 0, 13),
            createToken(L"please divide this", 0, 18),
            createToken(L"please divide this sentence", 0, 27),
            createToken(L"divide", 7, 13),
            createToken(L"divide this", 7, 18),
            createToken(L"divide this sentence", 7, 27),
            createToken(L"divide this sentence into", 7, 32),
            createToken(L"this", 14, 18),
            createToken(L"this sentence", 14, 27),
            createToken(L"this sentence into", 14, 32),
            createToken(L"this sentence into shingles", 14, 39),
            createToken(L"sentence", 19, 27),
            createToken(L"sentence into", 19, 32),
            createToken(L"sentence into shingles", 19, 39),
            createToken(L"into", 28, 32),
            createToken(L"into shingles", 28, 39),
            createToken(L"shingles", 33, 39)};
std::deque<int> const ShingleFilterTest::FOUR_GRAM_POSITION_INCREMENTS =
    std::deque<int>{1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1};
std::deque<wstring> const ShingleFilterTest::FOUR_GRAM_TYPES =
    std::deque<wstring>{
        L"word",    L"shingle", L"shingle", L"shingle", L"word",    L"shingle",
        L"shingle", L"shingle", L"word",    L"shingle", L"shingle", L"shingle",
        L"word",    L"shingle", L"shingle", L"word",    L"shingle", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::FOUR_GRAM_TOKENS_WITHOUT_UNIGRAMS =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please divide", 0, 13),
            createToken(L"please divide this", 0, 18),
            createToken(L"please divide this sentence", 0, 27),
            createToken(L"divide this", 7, 18),
            createToken(L"divide this sentence", 7, 27),
            createToken(L"divide this sentence into", 7, 32),
            createToken(L"this sentence", 14, 27),
            createToken(L"this sentence into", 14, 32),
            createToken(L"this sentence into shingles", 14, 39),
            createToken(L"sentence into", 19, 32),
            createToken(L"sentence into shingles", 19, 39),
            createToken(L"into shingles", 28, 39)};
std::deque<int> const
    ShingleFilterTest::FOUR_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS =
        std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1};
std::deque<wstring> const ShingleFilterTest::FOUR_GRAM_TYPES_WITHOUT_UNIGRAMS =
    std::deque<wstring>{L"shingle", L"shingle", L"shingle", L"shingle",
                         L"shingle", L"shingle", L"shingle", L"shingle",
                         L"shingle", L"shingle", L"shingle", L"shingle"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TRI_GRAM_TOKENS_MIN_TRI_GRAM =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"please divide this", 0, 18),
            createToken(L"divide", 7, 13),
            createToken(L"divide this sentence", 7, 27),
            createToken(L"this", 14, 18),
            createToken(L"this sentence into", 14, 32),
            createToken(L"sentence", 19, 27),
            createToken(L"sentence into shingles", 19, 39),
            createToken(L"into", 28, 32),
            createToken(L"shingles", 33, 39)};
std::deque<int> const
    ShingleFilterTest::TRI_GRAM_POSITION_INCREMENTS_MIN_TRI_GRAM =
        std::deque<int>{1, 0, 1, 0, 1, 0, 1, 0, 1, 1};
std::deque<wstring> const ShingleFilterTest::TRI_GRAM_TYPES_MIN_TRI_GRAM =
    std::deque<wstring>{L"word", L"shingle", L"word", L"shingle",
                         L"word", L"shingle", L"word", L"shingle",
                         L"word", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TRI_GRAM_TOKENS_WITHOUT_UNIGRAMS_MIN_TRI_GRAM =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please divide this", 0, 18),
            createToken(L"divide this sentence", 7, 27),
            createToken(L"this sentence into", 14, 32),
            createToken(L"sentence into shingles", 19, 39)};
std::deque<int> const ShingleFilterTest::
    TRI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_MIN_TRI_GRAM =
        std::deque<int>{1, 1, 1, 1};
std::deque<wstring> const
    ShingleFilterTest::TRI_GRAM_TYPES_WITHOUT_UNIGRAMS_MIN_TRI_GRAM =
        std::deque<wstring>{L"shingle", L"shingle", L"shingle", L"shingle"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::FOUR_GRAM_TOKENS_MIN_TRI_GRAM =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"please divide this", 0, 18),
            createToken(L"please divide this sentence", 0, 27),
            createToken(L"divide", 7, 13),
            createToken(L"divide this sentence", 7, 27),
            createToken(L"divide this sentence into", 7, 32),
            createToken(L"this", 14, 18),
            createToken(L"this sentence into", 14, 32),
            createToken(L"this sentence into shingles", 14, 39),
            createToken(L"sentence", 19, 27),
            createToken(L"sentence into shingles", 19, 39),
            createToken(L"into", 28, 32),
            createToken(L"shingles", 33, 39)};
std::deque<int> const
    ShingleFilterTest::FOUR_GRAM_POSITION_INCREMENTS_MIN_TRI_GRAM =
        std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1};
std::deque<wstring> const ShingleFilterTest::FOUR_GRAM_TYPES_MIN_TRI_GRAM =
    std::deque<wstring>{L"word",    L"shingle", L"shingle", L"word",
                         L"shingle", L"shingle", L"word",    L"shingle",
                         L"shingle", L"word",    L"shingle", L"word",
                         L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::FOUR_GRAM_TOKENS_WITHOUT_UNIGRAMS_MIN_TRI_GRAM =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please divide this", 0, 18),
            createToken(L"please divide this sentence", 0, 27),
            createToken(L"divide this sentence", 7, 27),
            createToken(L"divide this sentence into", 7, 32),
            createToken(L"this sentence into", 14, 32),
            createToken(L"this sentence into shingles", 14, 39),
            createToken(L"sentence into shingles", 19, 39)};
std::deque<int> const ShingleFilterTest::
    FOUR_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_MIN_TRI_GRAM =
        std::deque<int>{1, 0, 1, 0, 1, 0, 1};
std::deque<wstring> const
    ShingleFilterTest::FOUR_GRAM_TYPES_WITHOUT_UNIGRAMS_MIN_TRI_GRAM =
        std::deque<wstring>{L"shingle", L"shingle", L"shingle", L"shingle",
                             L"shingle", L"shingle", L"shingle"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::FOUR_GRAM_TOKENS_MIN_FOUR_GRAM =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"please divide this sentence", 0, 27),
            createToken(L"divide", 7, 13),
            createToken(L"divide this sentence into", 7, 32),
            createToken(L"this", 14, 18),
            createToken(L"this sentence into shingles", 14, 39),
            createToken(L"sentence", 19, 27),
            createToken(L"into", 28, 32),
            createToken(L"shingles", 33, 39)};
std::deque<int> const
    ShingleFilterTest::FOUR_GRAM_POSITION_INCREMENTS_MIN_FOUR_GRAM =
        std::deque<int>{1, 0, 1, 0, 1, 0, 1, 1, 1};
std::deque<wstring> const ShingleFilterTest::FOUR_GRAM_TYPES_MIN_FOUR_GRAM =
    std::deque<wstring>{L"word",    L"shingle", L"word", L"shingle", L"word",
                         L"shingle", L"word",    L"word", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::FOUR_GRAM_TOKENS_WITHOUT_UNIGRAMS_MIN_FOUR_GRAM =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please divide this sentence", 0, 27),
            createToken(L"divide this sentence into", 7, 32),
            createToken(L"this sentence into shingles", 14, 39)};
std::deque<int> const ShingleFilterTest::
    FOUR_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_MIN_FOUR_GRAM =
        std::deque<int>{1, 1, 1};
std::deque<wstring> const
    ShingleFilterTest::FOUR_GRAM_TYPES_WITHOUT_UNIGRAMS_MIN_FOUR_GRAM =
        std::deque<wstring>{L"shingle", L"shingle", L"shingle"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::BI_GRAM_TOKENS_NO_SEPARATOR =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"pleasedivide", 0, 13),
            createToken(L"divide", 7, 13),
            createToken(L"dividethis", 7, 18),
            createToken(L"this", 14, 18),
            createToken(L"thissentence", 14, 27),
            createToken(L"sentence", 19, 27),
            createToken(L"sentenceinto", 19, 32),
            createToken(L"into", 28, 32),
            createToken(L"intoshingles", 28, 39),
            createToken(L"shingles", 33, 39)};
std::deque<int> const
    ShingleFilterTest::BI_GRAM_POSITION_INCREMENTS_NO_SEPARATOR =
        std::deque<int>{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
std::deque<wstring> const ShingleFilterTest::BI_GRAM_TYPES_NO_SEPARATOR =
    std::deque<wstring>{L"word", L"shingle", L"word", L"shingle",
                         L"word", L"shingle", L"word", L"shingle",
                         L"word", L"shingle", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::BI_GRAM_TOKENS_WITHOUT_UNIGRAMS_NO_SEPARATOR =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"pleasedivide", 0, 13),
            createToken(L"dividethis", 7, 18),
            createToken(L"thissentence", 14, 27),
            createToken(L"sentenceinto", 19, 32),
            createToken(L"intoshingles", 28, 39)};
std::deque<int> const ShingleFilterTest::
    BI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_NO_SEPARATOR =
        std::deque<int>{1, 1, 1, 1, 1};
std::deque<wstring> const
    ShingleFilterTest::BI_GRAM_TYPES_WITHOUT_UNIGRAMS_NO_SEPARATOR =
        std::deque<wstring>{L"shingle", L"shingle", L"shingle", L"shingle",
                             L"shingle"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TRI_GRAM_TOKENS_NO_SEPARATOR =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"pleasedivide", 0, 13),
            createToken(L"pleasedividethis", 0, 18),
            createToken(L"divide", 7, 13),
            createToken(L"dividethis", 7, 18),
            createToken(L"dividethissentence", 7, 27),
            createToken(L"this", 14, 18),
            createToken(L"thissentence", 14, 27),
            createToken(L"thissentenceinto", 14, 32),
            createToken(L"sentence", 19, 27),
            createToken(L"sentenceinto", 19, 32),
            createToken(L"sentenceintoshingles", 19, 39),
            createToken(L"into", 28, 32),
            createToken(L"intoshingles", 28, 39),
            createToken(L"shingles", 33, 39)};
std::deque<int> const
    ShingleFilterTest::TRI_GRAM_POSITION_INCREMENTS_NO_SEPARATOR =
        std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1};
std::deque<wstring> const ShingleFilterTest::TRI_GRAM_TYPES_NO_SEPARATOR =
    std::deque<wstring>{L"word",    L"shingle", L"shingle", L"word",
                         L"shingle", L"shingle", L"word",    L"shingle",
                         L"shingle", L"word",    L"shingle", L"shingle",
                         L"word",    L"shingle", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TRI_GRAM_TOKENS_WITHOUT_UNIGRAMS_NO_SEPARATOR =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"pleasedivide", 0, 13),
            createToken(L"pleasedividethis", 0, 18),
            createToken(L"dividethis", 7, 18),
            createToken(L"dividethissentence", 7, 27),
            createToken(L"thissentence", 14, 27),
            createToken(L"thissentenceinto", 14, 32),
            createToken(L"sentenceinto", 19, 32),
            createToken(L"sentenceintoshingles", 19, 39),
            createToken(L"intoshingles", 28, 39)};
std::deque<int> const ShingleFilterTest::
    TRI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_NO_SEPARATOR =
        std::deque<int>{1, 0, 1, 0, 1, 0, 1, 0, 1};
std::deque<wstring> const
    ShingleFilterTest::TRI_GRAM_TYPES_WITHOUT_UNIGRAMS_NO_SEPARATOR =
        std::deque<wstring>{L"shingle", L"shingle", L"shingle",
                             L"shingle", L"shingle", L"shingle",
                             L"shingle", L"shingle", L"shingle"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::BI_GRAM_TOKENS_ALT_SEPARATOR =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"please<SEP>divide", 0, 13),
            createToken(L"divide", 7, 13),
            createToken(L"divide<SEP>this", 7, 18),
            createToken(L"this", 14, 18),
            createToken(L"this<SEP>sentence", 14, 27),
            createToken(L"sentence", 19, 27),
            createToken(L"sentence<SEP>into", 19, 32),
            createToken(L"into", 28, 32),
            createToken(L"into<SEP>shingles", 28, 39),
            createToken(L"shingles", 33, 39)};
std::deque<int> const
    ShingleFilterTest::BI_GRAM_POSITION_INCREMENTS_ALT_SEPARATOR =
        std::deque<int>{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
std::deque<wstring> const ShingleFilterTest::BI_GRAM_TYPES_ALT_SEPARATOR =
    std::deque<wstring>{L"word", L"shingle", L"word", L"shingle",
                         L"word", L"shingle", L"word", L"shingle",
                         L"word", L"shingle", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::BI_GRAM_TOKENS_WITHOUT_UNIGRAMS_ALT_SEPARATOR =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please<SEP>divide", 0, 13),
            createToken(L"divide<SEP>this", 7, 18),
            createToken(L"this<SEP>sentence", 14, 27),
            createToken(L"sentence<SEP>into", 19, 32),
            createToken(L"into<SEP>shingles", 28, 39)};
std::deque<int> const ShingleFilterTest::
    BI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_ALT_SEPARATOR =
        std::deque<int>{1, 1, 1, 1, 1};
std::deque<wstring> const
    ShingleFilterTest::BI_GRAM_TYPES_WITHOUT_UNIGRAMS_ALT_SEPARATOR =
        std::deque<wstring>{L"shingle", L"shingle", L"shingle", L"shingle",
                             L"shingle"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TRI_GRAM_TOKENS_ALT_SEPARATOR =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"please<SEP>divide", 0, 13),
            createToken(L"please<SEP>divide<SEP>this", 0, 18),
            createToken(L"divide", 7, 13),
            createToken(L"divide<SEP>this", 7, 18),
            createToken(L"divide<SEP>this<SEP>sentence", 7, 27),
            createToken(L"this", 14, 18),
            createToken(L"this<SEP>sentence", 14, 27),
            createToken(L"this<SEP>sentence<SEP>into", 14, 32),
            createToken(L"sentence", 19, 27),
            createToken(L"sentence<SEP>into", 19, 32),
            createToken(L"sentence<SEP>into<SEP>shingles", 19, 39),
            createToken(L"into", 28, 32),
            createToken(L"into<SEP>shingles", 28, 39),
            createToken(L"shingles", 33, 39)};
std::deque<int> const
    ShingleFilterTest::TRI_GRAM_POSITION_INCREMENTS_ALT_SEPARATOR =
        std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1};
std::deque<wstring> const ShingleFilterTest::TRI_GRAM_TYPES_ALT_SEPARATOR =
    std::deque<wstring>{L"word",    L"shingle", L"shingle", L"word",
                         L"shingle", L"shingle", L"word",    L"shingle",
                         L"shingle", L"word",    L"shingle", L"shingle",
                         L"word",    L"shingle", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TRI_GRAM_TOKENS_WITHOUT_UNIGRAMS_ALT_SEPARATOR =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please<SEP>divide", 0, 13),
            createToken(L"please<SEP>divide<SEP>this", 0, 18),
            createToken(L"divide<SEP>this", 7, 18),
            createToken(L"divide<SEP>this<SEP>sentence", 7, 27),
            createToken(L"this<SEP>sentence", 14, 27),
            createToken(L"this<SEP>sentence<SEP>into", 14, 32),
            createToken(L"sentence<SEP>into", 19, 32),
            createToken(L"sentence<SEP>into<SEP>shingles", 19, 39),
            createToken(L"into<SEP>shingles", 28, 39)};
std::deque<int> const ShingleFilterTest::
    TRI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_ALT_SEPARATOR =
        std::deque<int>{1, 0, 1, 0, 1, 0, 1, 0, 1};
std::deque<wstring> const
    ShingleFilterTest::TRI_GRAM_TYPES_WITHOUT_UNIGRAMS_ALT_SEPARATOR =
        std::deque<wstring>{L"shingle", L"shingle", L"shingle",
                             L"shingle", L"shingle", L"shingle",
                             L"shingle", L"shingle", L"shingle"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TRI_GRAM_TOKENS_NULL_SEPARATOR =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"pleasedivide", 0, 13),
            createToken(L"pleasedividethis", 0, 18),
            createToken(L"divide", 7, 13),
            createToken(L"dividethis", 7, 18),
            createToken(L"dividethissentence", 7, 27),
            createToken(L"this", 14, 18),
            createToken(L"thissentence", 14, 27),
            createToken(L"thissentenceinto", 14, 32),
            createToken(L"sentence", 19, 27),
            createToken(L"sentenceinto", 19, 32),
            createToken(L"sentenceintoshingles", 19, 39),
            createToken(L"into", 28, 32),
            createToken(L"intoshingles", 28, 39),
            createToken(L"shingles", 33, 39)};
std::deque<int> const
    ShingleFilterTest::TRI_GRAM_POSITION_INCREMENTS_NULL_SEPARATOR =
        std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1};
std::deque<wstring> const ShingleFilterTest::TRI_GRAM_TYPES_NULL_SEPARATOR =
    std::deque<wstring>{L"word",    L"shingle", L"shingle", L"word",
                         L"shingle", L"shingle", L"word",    L"shingle",
                         L"shingle", L"word",    L"shingle", L"shingle",
                         L"word",    L"shingle", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TEST_TOKEN_POS_INCR_EQUAL_TO_N =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6), createToken(L"divide", 7, 13),
            createToken(L"this", 14, 18), createToken(L"sentence", 29, 37, 3),
            createToken(L"into", 38, 42), createToken(L"shingles", 43, 49)};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TRI_GRAM_TOKENS_POS_INCR_EQUAL_TO_N =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"please divide", 0, 13),
            createToken(L"please divide this", 0, 18),
            createToken(L"divide", 7, 13),
            createToken(L"divide this", 7, 18),
            createToken(L"divide this _", 7, 29),
            createToken(L"this", 14, 18),
            createToken(L"this _", 14, 29),
            createToken(L"this _ _", 14, 29),
            createToken(L"_ _ sentence", 29, 37),
            createToken(L"_ sentence", 29, 37),
            createToken(L"_ sentence into", 29, 42),
            createToken(L"sentence", 29, 37),
            createToken(L"sentence into", 29, 42),
            createToken(L"sentence into shingles", 29, 49),
            createToken(L"into", 38, 42),
            createToken(L"into shingles", 38, 49),
            createToken(L"shingles", 43, 49)};
std::deque<int> const
    ShingleFilterTest::TRI_GRAM_POSITION_INCREMENTS_POS_INCR_EQUAL_TO_N =
        std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1};
std::deque<wstring> const
    ShingleFilterTest::TRI_GRAM_TYPES_POS_INCR_EQUAL_TO_N =
        std::deque<wstring>{
            L"word", L"shingle", L"shingle", L"word",    L"shingle", L"shingle",
            L"word", L"shingle", L"shingle", L"shingle", L"shingle", L"shingle",
            L"word", L"shingle", L"shingle", L"word",    L"shingle", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TRI_GRAM_TOKENS_POS_INCR_EQUAL_TO_N_WITHOUT_UNIGRAMS =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please divide", 0, 13),
            createToken(L"please divide this", 0, 18),
            createToken(L"divide this", 7, 18),
            createToken(L"divide this _", 7, 29),
            createToken(L"this _", 14, 29),
            createToken(L"this _ _", 14, 29),
            createToken(L"_ _ sentence", 29, 37),
            createToken(L"_ sentence", 29, 37),
            createToken(L"_ sentence into", 29, 42),
            createToken(L"sentence into", 29, 42),
            createToken(L"sentence into shingles", 29, 49),
            createToken(L"into shingles", 38, 49)};
std::deque<int> const ShingleFilterTest::
    TRI_GRAM_POSITION_INCREMENTS_POS_INCR_EQUAL_TO_N_WITHOUT_UNIGRAMS =
        std::deque<int>{1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1};
std::deque<wstring> const
    ShingleFilterTest::TRI_GRAM_TYPES_POS_INCR_EQUAL_TO_N_WITHOUT_UNIGRAMS =
        std::deque<wstring>{L"shingle", L"shingle", L"shingle", L"shingle",
                             L"shingle", L"shingle", L"shingle", L"shingle",
                             L"shingle", L"shingle", L"shingle", L"shingle"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TEST_TOKEN_POS_INCR_GREATER_THAN_N =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6), createToken(L"divide", 57, 63, 8),
            createToken(L"this", 64, 68), createToken(L"sentence", 69, 77),
            createToken(L"into", 78, 82), createToken(L"shingles", 83, 89)};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::TRI_GRAM_TOKENS_POS_INCR_GREATER_THAN_N =
        std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
            createToken(L"please", 0, 6),
            createToken(L"please _", 0, 57),
            createToken(L"please _ _", 0, 57),
            createToken(L"_ _ divide", 57, 63),
            createToken(L"_ divide", 57, 63),
            createToken(L"_ divide this", 57, 68),
            createToken(L"divide", 57, 63),
            createToken(L"divide this", 57, 68),
            createToken(L"divide this sentence", 57, 77),
            createToken(L"this", 64, 68),
            createToken(L"this sentence", 64, 77),
            createToken(L"this sentence into", 64, 82),
            createToken(L"sentence", 69, 77),
            createToken(L"sentence into", 69, 82),
            createToken(L"sentence into shingles", 69, 89),
            createToken(L"into", 78, 82),
            createToken(L"into shingles", 78, 89),
            createToken(L"shingles", 83, 89)};
std::deque<int> const
    ShingleFilterTest::TRI_GRAM_POSITION_INCREMENTS_POS_INCR_GREATER_THAN_N =
        std::deque<int>{1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1};
std::deque<wstring> const
    ShingleFilterTest::TRI_GRAM_TYPES_POS_INCR_GREATER_THAN_N =
        std::deque<wstring>{
            L"word", L"shingle", L"shingle", L"shingle", L"shingle", L"shingle",
            L"word", L"shingle", L"shingle", L"word",    L"shingle", L"shingle",
            L"word", L"shingle", L"shingle", L"word",    L"shingle", L"word"};
std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>> const
    ShingleFilterTest::
        TRI_GRAM_TOKENS_POS_INCR_GREATER_THAN_N_WITHOUT_UNIGRAMS =
            std::deque<std::shared_ptr<org::apache::lucene::analysis::Token>>{
                createToken(L"please _", 0, 57),
                createToken(L"please _ _", 0, 57),
                createToken(L"_ _ divide", 57, 63),
                createToken(L"_ divide", 57, 63),
                createToken(L"_ divide this", 57, 68),
                createToken(L"divide this", 57, 68),
                createToken(L"divide this sentence", 57, 77),
                createToken(L"this sentence", 64, 77),
                createToken(L"this sentence into", 64, 82),
                createToken(L"sentence into", 69, 82),
                createToken(L"sentence into shingles", 69, 89),
                createToken(L"into shingles", 78, 89)};
std::deque<int> const ShingleFilterTest::
    TRI_GRAM_POSITION_INCREMENTS_POS_INCR_GREATER_THAN_N_WITHOUT_UNIGRAMS =
        std::deque<int>{1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1};
std::deque<wstring> const
    ShingleFilterTest::TRI_GRAM_TYPES_POS_INCR_GREATER_THAN_N_WITHOUT_UNIGRAMS =
        std::deque<wstring>{L"shingle", L"shingle", L"shingle", L"shingle",
                             L"shingle", L"shingle", L"shingle", L"shingle",
                             L"shingle", L"shingle", L"shingle", L"shingle"};

void ShingleFilterTest::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  testTokenWithHoles = std::deque<std::shared_ptr<Token>>{
      createToken(L"please", 0, 6), createToken(L"divide", 7, 13),
      createToken(L"sentence", 19, 27, 2), createToken(L"shingles", 33, 39, 2)};
}

void ShingleFilterTest::testBiGramFilter() 
{
  this->shingleFilterTest(2, TEST_TOKEN, BI_GRAM_TOKENS,
                          BI_GRAM_POSITION_INCREMENTS, BI_GRAM_TYPES, true);
}

void ShingleFilterTest::testBiGramFilterWithHoles() 
{
  this->shingleFilterTest(2, testTokenWithHoles, BI_GRAM_TOKENS_WITH_HOLES,
                          BI_GRAM_POSITION_INCREMENTS_WITH_HOLES,
                          BI_GRAM_TYPES_WITH_HOLES, true);
}

void ShingleFilterTest::testBiGramFilterWithoutUnigrams() 
{
  this->shingleFilterTest(2, TEST_TOKEN, BI_GRAM_TOKENS_WITHOUT_UNIGRAMS,
                          BI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS,
                          BI_GRAM_TYPES_WITHOUT_UNIGRAMS, false);
}

void ShingleFilterTest::testBiGramFilterWithHolesWithoutUnigrams() throw(
    IOException)
{
  this->shingleFilterTest(
      2, testTokenWithHoles, BI_GRAM_TOKENS_WITH_HOLES_WITHOUT_UNIGRAMS,
      BI_GRAM_POSITION_INCREMENTS_WITH_HOLES_WITHOUT_UNIGRAMS,
      BI_GRAM_TYPES_WITHOUT_UNIGRAMS, false);
}

void ShingleFilterTest::testBiGramFilterWithSingleToken() 
{
  this->shingleFilterTest(2, TEST_SINGLE_TOKEN, SINGLE_TOKEN,
                          SINGLE_TOKEN_INCREMENTS, SINGLE_TOKEN_TYPES, true);
}

void ShingleFilterTest::testBiGramFilterWithSingleTokenWithoutUnigrams() throw(
    IOException)
{
  this->shingleFilterTest(2, TEST_SINGLE_TOKEN, EMPTY_TOKEN_ARRAY,
                          EMPTY_TOKEN_INCREMENTS_ARRAY, EMPTY_TOKEN_TYPES_ARRAY,
                          false);
}

void ShingleFilterTest::testBiGramFilterWithEmptyTokenStream() throw(
    IOException)
{
  this->shingleFilterTest(2, EMPTY_TOKEN_ARRAY, EMPTY_TOKEN_ARRAY,
                          EMPTY_TOKEN_INCREMENTS_ARRAY, EMPTY_TOKEN_TYPES_ARRAY,
                          true);
}

void ShingleFilterTest::
    testBiGramFilterWithEmptyTokenStreamWithoutUnigrams() 
{
  this->shingleFilterTest(2, EMPTY_TOKEN_ARRAY, EMPTY_TOKEN_ARRAY,
                          EMPTY_TOKEN_INCREMENTS_ARRAY, EMPTY_TOKEN_TYPES_ARRAY,
                          false);
}

void ShingleFilterTest::testTriGramFilter() 
{
  this->shingleFilterTest(3, TEST_TOKEN, TRI_GRAM_TOKENS,
                          TRI_GRAM_POSITION_INCREMENTS, TRI_GRAM_TYPES, true);
}

void ShingleFilterTest::testTriGramFilterWithoutUnigrams() 
{
  this->shingleFilterTest(3, TEST_TOKEN, TRI_GRAM_TOKENS_WITHOUT_UNIGRAMS,
                          TRI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS,
                          TRI_GRAM_TYPES_WITHOUT_UNIGRAMS, false);
}

void ShingleFilterTest::testFourGramFilter() 
{
  this->shingleFilterTest(4, TEST_TOKEN, FOUR_GRAM_TOKENS,
                          FOUR_GRAM_POSITION_INCREMENTS, FOUR_GRAM_TYPES, true);
}

void ShingleFilterTest::testFourGramFilterWithoutUnigrams() 
{
  this->shingleFilterTest(4, TEST_TOKEN, FOUR_GRAM_TOKENS_WITHOUT_UNIGRAMS,
                          FOUR_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS,
                          FOUR_GRAM_TYPES_WITHOUT_UNIGRAMS, false);
}

void ShingleFilterTest::testTriGramFilterMinTriGram() 
{
  this->shingleFilterTest(3, 3, TEST_TOKEN, TRI_GRAM_TOKENS_MIN_TRI_GRAM,
                          TRI_GRAM_POSITION_INCREMENTS_MIN_TRI_GRAM,
                          TRI_GRAM_TYPES_MIN_TRI_GRAM, true);
}

void ShingleFilterTest::testTriGramFilterWithoutUnigramsMinTriGram() throw(
    IOException)
{
  this->shingleFilterTest(
      3, 3, TEST_TOKEN, TRI_GRAM_TOKENS_WITHOUT_UNIGRAMS_MIN_TRI_GRAM,
      TRI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_MIN_TRI_GRAM,
      TRI_GRAM_TYPES_WITHOUT_UNIGRAMS_MIN_TRI_GRAM, false);
}

void ShingleFilterTest::testFourGramFilterMinTriGram() 
{
  this->shingleFilterTest(3, 4, TEST_TOKEN, FOUR_GRAM_TOKENS_MIN_TRI_GRAM,
                          FOUR_GRAM_POSITION_INCREMENTS_MIN_TRI_GRAM,
                          FOUR_GRAM_TYPES_MIN_TRI_GRAM, true);
}

void ShingleFilterTest::testFourGramFilterWithoutUnigramsMinTriGram() throw(
    IOException)
{
  this->shingleFilterTest(
      3, 4, TEST_TOKEN, FOUR_GRAM_TOKENS_WITHOUT_UNIGRAMS_MIN_TRI_GRAM,
      FOUR_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_MIN_TRI_GRAM,
      FOUR_GRAM_TYPES_WITHOUT_UNIGRAMS_MIN_TRI_GRAM, false);
}

void ShingleFilterTest::testFourGramFilterMinFourGram() 
{
  this->shingleFilterTest(4, 4, TEST_TOKEN, FOUR_GRAM_TOKENS_MIN_FOUR_GRAM,
                          FOUR_GRAM_POSITION_INCREMENTS_MIN_FOUR_GRAM,
                          FOUR_GRAM_TYPES_MIN_FOUR_GRAM, true);
}

void ShingleFilterTest::testFourGramFilterWithoutUnigramsMinFourGram() throw(
    IOException)
{
  this->shingleFilterTest(
      4, 4, TEST_TOKEN, FOUR_GRAM_TOKENS_WITHOUT_UNIGRAMS_MIN_FOUR_GRAM,
      FOUR_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_MIN_FOUR_GRAM,
      FOUR_GRAM_TYPES_WITHOUT_UNIGRAMS_MIN_FOUR_GRAM, false);
}

void ShingleFilterTest::testBiGramFilterNoSeparator() 
{
  this->shingleFilterTest(L"", 2, 2, TEST_TOKEN, BI_GRAM_TOKENS_NO_SEPARATOR,
                          BI_GRAM_POSITION_INCREMENTS_NO_SEPARATOR,
                          BI_GRAM_TYPES_NO_SEPARATOR, true);
}

void ShingleFilterTest::testBiGramFilterWithoutUnigramsNoSeparator() throw(
    IOException)
{
  this->shingleFilterTest(
      L"", 2, 2, TEST_TOKEN, BI_GRAM_TOKENS_WITHOUT_UNIGRAMS_NO_SEPARATOR,
      BI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_NO_SEPARATOR,
      BI_GRAM_TYPES_WITHOUT_UNIGRAMS_NO_SEPARATOR, false);
}

void ShingleFilterTest::testTriGramFilterNoSeparator() 
{
  this->shingleFilterTest(L"", 2, 3, TEST_TOKEN, TRI_GRAM_TOKENS_NO_SEPARATOR,
                          TRI_GRAM_POSITION_INCREMENTS_NO_SEPARATOR,
                          TRI_GRAM_TYPES_NO_SEPARATOR, true);
}

void ShingleFilterTest::testTriGramFilterWithoutUnigramsNoSeparator() throw(
    IOException)
{
  this->shingleFilterTest(
      L"", 2, 3, TEST_TOKEN, TRI_GRAM_TOKENS_WITHOUT_UNIGRAMS_NO_SEPARATOR,
      TRI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_NO_SEPARATOR,
      TRI_GRAM_TYPES_WITHOUT_UNIGRAMS_NO_SEPARATOR, false);
}

void ShingleFilterTest::testBiGramFilterAltSeparator() 
{
  this->shingleFilterTest(L"<SEP>", 2, 2, TEST_TOKEN,
                          BI_GRAM_TOKENS_ALT_SEPARATOR,
                          BI_GRAM_POSITION_INCREMENTS_ALT_SEPARATOR,
                          BI_GRAM_TYPES_ALT_SEPARATOR, true);
}

void ShingleFilterTest::testBiGramFilterWithoutUnigramsAltSeparator() throw(
    IOException)
{
  this->shingleFilterTest(
      L"<SEP>", 2, 2, TEST_TOKEN, BI_GRAM_TOKENS_WITHOUT_UNIGRAMS_ALT_SEPARATOR,
      BI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_ALT_SEPARATOR,
      BI_GRAM_TYPES_WITHOUT_UNIGRAMS_ALT_SEPARATOR, false);
}

void ShingleFilterTest::testTriGramFilterAltSeparator() 
{
  this->shingleFilterTest(L"<SEP>", 2, 3, TEST_TOKEN,
                          TRI_GRAM_TOKENS_ALT_SEPARATOR,
                          TRI_GRAM_POSITION_INCREMENTS_ALT_SEPARATOR,
                          TRI_GRAM_TYPES_ALT_SEPARATOR, true);
}

void ShingleFilterTest::testTriGramFilterWithoutUnigramsAltSeparator() throw(
    IOException)
{
  this->shingleFilterTest(
      L"<SEP>", 2, 3, TEST_TOKEN,
      TRI_GRAM_TOKENS_WITHOUT_UNIGRAMS_ALT_SEPARATOR,
      TRI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS_ALT_SEPARATOR,
      TRI_GRAM_TYPES_WITHOUT_UNIGRAMS_ALT_SEPARATOR, false);
}

void ShingleFilterTest::testTriGramFilterNullSeparator() 
{
  this->shingleFilterTest(nullptr, 2, 3, TEST_TOKEN,
                          TRI_GRAM_TOKENS_NULL_SEPARATOR,
                          TRI_GRAM_POSITION_INCREMENTS_NULL_SEPARATOR,
                          TRI_GRAM_TYPES_NULL_SEPARATOR, true);
}

void ShingleFilterTest::testPositionIncrementEqualToN() 
{
  this->shingleFilterTest(2, 3, TEST_TOKEN_POS_INCR_EQUAL_TO_N,
                          TRI_GRAM_TOKENS_POS_INCR_EQUAL_TO_N,
                          TRI_GRAM_POSITION_INCREMENTS_POS_INCR_EQUAL_TO_N,
                          TRI_GRAM_TYPES_POS_INCR_EQUAL_TO_N, true);
}

void ShingleFilterTest::testPositionIncrementEqualToNWithoutUnigrams() throw(
    IOException)
{
  this->shingleFilterTest(
      2, 3, TEST_TOKEN_POS_INCR_EQUAL_TO_N,
      TRI_GRAM_TOKENS_POS_INCR_EQUAL_TO_N_WITHOUT_UNIGRAMS,
      TRI_GRAM_POSITION_INCREMENTS_POS_INCR_EQUAL_TO_N_WITHOUT_UNIGRAMS,
      TRI_GRAM_TYPES_POS_INCR_EQUAL_TO_N_WITHOUT_UNIGRAMS, false);
}

void ShingleFilterTest::testPositionIncrementGreaterThanN() 
{
  this->shingleFilterTest(2, 3, TEST_TOKEN_POS_INCR_GREATER_THAN_N,
                          TRI_GRAM_TOKENS_POS_INCR_GREATER_THAN_N,
                          TRI_GRAM_POSITION_INCREMENTS_POS_INCR_GREATER_THAN_N,
                          TRI_GRAM_TYPES_POS_INCR_GREATER_THAN_N, true);
}

void ShingleFilterTest::
    testPositionIncrementGreaterThanNWithoutUnigrams() 
{
  this->shingleFilterTest(
      2, 3, TEST_TOKEN_POS_INCR_GREATER_THAN_N,
      TRI_GRAM_TOKENS_POS_INCR_GREATER_THAN_N_WITHOUT_UNIGRAMS,
      TRI_GRAM_POSITION_INCREMENTS_POS_INCR_GREATER_THAN_N_WITHOUT_UNIGRAMS,
      TRI_GRAM_TYPES_POS_INCR_GREATER_THAN_N_WITHOUT_UNIGRAMS, false);
}

void ShingleFilterTest::testReset() 
{
  shared_ptr<Tokenizer> wsTokenizer = make_shared<WhitespaceTokenizer>();
  wsTokenizer->setReader(
      make_shared<StringReader>(L"please divide this sentence"));
  shared_ptr<TokenStream> filter = make_shared<ShingleFilter>(wsTokenizer, 2);
  assertTokenStreamContents(
      filter,
      std::deque<wstring>{L"please", L"please divide", L"divide",
                           L"divide this", L"this", L"this sentence",
                           L"sentence"},
      std::deque<int>{0, 0, 7, 7, 14, 14, 19},
      std::deque<int>{6, 13, 13, 18, 18, 27, 27},
      std::deque<wstring>{TypeAttribute::DEFAULT_TYPE, L"shingle",
                           TypeAttribute::DEFAULT_TYPE, L"shingle",
                           TypeAttribute::DEFAULT_TYPE, L"shingle",
                           TypeAttribute::DEFAULT_TYPE},
      std::deque<int>{1, 0, 1, 0, 1, 0, 1});
  wsTokenizer->setReader(
      make_shared<StringReader>(L"please divide this sentence"));
  assertTokenStreamContents(
      filter,
      std::deque<wstring>{L"please", L"please divide", L"divide",
                           L"divide this", L"this", L"this sentence",
                           L"sentence"},
      std::deque<int>{0, 0, 7, 7, 14, 14, 19},
      std::deque<int>{6, 13, 13, 18, 18, 27, 27},
      std::deque<wstring>{TypeAttribute::DEFAULT_TYPE, L"shingle",
                           TypeAttribute::DEFAULT_TYPE, L"shingle",
                           TypeAttribute::DEFAULT_TYPE, L"shingle",
                           TypeAttribute::DEFAULT_TYPE},
      std::deque<int>{1, 0, 1, 0, 1, 0, 1});
}

void ShingleFilterTest::testOutputUnigramsIfNoShinglesSingleTokenCase() throw(
    IOException)
{
  // Single token input with outputUnigrams==false is the primary case where
  // enabling this option should alter program behavior.
  this->shingleFilterTest(2, 2, TEST_SINGLE_TOKEN, SINGLE_TOKEN,
                          SINGLE_TOKEN_INCREMENTS, SINGLE_TOKEN_TYPES, false,
                          true);
}

void ShingleFilterTest::testOutputUnigramsIfNoShinglesWithSimpleBigram() throw(
    IOException)
{
  // Here we expect the same result as with testBiGramFilter().
  this->shingleFilterTest(2, 2, TEST_TOKEN, BI_GRAM_TOKENS,
                          BI_GRAM_POSITION_INCREMENTS, BI_GRAM_TYPES, true,
                          true);
}

void ShingleFilterTest::
    testOutputUnigramsIfNoShinglesWithSimpleUnigramlessBigram() throw(
        IOException)
{
  // Here we expect the same result as with testBiGramFilterWithoutUnigrams().
  this->shingleFilterTest(2, 2, TEST_TOKEN, BI_GRAM_TOKENS_WITHOUT_UNIGRAMS,
                          BI_GRAM_POSITION_INCREMENTS_WITHOUT_UNIGRAMS,
                          BI_GRAM_TYPES_WITHOUT_UNIGRAMS, false, true);
}

void ShingleFilterTest::
    testOutputUnigramsIfNoShinglesWithMultipleInputTokens() 
{
  // Test when the minimum shingle size is greater than the number of input
  // tokens
  this->shingleFilterTest(7, 7, TEST_TOKEN, TEST_TOKEN,
                          UNIGRAM_ONLY_POSITION_INCREMENTS, UNIGRAM_ONLY_TYPES,
                          false, true);
}

void ShingleFilterTest::shingleFilterTest(
    int maxSize, std::deque<std::shared_ptr<Token>> &tokensToShingle,
    std::deque<std::shared_ptr<Token>> &tokensToCompare,
    std::deque<int> &positionIncrements, std::deque<wstring> &types,
    bool outputUnigrams) 
{

  shared_ptr<ShingleFilter> filter = make_shared<ShingleFilter>(
      make_shared<CannedTokenStream>(tokensToShingle), maxSize);
  filter->setOutputUnigrams(outputUnigrams);
  shingleFilterTestCommon(filter, tokensToCompare, positionIncrements, types);
}

void ShingleFilterTest::shingleFilterTest(
    int minSize, int maxSize,
    std::deque<std::shared_ptr<Token>> &tokensToShingle,
    std::deque<std::shared_ptr<Token>> &tokensToCompare,
    std::deque<int> &positionIncrements, std::deque<wstring> &types,
    bool outputUnigrams) 
{
  shared_ptr<ShingleFilter> filter = make_shared<ShingleFilter>(
      make_shared<CannedTokenStream>(tokensToShingle), minSize, maxSize);
  filter->setOutputUnigrams(outputUnigrams);
  shingleFilterTestCommon(filter, tokensToCompare, positionIncrements, types);
}

void ShingleFilterTest::shingleFilterTest(
    int minSize, int maxSize,
    std::deque<std::shared_ptr<Token>> &tokensToShingle,
    std::deque<std::shared_ptr<Token>> &tokensToCompare,
    std::deque<int> &positionIncrements, std::deque<wstring> &types,
    bool outputUnigrams, bool outputUnigramsIfNoShingles) 
{
  shared_ptr<ShingleFilter> filter = make_shared<ShingleFilter>(
      make_shared<CannedTokenStream>(tokensToShingle), minSize, maxSize);
  filter->setOutputUnigrams(outputUnigrams);
  filter->setOutputUnigramsIfNoShingles(outputUnigramsIfNoShingles);
  shingleFilterTestCommon(filter, tokensToCompare, positionIncrements, types);
}

void ShingleFilterTest::shingleFilterTest(
    const wstring &tokenSeparator, int minSize, int maxSize,
    std::deque<std::shared_ptr<Token>> &tokensToShingle,
    std::deque<std::shared_ptr<Token>> &tokensToCompare,
    std::deque<int> &positionIncrements, std::deque<wstring> &types,
    bool outputUnigrams) 
{
  shared_ptr<ShingleFilter> filter = make_shared<ShingleFilter>(
      make_shared<CannedTokenStream>(tokensToShingle), minSize, maxSize);
  filter->setTokenSeparator(tokenSeparator);
  filter->setOutputUnigrams(outputUnigrams);
  shingleFilterTestCommon(filter, tokensToCompare, positionIncrements, types);
}

void ShingleFilterTest::shingleFilterTestCommon(
    shared_ptr<ShingleFilter> filter,
    std::deque<std::shared_ptr<Token>> &tokensToCompare,
    std::deque<int> &positionIncrements,
    std::deque<wstring> &types) 
{
  std::deque<wstring> text(tokensToCompare.size());
  std::deque<int> startOffsets(tokensToCompare.size());
  std::deque<int> endOffsets(tokensToCompare.size());

  for (int i = 0; i < tokensToCompare.size(); i++) {
    text[i] =
        wstring(tokensToCompare[i]->buffer(), 0, tokensToCompare[i]->length());
    startOffsets[i] = tokensToCompare[i]->startOffset();
    endOffsets[i] = tokensToCompare[i]->endOffset();
  }

  assertTokenStreamContents(filter, text, startOffsets, endOffsets, types,
                            positionIncrements);
}

shared_ptr<Token> ShingleFilterTest::createToken(const wstring &term, int start,
                                                 int offset)
{
  return createToken(term, start, offset, 1);
}

shared_ptr<Token> ShingleFilterTest::createToken(const wstring &term, int start,
                                                 int offset,
                                                 int positionIncrement)
{
  shared_ptr<Token> token = make_shared<Token>();
  token->setOffset(start, offset);
  token->copyBuffer(term.toCharArray(), 0, term.length());
  token->setPositionIncrement(positionIncrement);
  return token;
}

void ShingleFilterTest::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}

ShingleFilterTest::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<ShingleFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
ShingleFilterTest::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ShingleFilter>(tokenizer));
}

void ShingleFilterTest::testRandomHugeStrings() 
{
  shared_ptr<Random> random = ShingleFilterTest::random();
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkRandomData(random, a, 100 * RANDOM_MULTIPLIER, 8192);
  delete a;
}

ShingleFilterTest::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<ShingleFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
ShingleFilterTest::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ShingleFilter>(tokenizer));
}

void ShingleFilterTest::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

ShingleFilterTest::AnalyzerAnonymousInnerClass3::AnalyzerAnonymousInnerClass3(
    shared_ptr<ShingleFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
ShingleFilterTest::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<ShingleFilter>(tokenizer));
}

void ShingleFilterTest::testTrailingHole1() 
{
  // Analyzing "wizard of", where of is removed as a
  // stopword leaving a trailing hole:
  std::deque<std::shared_ptr<Token>> inputTokens = {
      createToken(L"wizard", 0, 6)};
  shared_ptr<ShingleFilter> filter = make_shared<ShingleFilter>(
      make_shared<CannedTokenStream>(1, 9, inputTokens), 2, 2);

  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"wizard", L"wizard _"},
                            std::deque<int>{0, 0}, std::deque<int>{6, 9},
                            std::deque<int>{1, 0}, 9);
}

void ShingleFilterTest::testTrailingHole2() 
{
  // Analyzing "purple wizard of", where of is removed as a
  // stopword leaving a trailing hole:
  std::deque<std::shared_ptr<Token>> inputTokens = {
      createToken(L"purple", 0, 6), createToken(L"wizard", 7, 13)};
  shared_ptr<ShingleFilter> filter = make_shared<ShingleFilter>(
      make_shared<CannedTokenStream>(1, 16, inputTokens), 2, 2);

  assertTokenStreamContents(
      filter,
      std::deque<wstring>{L"purple", L"purple wizard", L"wizard", L"wizard _"},
      std::deque<int>{0, 0, 7, 7}, std::deque<int>{6, 13, 13, 16},
      std::deque<int>{1, 0, 1, 0}, 16);
}

void ShingleFilterTest::testTwoTrailingHoles() 
{
  // Analyzing "purple wizard of the", where of and the are removed as a
  // stopwords, leaving two trailing holes:
  std::deque<std::shared_ptr<Token>> inputTokens = {
      createToken(L"purple", 0, 6), createToken(L"wizard", 7, 13)};
  shared_ptr<ShingleFilter> filter = make_shared<ShingleFilter>(
      make_shared<CannedTokenStream>(2, 20, inputTokens), 2, 2);

  assertTokenStreamContents(
      filter,
      std::deque<wstring>{L"purple", L"purple wizard", L"wizard", L"wizard _"},
      std::deque<int>{0, 0, 7, 7}, std::deque<int>{6, 13, 13, 20},
      std::deque<int>{1, 0, 1, 0}, 20);
}

void ShingleFilterTest::testTwoTrailingHolesTriShingle() 
{
  // Analyzing "purple wizard of the", where of and the are removed as a
  // stopwords, leaving two trailing holes:
  std::deque<std::shared_ptr<Token>> inputTokens = {
      createToken(L"purple", 0, 6), createToken(L"wizard", 7, 13)};
  shared_ptr<ShingleFilter> filter = make_shared<ShingleFilter>(
      make_shared<CannedTokenStream>(2, 20, inputTokens), 2, 3);

  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"purple", L"purple wizard",
                                                 L"purple wizard _", L"wizard",
                                                 L"wizard _", L"wizard _ _"},
                            std::deque<int>{0, 0, 0, 7, 7, 7},
                            std::deque<int>{6, 13, 20, 13, 20, 20},
                            std::deque<int>{1, 0, 0, 1, 0, 0}, 20);
}

void ShingleFilterTest::testTwoTrailingHolesTriShingleWithTokenFiller() throw(
    IOException)
{
  // Analyzing "purple wizard of the", where of and the are removed as a
  // stopwords, leaving two trailing holes:
  std::deque<std::shared_ptr<Token>> inputTokens = {
      createToken(L"purple", 0, 6), createToken(L"wizard", 7, 13)};
  shared_ptr<ShingleFilter> filter = make_shared<ShingleFilter>(
      make_shared<CannedTokenStream>(2, 20, inputTokens), 2, 3);
  filter->setFillerToken(L"--");

  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"purple", L"purple wizard",
                                                 L"purple wizard --", L"wizard",
                                                 L"wizard --", L"wizard -- --"},
                            std::deque<int>{0, 0, 0, 7, 7, 7},
                            std::deque<int>{6, 13, 20, 13, 20, 20},
                            std::deque<int>{1, 0, 0, 1, 0, 0}, 20);

  filter = make_shared<ShingleFilter>(
      make_shared<CannedTokenStream>(2, 20, inputTokens), 2, 3);
  filter->setFillerToken(L"");

  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"purple", L"purple wizard",
                                                 L"purple wizard ", L"wizard",
                                                 L"wizard ", L"wizard  "},
                            std::deque<int>{0, 0, 0, 7, 7, 7},
                            std::deque<int>{6, 13, 20, 13, 20, 20},
                            std::deque<int>{1, 0, 0, 1, 0, 0}, 20);

  filter = make_shared<ShingleFilter>(
      make_shared<CannedTokenStream>(2, 20, inputTokens), 2, 3);
  filter->setFillerToken(L"");

  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"purple", L"purple wizard",
                                                 L"purple wizard ", L"wizard",
                                                 L"wizard ", L"wizard  "},
                            std::deque<int>{0, 0, 0, 7, 7, 7},
                            std::deque<int>{6, 13, 20, 13, 20, 20},
                            std::deque<int>{1, 0, 0, 1, 0, 0}, 20);

  filter = make_shared<ShingleFilter>(
      make_shared<CannedTokenStream>(2, 20, inputTokens), 2, 3);
  filter->setFillerToken(L"");
  filter->setTokenSeparator(L"");
  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"purple", L"purplewizard",
                                                 L"purplewizard", L"wizard",
                                                 L"wizard", L"wizard"},
                            std::deque<int>{0, 0, 0, 7, 7, 7},
                            std::deque<int>{6, 13, 20, 13, 20, 20},
                            std::deque<int>{1, 0, 0, 1, 0, 0}, 20);
}

void ShingleFilterTest::testPositionLength() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this());
  assertTokenStreamContents(
      a->tokenStream(L"", L"to be or not to be"),
      std::deque<wstring>{L"to be or not", L"be or not to", L"or not to be"},
      std::deque<int>{0, 3, 6}, std::deque<int>{12, 15, 18}, nullptr,
      std::deque<int>{1, 1, 1}, std::deque<int>{1, 1, 1}, 18, false);

  a = make_shared<AnalyzerAnonymousInnerClass5>(shared_from_this());
  assertTokenStreamContents(
      a->tokenStream(L"", L"to be or not to be"),
      std::deque<wstring>{L"to be", L"to be or", L"to be or not", L"be or",
                           L"be or not", L"be or not to", L"or not",
                           L"or not to", L"or not to be", L"not to",
                           L"not to be", L"to be"},
      std::deque<int>{0, 0, 0, 3, 3, 3, 6, 6, 6, 9, 9, 13},
      std::deque<int>{5, 8, 12, 8, 12, 15, 12, 15, 18, 15, 18, 18}, nullptr,
      std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1},
      std::deque<int>{1, 2, 3, 1, 2, 3, 1, 2, 3, 1, 2, 1}, 18, false);

  a = make_shared<AnalyzerAnonymousInnerClass6>(shared_from_this());

  assertTokenStreamContents(
      a->tokenStream(L"", L"to be or not to be"),
      std::deque<wstring>{L"to be or", L"to be or not", L"be or not",
                           L"be or not to", L"or not to", L"or not to be",
                           L"not to be"},
      std::deque<int>{0, 0, 3, 3, 6, 6, 9},
      std::deque<int>{8, 12, 12, 15, 15, 18, 18}, nullptr,
      std::deque<int>{1, 0, 1, 0, 1, 0, 1, 0},
      std::deque<int>{1, 2, 1, 2, 1, 2, 1, 2}, 18, false);

  a = make_shared<AnalyzerAnonymousInnerClass7>(shared_from_this());
  assertTokenStreamContents(
      a->tokenStream(L"", L"to be or not to be"),
      std::deque<wstring>{L"to be or", L"to be or not", L"to be or not to",
                           L"be or not", L"be or not to", L"be or not to be",
                           L"or not to", L"or not to be", L"not to be"},
      std::deque<int>{0, 0, 0, 3, 3, 3, 6, 6, 9, 9},
      std::deque<int>{8, 12, 15, 12, 15, 18, 15, 18, 18}, nullptr,
      std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 1, 0},
      std::deque<int>{1, 2, 3, 1, 2, 3, 1, 2, 1}, 18, false);
}

ShingleFilterTest::AnalyzerAnonymousInnerClass4::AnalyzerAnonymousInnerClass4(
    shared_ptr<ShingleFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
ShingleFilterTest::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<ShingleFilter> filter =
      make_shared<ShingleFilter>(tokenizer, 4, 4);
  filter->setOutputUnigrams(false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

ShingleFilterTest::AnalyzerAnonymousInnerClass5::AnalyzerAnonymousInnerClass5(
    shared_ptr<ShingleFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
ShingleFilterTest::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<ShingleFilter> filter =
      make_shared<ShingleFilter>(tokenizer, 2, 4);
  filter->setOutputUnigrams(false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

ShingleFilterTest::AnalyzerAnonymousInnerClass6::AnalyzerAnonymousInnerClass6(
    shared_ptr<ShingleFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
ShingleFilterTest::AnalyzerAnonymousInnerClass6::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<ShingleFilter> filter =
      make_shared<ShingleFilter>(tokenizer, 3, 4);
  filter->setOutputUnigrams(false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}

ShingleFilterTest::AnalyzerAnonymousInnerClass7::AnalyzerAnonymousInnerClass7(
    shared_ptr<ShingleFilterTest> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
ShingleFilterTest::AnalyzerAnonymousInnerClass7::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<ShingleFilter> filter =
      make_shared<ShingleFilter>(tokenizer, 3, 5);
  filter->setOutputUnigrams(false);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, filter);
}
} // namespace org::apache::lucene::analysis::shingle