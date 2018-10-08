using namespace std;

#include "WordBreakTestUnicode_6_3_0.h"

namespace org::apache::lucene::analysis::standard
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using org::junit::Ignore;

void WordBreakTestUnicode_6_3_0::test(shared_ptr<Analyzer> analyzer) throw(
    runtime_error)
{
  // ÷ 0001 ÷ 0001 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0] <START OF
  // HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0001", std::deque<wstring>());

  // ÷ 0001 × 0308 ÷ 0001 ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0001", std::deque<wstring>());

  // ÷ 0001 ÷ 000D ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [3.2] <CARRIAGE
  // RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\r", std::deque<wstring>());

  // ÷ 0001 × 0308 ÷ 000D ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\r", std::deque<wstring>());

  // ÷ 0001 ÷ 000A ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [3.2] <LINE FEED
  // (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\n", std::deque<wstring>());

  // ÷ 0001 × 0308 ÷ 000A ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\n", std::deque<wstring>());

  // ÷ 0001 ÷ 000B ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [3.2] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u000B", std::deque<wstring>());

  // ÷ 0001 × 0308 ÷ 000B ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u000B", std::deque<wstring>());

  // ÷ 0001 ÷ 3031 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0] VERTICAL
  // KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u3031", std::deque<wstring>{L"\u3031"});

  // ÷ 0001 × 0308 ÷ 3031 ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 0001 ÷ 0041 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0] LATIN
  // CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0041", std::deque<wstring>{L"\u0041"});

  // ÷ 0001 × 0308 ÷ 0041 ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 0001 ÷ 003A ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u003A", std::deque<wstring>());

  // ÷ 0001 × 0308 ÷ 003A ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u003A", std::deque<wstring>());

  // ÷ 0001 ÷ 002C ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u002C", std::deque<wstring>());

  // ÷ 0001 × 0308 ÷ 002C ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u002C", std::deque<wstring>());

  // ÷ 0001 ÷ 002E ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u002E", std::deque<wstring>());

  // ÷ 0001 × 0308 ÷ 002E ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u002E", std::deque<wstring>());

  // ÷ 0001 ÷ 0030 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0030", std::deque<wstring>{L"\u0030"});

  // ÷ 0001 × 0308 ÷ 0030 ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 0001 ÷ 005F ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u005F", std::deque<wstring>());

  // ÷ 0001 × 0308 ÷ 005F ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u005F", std::deque<wstring>());

  // ÷ 0001 ÷ 1F1E6 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0] REGIONAL
  // INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 0001 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\uD83C\uDDE6",
                   std::deque<wstring>());

  // ÷ 0001 ÷ 05D0 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0] HEBREW
  // LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u05D0", std::deque<wstring>{L"\u05D0"});

  // ÷ 0001 × 0308 ÷ 05D0 ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 0001 ÷ 0022 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0] QUOTATION
  // MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\"", std::deque<wstring>());

  // ÷ 0001 × 0308 ÷ 0022 ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\"", std::deque<wstring>());

  // ÷ 0001 ÷ 0027 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0027", std::deque<wstring>());

  // ÷ 0001 × 0308 ÷ 0027 ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0027", std::deque<wstring>());

  // ÷ 0001 × 00AD ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u00AD", std::deque<wstring>());

  // ÷ 0001 × 0308 × 00AD ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u00AD", std::deque<wstring>());

  // ÷ 0001 × 0300 ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0] COMBINING
  // GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0300", std::deque<wstring>());

  // ÷ 0001 × 0308 × 0300 ÷  #  ÷ [0.2] <START OF HEADING> (Other) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0300", std::deque<wstring>());

  // ÷ 0001 ÷ 0061 × 2060 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0001 × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0001 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0001 × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] <START OF HEADING> (Other) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0001 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0001 × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0001 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote)
  // × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0001 × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] <START OF HEADING> (Other)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0001 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0001 × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] <START OF HEADING> (Other) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0001 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0001 × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] <START OF HEADING> (Other) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0001 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0001 × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0001 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0001 × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] <START OF HEADING> (Other) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0001 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] <START OF HEADING> (Other) ÷
  // [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0001 × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] <START OF HEADING> (Other)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0001\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000D ÷ 0001 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] <START OF
  // HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0001", std::deque<wstring>());

  // ÷ 000D ÷ 0308 ÷ 0001 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0001", std::deque<wstring>());

  // ÷ 000D ÷ 000D ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] <CARRIAGE
  // RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\r", std::deque<wstring>());

  // ÷ 000D ÷ 0308 ÷ 000D ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\r", std::deque<wstring>());

  // ÷ 000D × 000A ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) × [3.0] <LINE FEED
  // (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\n", std::deque<wstring>());

  // ÷ 000D ÷ 0308 ÷ 000A ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\n", std::deque<wstring>());

  // ÷ 000D ÷ 000B ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u000B", std::deque<wstring>());

  // ÷ 000D ÷ 0308 ÷ 000B ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u000B", std::deque<wstring>());

  // ÷ 000D ÷ 3031 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] VERTICAL
  // KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u3031", std::deque<wstring>{L"\u3031"});

  // ÷ 000D ÷ 0308 ÷ 3031 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 000D ÷ 0041 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] LATIN
  // CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0041", std::deque<wstring>{L"\u0041"});

  // ÷ 000D ÷ 0308 ÷ 0041 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 000D ÷ 003A ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u003A", std::deque<wstring>());

  // ÷ 000D ÷ 0308 ÷ 003A ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u003A", std::deque<wstring>());

  // ÷ 000D ÷ 002C ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u002C", std::deque<wstring>());

  // ÷ 000D ÷ 0308 ÷ 002C ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u002C", std::deque<wstring>());

  // ÷ 000D ÷ 002E ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u002E", std::deque<wstring>());

  // ÷ 000D ÷ 0308 ÷ 002E ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u002E", std::deque<wstring>());

  // ÷ 000D ÷ 0030 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0030", std::deque<wstring>{L"\u0030"});

  // ÷ 000D ÷ 0308 ÷ 0030 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 000D ÷ 005F ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u005F", std::deque<wstring>());

  // ÷ 000D ÷ 0308 ÷ 005F ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u005F", std::deque<wstring>());

  // ÷ 000D ÷ 1F1E6 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] REGIONAL
  // INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 000D ÷ 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 000D ÷ 05D0 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] HEBREW
  // LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u05D0", std::deque<wstring>{L"\u05D0"});

  // ÷ 000D ÷ 0308 ÷ 05D0 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 000D ÷ 0022 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] QUOTATION
  // MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\"", std::deque<wstring>());

  // ÷ 000D ÷ 0308 ÷ 0022 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\"", std::deque<wstring>());

  // ÷ 000D ÷ 0027 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0027", std::deque<wstring>());

  // ÷ 000D ÷ 0308 ÷ 0027 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0027", std::deque<wstring>());

  // ÷ 000D ÷ 00AD ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u00AD", std::deque<wstring>());

  // ÷ 000D ÷ 0308 × 00AD ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u00AD", std::deque<wstring>());

  // ÷ 000D ÷ 0300 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1] COMBINING
  // GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0300", std::deque<wstring>());

  // ÷ 000D ÷ 0308 × 0300 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0300", std::deque<wstring>());

  // ÷ 000D ÷ 0061 × 2060 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 000D ÷ 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 000D ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000D ÷ 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000D ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000D ÷ 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000D ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷
  // [3.1] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000D ÷ 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)>
  // (CR) ÷ [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000D ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000D ÷ 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000D ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000D ÷ 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000D ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000D ÷ 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000D ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷ [3.1]
  // DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000D ÷ 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000D ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)> (CR) ÷
  // [3.1] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000D ÷ 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] <CARRIAGE RETURN (CR)>
  // (CR) ÷ [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric)
  // ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\r\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000A ÷ 0001 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] <START OF
  // HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0001", std::deque<wstring>());

  // ÷ 000A ÷ 0308 ÷ 0001 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0001", std::deque<wstring>());

  // ÷ 000A ÷ 000D ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] <CARRIAGE RETURN
  // (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\r", std::deque<wstring>());

  // ÷ 000A ÷ 0308 ÷ 000D ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\r", std::deque<wstring>());

  // ÷ 000A ÷ 000A ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] <LINE FEED (LF)>
  // (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\n", std::deque<wstring>());

  // ÷ 000A ÷ 0308 ÷ 000A ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\n", std::deque<wstring>());

  // ÷ 000A ÷ 000B ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] <LINE TABULATION>
  // (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u000B", std::deque<wstring>());

  // ÷ 000A ÷ 0308 ÷ 000B ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u000B", std::deque<wstring>());

  // ÷ 000A ÷ 3031 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] VERTICAL KANA
  // REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u3031", std::deque<wstring>{L"\u3031"});

  // ÷ 000A ÷ 0308 ÷ 3031 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 000A ÷ 0041 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] LATIN CAPITAL
  // LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0041", std::deque<wstring>{L"\u0041"});

  // ÷ 000A ÷ 0308 ÷ 0041 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 000A ÷ 003A ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COLON (MidLetter)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u003A", std::deque<wstring>());

  // ÷ 000A ÷ 0308 ÷ 003A ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u003A", std::deque<wstring>());

  // ÷ 000A ÷ 002C ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMMA (MidNum) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\n\u002C", std::deque<wstring>());

  // ÷ 000A ÷ 0308 ÷ 002C ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u002C", std::deque<wstring>());

  // ÷ 000A ÷ 002E ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u002E", std::deque<wstring>());

  // ÷ 000A ÷ 0308 ÷ 002E ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u002E", std::deque<wstring>());

  // ÷ 000A ÷ 0030 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0030", std::deque<wstring>{L"\u0030"});

  // ÷ 000A ÷ 0308 ÷ 0030 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 000A ÷ 005F ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u005F", std::deque<wstring>());

  // ÷ 000A ÷ 0308 ÷ 005F ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u005F", std::deque<wstring>());

  // ÷ 000A ÷ 1F1E6 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] REGIONAL
  // INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 000A ÷ 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 000A ÷ 05D0 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] HEBREW LETTER
  // ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u05D0", std::deque<wstring>{L"\u05D0"});

  // ÷ 000A ÷ 0308 ÷ 05D0 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 000A ÷ 0022 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] QUOTATION MARK
  // (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\"", std::deque<wstring>());

  // ÷ 000A ÷ 0308 ÷ 0022 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\"", std::deque<wstring>());

  // ÷ 000A ÷ 0027 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0027", std::deque<wstring>());

  // ÷ 000A ÷ 0308 ÷ 0027 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0027", std::deque<wstring>());

  // ÷ 000A ÷ 00AD ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u00AD", std::deque<wstring>());

  // ÷ 000A ÷ 0308 × 00AD ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u00AD", std::deque<wstring>());

  // ÷ 000A ÷ 0300 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING GRAVE
  // ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0300", std::deque<wstring>());

  // ÷ 000A ÷ 0308 × 0300 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] COMBINING
  // DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0300", std::deque<wstring>());

  // ÷ 000A ÷ 0061 × 2060 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] LATIN
  // SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 000A ÷ 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 000A ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000A ÷ 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000A ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000A ÷ 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000A ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0]
  // WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000A ÷ 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000A ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000A ÷ 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000A ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] DIGIT ONE
  // (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000A ÷ 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000A ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] DIGIT ONE
  // (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000A ÷ 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000A ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1] DIGIT ONE
  // (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000A ÷ 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000A ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷ [3.1]
  // DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000A ÷ 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] <LINE FEED (LF)> (LF) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\n\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000B ÷ 0001 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] <START OF
  // HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0001", std::deque<wstring>());

  // ÷ 000B ÷ 0308 ÷ 0001 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0001", std::deque<wstring>());

  // ÷ 000B ÷ 000D ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] <CARRIAGE
  // RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\r", std::deque<wstring>());

  // ÷ 000B ÷ 0308 ÷ 000D ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\r", std::deque<wstring>());

  // ÷ 000B ÷ 000A ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] <LINE FEED
  // (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\n", std::deque<wstring>());

  // ÷ 000B ÷ 0308 ÷ 000A ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\n", std::deque<wstring>());

  // ÷ 000B ÷ 000B ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u000B", std::deque<wstring>());

  // ÷ 000B ÷ 0308 ÷ 000B ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u000B", std::deque<wstring>());

  // ÷ 000B ÷ 3031 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] VERTICAL
  // KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u3031", std::deque<wstring>{L"\u3031"});

  // ÷ 000B ÷ 0308 ÷ 3031 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 000B ÷ 0041 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] LATIN
  // CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0041", std::deque<wstring>{L"\u0041"});

  // ÷ 000B ÷ 0308 ÷ 0041 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 000B ÷ 003A ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u003A", std::deque<wstring>());

  // ÷ 000B ÷ 0308 ÷ 003A ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u003A", std::deque<wstring>());

  // ÷ 000B ÷ 002C ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u002C", std::deque<wstring>());

  // ÷ 000B ÷ 0308 ÷ 002C ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u002C", std::deque<wstring>());

  // ÷ 000B ÷ 002E ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u002E", std::deque<wstring>());

  // ÷ 000B ÷ 0308 ÷ 002E ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u002E", std::deque<wstring>());

  // ÷ 000B ÷ 0030 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0030", std::deque<wstring>{L"\u0030"});

  // ÷ 000B ÷ 0308 ÷ 0030 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 000B ÷ 005F ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u005F", std::deque<wstring>());

  // ÷ 000B ÷ 0308 ÷ 005F ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u005F", std::deque<wstring>());

  // ÷ 000B ÷ 1F1E6 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] REGIONAL
  // INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 000B ÷ 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\uD83C\uDDE6",
                   std::deque<wstring>());

  // ÷ 000B ÷ 05D0 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] HEBREW
  // LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u05D0", std::deque<wstring>{L"\u05D0"});

  // ÷ 000B ÷ 0308 ÷ 05D0 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 000B ÷ 0022 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] QUOTATION
  // MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\"", std::deque<wstring>());

  // ÷ 000B ÷ 0308 ÷ 0022 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\"", std::deque<wstring>());

  // ÷ 000B ÷ 0027 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0027", std::deque<wstring>());

  // ÷ 000B ÷ 0308 ÷ 0027 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0027", std::deque<wstring>());

  // ÷ 000B ÷ 00AD ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u00AD", std::deque<wstring>());

  // ÷ 000B ÷ 0308 × 00AD ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u00AD", std::deque<wstring>());

  // ÷ 000B ÷ 0300 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1] COMBINING
  // GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0300", std::deque<wstring>());

  // ÷ 000B ÷ 0308 × 0300 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0300", std::deque<wstring>());

  // ÷ 000B ÷ 0061 × 2060 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 000B ÷ 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 000B ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000B ÷ 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000B ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000B ÷ 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000B ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷
  // [3.1] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000B ÷ 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] <LINE TABULATION>
  // (Newline) ÷ [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000B ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000B ÷ 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 000B ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000B ÷ 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000B ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000B ÷ 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000B ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷ [3.1]
  // DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000B ÷ 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷
  // [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000B ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] <LINE TABULATION> (Newline) ÷
  // [3.1] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 000B ÷ 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] <LINE TABULATION>
  // (Newline) ÷ [3.1] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u000B\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 3031 ÷ 0001 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [999.0]
  // <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0001", std::deque<wstring>{L"\u3031"});

  // ÷ 3031 × 0308 ÷ 0001 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0001",
                   std::deque<wstring>{L"\u3031\u0308"});

  // ÷ 3031 ÷ 000D ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [3.2]
  // <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\r", std::deque<wstring>{L"\u3031"});

  // ÷ 3031 × 0308 ÷ 000D ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\r",
                   std::deque<wstring>{L"\u3031\u0308"});

  // ÷ 3031 ÷ 000A ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [3.2]
  // <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\n", std::deque<wstring>{L"\u3031"});

  // ÷ 3031 × 0308 ÷ 000A ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\n",
                   std::deque<wstring>{L"\u3031\u0308"});

  // ÷ 3031 ÷ 000B ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [3.2]
  // <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u000B", std::deque<wstring>{L"\u3031"});

  // ÷ 3031 × 0308 ÷ 000B ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u000B",
                   std::deque<wstring>{L"\u3031\u0308"});

  // ÷ 3031 × 3031 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) × [13.0]
  // VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u3031",
                   std::deque<wstring>{L"\u3031\u3031"});

  // ÷ 3031 × 0308 × 3031 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [13.0] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u3031",
                   std::deque<wstring>{L"\u3031\u0308\u3031"});

  // ÷ 3031 ÷ 0041 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [999.0]
  // LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0041",
                   std::deque<wstring>{L"\u3031", L"\u0041"});

  // ÷ 3031 × 0308 ÷ 0041 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A
  // (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0041",
                   std::deque<wstring>{L"\u3031\u0308", L"\u0041"});

  // ÷ 3031 ÷ 003A ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u003A", std::deque<wstring>{L"\u3031"});

  // ÷ 3031 × 0308 ÷ 003A ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u003A",
                   std::deque<wstring>{L"\u3031\u0308"});

  // ÷ 3031 ÷ 002C ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u002C", std::deque<wstring>{L"\u3031"});

  // ÷ 3031 × 0308 ÷ 002C ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u002C",
                   std::deque<wstring>{L"\u3031\u0308"});

  // ÷ 3031 ÷ 002E ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [999.0]
  // FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u002E", std::deque<wstring>{L"\u3031"});

  // ÷ 3031 × 0308 ÷ 002E ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u002E",
                   std::deque<wstring>{L"\u3031\u0308"});

  // ÷ 3031 ÷ 0030 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [999.0]
  // DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0030",
                   std::deque<wstring>{L"\u3031", L"\u0030"});

  // ÷ 3031 × 0308 ÷ 0030 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO (Numeric) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0030",
                   std::deque<wstring>{L"\u3031\u0308", L"\u0030"});

  // ÷ 3031 × 005F ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) × [13.1]
  // LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u005F",
                   std::deque<wstring>{L"\u3031\u005F"});

  // ÷ 3031 × 0308 × 005F ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [13.1] LOW LINE (ExtendNumLet) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u005F",
                   std::deque<wstring>{L"\u3031\u0308\u005F"});

  // ÷ 3031 ÷ 1F1E6 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [999.0]
  // REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\uD83C\uDDE6",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 3031 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL
  // LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u3031\u0308"});

  // ÷ 3031 ÷ 05D0 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [999.0]
  // HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u05D0",
                   std::deque<wstring>{L"\u3031", L"\u05D0"});

  // ÷ 3031 × 0308 ÷ 05D0 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u05D0",
                   std::deque<wstring>{L"\u3031\u0308", L"\u05D0"});

  // ÷ 3031 ÷ 0022 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [999.0]
  // QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\"", std::deque<wstring>{L"\u3031"});

  // ÷ 3031 × 0308 ÷ 0022 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK
  // (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\"",
                   std::deque<wstring>{L"\u3031\u0308"});

  // ÷ 3031 ÷ 0027 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0027", std::deque<wstring>{L"\u3031"});

  // ÷ 3031 × 0308 ÷ 0027 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0027",
                   std::deque<wstring>{L"\u3031\u0308"});

  // ÷ 3031 × 00AD ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) × [4.0]
  // SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u00AD",
                   std::deque<wstring>{L"\u3031\u00AD"});

  // ÷ 3031 × 0308 × 00AD ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u00AD",
                   std::deque<wstring>{L"\u3031\u0308\u00AD"});

  // ÷ 3031 × 0300 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) × [4.0]
  // COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0300",
                   std::deque<wstring>{L"\u3031\u0300"});

  // ÷ 3031 × 0308 × 0300 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT
  // (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0300",
                   std::deque<wstring>{L"\u3031\u0308\u0300"});

  // ÷ 3031 ÷ 0061 × 2060 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0061\u2060",
                   std::deque<wstring>{L"\u3031", L"\u0061\u2060"});

  // ÷ 3031 × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK
  // (Katakana) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u3031\u0308", L"\u0061\u2060"});

  // ÷ 3031 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0061\u003A",
                   std::deque<wstring>{L"\u3031", L"\u0061"});

  // ÷ 3031 × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK
  // (Katakana) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u3031\u0308", L"\u0061"});

  // ÷ 3031 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0061\u0027",
                   std::deque<wstring>{L"\u3031", L"\u0061"});

  // ÷ 3031 × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK
  // (Katakana) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u3031\u0308", L"\u0061"});

  // ÷ 3031 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u3031", L"\u0061"});

  // ÷ 3031 × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK
  // (Katakana) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u3031\u0308", L"\u0061"});

  // ÷ 3031 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0061\u002C",
                   std::deque<wstring>{L"\u3031", L"\u0061"});

  // ÷ 3031 × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK
  // (Katakana) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u3031\u0308", L"\u0061"});

  // ÷ 3031 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [999.0] DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0031\u003A",
                   std::deque<wstring>{L"\u3031", L"\u0031"});

  // ÷ 3031 × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK
  // (Katakana) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u3031\u0308", L"\u0031"});

  // ÷ 3031 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [999.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0031\u0027",
                   std::deque<wstring>{L"\u3031", L"\u0031"});

  // ÷ 3031 × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK
  // (Katakana) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u3031\u0308", L"\u0031"});

  // ÷ 3031 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [999.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0031\u002C",
                   std::deque<wstring>{L"\u3031", L"\u0031"});

  // ÷ 3031 × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK
  // (Katakana) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u3031\u0308", L"\u0031"});

  // ÷ 3031 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u3031", L"\u0031"});

  // ÷ 3031 × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] VERTICAL KANA REPEAT MARK
  // (Katakana) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u3031\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u3031\u0308", L"\u0031"});

  // ÷ 0041 ÷ 0001 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) ÷ [999.0]
  // <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0001", std::deque<wstring>{L"\u0041"});

  // ÷ 0041 × 0308 ÷ 0001 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0001",
                   std::deque<wstring>{L"\u0041\u0308"});

  // ÷ 0041 ÷ 000D ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) ÷ [3.2]
  // <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\r", std::deque<wstring>{L"\u0041"});

  // ÷ 0041 × 0308 ÷ 000D ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\r",
                   std::deque<wstring>{L"\u0041\u0308"});

  // ÷ 0041 ÷ 000A ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) ÷ [3.2] <LINE
  // FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\n", std::deque<wstring>{L"\u0041"});

  // ÷ 0041 × 0308 ÷ 000A ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\n",
                   std::deque<wstring>{L"\u0041\u0308"});

  // ÷ 0041 ÷ 000B ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) ÷ [3.2] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u000B", std::deque<wstring>{L"\u0041"});

  // ÷ 0041 × 0308 ÷ 000B ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u000B",
                   std::deque<wstring>{L"\u0041\u0308"});

  // ÷ 0041 ÷ 3031 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) ÷ [999.0]
  // VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u3031",
                   std::deque<wstring>{L"\u0041", L"\u3031"});

  // ÷ 0041 × 0308 ÷ 3031 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u3031",
                   std::deque<wstring>{L"\u0041\u0308", L"\u3031"});

  // ÷ 0041 × 0041 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [5.0] LATIN
  // CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0041",
                   std::deque<wstring>{L"\u0041\u0041"});

  // ÷ 0041 × 0308 × 0041 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [5.0] LATIN CAPITAL LETTER A (ALetter) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0041",
                   std::deque<wstring>{L"\u0041\u0308\u0041"});

  // ÷ 0041 ÷ 003A ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u003A", std::deque<wstring>{L"\u0041"});

  // ÷ 0041 × 0308 ÷ 003A ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u003A",
                   std::deque<wstring>{L"\u0041\u0308"});

  // ÷ 0041 ÷ 002C ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u002C", std::deque<wstring>{L"\u0041"});

  // ÷ 0041 × 0308 ÷ 002C ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u002C",
                   std::deque<wstring>{L"\u0041\u0308"});

  // ÷ 0041 ÷ 002E ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) ÷ [999.0] FULL
  // STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u002E", std::deque<wstring>{L"\u0041"});

  // ÷ 0041 × 0308 ÷ 002E ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u002E",
                   std::deque<wstring>{L"\u0041\u0308"});

  // ÷ 0041 × 0030 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [9.0] DIGIT
  // ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0030",
                   std::deque<wstring>{L"\u0041\u0030"});

  // ÷ 0041 × 0308 × 0030 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [9.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0030",
                   std::deque<wstring>{L"\u0041\u0308\u0030"});

  // ÷ 0041 × 005F ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [13.1] LOW
  // LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u005F",
                   std::deque<wstring>{L"\u0041\u005F"});

  // ÷ 0041 × 0308 × 005F ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.1] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u005F",
                   std::deque<wstring>{L"\u0041\u0308\u005F"});

  // ÷ 0041 ÷ 1F1E6 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) ÷ [999.0]
  // REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 0041 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL
  // LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0041\u0308"});

  // ÷ 0041 × 05D0 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [5.0] HEBREW
  // LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u05D0",
                   std::deque<wstring>{L"\u0041\u05D0"});

  // ÷ 0041 × 0308 × 05D0 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [5.0] HEBREW LETTER ALEF (Hebrew_Letter)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u05D0",
                   std::deque<wstring>{L"\u0041\u0308\u05D0"});

  // ÷ 0041 ÷ 0022 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) ÷ [999.0]
  // QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\"", std::deque<wstring>{L"\u0041"});

  // ÷ 0041 × 0308 ÷ 0022 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\"",
                   std::deque<wstring>{L"\u0041\u0308"});

  // ÷ 0041 ÷ 0027 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0027", std::deque<wstring>{L"\u0041"});

  // ÷ 0041 × 0308 ÷ 0027 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0027",
                   std::deque<wstring>{L"\u0041\u0308"});

  // ÷ 0041 × 00AD ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0] SOFT
  // HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u00AD",
                   std::deque<wstring>{L"\u0041\u00AD"});

  // ÷ 0041 × 0308 × 00AD ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u00AD",
                   std::deque<wstring>{L"\u0041\u0308\u00AD"});

  // ÷ 0041 × 0300 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0300",
                   std::deque<wstring>{L"\u0041\u0300"});

  // ÷ 0041 × 0308 × 0300 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0300",
                   std::deque<wstring>{L"\u0041\u0308\u0300"});

  // ÷ 0041 × 0061 × 2060 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [5.0]
  // LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0061\u2060",
                   std::deque<wstring>{L"\u0041\u0061\u2060"});

  // ÷ 0041 × 0308 × 0061 × 2060 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) × [5.0] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0041\u0308\u0061\u2060"});

  // ÷ 0041 × 0061 ÷ 003A ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [5.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0061\u003A",
                   std::deque<wstring>{L"\u0041\u0061"});

  // ÷ 0041 × 0308 × 0061 ÷ 003A ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) × [5.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0041\u0308\u0061"});

  // ÷ 0041 × 0061 ÷ 0027 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [5.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0061\u0027",
                   std::deque<wstring>{L"\u0041\u0061"});

  // ÷ 0041 × 0308 × 0061 ÷ 0027 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) × [5.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0041\u0308\u0061"});

  // ÷ 0041 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter)
  // × [5.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote)
  // × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0041\u0061"});

  // ÷ 0041 × 0308 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A
  // (ALetter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [5.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0041\u0308\u0061"});

  // ÷ 0041 × 0061 ÷ 002C ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [5.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0061\u002C",
                   std::deque<wstring>{L"\u0041\u0061"});

  // ÷ 0041 × 0308 × 0061 ÷ 002C ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) × [5.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0041\u0308\u0061"});

  // ÷ 0041 × 0031 ÷ 003A ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [9.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0031\u003A",
                   std::deque<wstring>{L"\u0041\u0031"});

  // ÷ 0041 × 0308 × 0031 ÷ 003A ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) × [9.0] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0041\u0308\u0031"});

  // ÷ 0041 × 0031 ÷ 0027 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [9.0]
  // DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0031\u0027",
                   std::deque<wstring>{L"\u0041\u0031"});

  // ÷ 0041 × 0308 × 0031 ÷ 0027 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) × [9.0] DIGIT ONE (Numeric) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0041\u0308\u0031"});

  // ÷ 0041 × 0031 ÷ 002C ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter) × [9.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0031\u002C",
                   std::deque<wstring>{L"\u0041\u0031"});

  // ÷ 0041 × 0308 × 0031 ÷ 002C ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) × [9.0] DIGIT ONE (Numeric) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0041\u0308\u0031"});

  // ÷ 0041 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A (ALetter)
  // × [9.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0041\u0031"});

  // ÷ 0041 × 0308 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LATIN CAPITAL LETTER A
  // (ALetter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [9.0] DIGIT ONE
  // (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0041\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0041\u0308\u0031"});

  // ÷ 003A ÷ 0001 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] <START OF HEADING>
  // (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0001", std::deque<wstring>());

  // ÷ 003A × 0308 ÷ 0001 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0001", std::deque<wstring>());

  // ÷ 003A ÷ 000D ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [3.2] <CARRIAGE RETURN
  // (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\r", std::deque<wstring>());

  // ÷ 003A × 0308 ÷ 000D ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\r", std::deque<wstring>());

  // ÷ 003A ÷ 000A ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [3.2] <LINE FEED (LF)> (LF)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\n", std::deque<wstring>());

  // ÷ 003A × 0308 ÷ 000A ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\n", std::deque<wstring>());

  // ÷ 003A ÷ 000B ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [3.2] <LINE TABULATION>
  // (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u000B", std::deque<wstring>());

  // ÷ 003A × 0308 ÷ 000B ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u000B", std::deque<wstring>());

  // ÷ 003A ÷ 3031 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] VERTICAL KANA
  // REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u3031", std::deque<wstring>{L"\u3031"});

  // ÷ 003A × 0308 ÷ 3031 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 003A ÷ 0041 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] LATIN CAPITAL
  // LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0041", std::deque<wstring>{L"\u0041"});

  // ÷ 003A × 0308 ÷ 0041 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 003A ÷ 003A ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] COLON (MidLetter) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u003A", std::deque<wstring>());

  // ÷ 003A × 0308 ÷ 003A ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u003A", std::deque<wstring>());

  // ÷ 003A ÷ 002C ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] COMMA (MidNum) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u002C", std::deque<wstring>());

  // ÷ 003A × 0308 ÷ 002C ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u002C", std::deque<wstring>());

  // ÷ 003A ÷ 002E ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u002E", std::deque<wstring>());

  // ÷ 003A × 0308 ÷ 002E ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u002E", std::deque<wstring>());

  // ÷ 003A ÷ 0030 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0030", std::deque<wstring>{L"\u0030"});

  // ÷ 003A × 0308 ÷ 0030 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 003A ÷ 005F ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u005F", std::deque<wstring>());

  // ÷ 003A × 0308 ÷ 005F ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u005F", std::deque<wstring>());

  // ÷ 003A ÷ 1F1E6 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] REGIONAL INDICATOR
  // SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 003A × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\uD83C\uDDE6",
                   std::deque<wstring>());

  // ÷ 003A ÷ 05D0 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u05D0", std::deque<wstring>{L"\u05D0"});

  // ÷ 003A × 0308 ÷ 05D0 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 003A ÷ 0022 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] QUOTATION MARK
  // (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\"", std::deque<wstring>());

  // ÷ 003A × 0308 ÷ 0022 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\"", std::deque<wstring>());

  // ÷ 003A ÷ 0027 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0027", std::deque<wstring>());

  // ÷ 003A × 0308 ÷ 0027 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0027", std::deque<wstring>());

  // ÷ 003A × 00AD ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u00AD", std::deque<wstring>());

  // ÷ 003A × 0308 × 00AD ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u00AD", std::deque<wstring>());

  // ÷ 003A × 0300 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING GRAVE
  // ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0300", std::deque<wstring>());

  // ÷ 003A × 0308 × 0300 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0300", std::deque<wstring>());

  // ÷ 003A ÷ 0061 × 2060 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 003A × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 003A ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 003A × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 003A ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 003A × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 003A ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 003A × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 003A ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 003A × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 003A ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 003A × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 003A ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 003A × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 003A ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 003A × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 003A ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] COLON (MidLetter) ÷ [999.0] DIGIT
  // ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 003A × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] COLON (MidLetter) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u003A\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002C ÷ 0001 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] <START OF HEADING>
  // (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0001", std::deque<wstring>());

  // ÷ 002C × 0308 ÷ 0001 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0001", std::deque<wstring>());

  // ÷ 002C ÷ 000D ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [3.2] <CARRIAGE RETURN (CR)>
  // (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\r", std::deque<wstring>());

  // ÷ 002C × 0308 ÷ 000D ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\r", std::deque<wstring>());

  // ÷ 002C ÷ 000A ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [3.2] <LINE FEED (LF)> (LF) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\n", std::deque<wstring>());

  // ÷ 002C × 0308 ÷ 000A ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\n", std::deque<wstring>());

  // ÷ 002C ÷ 000B ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [3.2] <LINE TABULATION>
  // (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u000B", std::deque<wstring>());

  // ÷ 002C × 0308 ÷ 000B ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u000B", std::deque<wstring>());

  // ÷ 002C ÷ 3031 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] VERTICAL KANA REPEAT
  // MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u3031", std::deque<wstring>{L"\u3031"});

  // ÷ 002C × 0308 ÷ 3031 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 002C ÷ 0041 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] LATIN CAPITAL LETTER A
  // (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0041", std::deque<wstring>{L"\u0041"});

  // ÷ 002C × 0308 ÷ 0041 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 002C ÷ 003A ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] COLON (MidLetter) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u003A", std::deque<wstring>());

  // ÷ 002C × 0308 ÷ 003A ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u003A", std::deque<wstring>());

  // ÷ 002C ÷ 002C ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u002C", std::deque<wstring>());

  // ÷ 002C × 0308 ÷ 002C ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u002C", std::deque<wstring>());

  // ÷ 002C ÷ 002E ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] FULL STOP (MidNumLet)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u002E", std::deque<wstring>());

  // ÷ 002C × 0308 ÷ 002E ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u002E", std::deque<wstring>());

  // ÷ 002C ÷ 0030 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] DIGIT ZERO (Numeric) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0030", std::deque<wstring>{L"\u0030"});

  // ÷ 002C × 0308 ÷ 0030 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 002C ÷ 005F ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u005F", std::deque<wstring>());

  // ÷ 002C × 0308 ÷ 005F ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u005F", std::deque<wstring>());

  // ÷ 002C ÷ 1F1E6 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] REGIONAL INDICATOR
  // SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 002C × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\uD83C\uDDE6",
                   std::deque<wstring>());

  // ÷ 002C ÷ 05D0 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u05D0", std::deque<wstring>{L"\u05D0"});

  // ÷ 002C × 0308 ÷ 05D0 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 002C ÷ 0022 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] QUOTATION MARK
  // (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\"", std::deque<wstring>());

  // ÷ 002C × 0308 ÷ 0022 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\"", std::deque<wstring>());

  // ÷ 002C ÷ 0027 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0027", std::deque<wstring>());

  // ÷ 002C × 0308 ÷ 0027 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0027", std::deque<wstring>());

  // ÷ 002C × 00AD ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] SOFT HYPHEN (Format_FE)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u00AD", std::deque<wstring>());

  // ÷ 002C × 0308 × 00AD ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u00AD", std::deque<wstring>());

  // ÷ 002C × 0300 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING GRAVE ACCENT
  // (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0300", std::deque<wstring>());

  // ÷ 002C × 0308 × 0300 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0300", std::deque<wstring>());

  // ÷ 002C ÷ 0061 × 2060 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 002C × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 002C ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002C × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002C ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002C × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002C ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002C × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002C ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002C × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002C ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002C × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002C ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002C × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002C ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002C × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002C ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] COMMA (MidNum) ÷ [999.0] DIGIT
  // ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002C × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] COMMA (MidNum) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002C\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002E ÷ 0001 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] <START OF
  // HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0001", std::deque<wstring>());

  // ÷ 002E × 0308 ÷ 0001 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0001", std::deque<wstring>());

  // ÷ 002E ÷ 000D ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [3.2] <CARRIAGE RETURN
  // (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\r", std::deque<wstring>());

  // ÷ 002E × 0308 ÷ 000D ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\r", std::deque<wstring>());

  // ÷ 002E ÷ 000A ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [3.2] <LINE FEED (LF)>
  // (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\n", std::deque<wstring>());

  // ÷ 002E × 0308 ÷ 000A ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\n", std::deque<wstring>());

  // ÷ 002E ÷ 000B ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [3.2] <LINE TABULATION>
  // (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u000B", std::deque<wstring>());

  // ÷ 002E × 0308 ÷ 000B ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u000B", std::deque<wstring>());

  // ÷ 002E ÷ 3031 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] VERTICAL KANA
  // REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u3031", std::deque<wstring>{L"\u3031"});

  // ÷ 002E × 0308 ÷ 3031 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 002E ÷ 0041 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] LATIN CAPITAL
  // LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0041", std::deque<wstring>{L"\u0041"});

  // ÷ 002E × 0308 ÷ 0041 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 002E ÷ 003A ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u003A", std::deque<wstring>());

  // ÷ 002E × 0308 ÷ 003A ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u003A", std::deque<wstring>());

  // ÷ 002E ÷ 002C ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] COMMA (MidNum)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u002C", std::deque<wstring>());

  // ÷ 002E × 0308 ÷ 002C ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u002C", std::deque<wstring>());

  // ÷ 002E ÷ 002E ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u002E", std::deque<wstring>());

  // ÷ 002E × 0308 ÷ 002E ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u002E", std::deque<wstring>());

  // ÷ 002E ÷ 0030 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0030", std::deque<wstring>{L"\u0030"});

  // ÷ 002E × 0308 ÷ 0030 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 002E ÷ 005F ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u005F", std::deque<wstring>());

  // ÷ 002E × 0308 ÷ 005F ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u005F", std::deque<wstring>());

  // ÷ 002E ÷ 1F1E6 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] REGIONAL
  // INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 002E × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\uD83C\uDDE6",
                   std::deque<wstring>());

  // ÷ 002E ÷ 05D0 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] HEBREW LETTER
  // ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u05D0", std::deque<wstring>{L"\u05D0"});

  // ÷ 002E × 0308 ÷ 05D0 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 002E ÷ 0022 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] QUOTATION MARK
  // (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\"", std::deque<wstring>());

  // ÷ 002E × 0308 ÷ 0022 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\"", std::deque<wstring>());

  // ÷ 002E ÷ 0027 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0027", std::deque<wstring>());

  // ÷ 002E × 0308 ÷ 0027 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0027", std::deque<wstring>());

  // ÷ 002E × 00AD ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u00AD", std::deque<wstring>());

  // ÷ 002E × 0308 × 00AD ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u00AD", std::deque<wstring>());

  // ÷ 002E × 0300 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING GRAVE
  // ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0300", std::deque<wstring>());

  // ÷ 002E × 0308 × 0300 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0300", std::deque<wstring>());

  // ÷ 002E ÷ 0061 × 2060 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] LATIN
  // SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 002E × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 002E ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002E × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002E ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002E × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002E ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0]
  // WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002E × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002E ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002E × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 002E ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] DIGIT
  // ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002E × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002E ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] DIGIT
  // ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002E × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002E ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0] DIGIT
  // ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002E × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] FULL STOP (MidNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002E ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 002E × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] FULL STOP (MidNumLet) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u002E\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0030 ÷ 0001 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ÷ [999.0] <START OF
  // HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0001", std::deque<wstring>{L"\u0030"});

  // ÷ 0030 × 0308 ÷ 0001 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0001",
                   std::deque<wstring>{L"\u0030\u0308"});

  // ÷ 0030 ÷ 000D ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ÷ [3.2] <CARRIAGE RETURN
  // (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\r", std::deque<wstring>{L"\u0030"});

  // ÷ 0030 × 0308 ÷ 000D ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\r",
                   std::deque<wstring>{L"\u0030\u0308"});

  // ÷ 0030 ÷ 000A ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ÷ [3.2] <LINE FEED (LF)>
  // (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\n", std::deque<wstring>{L"\u0030"});

  // ÷ 0030 × 0308 ÷ 000A ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\n",
                   std::deque<wstring>{L"\u0030\u0308"});

  // ÷ 0030 ÷ 000B ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ÷ [3.2] <LINE TABULATION>
  // (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u000B", std::deque<wstring>{L"\u0030"});

  // ÷ 0030 × 0308 ÷ 000B ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u000B",
                   std::deque<wstring>{L"\u0030\u0308"});

  // ÷ 0030 ÷ 3031 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ÷ [999.0] VERTICAL KANA
  // REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u3031",
                   std::deque<wstring>{L"\u0030", L"\u3031"});

  // ÷ 0030 × 0308 ÷ 3031 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u3031",
                   std::deque<wstring>{L"\u0030\u0308", L"\u3031"});

  // ÷ 0030 × 0041 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [10.0] LATIN CAPITAL
  // LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0041",
                   std::deque<wstring>{L"\u0030\u0041"});

  // ÷ 0030 × 0308 × 0041 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) × [10.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0041",
                   std::deque<wstring>{L"\u0030\u0308\u0041"});

  // ÷ 0030 ÷ 003A ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u003A", std::deque<wstring>{L"\u0030"});

  // ÷ 0030 × 0308 ÷ 003A ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u003A",
                   std::deque<wstring>{L"\u0030\u0308"});

  // ÷ 0030 ÷ 002C ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ÷ [999.0] COMMA (MidNum) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u002C", std::deque<wstring>{L"\u0030"});

  // ÷ 0030 × 0308 ÷ 002C ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u002C",
                   std::deque<wstring>{L"\u0030\u0308"});

  // ÷ 0030 ÷ 002E ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ÷ [999.0] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u002E", std::deque<wstring>{L"\u0030"});

  // ÷ 0030 × 0308 ÷ 002E ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u002E",
                   std::deque<wstring>{L"\u0030\u0308"});

  // ÷ 0030 × 0030 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [8.0] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0030",
                   std::deque<wstring>{L"\u0030\u0030"});

  // ÷ 0030 × 0308 × 0030 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) × [8.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0030",
                   std::deque<wstring>{L"\u0030\u0308\u0030"});

  // ÷ 0030 × 005F ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [13.1] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u005F",
                   std::deque<wstring>{L"\u0030\u005F"});

  // ÷ 0030 × 0308 × 005F ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) × [13.1] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u005F",
                   std::deque<wstring>{L"\u0030\u0308\u005F"});

  // ÷ 0030 ÷ 1F1E6 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ÷ [999.0] REGIONAL
  // INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 0030 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0030\u0308"});

  // ÷ 0030 × 05D0 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [10.0] HEBREW LETTER
  // ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u05D0",
                   std::deque<wstring>{L"\u0030\u05D0"});

  // ÷ 0030 × 0308 × 05D0 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) × [10.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u05D0",
                   std::deque<wstring>{L"\u0030\u0308\u05D0"});

  // ÷ 0030 ÷ 0022 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ÷ [999.0] QUOTATION MARK
  // (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\"", std::deque<wstring>{L"\u0030"});

  // ÷ 0030 × 0308 ÷ 0022 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\"",
                   std::deque<wstring>{L"\u0030\u0308"});

  // ÷ 0030 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0027", std::deque<wstring>{L"\u0030"});

  // ÷ 0030 × 0308 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0027",
                   std::deque<wstring>{L"\u0030\u0308"});

  // ÷ 0030 × 00AD ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u00AD",
                   std::deque<wstring>{L"\u0030\u00AD"});

  // ÷ 0030 × 0308 × 00AD ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u00AD",
                   std::deque<wstring>{L"\u0030\u0308\u00AD"});

  // ÷ 0030 × 0300 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING GRAVE
  // ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0300",
                   std::deque<wstring>{L"\u0030\u0300"});

  // ÷ 0030 × 0308 × 0300 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0] COMBINING
  // DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0300",
                   std::deque<wstring>{L"\u0030\u0308\u0300"});

  // ÷ 0030 × 0061 × 2060 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [10.0] LATIN
  // SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0061\u2060",
                   std::deque<wstring>{L"\u0030\u0061\u2060"});

  // ÷ 0030 × 0308 × 0061 × 2060 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [10.0] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0030\u0308\u0061\u2060"});

  // ÷ 0030 × 0061 ÷ 003A ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [10.0] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0061\u003A",
                   std::deque<wstring>{L"\u0030\u0061"});

  // ÷ 0030 × 0308 × 0061 ÷ 003A ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [10.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0030\u0308\u0061"});

  // ÷ 0030 × 0061 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [10.0] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0061\u0027",
                   std::deque<wstring>{L"\u0030\u0061"});

  // ÷ 0030 × 0308 × 0061 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [10.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0030\u0308\u0061"});

  // ÷ 0030 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [10.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0]
  // WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0030\u0061"});

  // ÷ 0030 × 0308 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [10.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0030\u0308\u0061"});

  // ÷ 0030 × 0061 ÷ 002C ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [10.0] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0061\u002C",
                   std::deque<wstring>{L"\u0030\u0061"});

  // ÷ 0030 × 0308 × 0061 ÷ 002C ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [10.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0030\u0308\u0061"});

  // ÷ 0030 × 0031 ÷ 003A ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [8.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0031\u003A",
                   std::deque<wstring>{L"\u0030\u0031"});

  // ÷ 0030 × 0308 × 0031 ÷ 003A ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [8.0] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0030\u0308\u0031"});

  // ÷ 0030 × 0031 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [8.0] DIGIT ONE
  // (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0031\u0027",
                   std::deque<wstring>{L"\u0030\u0031"});

  // ÷ 0030 × 0308 × 0031 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [8.0] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0030\u0308\u0031"});

  // ÷ 0030 × 0031 ÷ 002C ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [8.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0031\u002C",
                   std::deque<wstring>{L"\u0030\u0031"});

  // ÷ 0030 × 0308 × 0031 ÷ 002C ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [8.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0030\u0308\u0031"});

  // ÷ 0030 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) × [8.0]
  // DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0030\u0031"});

  // ÷ 0030 × 0308 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] DIGIT ZERO (Numeric) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [8.0] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0030\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0030\u0308\u0031"});

  // ÷ 005F ÷ 0001 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) ÷ [999.0] <START OF
  // HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0001", std::deque<wstring>());

  // ÷ 005F × 0308 ÷ 0001 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0001", std::deque<wstring>());

  // ÷ 005F ÷ 000D ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) ÷ [3.2] <CARRIAGE
  // RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\r", std::deque<wstring>());

  // ÷ 005F × 0308 ÷ 000D ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\r", std::deque<wstring>());

  // ÷ 005F ÷ 000A ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) ÷ [3.2] <LINE FEED
  // (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\n", std::deque<wstring>());

  // ÷ 005F × 0308 ÷ 000A ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\n", std::deque<wstring>());

  // ÷ 005F ÷ 000B ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) ÷ [3.2] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u000B", std::deque<wstring>());

  // ÷ 005F × 0308 ÷ 000B ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u000B", std::deque<wstring>());

  // ÷ 005F × 3031 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2] VERTICAL KANA
  // REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u3031",
                   std::deque<wstring>{L"\u005F\u3031"});

  // ÷ 005F × 0308 × 3031 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.2] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u3031",
                   std::deque<wstring>{L"\u005F\u0308\u3031"});

  // ÷ 005F × 0041 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2] LATIN CAPITAL
  // LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0041",
                   std::deque<wstring>{L"\u005F\u0041"});

  // ÷ 005F × 0308 × 0041 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.2] LATIN CAPITAL LETTER A (ALetter) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0041",
                   std::deque<wstring>{L"\u005F\u0308\u0041"});

  // ÷ 005F ÷ 003A ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u003A", std::deque<wstring>());

  // ÷ 005F × 0308 ÷ 003A ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u003A", std::deque<wstring>());

  // ÷ 005F ÷ 002C ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u002C", std::deque<wstring>());

  // ÷ 005F × 0308 ÷ 002C ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u002C", std::deque<wstring>());

  // ÷ 005F ÷ 002E ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) ÷ [999.0] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u002E", std::deque<wstring>());

  // ÷ 005F × 0308 ÷ 002E ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u002E", std::deque<wstring>());

  // ÷ 005F × 0030 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0030",
                   std::deque<wstring>{L"\u005F\u0030"});

  // ÷ 005F × 0308 × 0030 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.2] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0030",
                   std::deque<wstring>{L"\u005F\u0308\u0030"});

  // ÷ 005F × 005F ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.1] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u005F", std::deque<wstring>());

  // ÷ 005F × 0308 × 005F ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.1] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u005F", std::deque<wstring>());

  // ÷ 005F ÷ 1F1E6 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) ÷ [999.0] REGIONAL
  // INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 005F × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\uD83C\uDDE6",
                   std::deque<wstring>());

  // ÷ 005F × 05D0 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2] HEBREW LETTER
  // ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u05D0",
                   std::deque<wstring>{L"\u005F\u05D0"});

  // ÷ 005F × 0308 × 05D0 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.2] HEBREW LETTER ALEF (Hebrew_Letter)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u05D0",
                   std::deque<wstring>{L"\u005F\u0308\u05D0"});

  // ÷ 005F ÷ 0022 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) ÷ [999.0] QUOTATION
  // MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\"", std::deque<wstring>());

  // ÷ 005F × 0308 ÷ 0022 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\"", std::deque<wstring>());

  // ÷ 005F ÷ 0027 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0027", std::deque<wstring>());

  // ÷ 005F × 0308 ÷ 0027 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0027", std::deque<wstring>());

  // ÷ 005F × 00AD ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u00AD", std::deque<wstring>());

  // ÷ 005F × 0308 × 00AD ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u00AD", std::deque<wstring>());

  // ÷ 005F × 0300 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0] COMBINING GRAVE
  // ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0300", std::deque<wstring>());

  // ÷ 005F × 0308 × 0300 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0300", std::deque<wstring>());

  // ÷ 005F × 0061 × 2060 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2] LATIN
  // SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0061\u2060",
                   std::deque<wstring>{L"\u005F\u0061\u2060"});

  // ÷ 005F × 0308 × 0061 × 2060 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u005F\u0308\u0061\u2060"});

  // ÷ 005F × 0061 ÷ 003A ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0061\u003A",
                   std::deque<wstring>{L"\u005F\u0061"});

  // ÷ 005F × 0308 × 0061 ÷ 003A ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u005F\u0308\u0061"});

  // ÷ 005F × 0061 ÷ 0027 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0061\u0027",
                   std::deque<wstring>{L"\u005F\u0061"});

  // ÷ 005F × 0308 × 0061 ÷ 0027 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u005F\u0308\u0061"});

  // ÷ 005F × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0]
  // WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u005F\u0061"});

  // ÷ 005F × 0308 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [13.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u005F\u0308\u0061"});

  // ÷ 005F × 0061 ÷ 002C ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0061\u002C",
                   std::deque<wstring>{L"\u005F\u0061"});

  // ÷ 005F × 0308 × 0061 ÷ 002C ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u005F\u0308\u0061"});

  // ÷ 005F × 0031 ÷ 003A ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2] DIGIT
  // ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0031\u003A",
                   std::deque<wstring>{L"\u005F\u0031"});

  // ÷ 005F × 0308 × 0031 ÷ 003A ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u005F\u0308\u0031"});

  // ÷ 005F × 0031 ÷ 0027 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2] DIGIT
  // ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0031\u0027",
                   std::deque<wstring>{L"\u005F\u0031"});

  // ÷ 005F × 0308 × 0031 ÷ 0027 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u005F\u0308\u0031"});

  // ÷ 005F × 0031 ÷ 002C ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2] DIGIT
  // ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0031\u002C",
                   std::deque<wstring>{L"\u005F\u0031"});

  // ÷ 005F × 0308 × 0031 ÷ 002C ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [13.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u005F\u0308\u0031"});

  // ÷ 005F × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) × [13.2]
  // DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u005F\u0031"});

  // ÷ 005F × 0308 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LOW LINE (ExtendNumLet) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [13.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u005F\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u005F\u0308\u0031"});

  // ÷ 1F1E6 ÷ 0001 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0001", std::deque<wstring>());

  // ÷ 1F1E6 × 0308 ÷ 0001 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0001",
                   std::deque<wstring>());

  // ÷ 1F1E6 ÷ 000D ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\r", std::deque<wstring>());

  // ÷ 1F1E6 × 0308 ÷ 000D ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2]
  // <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\r", std::deque<wstring>());

  // ÷ 1F1E6 ÷ 000A ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\n", std::deque<wstring>());

  // ÷ 1F1E6 × 0308 ÷ 000A ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE
  // FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\n", std::deque<wstring>());

  // ÷ 1F1E6 ÷ 000B ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u000B", std::deque<wstring>());

  // ÷ 1F1E6 × 0308 ÷ 000B ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u000B",
                   std::deque<wstring>());

  // ÷ 1F1E6 ÷ 3031 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 1F1E6 × 0308 ÷ 3031 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 1F1E6 ÷ 0041 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 1F1E6 × 0308 ÷ 0041 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 1F1E6 ÷ 003A ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u003A", std::deque<wstring>());

  // ÷ 1F1E6 × 0308 ÷ 003A ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u003A",
                   std::deque<wstring>());

  // ÷ 1F1E6 ÷ 002C ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u002C", std::deque<wstring>());

  // ÷ 1F1E6 × 0308 ÷ 002C ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u002C",
                   std::deque<wstring>());

  // ÷ 1F1E6 ÷ 002E ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u002E", std::deque<wstring>());

  // ÷ 1F1E6 × 0308 ÷ 002E ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL
  // STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u002E",
                   std::deque<wstring>());

  // ÷ 1F1E6 ÷ 0030 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 1F1E6 × 0308 ÷ 0030 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 1F1E6 ÷ 005F ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u005F", std::deque<wstring>());

  // ÷ 1F1E6 × 0308 ÷ 005F ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LOW
  // LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u005F",
                   std::deque<wstring>());

  // ÷ 1F1E6 × 1F1E6 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [13.3] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\uD83C\uDDE6",
                   std::deque<wstring>());

  // ÷ 1F1E6 × 0308 × 1F1E6 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) × [13.3]
  // REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\uD83C\uDDE6",
                   std::deque<wstring>());

  // ÷ 1F1E6 ÷ 05D0 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 1F1E6 × 0308 ÷ 05D0 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 1F1E6 ÷ 0022 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\"", std::deque<wstring>());

  // ÷ 1F1E6 × 0308 ÷ 0022 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\"", std::deque<wstring>());

  // ÷ 1F1E6 ÷ 0027 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0027", std::deque<wstring>());

  // ÷ 1F1E6 × 0308 ÷ 0027 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0027",
                   std::deque<wstring>());

  // ÷ 1F1E6 × 00AD ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u00AD", std::deque<wstring>());

  // ÷ 1F1E6 × 0308 × 00AD ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT
  // HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u00AD",
                   std::deque<wstring>());

  // ÷ 1F1E6 × 0300 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0300", std::deque<wstring>());

  // ÷ 1F1E6 × 0308 × 0300 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0]
  // COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0300",
                   std::deque<wstring>());

  // ÷ 1F1E6 ÷ 0061 × 2060 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 1F1E6 × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 1F1E6 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 1F1E6 × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 1F1E6 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 1F1E6 × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 1F1E6 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 1F1E6 × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL
  // LETTER A (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote)
  // × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 1F1E6 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 1F1E6 × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 1F1E6 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 1F1E6 × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 1F1E6 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 1F1E6 × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 1F1E6 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 1F1E6 × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 1F1E6 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP
  // (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 1F1E6 × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL
  // LETTER A (Regional_Indicator) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 05D0 ÷ 0001 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [999.0]
  // <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0001", std::deque<wstring>{L"\u05D0"});

  // ÷ 05D0 × 0308 ÷ 0001 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0001",
                   std::deque<wstring>{L"\u05D0\u0308"});

  // ÷ 05D0 ÷ 000D ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [3.2]
  // <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\r", std::deque<wstring>{L"\u05D0"});

  // ÷ 05D0 × 0308 ÷ 000D ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\r",
                   std::deque<wstring>{L"\u05D0\u0308"});

  // ÷ 05D0 ÷ 000A ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [3.2]
  // <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\n", std::deque<wstring>{L"\u05D0"});

  // ÷ 05D0 × 0308 ÷ 000A ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\n",
                   std::deque<wstring>{L"\u05D0\u0308"});

  // ÷ 05D0 ÷ 000B ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [3.2]
  // <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u000B", std::deque<wstring>{L"\u05D0"});

  // ÷ 05D0 × 0308 ÷ 000B ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u000B",
                   std::deque<wstring>{L"\u05D0\u0308"});

  // ÷ 05D0 ÷ 3031 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [999.0]
  // VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u3031",
                   std::deque<wstring>{L"\u05D0", L"\u3031"});

  // ÷ 05D0 × 0308 ÷ 3031 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u3031",
                   std::deque<wstring>{L"\u05D0\u0308", L"\u3031"});

  // ÷ 05D0 × 0041 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) × [5.0]
  // LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0041",
                   std::deque<wstring>{L"\u05D0\u0041"});

  // ÷ 05D0 × 0308 × 0041 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [5.0] LATIN CAPITAL LETTER A
  // (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0041",
                   std::deque<wstring>{L"\u05D0\u0308\u0041"});

  // ÷ 05D0 ÷ 003A ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u003A", std::deque<wstring>{L"\u05D0"});

  // ÷ 05D0 × 0308 ÷ 003A ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u003A",
                   std::deque<wstring>{L"\u05D0\u0308"});

  // ÷ 05D0 ÷ 002C ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u002C", std::deque<wstring>{L"\u05D0"});

  // ÷ 05D0 × 0308 ÷ 002C ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u002C",
                   std::deque<wstring>{L"\u05D0\u0308"});

  // ÷ 05D0 ÷ 002E ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [999.0]
  // FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u002E", std::deque<wstring>{L"\u05D0"});

  // ÷ 05D0 × 0308 ÷ 002E ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u002E",
                   std::deque<wstring>{L"\u05D0\u0308"});

  // ÷ 05D0 × 0030 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) × [9.0]
  // DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0030",
                   std::deque<wstring>{L"\u05D0\u0030"});

  // ÷ 05D0 × 0308 × 0030 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [9.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0030",
                   std::deque<wstring>{L"\u05D0\u0308\u0030"});

  // ÷ 05D0 × 005F ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) × [13.1] LOW
  // LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u005F",
                   std::deque<wstring>{L"\u05D0\u005F"});

  // ÷ 05D0 × 0308 × 005F ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [13.1] LOW LINE (ExtendNumLet) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u005F",
                   std::deque<wstring>{L"\u05D0\u0308\u005F"});

  // ÷ 05D0 ÷ 1F1E6 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [999.0]
  // REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\uD83C\uDDE6",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 05D0 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL
  // LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u05D0\u0308"});

  // ÷ 05D0 × 05D0 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) × [5.0]
  // HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u05D0",
                   std::deque<wstring>{L"\u05D0\u05D0"});

  // ÷ 05D0 × 0308 × 05D0 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [5.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0\u0308\u05D0"});

  // ÷ 05D0 ÷ 0022 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [999.0]
  // QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\"", std::deque<wstring>{L"\u05D0"});

  // ÷ 05D0 × 0308 ÷ 0022 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK
  // (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\"",
                   std::deque<wstring>{L"\u05D0\u0308"});

  // ÷ 05D0 × 0027 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) × [7.1]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0027",
                   std::deque<wstring>{L"\u05D0\u0027"});

  // ÷ 05D0 × 0308 × 0027 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [7.1] APOSTROPHE (Single_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0027",
                   std::deque<wstring>{L"\u05D0\u0308\u0027"});

  // ÷ 05D0 × 00AD ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) × [4.0] SOFT
  // HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u00AD",
                   std::deque<wstring>{L"\u05D0\u00AD"});

  // ÷ 05D0 × 0308 × 00AD ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u00AD",
                   std::deque<wstring>{L"\u05D0\u0308\u00AD"});

  // ÷ 05D0 × 0300 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) × [4.0]
  // COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0300",
                   std::deque<wstring>{L"\u05D0\u0300"});

  // ÷ 05D0 × 0308 × 0300 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT
  // (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0300",
                   std::deque<wstring>{L"\u05D0\u0308\u0300"});

  // ÷ 05D0 × 0061 × 2060 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [5.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0061\u2060",
                   std::deque<wstring>{L"\u05D0\u0061\u2060"});

  // ÷ 05D0 × 0308 × 0061 × 2060 ÷  #  ÷ [0.2] HEBREW LETTER ALEF
  // (Hebrew_Letter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [5.0] LATIN SMALL
  // LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u05D0\u0308\u0061\u2060"});

  // ÷ 05D0 × 0061 ÷ 003A ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [5.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0061\u003A",
                   std::deque<wstring>{L"\u05D0\u0061"});

  // ÷ 05D0 × 0308 × 0061 ÷ 003A ÷  #  ÷ [0.2] HEBREW LETTER ALEF
  // (Hebrew_Letter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [5.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u05D0\u0308\u0061"});

  // ÷ 05D0 × 0061 ÷ 0027 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [5.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0061\u0027",
                   std::deque<wstring>{L"\u05D0\u0061"});

  // ÷ 05D0 × 0308 × 0061 ÷ 0027 ÷  #  ÷ [0.2] HEBREW LETTER ALEF
  // (Hebrew_Letter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [5.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u05D0\u0308\u0061"});

  // ÷ 05D0 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] HEBREW LETTER ALEF
  // (Hebrew_Letter) × [5.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u05D0\u0061"});

  // ÷ 05D0 × 0308 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] HEBREW LETTER ALEF
  // (Hebrew_Letter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [5.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u05D0\u0308\u0061"});

  // ÷ 05D0 × 0061 ÷ 002C ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [5.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0061\u002C",
                   std::deque<wstring>{L"\u05D0\u0061"});

  // ÷ 05D0 × 0308 × 0061 ÷ 002C ÷  #  ÷ [0.2] HEBREW LETTER ALEF
  // (Hebrew_Letter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [5.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u05D0\u0308\u0061"});

  // ÷ 05D0 × 0031 ÷ 003A ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [9.0] DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0031\u003A",
                   std::deque<wstring>{L"\u05D0\u0031"});

  // ÷ 05D0 × 0308 × 0031 ÷ 003A ÷  #  ÷ [0.2] HEBREW LETTER ALEF
  // (Hebrew_Letter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [9.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u05D0\u0308\u0031"});

  // ÷ 05D0 × 0031 ÷ 0027 ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [9.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0031\u0027",
                   std::deque<wstring>{L"\u05D0\u0031"});

  // ÷ 05D0 × 0308 × 0031 ÷ 0027 ÷  #  ÷ [0.2] HEBREW LETTER ALEF
  // (Hebrew_Letter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [9.0] DIGIT ONE
  // (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u05D0\u0308\u0031"});

  // ÷ 05D0 × 0031 ÷ 002C ÷  #  ÷ [0.2] HEBREW LETTER ALEF (Hebrew_Letter) ×
  // [9.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0031\u002C",
                   std::deque<wstring>{L"\u05D0\u0031"});

  // ÷ 05D0 × 0308 × 0031 ÷ 002C ÷  #  ÷ [0.2] HEBREW LETTER ALEF
  // (Hebrew_Letter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [9.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u05D0\u0308\u0031"});

  // ÷ 05D0 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] HEBREW LETTER ALEF
  // (Hebrew_Letter) × [9.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet)
  // × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u05D0\u0031"});

  // ÷ 05D0 × 0308 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] HEBREW LETTER ALEF
  // (Hebrew_Letter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [9.0] DIGIT ONE
  // (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u05D0\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u05D0\u0308\u0031"});

  // ÷ 0022 ÷ 0001 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0] <START
  // OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0001", std::deque<wstring>());

  // ÷ 0022 × 0308 ÷ 0001 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0001", std::deque<wstring>());

  // ÷ 0022 ÷ 000D ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [3.2] <CARRIAGE
  // RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\r", std::deque<wstring>());

  // ÷ 0022 × 0308 ÷ 000D ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\r", std::deque<wstring>());

  // ÷ 0022 ÷ 000A ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [3.2] <LINE
  // FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\n", std::deque<wstring>());

  // ÷ 0022 × 0308 ÷ 000A ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\n", std::deque<wstring>());

  // ÷ 0022 ÷ 000B ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [3.2] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u000B", std::deque<wstring>());

  // ÷ 0022 × 0308 ÷ 000B ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u000B", std::deque<wstring>());

  // ÷ 0022 ÷ 3031 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0]
  // VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u3031", std::deque<wstring>{L"\u3031"});

  // ÷ 0022 × 0308 ÷ 3031 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 0022 ÷ 0041 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0] LATIN
  // CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0041", std::deque<wstring>{L"\u0041"});

  // ÷ 0022 × 0308 ÷ 0041 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 0022 ÷ 003A ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u003A", std::deque<wstring>());

  // ÷ 0022 × 0308 ÷ 003A ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u003A", std::deque<wstring>());

  // ÷ 0022 ÷ 002C ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u002C", std::deque<wstring>());

  // ÷ 0022 × 0308 ÷ 002C ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u002C", std::deque<wstring>());

  // ÷ 0022 ÷ 002E ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0] FULL
  // STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u002E", std::deque<wstring>());

  // ÷ 0022 × 0308 ÷ 002E ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u002E", std::deque<wstring>());

  // ÷ 0022 ÷ 0030 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0] DIGIT
  // ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0030", std::deque<wstring>{L"\u0030"});

  // ÷ 0022 × 0308 ÷ 0030 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 0022 ÷ 005F ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0] LOW
  // LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u005F", std::deque<wstring>());

  // ÷ 0022 × 0308 ÷ 005F ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u005F", std::deque<wstring>());

  // ÷ 0022 ÷ 1F1E6 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0]
  // REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 0022 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 0022 ÷ 05D0 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0] HEBREW
  // LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u05D0", std::deque<wstring>{L"\u05D0"});

  // ÷ 0022 × 0308 ÷ 05D0 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 0022 ÷ 0022 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0]
  // QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\"", std::deque<wstring>());

  // ÷ 0022 × 0308 ÷ 0022 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\"", std::deque<wstring>());

  // ÷ 0022 ÷ 0027 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0027", std::deque<wstring>());

  // ÷ 0022 × 0308 ÷ 0027 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0027", std::deque<wstring>());

  // ÷ 0022 × 00AD ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0] SOFT
  // HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u00AD", std::deque<wstring>());

  // ÷ 0022 × 0308 × 00AD ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u00AD", std::deque<wstring>());

  // ÷ 0022 × 0300 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0] COMBINING
  // GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0300", std::deque<wstring>());

  // ÷ 0022 × 0308 × 0300 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0300", std::deque<wstring>());

  // ÷ 0022 ÷ 0061 × 2060 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0022 × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0022 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0022 × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0022 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0022 × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0022 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote)
  // × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0022 × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] QUOTATION MARK
  // (Double_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0022 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0022 × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0022 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0022 × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0022 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0022 × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0022 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0022 × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0022 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] QUOTATION MARK (Double_Quote) ÷
  // [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0022 × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] QUOTATION MARK
  // (Double_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\"\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0027 ÷ 0001 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0] <START OF
  // HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0001", std::deque<wstring>());

  // ÷ 0027 × 0308 ÷ 0001 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0001", std::deque<wstring>());

  // ÷ 0027 ÷ 000D ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [3.2] <CARRIAGE
  // RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\r", std::deque<wstring>());

  // ÷ 0027 × 0308 ÷ 000D ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\r", std::deque<wstring>());

  // ÷ 0027 ÷ 000A ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [3.2] <LINE FEED
  // (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\n", std::deque<wstring>());

  // ÷ 0027 × 0308 ÷ 000A ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\n", std::deque<wstring>());

  // ÷ 0027 ÷ 000B ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [3.2] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u000B", std::deque<wstring>());

  // ÷ 0027 × 0308 ÷ 000B ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u000B", std::deque<wstring>());

  // ÷ 0027 ÷ 3031 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0] VERTICAL
  // KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u3031", std::deque<wstring>{L"\u3031"});

  // ÷ 0027 × 0308 ÷ 3031 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 0027 ÷ 0041 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0] LATIN
  // CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0041", std::deque<wstring>{L"\u0041"});

  // ÷ 0027 × 0308 ÷ 0041 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 0027 ÷ 003A ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u003A", std::deque<wstring>());

  // ÷ 0027 × 0308 ÷ 003A ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u003A", std::deque<wstring>());

  // ÷ 0027 ÷ 002C ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u002C", std::deque<wstring>());

  // ÷ 0027 × 0308 ÷ 002C ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u002C", std::deque<wstring>());

  // ÷ 0027 ÷ 002E ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u002E", std::deque<wstring>());

  // ÷ 0027 × 0308 ÷ 002E ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u002E", std::deque<wstring>());

  // ÷ 0027 ÷ 0030 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0030", std::deque<wstring>{L"\u0030"});

  // ÷ 0027 × 0308 ÷ 0030 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 0027 ÷ 005F ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u005F", std::deque<wstring>());

  // ÷ 0027 × 0308 ÷ 005F ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u005F", std::deque<wstring>());

  // ÷ 0027 ÷ 1F1E6 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0] REGIONAL
  // INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 0027 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\uD83C\uDDE6",
                   std::deque<wstring>());

  // ÷ 0027 ÷ 05D0 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0] HEBREW
  // LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u05D0", std::deque<wstring>{L"\u05D0"});

  // ÷ 0027 × 0308 ÷ 05D0 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 0027 ÷ 0022 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0] QUOTATION
  // MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\"", std::deque<wstring>());

  // ÷ 0027 × 0308 ÷ 0022 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\"", std::deque<wstring>());

  // ÷ 0027 ÷ 0027 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0027", std::deque<wstring>());

  // ÷ 0027 × 0308 ÷ 0027 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0027", std::deque<wstring>());

  // ÷ 0027 × 00AD ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u00AD", std::deque<wstring>());

  // ÷ 0027 × 0308 × 00AD ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u00AD", std::deque<wstring>());

  // ÷ 0027 × 0300 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0] COMBINING
  // GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0300", std::deque<wstring>());

  // ÷ 0027 × 0308 × 0300 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0300", std::deque<wstring>());

  // ÷ 0027 ÷ 0061 × 2060 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0027 × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0027 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0027 × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0027 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0027 × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0027 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote)
  // × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0027 × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0027 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0027 × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0027 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0027 × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0027 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0027 × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0027 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0027 × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0027 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote) ÷
  // [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0027 × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] APOSTROPHE (Single_Quote)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0027\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 00AD ÷ 0001 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] <START OF
  // HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0001", std::deque<wstring>());

  // ÷ 00AD × 0308 ÷ 0001 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0001", std::deque<wstring>());

  // ÷ 00AD ÷ 000D ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [3.2] <CARRIAGE
  // RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\r", std::deque<wstring>());

  // ÷ 00AD × 0308 ÷ 000D ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\r", std::deque<wstring>());

  // ÷ 00AD ÷ 000A ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [3.2] <LINE FEED
  // (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\n", std::deque<wstring>());

  // ÷ 00AD × 0308 ÷ 000A ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\n", std::deque<wstring>());

  // ÷ 00AD ÷ 000B ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [3.2] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u000B", std::deque<wstring>());

  // ÷ 00AD × 0308 ÷ 000B ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u000B", std::deque<wstring>());

  // ÷ 00AD ÷ 3031 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] VERTICAL KANA
  // REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u3031", std::deque<wstring>{L"\u3031"});

  // ÷ 00AD × 0308 ÷ 3031 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 00AD ÷ 0041 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] LATIN CAPITAL
  // LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0041", std::deque<wstring>{L"\u0041"});

  // ÷ 00AD × 0308 ÷ 0041 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 00AD ÷ 003A ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u003A", std::deque<wstring>());

  // ÷ 00AD × 0308 ÷ 003A ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u003A", std::deque<wstring>());

  // ÷ 00AD ÷ 002C ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u002C", std::deque<wstring>());

  // ÷ 00AD × 0308 ÷ 002C ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u002C", std::deque<wstring>());

  // ÷ 00AD ÷ 002E ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u002E", std::deque<wstring>());

  // ÷ 00AD × 0308 ÷ 002E ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u002E", std::deque<wstring>());

  // ÷ 00AD ÷ 0030 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0030", std::deque<wstring>{L"\u0030"});

  // ÷ 00AD × 0308 ÷ 0030 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 00AD ÷ 005F ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u005F", std::deque<wstring>());

  // ÷ 00AD × 0308 ÷ 005F ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u005F", std::deque<wstring>());

  // ÷ 00AD ÷ 1F1E6 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] REGIONAL
  // INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 00AD × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\uD83C\uDDE6",
                   std::deque<wstring>());

  // ÷ 00AD ÷ 05D0 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] HEBREW LETTER
  // ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u05D0", std::deque<wstring>{L"\u05D0"});

  // ÷ 00AD × 0308 ÷ 05D0 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 00AD ÷ 0022 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] QUOTATION
  // MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\"", std::deque<wstring>());

  // ÷ 00AD × 0308 ÷ 0022 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\"", std::deque<wstring>());

  // ÷ 00AD ÷ 0027 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0027", std::deque<wstring>());

  // ÷ 00AD × 0308 ÷ 0027 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0027", std::deque<wstring>());

  // ÷ 00AD × 00AD ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u00AD", std::deque<wstring>());

  // ÷ 00AD × 0308 × 00AD ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u00AD", std::deque<wstring>());

  // ÷ 00AD × 0300 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0] COMBINING GRAVE
  // ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0300", std::deque<wstring>());

  // ÷ 00AD × 0308 × 0300 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0300", std::deque<wstring>());

  // ÷ 00AD ÷ 0061 × 2060 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] LATIN
  // SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 00AD × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 00AD ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 00AD × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 00AD ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 00AD × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 00AD ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0]
  // WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 00AD × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 00AD ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] LATIN
  // SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 00AD × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 00AD ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] DIGIT
  // ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 00AD × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 00AD ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] DIGIT
  // ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 00AD × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 00AD ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0] DIGIT
  // ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 00AD × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 00AD ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 00AD × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] SOFT HYPHEN (Format_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u00AD\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0300 ÷ 0001 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [999.0]
  // <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0001", std::deque<wstring>());

  // ÷ 0300 × 0308 ÷ 0001 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0001", std::deque<wstring>());

  // ÷ 0300 ÷ 000D ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [3.2]
  // <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\r", std::deque<wstring>());

  // ÷ 0300 × 0308 ÷ 000D ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\r", std::deque<wstring>());

  // ÷ 0300 ÷ 000A ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [3.2]
  // <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\n", std::deque<wstring>());

  // ÷ 0300 × 0308 ÷ 000A ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\n", std::deque<wstring>());

  // ÷ 0300 ÷ 000B ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [3.2]
  // <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u000B", std::deque<wstring>());

  // ÷ 0300 × 0308 ÷ 000B ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u000B", std::deque<wstring>());

  // ÷ 0300 ÷ 3031 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [999.0]
  // VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u3031", std::deque<wstring>{L"\u3031"});

  // ÷ 0300 × 0308 ÷ 3031 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u3031",
                   std::deque<wstring>{L"\u3031"});

  // ÷ 0300 ÷ 0041 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [999.0]
  // LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0041", std::deque<wstring>{L"\u0041"});

  // ÷ 0300 × 0308 ÷ 0041 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A
  // (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0041",
                   std::deque<wstring>{L"\u0041"});

  // ÷ 0300 ÷ 003A ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u003A", std::deque<wstring>());

  // ÷ 0300 × 0308 ÷ 003A ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u003A", std::deque<wstring>());

  // ÷ 0300 ÷ 002C ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u002C", std::deque<wstring>());

  // ÷ 0300 × 0308 ÷ 002C ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u002C", std::deque<wstring>());

  // ÷ 0300 ÷ 002E ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [999.0]
  // FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u002E", std::deque<wstring>());

  // ÷ 0300 × 0308 ÷ 002E ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u002E", std::deque<wstring>());

  // ÷ 0300 ÷ 0030 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [999.0]
  // DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0030", std::deque<wstring>{L"\u0030"});

  // ÷ 0300 × 0308 ÷ 0030 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO (Numeric) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0030",
                   std::deque<wstring>{L"\u0030"});

  // ÷ 0300 ÷ 005F ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [999.0]
  // LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u005F", std::deque<wstring>());

  // ÷ 0300 × 0308 ÷ 005F ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE (ExtendNumLet) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u005F", std::deque<wstring>());

  // ÷ 0300 ÷ 1F1E6 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [999.0]
  // REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\uD83C\uDDE6", std::deque<wstring>());

  // ÷ 0300 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL
  // LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\uD83C\uDDE6",
                   std::deque<wstring>());

  // ÷ 0300 ÷ 05D0 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [999.0]
  // HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u05D0", std::deque<wstring>{L"\u05D0"});

  // ÷ 0300 × 0308 ÷ 05D0 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u05D0",
                   std::deque<wstring>{L"\u05D0"});

  // ÷ 0300 ÷ 0022 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [999.0]
  // QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\"", std::deque<wstring>());

  // ÷ 0300 × 0308 ÷ 0022 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK
  // (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\"", std::deque<wstring>());

  // ÷ 0300 ÷ 0027 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0027", std::deque<wstring>());

  // ÷ 0300 × 0308 ÷ 0027 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0027", std::deque<wstring>());

  // ÷ 0300 × 00AD ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) × [4.0] SOFT
  // HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u00AD", std::deque<wstring>());

  // ÷ 0300 × 0308 × 00AD ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u00AD", std::deque<wstring>());

  // ÷ 0300 × 0300 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) × [4.0]
  // COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0300", std::deque<wstring>());

  // ÷ 0300 × 0308 × 0300 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT
  // (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0300", std::deque<wstring>());

  // ÷ 0300 ÷ 0061 × 2060 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0300 × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT
  // (Extend_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0300 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0300 × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT
  // (Extend_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0300 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0300 × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT
  // (Extend_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0300 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT
  // (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0300 × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT
  // (Extend_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0300 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0300 × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT
  // (Extend_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0300 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷
  // [999.0] DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0300 × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT
  // (Extend_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0300 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷
  // [999.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0300 × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT
  // (Extend_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0300 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT (Extend_FE) ÷
  // [999.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0300 × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT
  // (Extend_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0300 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT
  // (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0300 × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] COMBINING GRAVE ACCENT
  // (Extend_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0300\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0061 × 2060 ÷ 0001 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0001",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0061 × 2060 × 0308 ÷ 0001 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0001",
                   std::deque<wstring>{L"\u0061\u2060\u0308"});

  // ÷ 0061 × 2060 ÷ 000D ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\r",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0061 × 2060 × 0308 ÷ 000D ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\r",
                   std::deque<wstring>{L"\u0061\u2060\u0308"});

  // ÷ 0061 × 2060 ÷ 000A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\n",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0061 × 2060 × 0308 ÷ 000A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\n",
                   std::deque<wstring>{L"\u0061\u2060\u0308"});

  // ÷ 0061 × 2060 ÷ 000B ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u000B",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0061 × 2060 × 0308 ÷ 000B ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u000B",
                   std::deque<wstring>{L"\u0061\u2060\u0308"});

  // ÷ 0061 × 2060 ÷ 3031 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u3031",
                   std::deque<wstring>{L"\u0061\u2060", L"\u3031"});

  // ÷ 0061 × 2060 × 0308 ÷ 3031 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u3031",
                   std::deque<wstring>{L"\u0061\u2060\u0308", L"\u3031"});

  // ÷ 0061 × 2060 × 0041 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) × [5.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0041",
                   std::deque<wstring>{L"\u0061\u2060\u0041"});

  // ÷ 0061 × 2060 × 0308 × 0041 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [5.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0041",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u0041"});

  // ÷ 0061 × 2060 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u003A",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0061 × 2060 × 0308 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u003A",
                   std::deque<wstring>{L"\u0061\u2060\u0308"});

  // ÷ 0061 × 2060 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u002C",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0061 × 2060 × 0308 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u002C",
                   std::deque<wstring>{L"\u0061\u2060\u0308"});

  // ÷ 0061 × 2060 ÷ 002E ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u002E",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0061 × 2060 × 0308 ÷ 002E ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u002E",
                   std::deque<wstring>{L"\u0061\u2060\u0308"});

  // ÷ 0061 × 2060 × 0030 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) × [9.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0030",
                   std::deque<wstring>{L"\u0061\u2060\u0030"});

  // ÷ 0061 × 2060 × 0308 × 0030 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [9.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0030",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u0030"});

  // ÷ 0061 × 2060 × 005F ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) × [13.1] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u005F",
                   std::deque<wstring>{L"\u0061\u2060\u005F"});

  // ÷ 0061 × 2060 × 0308 × 005F ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [13.1] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u005F",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u005F"});

  // ÷ 0061 × 2060 ÷ 1F1E6 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0061 × 2060 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0061\u2060\u0308"});

  // ÷ 0061 × 2060 × 05D0 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) × [5.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u05D0",
                   std::deque<wstring>{L"\u0061\u2060\u05D0"});

  // ÷ 0061 × 2060 × 0308 × 05D0 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [5.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u05D0",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u05D0"});

  // ÷ 0061 × 2060 ÷ 0022 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\"",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0061 × 2060 × 0308 ÷ 0022 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\"",
                   std::deque<wstring>{L"\u0061\u2060\u0308"});

  // ÷ 0061 × 2060 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0027",
                   std::deque<wstring>{L"\u0061\u2060"});

  // ÷ 0061 × 2060 × 0308 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0027",
                   std::deque<wstring>{L"\u0061\u2060\u0308"});

  // ÷ 0061 × 2060 × 00AD ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u00AD",
                   std::deque<wstring>{L"\u0061\u2060\u00AD"});

  // ÷ 0061 × 2060 × 0308 × 00AD ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u00AD",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u00AD"});

  // ÷ 0061 × 2060 × 0300 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0300",
                   std::deque<wstring>{L"\u0061\u2060\u0300"});

  // ÷ 0061 × 2060 × 0308 × 0300 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0300",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u0300"});

  // ÷ 0061 × 2060 × 0061 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [5.0] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060\u0061\u2060"});

  // ÷ 0061 × 2060 × 0308 × 0061 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [5.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u0061\u2060"});

  // ÷ 0061 × 2060 × 0061 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [5.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0061\u003A",
                   std::deque<wstring>{L"\u0061\u2060\u0061"});

  // ÷ 0061 × 2060 × 0308 × 0061 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [5.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u0061"});

  // ÷ 0061 × 2060 × 0061 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [5.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0061\u0027",
                   std::deque<wstring>{L"\u0061\u2060\u0061"});

  // ÷ 0061 × 2060 × 0308 × 0061 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [5.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u0061"});

  // ÷ 0061 × 2060 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [5.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061\u2060\u0061"});

  // ÷ 0061 × 2060 × 0308 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [5.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u0061"});

  // ÷ 0061 × 2060 × 0061 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [5.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0061\u002C",
                   std::deque<wstring>{L"\u0061\u2060\u0061"});

  // ÷ 0061 × 2060 × 0308 × 0061 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [5.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u0061"});

  // ÷ 0061 × 2060 × 0031 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [9.0] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0031\u003A",
                   std::deque<wstring>{L"\u0061\u2060\u0031"});

  // ÷ 0061 × 2060 × 0308 × 0031 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [9.0] DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u0031"});

  // ÷ 0061 × 2060 × 0031 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [9.0] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0031\u0027",
                   std::deque<wstring>{L"\u0061\u2060\u0031"});

  // ÷ 0061 × 2060 × 0308 × 0031 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [9.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u0031"});

  // ÷ 0061 × 2060 × 0031 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) × [9.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0031\u002C",
                   std::deque<wstring>{L"\u0061\u2060\u0031"});

  // ÷ 0061 × 2060 × 0308 × 0031 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [9.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u0031"});

  // ÷ 0061 × 2060 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [9.0] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0061\u2060\u0031"});

  // ÷ 0061 × 2060 × 0308 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [9.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u2060\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0061\u2060\u0308\u0031"});

  // ÷ 0061 ÷ 003A ÷ 0001 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0001",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 0001 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0001",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A ÷ 000D ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\r",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 000D ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2]
  // <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\r",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A ÷ 000A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\n",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 000A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2]
  // <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\n",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A ÷ 000B ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u000B",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 000B ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2]
  // <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u000B",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A ÷ 3031 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u3031",
                   std::deque<wstring>{L"\u0061", L"\u3031"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 3031 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u3031",
                   std::deque<wstring>{L"\u0061", L"\u3031"});

  // ÷ 0061 × 003A × 0041 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [6.0]
  // COLON (MidLetter) × [7.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0041",
                   std::deque<wstring>{L"\u0061\u003A\u0041"});

  // ÷ 0061 × 003A × 0308 × 0041 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [7.0]
  // LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0041",
                   std::deque<wstring>{L"\u0061\u003A\u0308\u0041"});

  // ÷ 0061 ÷ 003A ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A ÷ 002E ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u002E",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 002E ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u002E",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A ÷ 0030 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0030",
                   std::deque<wstring>{L"\u0061", L"\u0030"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 0030 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0030",
                   std::deque<wstring>{L"\u0061", L"\u0030"});

  // ÷ 0061 ÷ 003A ÷ 005F ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u005F",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 005F ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u005F",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A ÷ 1F1E6 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 × 003A × 05D0 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [6.0]
  // COLON (MidLetter) × [7.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u05D0",
                   std::deque<wstring>{L"\u0061\u003A\u05D0"});

  // ÷ 0061 × 003A × 0308 × 05D0 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [7.0]
  // HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u05D0",
                   std::deque<wstring>{L"\u0061\u003A\u0308\u05D0"});

  // ÷ 0061 ÷ 003A ÷ 0022 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\"",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 0022 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\"",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 00AD ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u00AD",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 × 00AD ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0]
  // SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u00AD",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0300 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0300",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 003A × 0308 × 0300 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0]
  // COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0300",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 × 003A × 0061 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] COLON (MidLetter) × [7.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u003A\u0061\u2060"});

  // ÷ 0061 × 003A × 0308 × 0061 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // × [7.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u003A\u0308\u0061\u2060"});

  // ÷ 0061 × 003A × 0061 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] COLON (MidLetter) × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0061\u003A",
                   std::deque<wstring>{L"\u0061\u003A\u0061"});

  // ÷ 0061 × 003A × 0308 × 0061 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061\u003A\u0308\u0061"});

  // ÷ 0061 × 003A × 0061 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] COLON (MidLetter) × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0061\u0027",
                   std::deque<wstring>{L"\u0061\u003A\u0061"});

  // ÷ 0061 × 003A × 0308 × 0061 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061\u003A\u0308\u0061"});

  // ÷ 0061 × 003A × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] COLON (MidLetter) × [7.0] LATIN SMALL LETTER A (ALetter)
  // ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061\u003A\u0061"});

  // ÷ 0061 × 003A × 0308 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) × [6.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061\u003A\u0308\u0061"});

  // ÷ 0061 × 003A × 0061 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] COLON (MidLetter) × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0061\u002C",
                   std::deque<wstring>{L"\u0061\u003A\u0061"});

  // ÷ 0061 × 003A × 0308 × 0061 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061\u003A\u0308\u0061"});

  // ÷ 0061 ÷ 003A ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0031\u003A",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 003A ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0031\u0027",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 003A ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0031\u002C",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 003A ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [999.0] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 003A × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) ÷ [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u003A\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 ÷ 0001 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0001",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 0001 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0001",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 ÷ 000D ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\r",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 000D ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\r",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 ÷ 000A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\n",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 000A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\n",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 ÷ 000B ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u000B",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 000B ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u000B",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 ÷ 3031 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u3031",
                   std::deque<wstring>{L"\u0061", L"\u3031"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 3031 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u3031",
                   std::deque<wstring>{L"\u0061", L"\u3031"});

  // ÷ 0061 × 0027 × 0041 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [6.0]
  // APOSTROPHE (Single_Quote) × [7.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0041",
                   std::deque<wstring>{L"\u0061\u0027\u0041"});

  // ÷ 0061 × 0027 × 0308 × 0041 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [7.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0041",
                   std::deque<wstring>{L"\u0061\u0027\u0308\u0041"});

  // ÷ 0061 ÷ 0027 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 ÷ 002E ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u002E",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 002E ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u002E",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 ÷ 0030 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0030",
                   std::deque<wstring>{L"\u0061", L"\u0030"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 0030 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0030",
                   std::deque<wstring>{L"\u0061", L"\u0030"});

  // ÷ 0061 ÷ 0027 ÷ 005F ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u005F",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 005F ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u005F",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 ÷ 1F1E6 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [999.0] REGIONAL INDICATOR SYMBOL
  // LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 × 0027 × 05D0 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) × [6.0]
  // APOSTROPHE (Single_Quote) × [7.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u05D0",
                   std::deque<wstring>{L"\u0061\u0027\u05D0"});

  // ÷ 0061 × 0027 × 0308 × 05D0 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [7.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u05D0",
                   std::deque<wstring>{L"\u0061\u0027\u0308\u05D0"});

  // ÷ 0061 ÷ 0027 ÷ 0022 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\"",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 0022 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\"",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 00AD ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u00AD",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 × 00AD ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u00AD",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0300 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0300",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 0308 × 0300 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0300",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 × 0027 × 0061 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] APOSTROPHE (Single_Quote) × [7.0] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u0027\u0061\u2060"});

  // ÷ 0061 × 0027 × 0308 × 0061 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [7.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u0027\u0308\u0061\u2060"});

  // ÷ 0061 × 0027 × 0061 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] APOSTROPHE (Single_Quote) × [7.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0061\u003A",
                   std::deque<wstring>{L"\u0061\u0027\u0061"});

  // ÷ 0061 × 0027 × 0308 × 0061 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061\u0027\u0308\u0061"});

  // ÷ 0061 × 0027 × 0061 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] APOSTROPHE (Single_Quote) × [7.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0061\u0027",
                   std::deque<wstring>{L"\u0061\u0027\u0061"});

  // ÷ 0061 × 0027 × 0308 × 0061 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061\u0027\u0308\u0061"});

  // ÷ 0061 × 0027 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [7.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061\u0027\u0061"});

  // ÷ 0061 × 0027 × 0308 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061\u0027\u0308\u0061"});

  // ÷ 0061 × 0027 × 0061 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] APOSTROPHE (Single_Quote) × [7.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0061\u002C",
                   std::deque<wstring>{L"\u0061\u0027\u0061"});

  // ÷ 0061 × 0027 × 0308 × 0061 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061\u0027\u0308\u0061"});

  // ÷ 0061 ÷ 0027 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0031\u003A",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0031\u0027",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0031\u002C",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [999.0] DIGIT ONE (Numeric)
  // ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 0001 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0001",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 0001 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF
  // HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0001",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 000D ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [3.2]
  // <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\r",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 000D ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE
  // RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\r",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 000A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [3.2]
  // <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\n",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 000A ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED
  // (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\n",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 000B ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [3.2]
  // <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u000B",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 000B ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u000B",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 3031 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u3031",
                   std::deque<wstring>{L"\u0061", L"\u3031"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 3031 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA
  // REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u3031",
                   std::deque<wstring>{L"\u0061", L"\u3031"});

  // ÷ 0061 × 0027 × 2060 × 0041 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) × [7.0]
  // LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0041",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u0041"});

  // ÷ 0061 × 0027 × 2060 × 0308 × 0041 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) × [7.0] LATIN CAPITAL LETTER A
  // (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0041",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u0308\u0041"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 002E ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u002E",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 002E ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u002E",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 0030 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0030",
                   std::deque<wstring>{L"\u0061", L"\u0030"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 0030 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0030",
                   std::deque<wstring>{L"\u0061", L"\u0030"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 005F ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u005F",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 005F ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u005F",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 1F1E6 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL
  // INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 × 0027 × 2060 × 05D0 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ×
  // [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) × [7.0]
  // HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u05D0",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u05D0"});

  // ÷ 0061 × 0027 × 2060 × 0308 × 05D0 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE)
  // × [4.0] COMBINING DIAERESIS (Extend_FE) × [7.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u05D0",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u0308\u05D0"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 0022 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\"",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 0022 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION
  // MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\"",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 00AD ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u00AD",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 × 00AD ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u00AD",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0300 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0300",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 × 0300 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE
  // ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0300",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 × 0027 × 2060 × 0061 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE)
  // × [7.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0061\u2060",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u0061\u2060"});

  // ÷ 0061 × 0027 × 2060 × 0308 × 0061 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) × [7.0] LATIN SMALL
  // LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(
      analyzer, L"\u0061\u0027\u2060\u0308\u0061\u2060",
      std::deque<wstring>{L"\u0061\u0027\u2060\u0308\u0061\u2060"});

  // ÷ 0061 × 0027 × 2060 × 0061 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE)
  // × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0061\u003A",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u0061"});

  // ÷ 0061 × 0027 × 2060 × 0308 × 0061 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) × [7.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u0308\u0061"});

  // ÷ 0061 × 0027 × 2060 × 0061 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE)
  // × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0061\u0027",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u0061"});

  // ÷ 0061 × 0027 × 2060 × 0308 × 0061 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) × [7.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u0308\u0061"});

  // ÷ 0061 × 0027 × 2060 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u0061"});

  // ÷ 0061 × 0027 × 2060 × 0308 × 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LATIN SMALL
  // LETTER A (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) × [7.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u0308\u0061"});

  // ÷ 0061 × 0027 × 2060 × 0061 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE)
  // × [7.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0061\u002C",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u0061"});

  // ÷ 0061 × 0027 × 2060 × 0308 × 0061 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) × [7.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061\u0027\u2060\u0308\u0061"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0031\u003A",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0031\u0027",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0031\u002C",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 × 2060 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 0027 × 2060 × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LATIN SMALL
  // LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT ONE
  // (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0027\u2060\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 002C ÷ 0001 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0001",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0001 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0001",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 000D ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\r",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 000D ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2]
  // <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\r",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 000A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\n",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 000A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2]
  // <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\n",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 000B ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u000B",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 000B ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2]
  // <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u000B",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 3031 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u3031",
                   std::deque<wstring>{L"\u0061", L"\u3031"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 3031 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u3031",
                   std::deque<wstring>{L"\u0061", L"\u3031"});

  // ÷ 0061 ÷ 002C ÷ 0041 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0041",
                   std::deque<wstring>{L"\u0061", L"\u0041"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0041 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0041",
                   std::deque<wstring>{L"\u0061", L"\u0041"});

  // ÷ 0061 ÷ 002C ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u003A",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u002C",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 002E ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u002E",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 002E ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u002E",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 0030 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0030",
                   std::deque<wstring>{L"\u0061", L"\u0030"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0030 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0030",
                   std::deque<wstring>{L"\u0061", L"\u0030"});

  // ÷ 0061 ÷ 002C ÷ 005F ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u005F",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 005F ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u005F",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 1F1E6 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 05D0 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u05D0",
                   std::deque<wstring>{L"\u0061", L"\u05D0"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 05D0 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u05D0",
                   std::deque<wstring>{L"\u0061", L"\u05D0"});

  // ÷ 0061 ÷ 002C ÷ 0022 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\"",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0022 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\"",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0027",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 00AD ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u00AD",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 × 00AD ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT
  // HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u00AD",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0300 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0300",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 × 0300 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0]
  // COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0300",
                   std::deque<wstring>{L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 0061 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [999.0] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0061\u2060",
                   std::deque<wstring>{L"\u0061", L"\u0061\u2060"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // ÷ [999.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0061", L"\u0061\u2060"});

  // ÷ 0061 ÷ 002C ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0061\u003A",
                   std::deque<wstring>{L"\u0061", L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0061", L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0061\u0027",
                   std::deque<wstring>{L"\u0061", L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0061", L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [999.0] LATIN SMALL LETTER A (ALetter)
  // ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061", L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) ÷ [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0061", L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0061\u002C",
                   std::deque<wstring>{L"\u0061", L"\u0061"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0061", L"\u0061"});

  // ÷ 0061 ÷ 002C ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0031\u003A",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 002C ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0031\u0027",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 002C ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0031\u002C",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 002C ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0061 ÷ 002C × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] LATIN SMALL LETTER
  // A (ALetter) ÷ [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS
  // (Extend_FE) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u002C\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0061", L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 0001 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0001",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0001 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START
  // OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0001",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 000D ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\r",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 000D ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE
  // RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\r",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 000A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\n",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 000A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE
  // FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\n",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 000B ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u000B",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 000B ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u000B",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 3031 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u3031",
                   std::deque<wstring>{L"\u0031", L"\u3031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 3031 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u3031",
                   std::deque<wstring>{L"\u0031", L"\u3031"});

  // ÷ 0031 ÷ 003A ÷ 0041 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0041",
                   std::deque<wstring>{L"\u0031", L"\u0041"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0041 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN
  // CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0041",
                   std::deque<wstring>{L"\u0031", L"\u0041"});

  // ÷ 0031 ÷ 003A ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 002E ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u002E",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 002E ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL
  // STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u002E",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 0030 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [999.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0030",
                   std::deque<wstring>{L"\u0031", L"\u0030"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0030 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] DIGIT
  // ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0030",
                   std::deque<wstring>{L"\u0031", L"\u0030"});

  // ÷ 0031 ÷ 003A ÷ 005F ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u005F",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 005F ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LOW
  // LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u005F",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 1F1E6 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 05D0 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [999.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u05D0",
                   std::deque<wstring>{L"\u0031", L"\u05D0"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 05D0 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] HEBREW
  // LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u05D0",
                   std::deque<wstring>{L"\u0031", L"\u05D0"});

  // ÷ 0031 ÷ 003A ÷ 0022 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\"",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0022 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\"",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 00AD ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u00AD",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 × 00AD ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT
  // HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u00AD",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0300 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0300",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 × 0300 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING
  // GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0300",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 0061 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0061\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061\u2060"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061\u2060"});

  // ÷ 0031 ÷ 003A ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0061\u003A",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 003A ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0061\u0027",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 003A ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // ÷ [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote)
  // × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 003A ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0061\u002C",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 003A ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0031\u003A",
                   std::deque<wstring>{L"\u0031", L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0031 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031", L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0031\u0027",
                   std::deque<wstring>{L"\u0031", L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0031 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031", L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0031\u002C",
                   std::deque<wstring>{L"\u0031", L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0031 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031", L"\u0031"});

  // ÷ 0031 ÷ 003A ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COLON (MidLetter) ÷ [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP
  // (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0031"});

  // ÷ 0031 ÷ 003A × 0308 ÷ 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // ÷ [999.0] COLON (MidLetter) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u003A\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0031"});

  // ÷ 0031 ÷ 0027 ÷ 0001 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0001",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 0001 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0001",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 ÷ 000D ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\r",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 000D ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2]
  // <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\r",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 ÷ 000A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\n",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 000A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2]
  // <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\n",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 ÷ 000B ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u000B",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 000B ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2]
  // <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u000B",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 ÷ 3031 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u3031",
                   std::deque<wstring>{L"\u0031", L"\u3031"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 3031 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u3031",
                   std::deque<wstring>{L"\u0031", L"\u3031"});

  // ÷ 0031 ÷ 0027 ÷ 0041 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0041",
                   std::deque<wstring>{L"\u0031", L"\u0041"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 0041 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0041",
                   std::deque<wstring>{L"\u0031", L"\u0041"});

  // ÷ 0031 ÷ 0027 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 ÷ 002E ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u002E",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 002E ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u002E",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 × 0027 × 0030 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) × [12.0] APOSTROPHE
  // (Single_Quote) × [11.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0030",
                   std::deque<wstring>{L"\u0031\u0027\u0030"});

  // ÷ 0031 × 0027 × 0308 × 0030 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) × [12.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) × [11.0]
  // DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0030",
                   std::deque<wstring>{L"\u0031\u0027\u0308\u0030"});

  // ÷ 0031 ÷ 0027 ÷ 005F ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u005F",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 005F ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u005F",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 ÷ 1F1E6 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 ÷ 05D0 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [999.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u05D0",
                   std::deque<wstring>{L"\u0031", L"\u05D0"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 05D0 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u05D0",
                   std::deque<wstring>{L"\u0031", L"\u05D0"});

  // ÷ 0031 ÷ 0027 ÷ 0022 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\"",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 0022 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\"",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 00AD ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u00AD",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 × 00AD ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0]
  // SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u00AD",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0300 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0300",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 × 0308 × 0300 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0]
  // COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0300",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 0027 ÷ 0061 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] LATIN SMALL LETTER A (ALetter) × [4.0]
  // WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0061\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061\u2060"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061\u2060"});

  // ÷ 0031 ÷ 0027 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0061\u003A",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 0027 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0061\u0027",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 0027 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [999.0] LATIN SMALL LETTER A (ALetter)
  // ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE
  // (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 0027 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0061\u002C",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 0027 × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷
  // [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 × 0027 × 0031 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) × [12.0]
  // APOSTROPHE (Single_Quote) × [11.0] DIGIT ONE (Numeric) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0031\u003A",
                   std::deque<wstring>{L"\u0031\u0027\u0031"});

  // ÷ 0031 × 0027 × 0308 × 0031 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ×
  // [12.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [11.0] DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031\u0027\u0308\u0031"});

  // ÷ 0031 × 0027 × 0031 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) × [12.0]
  // APOSTROPHE (Single_Quote) × [11.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0031\u0027",
                   std::deque<wstring>{L"\u0031\u0027\u0031"});

  // ÷ 0031 × 0027 × 0308 × 0031 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ×
  // [12.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [11.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031\u0027\u0308\u0031"});

  // ÷ 0031 × 0027 × 0031 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) × [12.0]
  // APOSTROPHE (Single_Quote) × [11.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0031\u002C",
                   std::deque<wstring>{L"\u0031\u0027\u0031"});

  // ÷ 0031 × 0027 × 0308 × 0031 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ×
  // [12.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE) ×
  // [11.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031\u0027\u0308\u0031"});

  // ÷ 0031 × 0027 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ×
  // [12.0] APOSTROPHE (Single_Quote) × [11.0] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031\u0027\u0031"});

  // ÷ 0031 × 0027 × 0308 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // × [12.0] APOSTROPHE (Single_Quote) × [4.0] COMBINING DIAERESIS (Extend_FE)
  // × [11.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u0027\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031\u0027\u0308\u0031"});

  // ÷ 0031 ÷ 002C ÷ 0001 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [999.0] <START OF HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0001",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 0001 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF
  // HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0001",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C ÷ 000D ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\r",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 000D ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE
  // RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\r",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C ÷ 000A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\n",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 000A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED
  // (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\n",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C ÷ 000B ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u000B",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 000B ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u000B",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C ÷ 3031 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [999.0] VERTICAL KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u3031",
                   std::deque<wstring>{L"\u0031", L"\u3031"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 3031 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL
  // KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u3031",
                   std::deque<wstring>{L"\u0031", L"\u3031"});

  // ÷ 0031 ÷ 002C ÷ 0041 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0041",
                   std::deque<wstring>{L"\u0031", L"\u0041"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 0041 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN
  // CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0041",
                   std::deque<wstring>{L"\u0031", L"\u0041"});

  // ÷ 0031 ÷ 002C ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C ÷ 002E ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u002E",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 002E ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u002E",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 × 002C × 0030 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) × [12.0] COMMA
  // (MidNum) × [11.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0030",
                   std::deque<wstring>{L"\u0031\u002C\u0030"});

  // ÷ 0031 × 002C × 0308 × 0030 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) × [12.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) × [11.0] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0030",
                   std::deque<wstring>{L"\u0031\u002C\u0308\u0030"});

  // ÷ 0031 ÷ 002C ÷ 005F ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u005F",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 005F ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u005F",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C ÷ 1F1E6 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL
  // INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C ÷ 05D0 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [999.0] HEBREW LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u05D0",
                   std::deque<wstring>{L"\u0031", L"\u05D0"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 05D0 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] HEBREW
  // LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u05D0",
                   std::deque<wstring>{L"\u0031", L"\u05D0"});

  // ÷ 0031 ÷ 002C ÷ 0022 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\"",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 0022 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION
  // MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\"",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 00AD ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u00AD",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 × 00AD ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u00AD",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0300 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0] COMMA
  // (MidNum) × [4.0] COMBINING GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0300",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C × 0308 × 0300 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING
  // GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0300",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002C ÷ 0061 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0061\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061\u2060"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061\u2060"});

  // ÷ 0031 ÷ 002C ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0061\u003A",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002C ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0061\u0027",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002C ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COMMA (MidNum) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0]
  // APOSTROPHE (Single_Quote) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // ÷ [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0]
  // WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002C ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0061\u002C",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002C × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 × 002C × 0031 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) × [12.0]
  // COMMA (MidNum) × [11.0] DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0031\u003A",
                   std::deque<wstring>{L"\u0031\u002C\u0031"});

  // ÷ 0031 × 002C × 0308 × 0031 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ×
  // [12.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) × [11.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031\u002C\u0308\u0031"});

  // ÷ 0031 × 002C × 0031 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) × [12.0]
  // COMMA (MidNum) × [11.0] DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0031\u0027",
                   std::deque<wstring>{L"\u0031\u002C\u0031"});

  // ÷ 0031 × 002C × 0308 × 0031 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ×
  // [12.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) × [11.0]
  // DIGIT ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031\u002C\u0308\u0031"});

  // ÷ 0031 × 002C × 0031 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) × [12.0]
  // COMMA (MidNum) × [11.0] DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0031\u002C",
                   std::deque<wstring>{L"\u0031\u002C\u0031"});

  // ÷ 0031 × 002C × 0308 × 0031 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ×
  // [12.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) × [11.0]
  // DIGIT ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031\u002C\u0308\u0031"});

  // ÷ 0031 × 002C × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ×
  // [12.0] COMMA (MidNum) × [11.0] DIGIT ONE (Numeric) ÷ [999.0] FULL STOP
  // (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031\u002C\u0031"});

  // ÷ 0031 × 002C × 0308 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // × [12.0] COMMA (MidNum) × [4.0] COMBINING DIAERESIS (Extend_FE) × [11.0]
  // DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002C\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031\u002C\u0308\u0031"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 0001 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] <START OF
  // HEADING> (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0001",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 0001 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] <START OF HEADING> (Other) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0001",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 000D ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [3.2] <CARRIAGE
  // RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\r",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 000D ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <CARRIAGE RETURN (CR)> (CR) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\r",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 000A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [3.2] <LINE FEED
  // (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\n",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 000A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE FEED (LF)> (LF) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\n",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 000B ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [3.2] <LINE
  // TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u000B",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 000B ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [3.2] <LINE TABULATION> (Newline) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u000B",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 3031 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] VERTICAL
  // KANA REPEAT MARK (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u3031",
                   std::deque<wstring>{L"\u0031", L"\u3031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 3031 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] VERTICAL KANA REPEAT MARK
  // (Katakana) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u3031",
                   std::deque<wstring>{L"\u0031", L"\u3031"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 0041 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] LATIN
  // CAPITAL LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0041",
                   std::deque<wstring>{L"\u0031", L"\u0041"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 0041 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN CAPITAL LETTER A (ALetter)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0041",
                   std::deque<wstring>{L"\u0031", L"\u0041"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] COLON
  // (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u003A",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] COMMA
  // (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u002C",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 002E ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] FULL STOP
  // (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u002E",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 002E ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] FULL STOP (MidNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u002E",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 × 002E × 2060 × 0030 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) × [12.0] FULL
  // STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [11.0] DIGIT ZERO
  // (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0030",
                   std::deque<wstring>{L"\u0031\u002E\u2060\u0030"});

  // ÷ 0031 × 002E × 2060 × 0308 × 0030 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ×
  // [12.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [11.0] DIGIT ZERO (Numeric) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0030",
                   std::deque<wstring>{L"\u0031\u002E\u2060\u0308\u0030"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 005F ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] LOW LINE
  // (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u005F",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 005F ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LOW LINE (ExtendNumLet) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u005F",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 1F1E6 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] REGIONAL
  // INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 1F1E6 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] REGIONAL INDICATOR SYMBOL LETTER
  // A (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\uD83C\uDDE6",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 05D0 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] HEBREW
  // LETTER ALEF (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u05D0",
                   std::deque<wstring>{L"\u0031", L"\u05D0"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 05D0 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] HEBREW LETTER ALEF
  // (Hebrew_Letter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u05D0",
                   std::deque<wstring>{L"\u0031", L"\u05D0"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 0022 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] QUOTATION
  // MARK (Double_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\"",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 0022 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] QUOTATION MARK (Double_Quote) ÷
  // [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\"",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] APOSTROPHE
  // (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0027",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 00AD ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0] SOFT HYPHEN
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u00AD",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 × 00AD ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] SOFT HYPHEN (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u00AD",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0300 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷ [999.0]
  // FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0] COMBINING
  // GRAVE ACCENT (Extend_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0300",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 × 0300 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [4.0] COMBINING GRAVE ACCENT (Extend_FE)
  // ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0300",
                   std::deque<wstring>{L"\u0031"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 0061 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0061\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061\u2060"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 0061 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0061\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061\u2060"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0061\u003A",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 0061 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0061\u003A",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0061\u0027",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 0061 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0061\u0027",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0]
  // WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 0061 ÷ 0027 × 2060 ÷  #  ÷ [0.2] DIGIT ONE
  // (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A
  // (ALetter) ÷ [999.0] APOSTROPHE (Single_Quote) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0061\u0027\u2060",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002E × 2060 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [999.0]
  // LATIN SMALL LETTER A (ALetter) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0061\u002C",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 ÷ 002E × 2060 × 0308 ÷ 0061 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0061\u002C",
                   std::deque<wstring>{L"\u0031", L"\u0061"});

  // ÷ 0031 × 002E × 2060 × 0031 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ×
  // [12.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [11.0] DIGIT
  // ONE (Numeric) ÷ [999.0] COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0031\u003A",
                   std::deque<wstring>{L"\u0031\u002E\u2060\u0031"});

  // ÷ 0031 × 002E × 2060 × 0308 × 0031 ÷ 003A ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // × [12.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [11.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COLON (MidLetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0031\u003A",
                   std::deque<wstring>{L"\u0031\u002E\u2060\u0308\u0031"});

  // ÷ 0031 × 002E × 2060 × 0031 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ×
  // [12.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [11.0] DIGIT
  // ONE (Numeric) ÷ [999.0] APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0031\u0027",
                   std::deque<wstring>{L"\u0031\u002E\u2060\u0031"});

  // ÷ 0031 × 002E × 2060 × 0308 × 0031 ÷ 0027 ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // × [12.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [11.0] DIGIT ONE (Numeric) ÷ [999.0]
  // APOSTROPHE (Single_Quote) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0031\u0027",
                   std::deque<wstring>{L"\u0031\u002E\u2060\u0308\u0031"});

  // ÷ 0031 × 002E × 2060 × 0031 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric) ×
  // [12.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [11.0] DIGIT
  // ONE (Numeric) ÷ [999.0] COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0031\u002C",
                   std::deque<wstring>{L"\u0031\u002E\u2060\u0031"});

  // ÷ 0031 × 002E × 2060 × 0308 × 0031 ÷ 002C ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // × [12.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [4.0]
  // COMBINING DIAERESIS (Extend_FE) × [11.0] DIGIT ONE (Numeric) ÷ [999.0]
  // COMMA (MidNum) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0031\u002C",
                   std::deque<wstring>{L"\u0031\u002E\u2060\u0308\u0031"});

  // ÷ 0031 × 002E × 2060 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] DIGIT ONE (Numeric)
  // × [12.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [11.0]
  // DIGIT ONE (Numeric) ÷ [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER
  // (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031\u002E\u2060\u0031"});

  // ÷ 0031 × 002E × 2060 × 0308 × 0031 ÷ 002E × 2060 ÷  #  ÷ [0.2] DIGIT ONE
  // (Numeric) × [12.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ×
  // [4.0] COMBINING DIAERESIS (Extend_FE) × [11.0] DIGIT ONE (Numeric) ÷
  // [999.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0031\u002E\u2060\u0308\u0031\u002E\u2060",
                   std::deque<wstring>{L"\u0031\u002E\u2060\u0308\u0031"});

  // ÷ 0063 × 0061 × 006E × 0027 × 0074 ÷  #  ÷ [0.2] LATIN SMALL LETTER C
  // (ALetter) × [5.0] LATIN SMALL LETTER A (ALetter) × [5.0] LATIN SMALL LETTER
  // N (ALetter) × [6.0] APOSTROPHE (Single_Quote) × [7.0] LATIN SMALL LETTER T
  // (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0063\u0061\u006E\u0027\u0074",
                   std::deque<wstring>{L"\u0063\u0061\u006E\u0027\u0074"});

  // ÷ 0063 × 0061 × 006E × 2019 × 0074 ÷  #  ÷ [0.2] LATIN SMALL LETTER C
  // (ALetter) × [5.0] LATIN SMALL LETTER A (ALetter) × [5.0] LATIN SMALL LETTER
  // N (ALetter) × [6.0] RIGHT SINGLE QUOTATION MARK (MidNumLet) × [7.0] LATIN
  // SMALL LETTER T (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0063\u0061\u006E\u2019\u0074",
                   std::deque<wstring>{L"\u0063\u0061\u006E\u2019\u0074"});

  // ÷ 0061 × 0062 × 00AD × 0062 × 0079 ÷  #  ÷ [0.2] LATIN SMALL LETTER A
  // (ALetter) × [5.0] LATIN SMALL LETTER B (ALetter) × [4.0] SOFT HYPHEN
  // (Format_FE) × [5.0] LATIN SMALL LETTER B (ALetter) × [5.0] LATIN SMALL
  // LETTER Y (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\u0062\u00AD\u0062\u0079",
                   std::deque<wstring>{L"\u0061\u0062\u00AD\u0062\u0079"});

  // ÷ 0061 ÷ 0024 ÷ 002D ÷ 0033 × 0034 × 002C × 0035 × 0036 × 0037 × 002E ×
  // 0031 × 0034 ÷ 0025 ÷ 0062 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] DOLLAR SIGN (Other) ÷ [999.0] HYPHEN-MINUS (Other) ÷ [999.0] DIGIT
  // THREE (Numeric) × [8.0] DIGIT FOUR (Numeric) × [12.0] COMMA (MidNum) ×
  // [11.0] DIGIT FIVE (Numeric) × [8.0] DIGIT SIX (Numeric) × [8.0] DIGIT SEVEN
  // (Numeric) × [12.0] FULL STOP (MidNumLet) × [11.0] DIGIT ONE (Numeric) ×
  // [8.0] DIGIT FOUR (Numeric) ÷ [999.0] PERCENT SIGN (Other) ÷ [999.0] LATIN
  // SMALL LETTER B (ALetter) ÷ [0.3]
  assertAnalyzesTo(
      analyzer,
      L"\u0061\u0024\u002D\u0033\u0034\u002C\u0035\u0036\u0037\u002E\u0031"
      L"\u0034\u0025\u0062",
      std::deque<wstring>{
          L"\u0061", L"\u0033\u0034\u002C\u0035\u0036\u0037\u002E\u0031\u0034",
          L"\u0062"});

  // ÷ 0033 × 0061 ÷  #  ÷ [0.2] DIGIT THREE (Numeric) × [10.0] LATIN SMALL
  // LETTER A (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0033\u0061",
                   std::deque<wstring>{L"\u0033\u0061"});

  // ÷ 2060 ÷ 0063 × 2060 × 0061 × 2060 × 006E × 2060 × 0027 × 2060 × 0074 ×
  // 2060 × 2060 ÷  #  ÷ [0.2] WORD JOINER (Format_FE) ÷ [999.0] LATIN SMALL
  // LETTER C (ALetter) × [4.0] WORD JOINER (Format_FE) × [5.0] LATIN SMALL
  // LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) × [5.0] LATIN SMALL
  // LETTER N (ALetter) × [4.0] WORD JOINER (Format_FE) × [6.0] APOSTROPHE
  // (Single_Quote) × [4.0] WORD JOINER (Format_FE) × [7.0] LATIN SMALL LETTER T
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer,
                   L"\u2060\u0063\u2060\u0061\u2060\u006E\u2060\u0027\u2060"
                   L"\u0074\u2060\u2060",
                   std::deque<wstring>{L"\u0063\u2060\u0061\u2060\u006E\u2060"
                                        L"\u0027\u2060\u0074\u2060\u2060"});

  // ÷ 2060 ÷ 0063 × 2060 × 0061 × 2060 × 006E × 2060 × 2019 × 2060 × 0074 ×
  // 2060 × 2060 ÷  #  ÷ [0.2] WORD JOINER (Format_FE) ÷ [999.0] LATIN SMALL
  // LETTER C (ALetter) × [4.0] WORD JOINER (Format_FE) × [5.0] LATIN SMALL
  // LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) × [5.0] LATIN SMALL
  // LETTER N (ALetter) × [4.0] WORD JOINER (Format_FE) × [6.0] RIGHT SINGLE
  // QUOTATION MARK (MidNumLet) × [4.0] WORD JOINER (Format_FE) × [7.0] LATIN
  // SMALL LETTER T (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] WORD
  // JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer,
                   L"\u2060\u0063\u2060\u0061\u2060\u006E\u2060\u2019\u2060"
                   L"\u0074\u2060\u2060",
                   std::deque<wstring>{L"\u0063\u2060\u0061\u2060\u006E\u2060"
                                        L"\u2019\u2060\u0074\u2060\u2060"});

  // ÷ 2060 ÷ 0061 × 2060 × 0062 × 2060 × 00AD × 2060 × 0062 × 2060 × 0079 ×
  // 2060 × 2060 ÷  #  ÷ [0.2] WORD JOINER (Format_FE) ÷ [999.0] LATIN SMALL
  // LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) × [5.0] LATIN SMALL
  // LETTER B (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] SOFT HYPHEN
  // (Format_FE) × [4.0] WORD JOINER (Format_FE) × [5.0] LATIN SMALL LETTER B
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [5.0] LATIN SMALL LETTER Y
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(analyzer,
                   L"\u2060\u0061\u2060\u0062\u2060\u00AD\u2060\u0062\u2060"
                   L"\u0079\u2060\u2060",
                   std::deque<wstring>{L"\u0061\u2060\u0062\u2060\u00AD\u2060"
                                        L"\u0062\u2060\u0079\u2060\u2060"});

  // ÷ 2060 ÷ 0061 × 2060 ÷ 0024 × 2060 ÷ 002D × 2060 ÷ 0033 × 2060 × 0034 ×
  // 2060 × 002C × 2060 × 0035 × 2060 × 0036 × 2060 × 0037 × 2060 × 002E × 2060
  // × 0031 × 2060 × 0034 × 2060 ÷ 0025 × 2060 ÷ 0062 × 2060 × 2060 ÷  #  ÷
  // [0.2] WORD JOINER (Format_FE) ÷ [999.0] LATIN SMALL LETTER A (ALetter) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [999.0] DOLLAR SIGN (Other) × [4.0] WORD
  // JOINER (Format_FE) ÷ [999.0] HYPHEN-MINUS (Other) × [4.0] WORD JOINER
  // (Format_FE) ÷ [999.0] DIGIT THREE (Numeric) × [4.0] WORD JOINER (Format_FE)
  // × [8.0] DIGIT FOUR (Numeric) × [4.0] WORD JOINER (Format_FE) × [12.0] COMMA
  // (MidNum) × [4.0] WORD JOINER (Format_FE) × [11.0] DIGIT FIVE (Numeric) ×
  // [4.0] WORD JOINER (Format_FE) × [8.0] DIGIT SIX (Numeric) × [4.0] WORD
  // JOINER (Format_FE) × [8.0] DIGIT SEVEN (Numeric) × [4.0] WORD JOINER
  // (Format_FE) × [12.0] FULL STOP (MidNumLet) × [4.0] WORD JOINER (Format_FE)
  // × [11.0] DIGIT ONE (Numeric) × [4.0] WORD JOINER (Format_FE) × [8.0] DIGIT
  // FOUR (Numeric) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] PERCENT SIGN
  // (Other) × [4.0] WORD JOINER (Format_FE) ÷ [999.0] LATIN SMALL LETTER B
  // (ALetter) × [4.0] WORD JOINER (Format_FE) × [4.0] WORD JOINER (Format_FE) ÷
  // [0.3]
  assertAnalyzesTo(
      analyzer,
      L"\u2060\u0061\u2060\u0024\u2060\u002D\u2060\u0033\u2060\u0034\u2060"
      L"\u002C\u2060\u0035\u2060\u0036\u2060\u0037\u2060\u002E\u2060\u0031"
      L"\u2060\u0034\u2060\u0025\u2060\u0062\u2060\u2060",
      std::deque<wstring>{
          L"\u0061\u2060",
          L"\u0033\u2060\u0034\u2060\u002C\u2060\u0035\u2060\u0036\u2060\u0037"
          L"\u2060\u002E\u2060\u0031\u2060\u0034\u2060",
          L"\u0062\u2060\u2060"});

  // ÷ 2060 ÷ 0033 × 2060 × 0061 × 2060 × 2060 ÷  #  ÷ [0.2] WORD JOINER
  // (Format_FE) ÷ [999.0] DIGIT THREE (Numeric) × [4.0] WORD JOINER (Format_FE)
  // × [10.0] LATIN SMALL LETTER A (ALetter) × [4.0] WORD JOINER (Format_FE) ×
  // [4.0] WORD JOINER (Format_FE) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u2060\u0033\u2060\u0061\u2060\u2060",
                   std::deque<wstring>{L"\u0033\u2060\u0061\u2060\u2060"});

  // ÷ 0061 ÷ 1F1E6 ÷ 0062 ÷  #  ÷ [0.2] LATIN SMALL LETTER A (ALetter) ÷
  // [999.0] REGIONAL INDICATOR SYMBOL LETTER A (Regional_Indicator) ÷ [999.0]
  // LATIN SMALL LETTER B (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0061\uD83C\uDDE6\u0062",
                   std::deque<wstring>{L"\u0061", L"\u0062"});

  // ÷ 1F1F7 × 1F1FA ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER R
  // (Regional_Indicator) × [13.3] REGIONAL INDICATOR SYMBOL LETTER U
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDF7\uD83C\uDDFA",
                   std::deque<wstring>());

  // ÷ 1F1F7 × 1F1FA × 1F1F8 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER R
  // (Regional_Indicator) × [13.3] REGIONAL INDICATOR SYMBOL LETTER U
  // (Regional_Indicator) × [13.3] REGIONAL INDICATOR SYMBOL LETTER S
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDF7\uD83C\uDDFA\uD83C\uDDF8",
                   std::deque<wstring>());

  // ÷ 1F1F7 × 1F1FA × 1F1F8 × 1F1EA ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL
  // LETTER R (Regional_Indicator) × [13.3] REGIONAL INDICATOR SYMBOL LETTER U
  // (Regional_Indicator) × [13.3] REGIONAL INDICATOR SYMBOL LETTER S
  // (Regional_Indicator) × [13.3] REGIONAL INDICATOR SYMBOL LETTER E
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer,
                   L"\uD83C\uDDF7\uD83C\uDDFA\uD83C\uDDF8\uD83C\uDDEA",
                   std::deque<wstring>());

  // ÷ 1F1F7 × 1F1FA ÷ 200B ÷ 1F1F8 × 1F1EA ÷  #  ÷ [0.2] REGIONAL INDICATOR
  // SYMBOL LETTER R (Regional_Indicator) × [13.3] REGIONAL INDICATOR SYMBOL
  // LETTER U (Regional_Indicator) ÷ [999.0] ZERO WIDTH SPACE (Other) ÷ [999.0]
  // REGIONAL INDICATOR SYMBOL LETTER S (Regional_Indicator) × [13.3] REGIONAL
  // INDICATOR SYMBOL LETTER E (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer,
                   L"\uD83C\uDDF7\uD83C\uDDFA\u200B\uD83C\uDDF8\uD83C\uDDEA",
                   std::deque<wstring>());

  // ÷ 1F1E6 × 1F1E7 × 1F1E8 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL LETTER A
  // (Regional_Indicator) × [13.3] REGIONAL INDICATOR SYMBOL LETTER B
  // (Regional_Indicator) × [13.3] REGIONAL INDICATOR SYMBOL LETTER C
  // (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\uD83C\uDDE7\uD83C\uDDE8",
                   std::deque<wstring>());

  // ÷ 1F1E6 × 200D × 1F1E7 × 1F1E8 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL
  // LETTER A (Regional_Indicator) × [4.0] ZERO WIDTH JOINER (Extend_FE) ×
  // [13.3] REGIONAL INDICATOR SYMBOL LETTER B (Regional_Indicator) × [13.3]
  // REGIONAL INDICATOR SYMBOL LETTER C (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\u200D\uD83C\uDDE7\uD83C\uDDE8",
                   std::deque<wstring>());

  // ÷ 1F1E6 × 1F1E7 × 200D × 1F1E8 ÷  #  ÷ [0.2] REGIONAL INDICATOR SYMBOL
  // LETTER A (Regional_Indicator) × [13.3] REGIONAL INDICATOR SYMBOL LETTER B
  // (Regional_Indicator) × [4.0] ZERO WIDTH JOINER (Extend_FE) × [13.3]
  // REGIONAL INDICATOR SYMBOL LETTER C (Regional_Indicator) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\uD83C\uDDE6\uD83C\uDDE7\u200D\uD83C\uDDE8",
                   std::deque<wstring>());

  // ÷ 0020 × 200D ÷ 0646 ÷  #  ÷ [0.2] SPACE (Other) × [4.0] ZERO WIDTH JOINER
  // (Extend_FE) ÷ [999.0] ARABIC LETTER NOON (ALetter) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0020\u200D\u0646",
                   std::deque<wstring>{L"\u0646"});

  // ÷ 0646 × 200D ÷ 0020 ÷  #  ÷ [0.2] ARABIC LETTER NOON (ALetter) × [4.0]
  // ZERO WIDTH JOINER (Extend_FE) ÷ [999.0] SPACE (Other) ÷ [0.3]
  assertAnalyzesTo(analyzer, L"\u0646\u200D\u0020",
                   std::deque<wstring>{L"\u0646\u200D"});
}
} // namespace org::apache::lucene::analysis::standard