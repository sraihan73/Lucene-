using namespace std;

#include "TestToStringUtil.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ja/util/ToStringUtil.h"

namespace org::apache::lucene::analysis::ja::util
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestToStringUtil::testPOS()
{
  assertEquals(L"noun-suffix-verbal",
               ToStringUtil::getPOSTranslation(L"名詞-接尾-サ変接続"));
}

void TestToStringUtil::testHepburn()
{
  assertEquals(L"majan", ToStringUtil::getRomanization(L"マージャン"));
  assertEquals(L"uroncha", ToStringUtil::getRomanization(L"ウーロンチャ"));
  assertEquals(L"chahan", ToStringUtil::getRomanization(L"チャーハン"));
  assertEquals(L"chashu", ToStringUtil::getRomanization(L"チャーシュー"));
  assertEquals(L"shumai", ToStringUtil::getRomanization(L"シューマイ"));
}

void TestToStringUtil::testHepburnTable()
{
  unordered_map<wstring, wstring> table =
      make_shared<HashMapAnonymousInnerClass>(shared_from_this());

  for (auto s : table) {
    assertEquals(s.first, table[s.first],
                 ToStringUtil::getRomanization(s.first));
  }
}

TestToStringUtil::HashMapAnonymousInnerClass::HashMapAnonymousInnerClass(
    shared_ptr<TestToStringUtil> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"ア", L"a");
  this->put(L"イ", L"i");
  this->put(L"ウ", L"u");
  this->put(L"エ", L"e");
  this->put(L"オ", L"o");
  this->put(L"カ", L"ka");
  this->put(L"キ", L"ki");
  this->put(L"ク", L"ku");
  this->put(L"ケ", L"ke");
  this->put(L"コ", L"ko");
  this->put(L"サ", L"sa");
  this->put(L"シ", L"shi");
  this->put(L"ス", L"su");
  this->put(L"セ", L"se");
  this->put(L"ソ", L"so");
  this->put(L"タ", L"ta");
  this->put(L"チ", L"chi");
  this->put(L"ツ", L"tsu");
  this->put(L"テ", L"te");
  this->put(L"ト", L"to");
  this->put(L"ナ", L"na");
  this->put(L"ニ", L"ni");
  this->put(L"ヌ", L"nu");
  this->put(L"ネ", L"ne");
  this->put(L"ノ", L"no");
  this->put(L"ハ", L"ha");
  this->put(L"ヒ", L"hi");
  this->put(L"フ", L"fu");
  this->put(L"ヘ", L"he");
  this->put(L"ホ", L"ho");
  this->put(L"マ", L"ma");
  this->put(L"ミ", L"mi");
  this->put(L"ム", L"mu");
  this->put(L"メ", L"me");
  this->put(L"モ", L"mo");
  this->put(L"ヤ", L"ya");
  this->put(L"ユ", L"yu");
  this->put(L"ヨ", L"yo");
  this->put(L"ラ", L"ra");
  this->put(L"リ", L"ri");
  this->put(L"ル", L"ru");
  this->put(L"レ", L"re");
  this->put(L"ロ", L"ro");
  this->put(L"ワ", L"wa");
  this->put(L"ヰ", L"i");
  this->put(L"ヱ", L"e");
  this->put(L"ヲ", L"o");
  this->put(L"ン", L"n");
  this->put(L"ガ", L"ga");
  this->put(L"ギ", L"gi");
  this->put(L"グ", L"gu");
  this->put(L"ゲ", L"ge");
  this->put(L"ゴ", L"go");
  this->put(L"ザ", L"za");
  this->put(L"ジ", L"ji");
  this->put(L"ズ", L"zu");
  this->put(L"ゼ", L"ze");
  this->put(L"ゾ", L"zo");
  this->put(L"ダ", L"da");
  this->put(L"ヂ", L"ji");
  this->put(L"ヅ", L"zu");
  this->put(L"デ", L"de");
  this->put(L"ド", L"do");
  this->put(L"バ", L"ba");
  this->put(L"ビ", L"bi");
  this->put(L"ブ", L"bu");
  this->put(L"ベ", L"be");
  this->put(L"ボ", L"bo");
  this->put(L"パ", L"pa");
  this->put(L"ピ", L"pi");
  this->put(L"プ", L"pu");
  this->put(L"ペ", L"pe");
  this->put(L"ポ", L"po");

  this->put(L"キャ", L"kya");
  this->put(L"キュ", L"kyu");
  this->put(L"キョ", L"kyo");
  this->put(L"シャ", L"sha");
  this->put(L"シュ", L"shu");
  this->put(L"ショ", L"sho");
  this->put(L"チャ", L"cha");
  this->put(L"チュ", L"chu");
  this->put(L"チョ", L"cho");
  this->put(L"ニャ", L"nya");
  this->put(L"ニュ", L"nyu");
  this->put(L"ニョ", L"nyo");
  this->put(L"ヒャ", L"hya");
  this->put(L"ヒュ", L"hyu");
  this->put(L"ヒョ", L"hyo");
  this->put(L"ミャ", L"mya");
  this->put(L"ミュ", L"myu");
  this->put(L"ミョ", L"myo");
  this->put(L"リャ", L"rya");
  this->put(L"リュ", L"ryu");
  this->put(L"リョ", L"ryo");
  this->put(L"ギャ", L"gya");
  this->put(L"ギュ", L"gyu");
  this->put(L"ギョ", L"gyo");
  this->put(L"ジャ", L"ja");
  this->put(L"ジュ", L"ju");
  this->put(L"ジョ", L"jo");
  this->put(L"ヂャ", L"ja");
  this->put(L"ヂュ", L"ju");
  this->put(L"ヂョ", L"jo");
  this->put(L"ビャ", L"bya");
  this->put(L"ビュ", L"byu");
  this->put(L"ビョ", L"byo");
  this->put(L"ピャ", L"pya");
  this->put(L"ピュ", L"pyu");
  this->put(L"ピョ", L"pyo");

  this->put(L"イィ", L"yi");
  this->put(L"イェ", L"ye");
  this->put(L"ウァ", L"wa");
  this->put(L"ウィ", L"wi");
  this->put(L"ウゥ", L"wu");
  this->put(L"ウェ", L"we");
  this->put(L"ウォ", L"wo");
  this->put(L"ウュ", L"wyu");
  // TODO: really should be vu
  this->put(L"ヴァ", L"va");
  this->put(L"ヴィ", L"vi");
  this->put(L"ヴ", L"v");
  this->put(L"ヴェ", L"ve");
  this->put(L"ヴォ", L"vo");
  this->put(L"ヴャ", L"vya");
  this->put(L"ヴュ", L"vyu");
  this->put(L"ヴィェ", L"vye");
  this->put(L"ヴョ", L"vyo");
  this->put(L"キェ", L"kye");
  this->put(L"ギェ", L"gye");
  this->put(L"クァ", L"kwa");
  this->put(L"クィ", L"kwi");
  this->put(L"クェ", L"kwe");
  this->put(L"クォ", L"kwo");
  this->put(L"クヮ", L"kwa");
  this->put(L"グァ", L"gwa");
  this->put(L"グィ", L"gwi");
  this->put(L"グェ", L"gwe");
  this->put(L"グォ", L"gwo");
  this->put(L"グヮ", L"gwa");
  this->put(L"シェ", L"she");
  this->put(L"ジェ", L"je");
  this->put(L"スィ", L"si");
  this->put(L"ズィ", L"zi");
  this->put(L"チェ", L"che");
  this->put(L"ツァ", L"tsa");
  this->put(L"ツィ", L"tsi");
  this->put(L"ツェ", L"tse");
  this->put(L"ツォ", L"tso");
  this->put(L"ツュ", L"tsyu");
  this->put(L"ティ", L"ti");
  this->put(L"トゥ", L"tu");
  this->put(L"テュ", L"tyu");
  this->put(L"ディ", L"di");
  this->put(L"ドゥ", L"du");
  this->put(L"デュ", L"dyu");
  this->put(L"ニェ", L"nye");
  this->put(L"ヒェ", L"hye");
  this->put(L"ビェ", L"bye");
  this->put(L"ピェ", L"pye");
  this->put(L"ファ", L"fa");
  this->put(L"フィ", L"fi");
  this->put(L"フェ", L"fe");
  this->put(L"フォ", L"fo");
  this->put(L"フャ", L"fya");
  this->put(L"フュ", L"fyu");
  this->put(L"フィェ", L"fye");
  this->put(L"フョ", L"fyo");
  this->put(L"ホゥ", L"hu");
  this->put(L"ミェ", L"mye");
  this->put(L"リェ", L"rye");
  this->put(L"ラ゜", L"la");
  this->put(L"リ゜", L"li");
  this->put(L"ル゜", L"lu");
  this->put(L"レ゜", L"le");
  this->put(L"ロ゜", L"lo");
  this->put(L"ヷ", L"va");
  this->put(L"ヸ", L"vi");
  this->put(L"ヹ", L"ve");
  this->put(L"ヺ", L"vo");
}
} // namespace org::apache::lucene::analysis::ja::util