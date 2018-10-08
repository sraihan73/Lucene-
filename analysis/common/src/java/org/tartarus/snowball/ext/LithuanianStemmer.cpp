using namespace std;

#include "LithuanianStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    LithuanianStemmer::methodObject =
        java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    LithuanianStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"a", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ia", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eria", 1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"osna", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iosna", 3, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uosna", 3, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iuosna", 5, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ysna", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117sna", 0, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"e", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ie", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enie", 10, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erie", 10, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oje", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ioje", 13, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uje", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iuje", 15, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"yje", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enyje", 17, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eryje", 17, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117je", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ame", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iame", 21, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sime", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ome", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117me", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tum\u0117me", 25, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ose", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iose", 27, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uose", 27, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iuose", 29, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"yse", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enyse", 31, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eryse", 31, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117se", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ate", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iate", 35, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ite", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kite", 37, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"site", 37, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ote", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tute", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117te", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tum\u0117te", 42, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ai", 44, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iai", 45, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eriai", 46, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ei", 44, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tumei", 48, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ki", 44, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imi", 44, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erimi", 51, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"umi", 44, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iumi", 53, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"si", 44, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asi", 55, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iasi", 56, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esi", 55, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iesi", 58, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"siesi", 59, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isi", 55, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aisi", 61, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eisi", 61, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tumeisi", 63, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uisi", 61, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"osi", 55, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117josi", 66, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uosi", 66, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iuosi", 68, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"siuosi", 69, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"usi", 55, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ausi", 71, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u010Diausi", 72, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0105si", 55, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117si", 55, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0173si", 55, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u0173si", 76, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ti", 44, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enti", 78, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"inti", 78, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oti", 78, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ioti", 81, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uoti", 81, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iuoti", 83, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"auti", 78, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iauti", 85, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"yti", 78, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117ti", 78, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tel\u0117ti", 88, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"in\u0117ti", 88, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ter\u0117ti", 88, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ui", 44, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iui", 92, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eniui", 93, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oj", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117j", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"k", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"am", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iam", 98, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iem", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"im", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sim", 101, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"om", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tum", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117m", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tum\u0117m", 105, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"an", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"on", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ion", 108, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"un", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iun", 110, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117n", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"o", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"io", 113, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enio", 114, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117jo", 113, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uo", 113, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"s", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"as", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ias", 119, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"es", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ies", 121, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"is", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ais", 123, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iais", 124, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tumeis", 123, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imis", 123, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enimis", 127, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"omis", 123, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iomis", 129, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"umis", 123, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117mis", 123, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enis", 123, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asis", 123, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ysis", 123, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ams", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iams", 136, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iems", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ims", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enims", 139, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erims", 139, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oms", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ioms", 142, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ums", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117ms", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ens", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"os", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ios", 147, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uos", 147, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iuos", 149, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ers", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"us", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aus", 152, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iaus", 153, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ius", 152, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ys", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enys", 156, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erys", 156, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"om\u00C4\u0097s", 118, -1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ot\u00C4\u0097s", 118, -1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0105s", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u0105s", 161, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117s", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"am\u0117s", 163, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iam\u0117s", 164, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"im\u0117s", 163, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"kim\u0117s", 166, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sim\u0117s", 166, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"om\u0117s", 163, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117m\u0117s", 163, -1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tum\u0117m\u0117s", 170,
                                                    -1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"at\u0117s", 163, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iat\u0117s", 172, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sit\u0117s", 163, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ot\u0117s", 163, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117t\u0117s", 163, -1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tum\u0117t\u0117s", 176,
                                                    -1, L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u012Fs", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u016Bs", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"t\u0173s", 118, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"at", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iat", 181, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"it", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sit", 183, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ot", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117t", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tum\u0117t", 186, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"u", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"au", 188, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iau", 189, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u010Diau", 190, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iu", 188, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eniu", 192, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"siu", 192, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"y", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0105", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u0105", 196, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0119", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u012F", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"en\u012F", 200, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u012F", 200, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0173", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u0173", 203, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u0173", 203, -1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    LithuanianStemmer::a_1 = {
        make_shared<org::tartarus::snowball::Among>(L"ing", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aj", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iaj", 1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iej", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oj", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ioj", 4, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uoj", 4, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iuoj", 6, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"auj", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0105j", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u0105j", 9, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117j", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0173j", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u0173j", 12, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ok", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iok", 14, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iuk", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uliuk", 16, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"u\u010Diuk", 16, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u0161k", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iul", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"yl", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117l", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"am", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dam", 23, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"jam", 23, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"zgan", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ain", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esn", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"op", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iop", 29, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ias", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ies", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ais", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iais", 33, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"os", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ios", 35, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uos", 35, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iuos", 37, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aus", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iaus", 39, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0105s", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u0105s", 41, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0119s", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ut\u0117ait", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ant", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iant", 45, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"siant", 46, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"int", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ot", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uot", 49, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iuot", 50, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"yt", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117t", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"yk\u0161t", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iau", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dav", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sv", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0161v", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"yk\u0161\u010D", -1, -1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0119", -1, -1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117j\u0119", 60, -1,
                                                    L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    LithuanianStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"ojime", -1, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u0117jime", -1, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"avime", -1, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"okate", -1, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aite", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uote", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asius", -1, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"okat\u0117s", -1, 10, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ait\u0117s", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uot\u0117s", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esiu", -1, 6, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    LithuanianStemmer::a_3 = {make_shared<org::tartarus::snowball::Among>(
                                  L"\u010D", -1, 1, L"", methodObject),
                              make_shared<org::tartarus::snowball::Among>(
                                  L"d\u017E", -1, 2, L"", methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    LithuanianStemmer::a_4 = {make_shared<org::tartarus::snowball::Among>(
        L"gd", -1, 1, L"", methodObject)};
std::deque<wchar_t> const LithuanianStemmer::g_v = {
    17, 65, 16, 1, 0,  0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  16, 0, 64, 1, 0, 64, 0, 0, 0, 0, 0, 0, 0, 4, 4};

void LithuanianStemmer::copy_from(shared_ptr<LithuanianStemmer> other)
{
  B_CHANGE = other->B_CHANGE;
  I_s = other->I_s;
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  org::tartarus::snowball::SnowballProgram::copy_from(other);
}

bool LithuanianStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool LithuanianStemmer::r_step1()
{
  int v_1;
  int v_2;
  // (, line 48
  // setlimit, line 49
  v_1 = limit - cursor;
  // tomark, line 49
  if (cursor < I_p1) {
    return false;
  }
  cursor = I_p1;
  v_2 = limit_backward;
  limit_backward = cursor;
  cursor = limit - v_1;
  // (, line 49
  // [, line 49
  ket = cursor;
  // substring, line 49
  if (find_among_b(a_0, 206) == 0) {
    limit_backward = v_2;
    return false;
  }
  // ], line 49
  bra = cursor;
  limit_backward = v_2;
  // call R1, line 49
  if (!r_R1()) {
    return false;
  }
  // delete, line 235
  slice_del();
  return true;
}

bool LithuanianStemmer::r_step2()
{
  int v_1;
  int v_2;
  int v_3;
  // repeat, line 238
  while (true) {
    v_1 = limit - cursor;
    do {
      // (, line 238
      // setlimit, line 239
      v_2 = limit - cursor;
      // tomark, line 239
      if (cursor < I_p1) {
        goto lab1Break;
      }
      cursor = I_p1;
      v_3 = limit_backward;
      limit_backward = cursor;
      cursor = limit - v_2;
      // (, line 239
      // [, line 239
      ket = cursor;
      // substring, line 239
      if (find_among_b(a_1, 62) == 0) {
        limit_backward = v_3;
        goto lab1Break;
      }
      // ], line 239
      bra = cursor;
      limit_backward = v_3;
      // delete, line 309
      slice_del();
      goto replab0Continue;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = limit - v_1;
    goto replab0Break;
  replab0Continue:;
  }
replab0Break:
  return true;
}

bool LithuanianStemmer::r_fix_conflicts()
{
  int among_var;
  // (, line 312
  // [, line 313
  ket = cursor;
  // substring, line 313
  among_var = find_among_b(a_2, 11);
  if (among_var == 0) {
    return false;
  }
  // ], line 313
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 315
    // <-, line 315
    slice_from(L"ait\u0117");
    // set CHANGE, line 315
    B_CHANGE = true;
    break;
  case 2:
    // (, line 317
    // <-, line 317
    slice_from(L"ait\u0117");
    // set CHANGE, line 317
    B_CHANGE = true;
    break;
  case 3:
    // (, line 320
    // <-, line 320
    slice_from(L"uot\u0117");
    // set CHANGE, line 320
    B_CHANGE = true;
    break;
  case 4:
    // (, line 322
    // <-, line 322
    slice_from(L"uot\u0117");
    // set CHANGE, line 322
    B_CHANGE = true;
    break;
  case 5:
    // (, line 325
    // <-, line 325
    slice_from(L"\u0117jimas");
    // set CHANGE, line 325
    B_CHANGE = true;
    break;
  case 6:
    // (, line 328
    // <-, line 328
    slice_from(L"esys");
    // set CHANGE, line 328
    B_CHANGE = true;
    break;
  case 7:
    // (, line 330
    // <-, line 330
    slice_from(L"asys");
    // set CHANGE, line 330
    B_CHANGE = true;
    break;
  case 8:
    // (, line 334
    // <-, line 334
    slice_from(L"avimas");
    // set CHANGE, line 334
    B_CHANGE = true;
    break;
  case 9:
    // (, line 335
    // <-, line 335
    slice_from(L"ojimas");
    // set CHANGE, line 335
    B_CHANGE = true;
    break;
  case 10:
    // (, line 338
    // <-, line 338
    slice_from(L"okat\u0117");
    // set CHANGE, line 338
    B_CHANGE = true;
    break;
  case 11:
    // (, line 340
    // <-, line 340
    slice_from(L"okat\u0117");
    // set CHANGE, line 340
    B_CHANGE = true;
    break;
  }
  return true;
}

bool LithuanianStemmer::r_fix_chdz()
{
  int among_var;
  // (, line 346
  // [, line 347
  ket = cursor;
  // substring, line 347
  among_var = find_among_b(a_3, 2);
  if (among_var == 0) {
    return false;
  }
  // ], line 347
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 348
    // <-, line 348
    slice_from(L"t");
    // set CHANGE, line 348
    B_CHANGE = true;
    break;
  case 2:
    // (, line 349
    // <-, line 349
    slice_from(L"d");
    // set CHANGE, line 349
    B_CHANGE = true;
    break;
  }
  return true;
}

bool LithuanianStemmer::r_fix_gd()
{
  int among_var;
  // (, line 353
  // [, line 354
  ket = cursor;
  // substring, line 354
  among_var = find_among_b(a_4, 1);
  if (among_var == 0) {
    return false;
  }
  // ], line 354
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 355
    // <-, line 355
    slice_from(L"g");
    // set CHANGE, line 355
    B_CHANGE = true;
    break;
  }
  return true;
}

bool LithuanianStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_8;
  int v_9;
  int v_10;
  int v_11;
  int v_12;
  int v_13;
  // (, line 362
  I_p1 = limit;
  I_p2 = limit;
  I_s = (getCurrent().length());
  // do, line 368
  v_1 = cursor;
  do {
    // (, line 368
    // try, line 370
    v_2 = cursor;
    do {
      // (, line 370
      // test, line 370
      v_3 = cursor;
      // literal, line 370
      if (!(eq_s(1, L"a"))) {
        cursor = v_2;
        goto lab1Break;
      }
      cursor = v_3;
      if (!(I_s > 6)) {
        cursor = v_2;
        goto lab1Break;
      }
      {
        // hop, line 370
        int c = cursor + 1;
        if (0 > c || c > limit) {
          cursor = v_2;
          goto lab1Break;
        }
        cursor = c;
      }
    } while (false);
  lab1Continue:;
  lab1Break:
    // gopast, line 372
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 371))) {
          goto lab3Break;
        }
        goto golab2Break;
      } while (false);
    lab3Continue:;
    lab3Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab2Continue:;
    }
  golab2Break:
    // gopast, line 372
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 371))) {
          goto lab5Break;
        }
        goto golab4Break;
      } while (false);
    lab5Continue:;
    lab5Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab4Continue:;
    }
  golab4Break:
    // setmark p1, line 372
    I_p1 = cursor;
    // gopast, line 373
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 371))) {
          goto lab7Break;
        }
        goto golab6Break;
      } while (false);
    lab7Continue:;
    lab7Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab6Continue:;
    }
  golab6Break:
    // gopast, line 373
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 371))) {
          goto lab9Break;
        }
        goto golab8Break;
      } while (false);
    lab9Continue:;
    lab9Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab8Continue:;
    }
  golab8Break:
    // setmark p2, line 373
    I_p2 = cursor;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // backwards, line 377
  limit_backward = cursor;
  cursor = limit;
  // (, line 377
  // do, line 378
  v_8 = limit - cursor;
  do {
    // call fix_conflicts, line 378
    if (!r_fix_conflicts()) {
      goto lab10Break;
    }
  } while (false);
lab10Continue:;
lab10Break:
  cursor = limit - v_8;
  // do, line 379
  v_9 = limit - cursor;
  do {
    // call step1, line 379
    if (!r_step1()) {
      goto lab11Break;
    }
  } while (false);
lab11Continue:;
lab11Break:
  cursor = limit - v_9;
  // do, line 380
  v_10 = limit - cursor;
  do {
    // call fix_chdz, line 380
    if (!r_fix_chdz()) {
      goto lab12Break;
    }
  } while (false);
lab12Continue:;
lab12Break:
  cursor = limit - v_10;
  // do, line 381
  v_11 = limit - cursor;
  do {
    // call step2, line 381
    if (!r_step2()) {
      goto lab13Break;
    }
  } while (false);
lab13Continue:;
lab13Break:
  cursor = limit - v_11;
  // do, line 382
  v_12 = limit - cursor;
  do {
    // call fix_chdz, line 382
    if (!r_fix_chdz()) {
      goto lab14Break;
    }
  } while (false);
lab14Continue:;
lab14Break:
  cursor = limit - v_12;
  // do, line 383
  v_13 = limit - cursor;
  do {
    // call fix_gd, line 383
    if (!r_fix_gd()) {
      goto lab15Break;
    }
  } while (false);
lab15Continue:;
lab15Break:
  cursor = limit - v_13;
  cursor = limit_backward;
  return true;
}

bool LithuanianStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<LithuanianStemmer>(o) != nullptr;
}

int LithuanianStemmer::hashCode()
{
  return LithuanianStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext