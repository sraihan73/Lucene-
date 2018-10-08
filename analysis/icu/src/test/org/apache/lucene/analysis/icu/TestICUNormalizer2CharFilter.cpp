using namespace std;

#include "TestICUNormalizer2CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/ngram/NGramTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/icu/ICUNormalizer2CharFilter.h"

namespace org::apache::lucene::analysis::icu
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using NGramTokenizer = org::apache::lucene::analysis::ngram::NGramTokenizer;
using TestUtil = org::apache::lucene::util::TestUtil;
using com::ibm::icu::text::Normalizer2;

void TestICUNormalizer2CharFilter::testNormalization() 
{
  wstring input = L"ʰ㌰゙5℃№㈱㌘，バッファーの正規化のテスト．㋐㋑㋒㋓㋔ｶｷｸｹｺｻﾞｼﾞ"
                  L"ｽﾞｾﾞｿﾞg̈각/각நிเกषिchkʷक्षि";
  shared_ptr<Normalizer2> normalizer =
      Normalizer2::getInstance(nullptr, L"nfkc_cf", Normalizer2::Mode::COMPOSE);
  wstring expectedOutput = normalizer->normalize(input);

  shared_ptr<CharFilter> reader = make_shared<ICUNormalizer2CharFilter>(
      make_shared<StringReader>(input), normalizer);
  std::deque<wchar_t> tempBuff(10);
  shared_ptr<StringBuilder> output = make_shared<StringBuilder>();
  while (true) {
    int length = reader->read(tempBuff);
    if (length == -1) {
      break;
    }
    output->append(tempBuff, 0, length);
    TestUtil::assertEquals(output->toString(),
                           normalizer->normalize(input.substr(
                               0, reader->correctOffset(output->length()))));
  }

  TestUtil::assertEquals(expectedOutput, output->toString());
}

void TestICUNormalizer2CharFilter::testTokenStream() 
{
  // '℃', '№', '㈱', '㌘', 'ｻ'+'<<', 'ｿ'+'<<', '㌰'+'<<'
  wstring input = L"℃ № ㈱ ㌘ ｻﾞ ｿﾞ ㌰ﾞ";

  shared_ptr<CharFilter> reader = make_shared<ICUNormalizer2CharFilter>(
      make_shared<StringReader>(input),
      Normalizer2::getInstance(nullptr, L"nfkc", Normalizer2::Mode::COMPOSE));

  shared_ptr<Tokenizer> tokenStream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenStream->setReader(reader);

  assertTokenStreamContents(
      tokenStream,
      std::deque<wstring>{L"°C", L"No", L"(株)", L"グラム", L"ザ", L"ゾ",
                           L"ピゴ"},
      std::deque<int>{0, 2, 4, 6, 8, 11, 14},
      std::deque<int>{1, 3, 5, 7, 10, 13, 16}, input.length());
}

void TestICUNormalizer2CharFilter::testTokenStream2() 
{
  // '㌰', '<<'゙, '5', '℃', '№', '㈱', '㌘', 'ｻ', '<<', 'ｿ', '<<'
  wstring input = L"㌰゙5℃№㈱㌘ｻﾞｿﾞ";

  shared_ptr<CharFilter> reader = make_shared<ICUNormalizer2CharFilter>(
      make_shared<StringReader>(input),
      Normalizer2::getInstance(nullptr, L"nfkc_cf",
                               Normalizer2::Mode::COMPOSE));

  shared_ptr<Tokenizer> tokenStream =
      make_shared<NGramTokenizer>(newAttributeFactory(), 1, 1);
  tokenStream->setReader(reader);

  assertTokenStreamContents(
      tokenStream,
      std::deque<wstring>{L"ピ", L"ゴ", L"5", L"°", L"c", L"n", L"o", L"(",
                           L"株", L")", L"グ", L"ラ", L"ム", L"ザ", L"ゾ"},
      std::deque<int>{0, 1, 2, 3, 3, 4, 4, 5, 5, 5, 6, 6, 6, 7, 9},
      std::deque<int>{1, 2, 3, 3, 4, 4, 5, 5, 5, 6, 6, 6, 7, 9, 11},
      input.length());
}

void TestICUNormalizer2CharFilter::testMassiveLigature() 
{
  wstring input = L"\uFDFA";

  shared_ptr<CharFilter> reader = make_shared<ICUNormalizer2CharFilter>(
      make_shared<StringReader>(input),
      Normalizer2::getInstance(nullptr, L"nfkc_cf",
                               Normalizer2::Mode::COMPOSE));

  shared_ptr<Tokenizer> tokenStream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenStream->setReader(reader);

  assertTokenStreamContents(
      tokenStream, std::deque<wstring>{L"صلى", L"الله", L"عليه", L"وسلم"},
      std::deque<int>{0, 0, 0, 0}, std::deque<int>{0, 0, 0, 1},
      input.length());
}

void TestICUNormalizer2CharFilter::doTestMode(
    shared_ptr<Normalizer2> normalizer, int maxLength, int iterations,
    int bufferSize) 
{
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass>(
      shared_from_this(), normalizer, bufferSize);

  for (int i = 0; i < iterations; i++) {
    wstring input = TestUtil::randomUnicodeString(random(), maxLength);
    if (input.length() == 0) {
      continue;
    }
    wstring normalized = normalizer->normalize(input);
    if (normalized.length() == 0) {
      continue; // MockTokenizer doesnt tokenize empty string...
    }
    checkOneTerm(a, input, normalized);
  }
  delete a;
}

TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestICUNormalizer2CharFilter> outerInstance,
        shared_ptr<Normalizer2> normalizer, int bufferSize)
{
  this->outerInstance = outerInstance;
  this->normalizer = normalizer;
  this->bufferSize = bufferSize;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false));
}

shared_ptr<Reader>
TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<ICUNormalizer2CharFilter>(reader, normalizer, bufferSize);
}

void TestICUNormalizer2CharFilter::testNFC() 
{
  doTestMode(
      Normalizer2::getInstance(nullptr, L"nfc", Normalizer2::Mode::COMPOSE), 20,
      RANDOM_MULTIPLIER * 1000, 128);
}

void TestICUNormalizer2CharFilter::testNFCHuge() 
{
  doTestMode(
      Normalizer2::getInstance(nullptr, L"nfc", Normalizer2::Mode::COMPOSE),
      256, RANDOM_MULTIPLIER * 500, 16);
}

void TestICUNormalizer2CharFilter::testNFD() 
{
  doTestMode(
      Normalizer2::getInstance(nullptr, L"nfc", Normalizer2::Mode::DECOMPOSE),
      20, RANDOM_MULTIPLIER * 1000, 128);
}

void TestICUNormalizer2CharFilter::testNFDHuge() 
{
  doTestMode(
      Normalizer2::getInstance(nullptr, L"nfc", Normalizer2::Mode::DECOMPOSE),
      256, RANDOM_MULTIPLIER * 500, 16);
}

void TestICUNormalizer2CharFilter::testNFKC() 
{
  doTestMode(
      Normalizer2::getInstance(nullptr, L"nfkc", Normalizer2::Mode::COMPOSE),
      20, RANDOM_MULTIPLIER * 1000, 128);
}

void TestICUNormalizer2CharFilter::testNFKCHuge() 
{
  doTestMode(
      Normalizer2::getInstance(nullptr, L"nfkc", Normalizer2::Mode::COMPOSE),
      256, RANDOM_MULTIPLIER * 500, 16);
}

void TestICUNormalizer2CharFilter::testNFKD() 
{
  doTestMode(
      Normalizer2::getInstance(nullptr, L"nfkc", Normalizer2::Mode::DECOMPOSE),
      20, RANDOM_MULTIPLIER * 1000, 128);
}

void TestICUNormalizer2CharFilter::testNFKDHuge() 
{
  doTestMode(
      Normalizer2::getInstance(nullptr, L"nfkc", Normalizer2::Mode::DECOMPOSE),
      256, RANDOM_MULTIPLIER * 500, 16);
}

void TestICUNormalizer2CharFilter::testNFKC_CF() 
{
  doTestMode(
      Normalizer2::getInstance(nullptr, L"nfkc_cf", Normalizer2::Mode::COMPOSE),
      20, RANDOM_MULTIPLIER * 1000, 128);
}

void TestICUNormalizer2CharFilter::testNFKC_CFHuge() 
{
  doTestMode(
      Normalizer2::getInstance(nullptr, L"nfkc_cf", Normalizer2::Mode::COMPOSE),
      256, RANDOM_MULTIPLIER * 500, 16);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AwaitsFix(bugUrl =
// "https://issues.apache.org/jira/browse/LUCENE-5595") public void
// testRandomStrings() throws java.io.IOException
void TestICUNormalizer2CharFilter::testRandomStrings() 
{
  // nfkc_cf
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  // huge strings
  checkRandomData(random(), a, 25 * RANDOM_MULTIPLIER, 8192);
  delete a;

  // nfkd
  a = make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  // huge strings
  checkRandomData(random(), a, 25 * RANDOM_MULTIPLIER, 8192);
  delete a;
}

TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestICUNormalizer2CharFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false));
}

shared_ptr<Reader>
TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass2::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<ICUNormalizer2CharFilter>(
      reader, Normalizer2::getInstance(nullptr, L"nfkc_cf",
                                       Normalizer2::Mode::COMPOSE));
}

TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestICUNormalizer2CharFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false));
}

shared_ptr<Reader>
TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass3::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<ICUNormalizer2CharFilter>(
      reader,
      Normalizer2::getInstance(nullptr, L"nfkc", Normalizer2::Mode::DECOMPOSE));
}

void TestICUNormalizer2CharFilter::testCuriousString() 
{
  wstring text = L"\udb40\udc3d\uf273\ue960\u06c8\ud955\udc13\ub7fc\u0692 "
                 L"\u2089\u207b\u2073\u2075";
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this());
  for (int i = 0; i < 1000; i++) {
    checkAnalysisConsistency(random(), a, false, text);
  }
  delete a;
}

TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass4::
    AnalyzerAnonymousInnerClass4(
        shared_ptr<TestICUNormalizer2CharFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false));
}

shared_ptr<Reader>
TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass4::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<ICUNormalizer2CharFilter>(
      reader, Normalizer2::getInstance(nullptr, L"nfkc_cf",
                                       Normalizer2::Mode::COMPOSE));
}

void TestICUNormalizer2CharFilter::testCuriousMassiveString() throw(
    runtime_error)
{
  wstring text =
      wstring(L"yqt \u0728\u0707\u0712\u0720\u0734 \u204c\u201d hyipy "
              L"\u2667\u2619") +
      L"\u26ec\u267b\u26da uboyjwfbv \u2012\u205d\u2042\u200a\u2047\u2040 "
      L"gyxmmz yvv %" +
      L"\ufb86 \n<script> hupvobbv jvsztd x rww ct{1,5} brteyi dfgyzqbm hdykd "
      L"ahgeizyhv" +
      L" kLn c#\ud8f8\udd74 fPezd ktedq "
      L"\ufcea=\ud997\uddc9\u876bJ\u0231\ud98a\udce0\uf872" +
      L" zquqah \ub9d6\u144e\uc686 3\ud93d\udfca\u1215\ud614 tnorask "
      L"\u0348\u0334\u0334" +
      L"\u0300\u033d geqdeoghh foojebut \ufb52\u227ag\ud9bd\udc3a\u07efK "
      L"nyantr lksxw fs" +
      L"zies ubzqfolksjpgk "
      L"\u6fa3\ud859\udc36\u0501\ucca0\u0306\u001e\ua756\u052f \ucaf7" +
      L"\u0247\u0009\ufdddg \ud83c\udd02\ud83c\uddaf "
      L"\u05628\u2b49\u02e3\u0718\u0769\u4f1b" +
      L"\u0496\u0766\uecaa\ufb44 \u001d "
      L"\u0006hr\u00f0\ue649\u041a\uda6f\udfa5\uf31b\ue274" +
      L" ptgjf  \ud8cc\udf83M\u0013\u04c6i "
      L"\u205f\u2004\u2032\u2001\u2057\u2066 \u07d0\uacdb" +
      L"\u06a5z  pqfxwgbwe \ud1bc\u2eba\u2d45\u02ee\u56df xnujtfs "
      L"\u1b19\u1b17\u1b39\u1b20" +
      L"\u1b69\u1b58\u1b03\u1b6e\u1b73\u1b20 afsl zxlzziqh ahrhckhktf "
      L"\ud801\udc5b\ud801\udc61" +
      L" bkpmeyyqobwi qnkunmpjpihezll plhhws "
      L"\u37f7\u41d6\u3dca\u3e80\u4923\u36b8 \u195a\u1959" +
      L"\u196f\u1968\u1958\u197f </p \u0006s\u019f\uda82\udc90H_\u079d\ufd6f: "
      L"idpp \u217c" +
      L"\u2168\u2185\u2179\u2156\u2159\u217c\u2154\u2158 ({1,5}( jkieylqzmb "
      L"bfirnaj \uea71" +
      L"\uf17f\u0749\u054c \ud9ea\udf83\udbea\ude91j "
      L"x\u3366\u09c2\ud828\udd13~\u6fda\ueeee" +
      L" \ud834\udcd3\ud834\udc2b\ud834\udc8b\ud834\udcd8 dl "
      L"\ud802\ude3a\ud802\ude36\ud802" +
      L"\ude23\ud802\ude56 \u20ad\u20a0\u20a0\u20a0\u20b8\u20b4\u20ad lcql "
      L"\u0547\u0156]" +
      L"\ue344V] \ud90c\udd7d\u000eS\uf965\uf15e\u008f qdn \udac3\ude3c buy m "
      L"qni \u31a4\u31a3" +
      L"\u31b2\u31a9\u31a7 \u20df\u20ee\u20d3\u20d4\u20f1\u20de "
      L"\u4dc1\u4dc4\u4dde\u4dec\u4df9" +
      L"\u4dee\u4dc5 \udb40\udc36 gstfqnfWY \u24ab\u8d69\u0301 n?]v||]i )- "
      L"\udb0b\ude77\uf634" +
      L"\u0762 tpkkjlbcntsk eebtzirw xo hktxy \n</     vxro  xpr mtlp "
      L"p|tjf|i?.- lxpfo \udbd7" +
      L"\udf78\udbf5\udf57 u..b jjj]p?e jtckvhqb "
      L"\u20eb\u20ea\u20fa\u20ef\u20e1\u20ed\u20eb vvm" +
      L"uhbsvyi jx mqwxckf \u00d7 qqzs \u05ae\u022d\udb7c\udfb1\u070b vnhkz "
      L"egnutyuat \udaa2" +
      L"\udf20\ufa45#O\u2b61\u0d0e \u09a2\u0996\u09cd\u09b4 v \udbdb\ude9bqp "
      L"owsln \ua837\ua833" +
      L"\ua83f\ua83f\ua83f\ua831\ua83c\ua839 \u1a15\u1a1c\u1a12 \ud83c\ude20 "
      L">&pt&#x>129 \ud9f1" +
      L"\udf8c\uecdd \ud809\udc48\ud809\udc72 wswskop \ua70d\ua70e ohcjsboriux "
      L"\u2025\u2000\u206e" +
      L"\u2039\u2036\u2002\u200e \u1741\u1741\u175e bsykot \u0696\ufab5W ifj "
      L"cnosdrAxa qtv wvrdn" +
      L" dmt (){0 udqg Z5U\ufcab\ue9ed\u0378 ts zndsksqtu uxbkAbn "
      L"\u0535\"\u001b\ud923\udcc3\udae7" +
      L"\udccf\u02ac \u0131\ua72e\u1273\u02f8\u2329\u5d83Q icakme oojxv hpec "
      L"xndjzxre )e{0,5}. " +
      L"\u0005!\u0009\u0004\u000bt\u0006N vqxnokp cdshqof "
      L"\ua654\ua681\ua667\ua676\ua698  vwwp " +
      L"\ufe4b\ufe3b\ufe4d\ufe42\ufe44\ufe38\ufe4b\ufe4c\ufe4b\ufe38 "
      L"\u2ffb\u2ffa\u2ff6\u2ff7" +
      L"\u2ffc\u2ff6\u2ff5 \u3116\u312c\u3100\u3102\u310f\u3116 agyueiij "
      L"\u0764\u1178\ud866\udca1" +
      L"\u00f2\u039d \ud802\udc12\ud802\udc1e\ud802\udc28\ud802\udc19 kygiv "
      L"fxctjyj           \n" +
      L"  omu \ud855\udeb1\u063c\ufd54\u9dbf\uf80a\ufc60 "
      L"\u76ee\u3365\uf998\u70a8\u02d2 \u05e5" +
      L"\u0478\u61bb "
      L"\ua71c\ua708\ua70d\ua71a\ua712\ua712\ua719\ua706\ua71f\ua71c\ua71e"
      L"\ua718 pgq" +
      L"arvtzcoduk pyok \u1779\u1766\u1778\u177b "
      L"\u16e5\u16a7\u16f3\u16fe\u16c8\u16ba\u16a4 \u0527" +
      L"\u052f\u0513\u0500 iisgh hxd \u13dd\u13c6\u13db\u13ee\u13d7 "
      L"\u0019\ufed4 \ud802\udd3c\ud802" +
      L"\udd30\ud802\udd3d\ud802\udd24\ud802\udd2c jm gvylt eax xsbt mvuvnw "
      L"\u0246\u5514\udb16\uddcf" +
      L"\u1dc2\ud07b\u07af\u12e8\u8e8f\u0004 phy haduzzw "
      L"\u04a1\u8334\u14b5\uf0de\udb4b\udec0\u6b69 " +
      L"dubdl \u0cd2\u06c7\uf0297\u45efy-\u05e9\u01a3\uf46f aiafsh   &# "
      L"\u0003\ue5ab\uedcd] xhz vil" +
      L"wdlytsj \uda10\udf4f\u87b2 tomuca \u1fe4\u1f4c\u1fab \u035d\u0332 "
      L"xgeel nzp -)r)]r([{ nbc " +
      L"\u01b4\ud80f\udff5\u0008\u0091 tbugdgv \ud9cc\udd57\u0776t\uda0f\udc17 "
      L"rsuwxqxm \u2d71\u2d3d" +
      L"\u2d6e zsvuicin \udb50\ude9d\u7424\u30c7\uff73\ue11e\u0014 qxtxx "
      L"dlssfvxg ud \u0c1f\ud9d9" +
      L"\udce4\u2317\u0482\u017b \ud83c\udd91\ud83c\uddaf\ud83c\udd84 "
      L"\ud834\udf7d\ud834\udf70\ud834" +
      L"\udf61 \uabfc\uabe1\uabcd\uabd1\uabe8\uabf9 "
      L"\u292a\u2916\u295d\u290b\u296d\u291f \uf476\u0283" +
      L"\u03d5\ucfe2 h)(o? lqeatt \u20c9\u20a5\u20cd "
      L"\u1634d\u001a\ua052:\u00db \ud83c\udc8a\ud83c" +
      L"\udc41\ud83c\udc8a\ud83c\udc6e k civyjm ptz "
      L"\uf20d\uea32&\ud8db\ude48\uf657s\u06dc\u9aa5\udbd7" +
      L"\udc74\u0111 cehuo "
      L"\u2090\u209b\u2099\u208c\u209a\u2088\u2092\u207e\u207b\u2089 efpifnvdd "
      L"zstz" +
      L"duuyb \u04af3 \u2e5f\u2e7e\u2e7c\u2e54\u2e0f\u2e68\u2e0d\u2e05 <??>  "
      L"\u28d3\udbb7\udf6fJ\uf089" +
      L"\ub617\ufb80\u07d0\uf141 "
      L"\u0820\u083b\u0800\u0801\u0832\u0823\u0820\u081c\u0835 r laxzpfbcvz " +
      L"iuwbmq scpeqaq nvw{1,5}s) "
      L"\u234c\u231f\u231e\u23cc\u23d8\u2302\u2345\u231b\u239d\u231e 154614 " +
      L"wgefnmgq \udbbe\udc2a\uee8c ayaeg \u243c\u2404\u2409\u241a dd "
      L"hxuawoswx jqghoznw \u0019\u70cd& " +
      L"\ufe0a\ufe02\ufe04\ufe0c\ufe0d "
      L"\u0510\u0523\u0510\u0501\u0508\u0513\u050e ropgywv guqsrcz kmf " +
      L"\u0d1f\u0d5c\u0d24\u0d5f\u0d0b\u0d14 xgklnql oprw "
      L"\u0365\u034e\u036a\u0310\u034f\u0316\u031f " +
      L"\u01b2\u55f6\uf1eeo\ue994\u07c4? wewz idjwsqwo "
      L"\ufe67\ufe66\ufe52\ufe6a\ufe5b\ufe57\ufe68\ufe62" +
      L" \u27fd\u27fe\u27f6\u27f8 fcsqqvoy edrso \u5580\ue897 vantkitto sm "
      L"\uff60\ud957\udf48\uf919w" +
      L"\ud889\udf3e\ud9c8\uddf6 jhas uqzmlr "
      L"\ua4ce\ua4aa\ua4b3\ua4b5\ua4c2\ua4a5 kvuop ><script> " +
      L"\ud802\udd0f\ud802\udd15\ud802\udd11 \u16bc gjyabb mlaewr "
      L"\u1175\u11cc\u1192 ri ]\u01eb\ue4ca" +
      L"  \uedca\ucd66\u597c\u03df\uaf8f\u0619 &#x; ]?e "
      L"\ua6c2\ua6ed\ua6eb\ua6ea\ua6cd\ua6e2 gfpafsr" +
      L" pooo \u20cc\u20c4\u20a7\u20c8\u20a6\u20b0\u20ad \udb40\udd5b "
      L"tgcdmeuau \u141f\u637d \ufba8" +
      L"\ufcc7\ufde1\ufc85\ufbfc\ufbed b "
      L"\u319a\u3193\u3192\u3195\u319e\u319d\u3195\u3194\u3197\u319c " +
      L"\u0514\u0503\u051c\u050e\u0502\u0520 \u1db3\u1db5\u1d96 "
      L"\ud977\udde8w\u02ec&\u01dd\u29ed\ufead" +
      L"y\u03e3 ukzqq {0,5}{0 \u000f\uf028$\u046f7\udb7e\uded2 <p><l "
      L"\uea5e(\uf1dcX\u376b ([mi[n(a Jev" +
      L"xsJl \ubd09\u04c1\ua0f3\uff7b "
      L"\u1cd3\u1cd9\u1cf6\u1cf0\u1cd8\u1cdd\u1cdd\u1ce7\u1ce3\u1cd9 " +
      L"\ud800\udf91\ud800\udf99\ud800\udf83 \"135541 "
      L"\u18b3\u18c0\u18c2\u18ea\u18c4\u18fe\u18b2\u18fd" +
      L"\u18c3 uwykvfd lqiflow afdfctcz ol[hemp strmhxmk "
      L"\ua732\ua748\ua731\ua791\ua78b\ua7ee\ua7ea" +
      L"\ua789\ua766\ua7e4 gmzpmrpzr dqfp  wfxwjzepdj M\udb03\udeff\u13c5 "
      L"afsncdwp \ue716\u0734\ud8f9" +
      L"\udc986\u0017\u0211 t r vhczf (]|q.{0  "
      L"\u195e\u1973\u1958\u1956\u196c\u1973\u1966\u196c\u197a" +
      L"  \u2595\u258e\u259a\u2591\u2583\u2595 l kgopyql wes "
      L"E\u6611\ub713\u7058^ bipq dx 7507 \ua90b" +
      L"\ua90b ktjeqqx \u0e1d\u0e7f\u0e35 #(\u71b7'\u06e5\u03e4(\uf714\u6ff2 "
      L"advbgh \u319c\u3191 \uef11" +
      L"% \uc8a7C\ud8ed\udf4c rjb \u02ca\uf5bd\ue379n \ud834\udd7d\ud834\udd83 "
      L"jracjcd rpearfd ujzvdc" +
      L" ofg \u09df\u09f4\u0980\u09b3\u09bf\u09b7 \ud9cc\uddf4$\udb08\udf72 "
      L"iqcnwsyjmo </scri ]-q jsit" +
      L"gjg naeajiix vvmq dnlihau o cgyp tqsfe uixlzmhz sixhftpr uvtbvv "
      L"mphcWojZs \u190b\ud9c3\ude7c" +
      L"\u008b\u0606\ua8b1 a  \u000ebq# \u1e57\u0b66\uda41\ude32\ubfd6 ohph b- "
      L"ze \ue2a9\u0000 zatuye" +
      L" \ufd26\ufdfa\ufbbf\ufdb4\ufde3\ufd14\ufc25\ufcb8 sbtpb  nxtacgjo "
      L"\ud834\ude2a\ud834\ude0f" +
      L"\ud834\ude14\ud834\ude27 \ua835\ua835 ujpjkkyhujx "
      L"\u001e\ud9a7\udc45\u0011>\u1aef\u0d0d <" +
      L" hcefg \u01f0\u01d3 gxlwv \ud802\udd2f\ud802\udd34 "
      L"\udb9c\udcc8\udbb6\ude1e\udbaf\ude33\udbae" +
      L"\udc49 xyzlzynd \ud83c\udd44 vynhdwh "
      L"\u315d\u3157\u314d\u3180\u317d\u318d\u317d\u3156 ?>\"  " +
      L"\ud800\udfdb\ud800\udfb8\ud800\udfa6\ud800\udfa7 hszn sspwldpdwjfhk "
      L"vrbag \ueabd\ud9f2\udfb5" +
      L"\udafb\udd28\uf6a4y\ufdeb \u0798\u078f\u0789 "
      L"\ue80c\uf5c1\u001a\ud84b\uddef ywdrudo odqsts nnp" +
      L"vfl nvotk rdwhr rquj cn \ud7d4\ud7b3\ud7c7\ud7bf\ud7bd &#x528f28 emdq "
      L"pnqsbbgzs \u2c7d\u2c7e fj" +
      L"kwhku >&c "
      L"\ud800\udf85\ud800\udf88\ud800\udf93\ud800\udf84\ud800\udf82\ud800"
      L"\udf8b '\n\"<p></p>" +
      L" xltgxaa vqjmhiz n .m]c]tr( qerokel lc kugmimmtfxi         <?&#x524963 "
      L"[g)|n|[ cij </ BwbZfg " +
      L"pta bmhn \uf60dz\u54ca fwdp gnkz "
      L"\u030ba\ue115}\udb7d\ude86\ud813\udc50\uedb9 \u1745\u1749\u174d" +
      L"\u1747\u174b\u174f \ud802\udf09\ud802\udf3f\ud802\udf03\ud802\udf00 "
      L"agmj \u1db7 \u60669\u000f" +
      L"j\u000f\u02e4e\u05f5f   p \udaac\udc9f\uc257\u6e58U zcwsy "
      L"\u19a7\u19cf\u19aa\u199f\u19b7 zhll" +
      L" sbmv .uud \u040c\u2e78\ud9fc\udd0d\ufb7f\uf2e1\u04bf vqkyxua "
      L"\ud834\udf5e\ud834\udf45\ud834" +
      L"\udf23 \ud834\uddbe\ud834\udd9b\ud834\uddc4 f [{0,5}t ovbsy tcj "
      L"nyalqyabn yzhes mlwpw \ud835" +
      L"\uddd5\ud835\udfdf\ud835\uddb4\ud835\ude3e epqfkk cssusatbbq "
      L"\u1424\u1413\u1652\u15f0 dtfy zN" +
      L" \u2417\u2435\u2407 qtz \u2fff\u2ff1\u2ff8\u2ff8\u2ff7\u2ff7 "
      L"\ud802\ude43 gfpe stimmb ugPwlr " +
      L"\u0737\u0712\u0712\u071c \u21de "
      L"\u01d8\u079e\u8215\ue5b9\u07ef\u3cff\u0478 \u05dd\u05e5 gwqset" +
      L"iyyh jbncv \u68ba\u52aa) kcoeiuogjsh n[nh- \uf5ff\u7ec3Z zrptjltqstzvx "
      L"ts tbod &#88087 \u07fd" +
      L"\u07c1\u07c0\u07e9\u07fa\u07f2\u07e3\u07e8\u07cb\u07eb\u07d8 fisslh "
      L"\ue40f\u012b\u02cf\u0766" +
      L" \u1c25\u1c4f\u1c1d\u1c20 \"--> dPhwz "
      L"\ud808\udef9\ud808\udf4a\ud808\uddd3 cicqhk D\ue7d3=\u5228" +
      L"\udbc3\udd18\ueb0d\u0012\u0744\ufb04U\u001e\uf388c\u0306 \u2c08\u2c1e "
      L"Xxzcd \u001d\u0230\u45e8" +
      L"\u0653 <zlx \u1e8f\u1e28\u1e3c\u1e8d\u1ee8\u1e69 zcoeork d gusesc "
      L"\ud802\udd3e nyr vkfrgi \u2cde" +
      L" mo nklnsqweh <script gtoppwsbik vrcdbsx  pz "
      L"\udb0d\ude0c|\u93d0\uf241\u28a8\u0531 \ud83c" +
      L"\udc2b\ud83c\udc10 \ud800\udf91\ud800\udf8e qwmmkty "
      L"\u19f7\u19f7\u19e8\u19e0\u19f9\u19f6\u19e6" +
      L" \u7a60\u7b7b\u878c\u603c\u53c6\u6552\u6dfe \u0776\u0775 foxaqi m cdjd "
      L"guyqxodsccy \ucd7d\ub5eb" +
      L"\ud29e\ub9ad\uba00\uac9d\ud2f0 pxjtj \ue362\u079fx\uf193 ){1,5}[{ "
      L"hmpujw \u3193\u319b\u3195" +
      L"\u319c\u3198\u3193\u3195\u319d\u319e "
      L"\udb40\udd65\udb40\udd29\udb40\udd5c\udb40\uddba \ud801" +
      L"\udc18\ud801\udc24\ud801\udc4f\ud801\udc15\ud801\udc04 "
      L"\u1970\u196c\u1963\u196f\u1979 vjpeg " +
      L"\ufeb9 lhoeh &#x540b Szu \u0c21\u0c21\u0c36\u0c0e oyb "
      L"\u1c7c\ue0ba\u001e gskf \ud826\udd47" +
      L"\u0018 ooxki \u001d\u5b0d \uf0e2\u05ba\u000e\u6200 "
      L"\u2ecc\u2e8a\u2eb8\u2ee5\u2edb\u2ee4\u2ec7" +
      L"\u2ef9\u2e9e\u2e99 xpyijb bqvtjfjrxkjv sruyitq jobm u "
      L"\u0752\u075d\u0778\u0754\u075c \ua4af" +
      L"\ua4a5\ua4be\ua4a6\ua4b9\ua4b9 \ua835\ua832\ua838\ua83d "
      L"\ud83c\udc3a\ud83c\udc9f\ud83c\udc4e" +
      L" \ud7fb\ud7ce\ud7c6\ud7f8 erkzdwed ftzpcx ecbylf geptanc jxbhc ophh "
      L"wqng \ue48c\u9c86Z imkj" +
      L"nwetjbz njivpvo  \u6d9a\ud8da\udcba "
      L"\u29f4\u29fd\u29a6\u2980\u2989\u29f3\u29ec\u2991\u29e5" +
      L"\u29c6 \udb08\ude9d\u2ecb\u037e chmap <!--<sCrip "
      L"\ud83c\udc34\ud83c\udc79 SoOq l botgy \ud83c" +
      L"\udc11\ud83c\udc2e\ud83c\udc10 -)h?|] \ud801\udc2e\ud801\udc47 "
      L"pjlrcjij lpdft v.o| qolrd b  " +
      L"uefppzjyvva D\u05de\u0425\u01a3\ue1c0f\uf117\ue266\u0268\u03ec ynik  "
      L"\udae4\udc38\udba0\udd4c" +
      L" M\ue775\u000f \u3170\u3171\u3185\u3140\u3165\u317f "
      L"\u07f6\u4250vp\u001c\u07a9\uba71 myfawjt" +
      L"wvc cqsc o uxabwm \ua9b0\ua9d3  \u0293P\u4dde\u034e "
      L"\udbe7\udd0b\udbce\udf4d  a\udb4a\ude26x" +
      L"\u0bc5\u0355 xtwyyxkj uiwqzvdn \u00c4\u00f4\u00b9\u00f3\u00e3 svfczoqm "
      L"fx \ua65a\ua696\ua675" +
      L"\ua651\ua661\ua686\ua644 cohpzoprr "
      L"\u000f\ud9d5\udcbd\ud8fa\udc16\ub733\ud8d9\udcf7\uefe9" +
      L"\u02da wssqee tikod iwdo cuoi  mvkvd "
      L"\ud834\udcb7\ud834\udc52\ud834\udc37\ud834\udc30 yqmvakwq" +
      L" bwgy ndp \u0b53\u0b75\u0b60\u0b25\u0b1d\u0b1b\u0b19\u0b62 <pmg  "
      L"cqponju tiwlftbdn jdmc <?" +
      L"</p waesof \u3fea\u43bd\u3524\u3d5b \uf87f\u03ab\\\u0727?\uf145 "
      L"vwznkqkz \ud83c\ude6c\ud83c" +
      L"\udea7\ud83c\udedd powof \u94f3\u0392\ue4b5$ "
      L"\u079f\u07b5\u0781\u07ba\u0786\u07ae\u0782\u0785" +
      L" \ud83c\udecc\ud83c\ude8e\ud83c\udea1\ud83c\ude74 "
      L"\u2afb\u2a2c\u2ae8\u2ab5\u2af4 x\u4c6f hlb" +
      L" oirm \ud83c\udc0f\ud83c\udc19 abzbdmkzc qsvvzzc \uf14b "
      L"\udb53\ude89\u04d2\u53fe\ueb79 uuexkn" +
      L" nyeevfme \ue210\uea3e zdwk licyc { cik o].o{1,5 "
      L"\ua9d1\ua984\ua997\ua99d\ua9a2\ua9b3\ua986" +
      L"\ua9d7 "
      L"\u13ea\u13fb\u13b8\u13b9\u13db\u13e2\u13cf\u13c3\u13c8\u13cc\u13bc "
      L"\ueeae\u3c1c\uf043" +
      L"\u3302   \ufb791\u0009\uc0b7\u039cWG\u4f35\u000f\uf28c "
      L"\ueb24\udb18\uddef\ufb2c n-jr wsmhtbq " +
      L"\ue76b\ud894\udec7\u37f8 box \u1726\u1720\u172b\u173c\u1727 gqcrnrhxcj "
      L"\u09f8 rof \ua7fa" +
      L"\ua7a1 \u07ef\u07f3\u07e2\u07e0\u07d7 udhuv gadgqab({1 "
      L"\u2c52\u2c30\u2c17\u2c16 P\u33f9\u06da" +
      L"\u284b\u0734\u0799 "
      L"\u289a\u28a1\u28f0\u2853\u282a\u284b\u2859\u2881\u283c qmkopt qbvxnls "
      L"\ud9c6" +
      L"\udc11Z\u7c39\u24ec\u0353 \u069c dfdgt \ue967\u0493\uf460 )yv-|. nl "
      L"qvmu x{1,5} \\'\\'  ' \u0969" +
      L"\u0926\u0943\u0966\u0931\u0970\u094d\u0911\u0970 phiyfw ;\\'\\ zngsxit "
      L"\u07ec&\ud914\udd55\u9ab7" +
      L" ?[| b "
      L"\ufffc\uffff\ufffb\ufff3\ufff7\ufff8\ufff8\ufffb\ufff5\ufff9\ufffd "
      L"\u2ffd\u2ff2\u2ff1" +
      L"\u2ff9\u2ff6\u2ff1\u2ff8\u2ff1\u2ff8 \ua73d\ua793\ua7d1\ua7cf "
      L"\u258d\u2599\u259e\u258e\u258d" +
      L"\u259f \u001fha f \u0b2e\u0b37\u0b71\u0b44\u0b40\u0b2b \uf1909\ud033 "
      L"ofkl tbuxhs \ufb06\ufb47" +
      L" rqcr \u043d\uf8cf\u001c "
      L"\ud87e\ude05\ud87e\ude0d\ud87e\udd99\ud87e\udcc0 qqzqzntfby \u0a3f" +
      L"\u0a0e\u0a16 \ud7b8\ud7cd\ud7c7\ud7cc\ud7ca\ud7e8\ud7f9\ud7b3\ud7df "
      L"arzxo \u0f23\u0f2b\u0f68" +
      L"\u0f1c\u0fe8\u0f97\u0f27\u0fbd 190854825 "
      L"\ua915\ua907\ua902\ua902\ua907 \ufbbb\ufdd1\ufbdb" +
      L"\ufbed\ufbbb\ufd81\ufd41\ufc3a rrxnh "
      L"\u0ead\u0ebb\u0e97\u0eff\u0eed\u0e94\u0e86 \ud8c0\udd29" +
      L"\u0016\ue050\uebf0;\u79c0\u07ba\uf8ed b \u0122\u0012\udaf5\udcfb+  mkt "
      L"dluv \u18db\u18d4\u18ea" +
      L" \uee53\ueb89\u0679 \u24c2\u24ee\u24e5\u24ab\u24e1\u2460  \ub41eq "
      L"\uf1e0Tu\u0018\ue5b5 cqf" +
      L"pwjzw  cadaxx \u2642\u26be\u2603\u26aa\u26b0 pybwrfqbzr wgyejg cbhzl "
      L"ipagqw \ud841\udd0d" +
      L"\ud84a\udc42\ud84b\udf94\ud85e\udf91\ud847\udd41 fgjm lhtmoqls "
      L"\u2cc1\u076af >\u034e\ud8a7" +
      L"\udd17U\uffcf \u42cb\u07d6\u1d08Y\u0570 o\u016c] .ed]l.u oxms "
      L":\uf3cc\u0f67\u0014\u22c6" +
      L"\u0720E \u1fef\u1f6f\u1f6a <scri "
      L"\u63fb\u0508d\ueb2a\u001d\ue3f5\ue915\ud33d \ud800\udf43" +
      L"\ud800\udf43\ud800\udf4c\ud800\udf46 \ud802\udc3c\ud802\udc00 "
      L"ktdajettd urkyb \u040e\uaacf" +
      L"\ufd7f\uf130\u048f\u80a6g\u0277\u0013\u8981\uc35d xckws icodgomr "
      L"\udbf2\ude88\u9e5f o " +
      L"h{0,5}x cu oqtvvb ohbuuew ggh 0\u001d=\u8479\ufc33\ue941\ue518  "
      L"\uff87\u0012\u0226\u743d" +
      L"\uef94e\ue0e2\u05cc \ue261\u0015\uf9dc\u8233\u0305/\u111e3\udbb7\udcb5 "
      L"mxgbvl \uf20f\ud860" +
      L"\udc00\uf9f2\uecd2 fl \u03d1\u1664\u5e1d\u619b\uda19\udfe0v/ "
      L"\ud863\udfa2U\ue0c1\u07f1" +
      L"\ue071\udb8f\udeb6 miz \u0641\udb66\udce0' "
      L">\ud9c0\udfaf\u07b3J\uf240\ud863\udff8\u01bf" +
      L"\u2257\u008b\u0186\u0006 "
      L"\uaa90\uaa92\uaa9a\uaad6\uaaa7\uaac1\uaa9d\uaaa0\uaaab vorp \u1380" +
      L"\u1392\u139e\u138b\u1390\u1386 \uf626\uda58\uddb3\u0014 qrzl fzrb rllb "
      L"\uc5e5\uf606\u0749" +
      L"\ufff8\ud88a\udec12\ud97e\udee4 zscmiqtr \u01be\n "
      L"\u05f2\u05a0\u05ca\u05de\u059d\u05ac  " +
      L"\u2e21\u2e62\u2e72 \u0293 \ufff0\ufff3\ufff8\uffff\ufff2 grtuzvl "
      L"\ua8bc\ua880\ua89a kprmh " +
      L"\ud802\ude51\ud802\ude2e\ud802\ude09\ud802\ude15 cwauqnjs Ou "
      L"\u31c9\u31dc\u31e4\u31d1\u31e5" +
      L"\u31c1\u31d1\u31ce\u31c8 "
      L"\u31f6\u31fd\u31f0\u31fa\u31f0\u31f2\u31f3\u31f9 wpyi  awanm " +
      L"irnupwe &#x7e345 vzwzc qhupnxm qbaboo gtxfdycm vnvaptcc "
      L"\u0356\ud93f\udf7a {0,5})[k oxnum" +
      L"pqyhkg \ufc2c\u0213\ue36e\u0017\ud98b\udc43 "
      L"\u27f3\u27f7\u27ff\u27ff\u27f5\u27ff\u27f1 hm" +
      L"kmwm j{1,5} \u0293\u0262\u2c6d\u0278\u0267\u2c64\u02a8\u0296\u0284 "
      L"thjrit \u12e3 \ud808" +
      L"\udf7d\ud808\udca7 b prkjpdeavdooly "
      L"\"\\\u06d5\ud9dc\uddb6;\ufdd6\u05bd\u077f kyql \u2d2e" +
      L"\u2d04\u2d2e\u2d2a\u2d03\u2d1d scjl higfadu "
      L"\u3041\u306c\u3073\u305c\u308a\u308e\u3075" +
      L"\u3086 akfmdqrrwkw rfmqcxyekf \ud86c\udd70\ud86c\udcdc\ud86b\udea2 c< "
      L"cwab t \ud800\udd13" +
      L"\ud800\udd23 \u0138\ud817\uddcd\uf9f2 zisakv "
      L"\uea3e\u0314\u07be\ufe67b\ud38b\u0439\r " +
      L"\ua766\ua7c5\ua769\ua7a8\ua794 ksfyrg ({1,5}j)?wl "
      L"\ua94a\ua943\ua932\ua939\ua946\ua95c" +
      L"\ua955\ua952\ua958\ua94c pshdyg lhxpypaug blqtrdsvc wycmf ndrzh ekvdkm "
      L"bnnceq napak n Ko" +
      L"KomfgoU \ud83c\uded0\ud83c\udeee \n-->169 mopdhsp "
      L"\uda82\udca1\\T\udb22\udea8\ufa82C\"" +
      L"\u06d9\u0218 \u8708 \u18cd\u18c0\u18e8\u18fc\u18be\u18fd\u18c0 "
      L"yldjbofabwj \u1720\u1739" +
      L"\u1729 ([[m{1,5} blqaoru pvsvfall  ydsz "
      L"\ufd6f\ufce2\ufd4d\ufd07\ufde5\ufddc\ufb6c\ufbc9" +
      L"\ufd14\ufc4f\ufd05 \u216b\u218a\u2152\u2172\u217d\u2181\u2188 savpwhs "
      L"{1,5}f[ha-y[) xnzz " +
      L"gksck \u783a\u517a\u513e\u7355\u8741 kicgsn "
      L"\u3117\u311c\u3104\u310c\u312e\u3104\u3103 " +
      L"\u0291\u430b\uc9bfd\ue6e1\uf2d6~0 \ud802\udd38 "
      L"\ub2cd\uca67\u1c0d\u034c\uf3e2 \u03a2\u0009" +
      L"\uda96\udfde \u0010\ufb41\u06dd\u06d0\ue4ef\u241b "
      L"\ue1a3d\ub55d=\ud8fd\udd54\ueb5f\ud844" +
      L"\udf25 xnygolayn txnlsggei yhn \u0e5c\u0e02 \\ fornos oe epp ";

  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass5>(shared_from_this());
  for (int i = 0; i < 25; i++) {
    checkAnalysisConsistency(random(), a, false, text);
  }
  delete a;
}

TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass5::
    AnalyzerAnonymousInnerClass5(
        shared_ptr<TestICUNormalizer2CharFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false));
}

shared_ptr<Reader>
TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass5::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<ICUNormalizer2CharFilter>(
      reader, Normalizer2::getInstance(nullptr, L"nfkc_cf",
                                       Normalizer2::Mode::COMPOSE));
}

void TestICUNormalizer2CharFilter::testVeryLargeInputOfNonInertChars() throw(
    runtime_error)
{
  std::deque<wchar_t> text(1000000);
  Arrays::fill(text, L'a');
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (org.apache.lucene.analysis.Analyzer a = new
  // org.apache.lucene.analysis.Analyzer()
  {
    org::apache::lucene::analysis::Analyzer a =
        AnalyzerAnonymousInnerClass6(shared_from_this());
    checkAnalysisConsistency(random(), a, false, wstring(text));
  }
}

TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass6::
    AnalyzerAnonymousInnerClass6(
        shared_ptr<TestICUNormalizer2CharFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass6::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<KeywordTokenizer>());
}

shared_ptr<Reader>
TestICUNormalizer2CharFilter::AnalyzerAnonymousInnerClass6::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<ICUNormalizer2CharFilter>(
      reader, Normalizer2::getInstance(nullptr, L"nfkc_cf",
                                       Normalizer2::Mode::COMPOSE));
}
} // namespace org::apache::lucene::analysis::icu