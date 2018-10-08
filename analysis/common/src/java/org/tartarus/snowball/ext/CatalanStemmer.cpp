using namespace std;

#include "CatalanStemmer.h"
#include "../Among.h"

namespace org::tartarus::snowball::ext
{
using Among = org::tartarus::snowball::Among;
using SnowballProgram = org::tartarus::snowball::SnowballProgram;
const shared_ptr<java::lang::invoke::MethodHandles::Lookup>
    CatalanStemmer::methodObject = java::lang::invoke::MethodHandles::lookup();
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    CatalanStemmer::a_0 = {
        make_shared<org::tartarus::snowball::Among>(L"", -1, 13, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00B7", 0, 12, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0", 0, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E8", 0, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9", 0, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EC", 0, 6, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00ED", 0, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EF", 0, 11, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F2", 0, 8, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F3", 0, 7, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00FA", 0, 9, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00FC", 0, 10, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    CatalanStemmer::a_1 = {
        make_shared<org::tartarus::snowball::Among>(L"la", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"-la", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sela", 0, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"le", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"me", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"-me", 4, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"se", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"-te", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"hi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"'hi", 8, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"li", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"-li", 10, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"'l", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"'m", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"-m", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"'n", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"-n", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ho", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"'ho", 17, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"lo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"selo", 19, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"'s", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"las", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"selas", 22, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"les", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"-les", 24, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"'ls", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"-ls", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"'ns", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"-ns", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ens", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"los", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"selos", 31, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"-nos", 33, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"vos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"us", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"-us", 36, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"'t", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    CatalanStemmer::a_2 = {
        make_shared<org::tartarus::snowball::Among>(L"ica", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"l\u00F3gica", 0, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enca", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ada", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ancia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"encia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E8ncia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDcia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"logia", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"inia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDinia", 9, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eria", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0ria", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"at\u00F2ria", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alla", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ella", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDvola", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ima", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDssima", 17, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"qu\u00EDssima", 18, 5,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ana", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ina", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"era", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sfera", 22, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ora", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dora", 24, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"adora", 25, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"adura", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"osa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"essa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eta", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ita", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ota", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ista", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ialista", 36, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ionista", 36, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iva", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ativa", 39, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"n\u00E7a", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"log\u00EDa", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ic", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDstic", 43, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"enc", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esc", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ud", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atge", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ble", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"able", 49, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ible", 49, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isme", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ialisme", 52, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ionisme", 52, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivisme", 52, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aire", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icte", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iste", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ici", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDci", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"logi", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ari", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tori", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"al", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"il", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"all", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ell", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDvol", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isam", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issem", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00ECssem", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDssem", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDssim", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"qu\u00EDssim", 73, 5, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"amen", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00ECssin", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ificar", 77, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"egar", 77, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ejar", 77, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itar", 77, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itzar", 77, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"fer", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"or", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dor", 84, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dur", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"doras", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ics", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"l\u00F3gics", 88, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uds", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"nces", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ades", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ancies", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"encies", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E8ncies", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDcies", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"logies", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"inies", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDnies", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eries", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0ries", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"at\u00F2ries", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bles", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ables", 103, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ibles", 103, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDssimes", 106, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"qu\u00EDssimes", 107, 5,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"formes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ismes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ialismes", 110, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ines", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eres", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ores", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dores", 114, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"idores", 115, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dures", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eses", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"oses", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asses", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ictes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ites", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"otes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"istes", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ialistes", 124, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ionistes", 124, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iques", -1, 4, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"l\u00F3giques", 127, 3,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ives", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atives", 129, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"log\u00EDes", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"alleng\u00FCes", -1, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"icis", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDcis", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"logis", -1, 3, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aris", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"toris", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ls", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"als", 138, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ells", 138, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ims", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDssims", 141, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"qu\u00EDssims", 142, 5,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ions", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"cions", 144, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"acions", 145, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"osos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ers", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ors", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dors", 152, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"adors", 153, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"idors", 153, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ats", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itats", 156, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bilitats", 157, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivitats", 157, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ativitats", 159, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFtats", 156, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ets", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ants", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ents", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ments", 164, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aments", 165, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ots", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uts", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ius", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"trius", 169, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atius", 169, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E8s", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9s", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDs", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"d\u00EDs", 174, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F3s", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itat", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"bilitat", 177, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ivitat", 177, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ativitat", 179, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFtat", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"et", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ant", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ent", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ient", 184, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ment", 184, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ament", 186, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isament", 187, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ot", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isseu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00ECsseu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDsseu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"triu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDssiu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atiu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00F3", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u00F3", 196, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ci\u00F3", 197, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aci\u00F3", 198, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<org::tartarus::snowball::Among>> const
    CatalanStemmer::a_3 = {
        make_shared<org::tartarus::snowball::Among>(L"aba", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esca", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isca", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFsca", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ada", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ida", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"uda", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFda", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ia", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aria", 8, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iria", 8, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ara", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iera", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ira", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"adora", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFra", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ava", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ixa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itza", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00EDa", 19, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00EDa", 19, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00EDa", 19, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFa", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isc", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFsc", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ad", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ed", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"id", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ie", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"re", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"dre", 30, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ase", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iese", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aste", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iste", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ii", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ini", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esqui", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eixi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itzi", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"am", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"em", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0rem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDrem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0ssem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9ssem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iguem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFguem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"avem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0vem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1vem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00ECem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDem", 42, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00EDem", 55, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00EDem", 55, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assim", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"essim", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issim", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0ssim", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E8ssim", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9ssim", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDssim", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFm", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"an", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aban", 66, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arian", 66, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aran", 66, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ieran", 66, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iran", 66, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDan", 66, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00EDan", 72, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00EDan", 72, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00EDan", 72, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"en", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ien", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arien", 77, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"irien", 77, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aren", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eren", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iren", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0ren", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFren", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iesen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"essen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9ssen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFssen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esquen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isquen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFsquen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aven", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ixen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eixen", 96, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFxen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFen", 76, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"in", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"inin", 100, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sin", 100, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isin", 102, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assin", 102, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"essin", 102, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issin", 102, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFssin", 102, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esquin", 100, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eixin", 100, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aron", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ieron", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00E1n", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00E1n", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00E1n", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u00EFn", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ado", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ido", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ando", -1, 2, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iendo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"io", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ixo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eixo", 121, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFxo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itzo", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"tzar", 125, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eixer", 127, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ador", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"as", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abas", 131, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"adas", 131, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"idas", 131, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aras", 131, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ieras", 131, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDas", 131, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00EDas", 137, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00EDas", 137, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00EDas", 137, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ids", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"es", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ades", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ides", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"udes", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFdes", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"atges", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ies", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aries", 148, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iries", 148, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ares", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ires", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"adores", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFres", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ases", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ieses", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asses", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esses", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isses", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFsses", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ques", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esques", 161, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFsques", 161, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aves", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ixes", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eixes", 165, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFxes", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFes", 142, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"abais", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"arais", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ierais", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDais", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00EDais", 172, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00EDais", 172, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00EDais", 172, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aseis", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ieseis", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asteis", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isteis", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"inis", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"sis", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"isis", 181, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assis", 181, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"essis", 181, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issis", 181, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFssis", 181, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esquis", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eixis", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itzis", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1is", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00E9is", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00E9is", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00E9is", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ams", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ados", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"idos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"amos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1bamos", 197, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1ramos", 197, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u00E9ramos", 197, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDamos", 197, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00EDamos", 201, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00EDamos", 201, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00EDamos", 201, 1,
                                                    L"", methodObject),
        make_shared<org::tartarus::snowball::Among>(L"aremos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eremos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"iremos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1semos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u00E9semos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"imos", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"adors", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ass", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erass", 212, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ess", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ats", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"its", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ents", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0s", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00E0s", 218, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00E0s", 218, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00E1s", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00E1s", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00E1s", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9s", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00E9s", 224, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDs", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u00EFs", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"at", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"it", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ant", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ent", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"int", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ut", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFt", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"au", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"erau", 235, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ieu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ineu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"areu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ireu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0reu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDreu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"asseu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"esseu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eresseu", 244, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0sseu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9sseu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"igueu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFgueu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0veu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E1veu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itzeu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00ECeu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00ECeu", 253, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDeu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00EDeu", 255, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00EDeu", 255, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"assiu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"issiu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E0ssiu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E8ssiu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00E9ssiu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EDssiu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFu", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ix", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"eix", 265, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00EFx", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itz", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u00E0", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00E0", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00E0", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"itz\u00E0", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00E1", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00E1", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00E1", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00E8", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ar\u00E9", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"er\u00E9", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"ir\u00E9", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"\u00ED", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u00EF", -1, 1, L"",
                                                    methodObject),
        make_shared<org::tartarus::snowball::Among>(L"i\u00F3", -1, 1, L"",
                                                    methodObject)};
std::deque<std::shared_ptr<
    org::tartarus::snowball::Among>> const CatalanStemmer::a_4 = {
    make_shared<org::tartarus::snowball::Among>(L"a", -1, 1, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"e", -1, 1, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"i", -1, 1, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"\u00EFn", -1, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"o", -1, 1, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"ir", -1, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"s", -1, 1, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"is", 6, 1, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"os", 6, 1, L"", methodObject),
    make_shared<org::tartarus::snowball::Among>(L"\u00EFs", 6, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"it", -1, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"eu", -1, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"iu", -1, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"iqu", -1, 2, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"itz", -1, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"\u00E0", -1, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"\u00E1", -1, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"\u00E9", -1, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"\u00EC", -1, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"\u00ED", -1, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"\u00EF", -1, 1, L"",
                                                methodObject),
    make_shared<org::tartarus::snowball::Among>(L"\u00F3", -1, 1, L"",
                                                methodObject)};
std::deque<wchar_t> const CatalanStemmer::g_v = {
    17, 65, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 128, 129, 81, 6, 10};

void CatalanStemmer::copy_from(shared_ptr<CatalanStemmer> other)
{
  I_p2 = other->I_p2;
  I_p1 = other->I_p1;
  SnowballProgram::copy_from(other);
}

bool CatalanStemmer::r_mark_regions()
{
  int v_1;
  // (, line 41
  I_p1 = limit;
  I_p2 = limit;
  // do, line 46
  v_1 = cursor;
  do {
    // (, line 46
    // gopast, line 47
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 252))) {
          goto lab2Break;
        }
        goto golab1Break;
      } while (false);
    lab2Continue:;
    lab2Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab1Continue:;
    }
  golab1Break:
    // gopast, line 47
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 252))) {
          goto lab4Break;
        }
        goto golab3Break;
      } while (false);
    lab4Continue:;
    lab4Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab3Continue:;
    }
  golab3Break:
    // setmark p1, line 47
    I_p1 = cursor;
    // gopast, line 48
    while (true) {
      do {
        if (!(in_grouping(g_v, 97, 252))) {
          goto lab6Break;
        }
        goto golab5Break;
      } while (false);
    lab6Continue:;
    lab6Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab5Continue:;
    }
  golab5Break:
    // gopast, line 48
    while (true) {
      do {
        if (!(out_grouping(g_v, 97, 252))) {
          goto lab8Break;
        }
        goto golab7Break;
      } while (false);
    lab8Continue:;
    lab8Break:
      if (cursor >= limit) {
        goto lab0Break;
      }
      cursor++;
    golab7Continue:;
    }
  golab7Break:
    // setmark p2, line 48
    I_p2 = cursor;
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  return true;
}

bool CatalanStemmer::r_cleaning()
{
  int among_var;
  int v_1;
  // repeat, line 52
  while (true) {
    v_1 = cursor;
    do {
      // (, line 52
      // [, line 53
      bra = cursor;
      // substring, line 53
      among_var = find_among(a_0, 13);
      if (among_var == 0) {
        goto lab1Break;
      }
      // ], line 53
      ket = cursor;
      switch (among_var) {
      case 0:
        goto lab1Break;
      case 1:
        // (, line 54
        // <-, line 54
        slice_from(L"a");
        break;
      case 2:
        // (, line 55
        // <-, line 55
        slice_from(L"a");
        break;
      case 3:
        // (, line 56
        // <-, line 56
        slice_from(L"e");
        break;
      case 4:
        // (, line 57
        // <-, line 57
        slice_from(L"e");
        break;
      case 5:
        // (, line 58
        // <-, line 58
        slice_from(L"i");
        break;
      case 6:
        // (, line 59
        // <-, line 59
        slice_from(L"i");
        break;
      case 7:
        // (, line 60
        // <-, line 60
        slice_from(L"o");
        break;
      case 8:
        // (, line 61
        // <-, line 61
        slice_from(L"o");
        break;
      case 9:
        // (, line 62
        // <-, line 62
        slice_from(L"u");
        break;
      case 10:
        // (, line 63
        // <-, line 63
        slice_from(L"u");
        break;
      case 11:
        // (, line 64
        // <-, line 64
        slice_from(L"i");
        break;
      case 12:
        // (, line 65
        // <-, line 65
        slice_from(L".");
        break;
      case 13:
        // (, line 66
        // next, line 66
        if (cursor >= limit) {
          goto lab1Break;
        }
        cursor++;
        break;
      }
      goto replab0Continue;
    } while (false);
  lab1Continue:;
  lab1Break:
    cursor = v_1;
    goto replab0Break;
  replab0Continue:;
  }
replab0Break:
  return true;
}

bool CatalanStemmer::r_R1()
{
  if (!(I_p1 <= cursor)) {
    return false;
  }
  return true;
}

bool CatalanStemmer::r_R2()
{
  if (!(I_p2 <= cursor)) {
    return false;
  }
  return true;
}

bool CatalanStemmer::r_attached_pronoun()
{
  int among_var;
  // (, line 75
  // [, line 76
  ket = cursor;
  // substring, line 76
  among_var = find_among_b(a_1, 39);
  if (among_var == 0) {
    return false;
  }
  // ], line 76
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 86
    // call R1, line 86
    if (!r_R1()) {
      return false;
    }
    // delete, line 86
    slice_del();
    break;
  }
  return true;
}

bool CatalanStemmer::r_standard_suffix()
{
  int among_var;
  // (, line 90
  // [, line 91
  ket = cursor;
  // substring, line 91
  among_var = find_among_b(a_2, 200);
  if (among_var == 0) {
    return false;
  }
  // ], line 91
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 115
    // call R1, line 115
    if (!r_R1()) {
      return false;
    }
    // delete, line 115
    slice_del();
    break;
  case 2:
    // (, line 117
    // call R2, line 117
    if (!r_R2()) {
      return false;
    }
    // delete, line 117
    slice_del();
    break;
  case 3:
    // (, line 119
    // call R2, line 119
    if (!r_R2()) {
      return false;
    }
    // <-, line 119
    slice_from(L"log");
    break;
  case 4:
    // (, line 121
    // call R2, line 121
    if (!r_R2()) {
      return false;
    }
    // <-, line 121
    slice_from(L"ic");
    break;
  case 5:
    // (, line 123
    // call R1, line 123
    if (!r_R1()) {
      return false;
    }
    // <-, line 123
    slice_from(L"c");
    break;
  }
  return true;
}

bool CatalanStemmer::r_verb_suffix()
{
  int among_var;
  // (, line 127
  // [, line 128
  ket = cursor;
  // substring, line 128
  among_var = find_among_b(a_3, 283);
  if (among_var == 0) {
    return false;
  }
  // ], line 128
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 173
    // call R1, line 173
    if (!r_R1()) {
      return false;
    }
    // delete, line 173
    slice_del();
    break;
  case 2:
    // (, line 175
    // call R2, line 175
    if (!r_R2()) {
      return false;
    }
    // delete, line 175
    slice_del();
    break;
  }
  return true;
}

bool CatalanStemmer::r_residual_suffix()
{
  int among_var;
  // (, line 179
  // [, line 180
  ket = cursor;
  // substring, line 180
  among_var = find_among_b(a_4, 22);
  if (among_var == 0) {
    return false;
  }
  // ], line 180
  bra = cursor;
  switch (among_var) {
  case 0:
    return false;
  case 1:
    // (, line 183
    // call R1, line 183
    if (!r_R1()) {
      return false;
    }
    // delete, line 183
    slice_del();
    break;
  case 2:
    // (, line 185
    // call R1, line 185
    if (!r_R1()) {
      return false;
    }
    // <-, line 185
    slice_from(L"ic");
    break;
  }
  return true;
}

bool CatalanStemmer::stem()
{
  int v_1;
  int v_2;
  int v_3;
  int v_4;
  int v_5;
  int v_6;
  // (, line 190
  // do, line 191
  v_1 = cursor;
  do {
    // call mark_regions, line 191
    if (!r_mark_regions()) {
      goto lab0Break;
    }
  } while (false);
lab0Continue:;
lab0Break:
  cursor = v_1;
  // backwards, line 192
  limit_backward = cursor;
  cursor = limit;
  // (, line 192
  // do, line 193
  v_2 = limit - cursor;
  do {
    // call attached_pronoun, line 193
    if (!r_attached_pronoun()) {
      goto lab1Break;
    }
  } while (false);
lab1Continue:;
lab1Break:
  cursor = limit - v_2;
  // do, line 194
  v_3 = limit - cursor;
  do {
    // (, line 194
    // or, line 194
    do {
      v_4 = limit - cursor;
      do {
        // call standard_suffix, line 194
        if (!r_standard_suffix()) {
          goto lab4Break;
        }
        goto lab3Break;
      } while (false);
    lab4Continue:;
    lab4Break:
      cursor = limit - v_4;
      // call verb_suffix, line 195
      if (!r_verb_suffix()) {
        goto lab2Break;
      }
    } while (false);
  lab3Continue:;
  lab3Break:;
  } while (false);
lab2Continue:;
lab2Break:
  cursor = limit - v_3;
  // do, line 197
  v_5 = limit - cursor;
  do {
    // call residual_suffix, line 197
    if (!r_residual_suffix()) {
      goto lab5Break;
    }
  } while (false);
lab5Continue:;
lab5Break:
  cursor = limit - v_5;
  cursor = limit_backward; // do, line 199
  v_6 = cursor;
  do {
    // call cleaning, line 199
    if (!r_cleaning()) {
      goto lab6Break;
    }
  } while (false);
lab6Continue:;
lab6Break:
  cursor = v_6;
  return true;
}

bool CatalanStemmer::equals(any o)
{
  return std::dynamic_pointer_cast<CatalanStemmer>(o) != nullptr;
}

int CatalanStemmer::hashCode()
{
  return CatalanStemmer::typeid->getName().hashCode();
}
} // namespace org::tartarus::snowball::ext