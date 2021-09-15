#include "fast_float/fast_float.h"
#include <iostream>
#include <vector>

#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)  || defined(sun) || defined(__sun)
// Anything at all that is related to cygwin, msys and so forth will
// always use this fallback because we cannot rely on it behaving as normal
// gcc.
#include <locale>
#include <sstream>
// workaround for CYGWIN
double cygwin_strtod_l(const char* start, char** end) {
    double d;
    std::stringstream ss;
    ss.imbue(std::locale::classic());
    ss << start;
    ss >> d;
    if(ss.fail()) { *end = nullptr; }
    if(ss.eof()) { ss.clear(); }
    auto nread = ss.tellg();
    *end = const_cast<char*>(start) + nread;
    return d;
}
float cygwin_strtof_l(const char* start, char** end) {
    float d;
    std::stringstream ss;
    ss.imbue(std::locale::classic());
    ss << start;
    ss >> d;
    if(ss.fail()) { *end = nullptr; }
    if(ss.eof()) { ss.clear(); }
    auto nread = ss.tellg();
    *end = const_cast<char*>(start) + nread;
    return d;
}
#endif

inline void Assert(bool Assertion) {
#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)  || defined(sun) || defined(__sun)
  if (!Assertion) { std::cerr << "Omitting hard falure on msys/cygwin/sun systems."; }
#else 
  if (!Assertion) { throw std::runtime_error("bug"); }
#endif
}

template <typename T> std::string to_string(T d) {
  std::string s(64, '\0');
  auto written = std::snprintf(&s[0], s.size(), "%.*e",
                               std::numeric_limits<T>::max_digits10 - 1, d);
  s.resize(size_t(written));
  return s;
}

template <typename T>
bool test() {
  std::string input = "0.1 1e1000 100000 3.14159265359  -1e-500 001    1e01  1e0000001  -inf";
  std::vector<T> answers = {T(0.1), std::numeric_limits<T>::infinity(), 100000, T(3.14159265359),  -0.0, 1,    10,  10, -std::numeric_limits<T>::infinity()};
  const char * begin = input.data();
  const char * end = input.data() + input.size();
  for(size_t i = 0; i < answers.size(); i++) {
    T result_value;
    while((begin < end) && (std::isspace(*begin))) { begin++; }
    auto result = fast_float::from_chars(begin, end,
                                      result_value);
    if (result.ec != std::errc()) {
      printf("parsing %.*s\n", int(end - begin), begin);
      std::cerr << " I could not parse " << std::endl;
      return false;
    }
    if(result_value != answers[i]) {
      printf("parsing %.*s\n", int(end - begin), begin);
      std::cerr << " Mismatch " << std::endl;
      return false;

    }
    begin = result.ptr;
  }
  if(begin != end) {
      std::cerr << " bad ending " << std::endl;
      return false;    
  }
  return true;
}

template <typename T>
void strtod_from_string(const std::string &st, T& d);

template <>
void strtod_from_string(const std::string &st, double& d) {
    char *pr = (char *)st.c_str();
#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)  || defined(sun) || defined(__sun)
    d = cygwin_strtod_l(pr, &pr);
#elif defined(_WIN32)
    static _locale_t c_locale = _create_locale(LC_ALL, "C");
    d = _strtod_l(st.c_str(), &pr,  c_locale);
#else
    static locale_t c_locale = newlocale(LC_ALL_MASK, "C", NULL);
    d = strtod_l(st.c_str(), &pr,  c_locale);
#endif
    if (pr == st.c_str()) {
      throw std::runtime_error("bug in strtod_from_string");
    }
}

template <>
void strtod_from_string(const std::string &st, float& d) {
    char *pr = (char *)st.c_str();
#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)  || defined(sun) || defined(__sun)
    d = cygwin_strtof_l(st.c_str(), &pr);
#elif defined(_WIN32)
    static _locale_t c_locale = _create_locale(LC_ALL, "C");
    d = _strtof_l(st.c_str(), &pr,  c_locale);
#else
    static locale_t c_locale = newlocale(LC_ALL_MASK, "C", NULL);
    d = strtof_l(st.c_str(), &pr,  c_locale);
#endif
    if (pr == st.c_str()) {
      throw std::runtime_error("bug in strtod_from_string");
    }
}

template <typename T>
bool partow_test() {
  // credit: https://github.com/ArashPartow/strtk/blob/master/strtk_tokenizer_cmp.cpp#L568
  // MIT license
  const std::string strint_list[] = { "9007199254740993",  "9007199254740994",  "9007199254740995" ,
                       "0",         "1",         "2",         "3",         "4",         "5",         "6",         "7",         "8",         "9",
                  "917049",   "4931205",   "6768064",   "6884243",   "5647132",   "7371203",  "-8629878",   "4941840",   "4543268",   "1075600",
                     "290",       "823",       "111",       "715",      "-866",       "367",       "666",      "-706",       "850",      "-161",
                 "9922547",   "6960207",   "1883152",   "2300759",   "-279294",   "4187292",   "3699841",   "8386395",  "-1441129",   "-887892",
                 "-635422",   "9742573",   "2326186",  "-5903851",   "5648486",   "3057647",   "2980079",   "2957468",   "7929158",   "1925615",
                     "879",       "130",       "292",       "705",       "817",       "446",       "576",       "750",       "523",      "-527",
                 "4365041",   "5624958",   "8990205",   "2652177",   "3993588",   "-298316",   "2901599",   "3887387",  "-5202979",   "1196268",
                 "5968501",   "7619928",   "3565643",   "1885272",   "-749485",   "2961381",   "2982579",   "2387454",   "4250081",   "5958205",
                   "00000",     "00001",     "00002",     "00003",     "00004",     "00005",     "00006",     "00007",     "00008",     "00009",
                 "4907034",   "2592882",   "3269234",    "549815",   "6256292",   "9721039",   "-595225",   "5587491",   "4596297",  "-3885009",
                     "673",      "-899",       "174",       "354",       "870",       "147",       "898",      "-510",       "369",       "859",
                 "6518423",   "5149762",   "8834164",  "-8085586",   "3233120",   "8166948",   "4172345",   "6735549",   "-934295",   "9481935",
                 "-430406",   "6932717",   "4087292",   "4047263",   "3236400",  "-3863050",   "4312079",   "6956261",   "5689446",   "3871332",
                     "535",       "691",       "326",      "-409",       "704",      "-568",       "301",       "951",       "121",       "384",
                 "4969414",   "9378599",   "7971781",   "5380630",   "5001363",   "1715827",   "6044615",   "9118925",   "9956168",  "-8865496",
                 "5962464",   "7408980",   "6646513",   "-634564",   "4188330",   "9805948",   "5625691",   "7641113",  "-4212929",   "7802447",
                       "0",         "1",         "2",         "3",         "4",         "5",         "6",         "7",         "8",         "9",
                 "2174248",   "7449361",   "9896659",    "-25961",   "1706598",   "2412368",  "-4617035",   "6314554",   "2225957",   "7521434",
                "-9530566",   "3914164",   "2394759",   "7157744",   "9919392",   "6406949",   "-744004",   "9899789",   "8380325",  "-1416284",
                 "3402833",   "2150043",   "5191009",   "8979538",   "9565778",   "3750211",   "7304823",   "2829359",   "6544236",   "-615740",
                     "363",      "-627",       "129",       "656",       "135",       "113",       "381",       "646",       "198",        "38",
                 "8060564",   "-176752",   "1184717",   "-666343",  "-1273292",   "-485827",   "6241066",   "6579411",   "8093119",   "7481306",
                "-4924485",   "7467889",   "9813178",   "7927100",   "3614859",   "7293354",   "9232973",   "4323115",   "1133911",   "9511638",
                 "4443188",   "2289448",   "5639726",   "9073898",   "8540394",   "5389992",   "1397726",   "-589230",   "1017086",   "1852330",
                    "-840",       "267",       "201",       "533",      "-675",       "494",       "315",       "706",      "-920",       "784",
                 "9097353",   "6002251",   "-308780",  "-3830169",   "4340467",   "2235284",   "3314444",   "1085967",   "4152107",   "5431117",
                   "-0000",     "-0001",     "-0002",     "-0003",     "-0004",     "-0005",     "-0006",     "-0007",     "-0008",     "-0009",
                 "-444999",   "2136400",   "6925907",   "6990614",   "3588271",   "8422028",  "-4034772",   "5804039",  "-6740545",   "9381873",
                 "-924923",   "1652367",   "2302616",   "6776663",   "2567821",   "-248935",   "2587688",   "7076742",  "-6461467",   "1562896",
                 "-768116",   "2338768",   "9887307",   "9992184",   "2045182",   "2797589",   "9784597",   "9696554",   "5113329",   "1067216",
               "-76247763",  "58169007",  "29408062",  "85342511",  "42092201", "-95817703",  "-1912517", "-26275135",  "54656606", "-58188878",
                     "473",        "74",       "374",       "-64",       "266",       "715",       "937",      "-249",       "249",       "780",
                 "3907360", "-23063423",  "59062754",  "83711047", "-95221044",  "34894840", "-38562139", "-82018330",  "14226223", "-10799717",
                 "8529722",  "88961903",  "25608618", "-39988247",  "33228241",  "38598533",  "21161480", "-33723784",   "8873948",  "96505557",
               "-47385048", "-79413272", "-85904404",  "87791158",  "49194195",  "13051222",  "57773302",  "31904423",   "3142966",  "27846156",
                 "7420011", "-72376922", "-68873971",  "23765361",   "4040725", "-22359806",  "85777219",  "10099223", "-90364256", "-40158172",
                "-7948696", "-64344821",  "34404238",  "84037448", "-85084788", "-42078409", "-56550310",  "96898389",   "-595829", "-73166703",
                      "-0",        "-1",        "-2",        "-3",        "-4",        "-5",        "-6",        "-7",        "-8",        "-9",
              "2147483647", "31",  "2147483610", "33",  "2147483573", "37",  "2147483536",
               "-82838342",  "64441808",  "43641062", "-64419642", "-44421934",  "75232413", "-75773725", "-89139509",  "12812089", "-97633526",
                "36090916", "-57706234",  "17804655",   "4189936",  "-4100124",  "38803710", "-39735126", "-62397437",  "75801648",  "51302332",
                "73433906",  "13015224", "-12624818",  "91360377",  "11576319", "-54467535",   "8892431",  "36319780",  "38832042",  "50172572",
                    "-317",       "109",      "-888",       "302",      "-463",       "716",       "916",       "665",       "826",       "513",
                "42423473",  "41078812",  "40445652", "-76722281",  "95092224",  "12075234",  "-4045888", "-74396490", "-57304222", "-21726885",
                "92038121", "-31899682",  "21589254", "-30260046",  "56000244",  "69686659",  "93327838",  "96882881", "-91419389",  "77529147",
                "43288506",   "1192435", "-74095920",  "76756590", "-31184683", "-35716724",   "9451980", "-63168350",  "62864002",  "26283194",
                "37188395",  "29151634",  "99343471", "-69450330", "-55680090", "-64957599",  "47577948",  "47107924",   "2490477",  "48633003",
               "-82740809", "-24122215",  "67301713", "-63649610",  "75499016",  "82746620",  "17052193",   "4602244", "-32721165",  "20837836",
                     "674",       "467",       "706",       "889",       "172",       "282",      "-795",       "188",        "87",       "153",
                "64501793",  "53146328",   "5152287",  "-9674493",  "68105580",  "57245637",  "39740229", "-74071854",  "86777268",  "86484437",
               "-86962508",  "12644427", "-62944073",  "59539680",  "43340539",  "30661534",  "20143968", "-68183731", "-48250926",  "42669063",
                     "000",       "001",       "002",       "003",       "004",       "005",       "006",       "007",       "008",       "009",
              "2147483499", "71",  "2147483462", "73",  "2147483425", "77",  "2147483388",
                "87736852",  "-4444906", "-48094147",  "54774735",  "54571890", "-22473078",  "95053418",    "393654", "-33229960",  "32276798",
               "-48361110",  "44295939", "-79813406",  "11630865",  "38544571",  "70972830",  "-9821748", "-60965384", "-13096675", "-24569041",
                     "708",      "-467",      "-794",       "610",       "929",       "766",       "152",       "482",       "397",      "-191",
                "97233152",  "51028396", "-13796948",  "95437272",  "71352512", "-83233730", "-68517318",  "61832742", "-42667174", "-18002395",
               "-92239407",  "12701336", "-63830875",  "41514172",  "-5726049",  "18668677",  "69555144", "-13737009", "-22626233", "-55078143",
                      "00",        "11",        "22",        "33",        "44",       "-00",       "-11",       "-22",       "-33",       "-44",
                     "000",       "111",       "222",       "333",       "444",      "-000",      "-111",      "-222",      "-333",      "-444",
                    "0000",      "1111",      "2222",      "3333",      "4444",     "-0000",     "-1111",     "-2222",     "-3333",     "-4444",
                   "00000",     "11111",     "22222",     "33333",     "44444",    "-00000",    "-11111",    "-22222",    "-33333",    "-44444",
                  "000000",    "111111",    "222222",    "333333",    "444444",   "-000000",   "-111111",   "-222222",   "-333333",   "-444444",
                 "0000000",   "1111111",   "2222222",   "3333333",   "4444444",  "-0000000",  "-1111111",  "-2222222",  "-3333333",  "-4444444",
                "00000000",  "11111111",  "22222222",  "33333333",  "44444444", "-00000000", "-11111111", "-22222222", "-33333333", "-44444444",
               "000000000", "111111111", "222222222", "333333333", "444444444","-000000000","-111111111","-222222222","-333333333","-444444444",
              "2147483351", "51",  "2147483314", "53", "-2147483648", "57", "-2147483611",
                      "55",        "66",        "77",        "88",        "99",       "-55",       "-66",       "-77",       "-88",       "-99",
                     "555",       "666",       "777",       "888",       "999",      "-555",      "-666",      "-777",      "-888",      "-999",
                    "5555",      "6666",      "7777",      "8888",      "9999",     "-5555",     "-6666",     "-7777",     "-8888",     "-9999",
                   "55555",     "66666",     "77777",     "88888",     "99999",    "-55555",    "-66666",    "-77777",    "-88888",    "-99999",
                  "555555",    "666666",    "777777",    "888888",    "999999",   "-555555",   "-666666",   "-777777",   "-888888",   "-999999",
                 "5555555",   "6666666",   "7777777",   "8888888",   "9999999",  "-5555555",  "-6666666",  "-7777777",  "-8888888",  "-9999999",
                "55555555",  "66666666",  "77777777",  "88888888",  "99999999", "-55555555", "-66666666", "-77777777", "-88888888", "-99999999",
               "555555555", "666666666", "777777777", "888888888", "999999999","-555555555","-666666666","-777777777","-888888888","-999999999",
             "-2147483574",        "91", "-2147483537", "93", "-2147483500", "97", "-2147483463",
              "0000000011", "0000000022", "0000000033", "0000000044", "-000000011", "-000000022", "-000000033", "-000000044", "-000000088",
              "0000000111", "0000000222", "0000000333", "0000000444", "-000000111", "-000000222", "-000000333", "-000000444", "-000000888",
              "0000001111", "0000002222", "0000003333", "0000004444", "-000001111", "-000002222", "-000003333", "-000004444", "-000008888",
              "0000011111", "0000022222", "0000033333", "0000044444", "-000011111", "-000022222", "-000033333", "-000044444", "-000088888",
              "0000111111", "0000222222", "0000333333", "0000444444", "-000111111", "-000222222", "-000333333", "-000444444", "-000888888",
              "0001111111", "0002222222", "0003333333", "0004444444", "-001111111", "-002222222", "-003333333", "-004444444", "-008888888",
              "0011111111", "0022222222", "0033333333", "0044444444", "-011111111", "-022222222", "-033333333", "-044444444", "-088888888",
              "0111111111", "0222222222", "0333333333", "0444444444", "-111111111", "-222222222", "-333333333", "-444444444", "-888888888",
              "0000000055", "0000000066", "0000000077", "0000000088", "0000000099", "-000000055", "-000000066", "-000000077", "-000000099",
              "0000000555", "0000000666", "0000000777", "0000000888", "0000000999", "-000000555", "-000000666", "-000000777", "-000000999",
              "0000005555", "0000006666", "0000007777", "0000008888", "0000009999", "-000005555", "-000006666", "-000007777", "-000009999",
              "0000055555", "0000066666", "0000077777", "0000088888", "0000099999", "-000055555", "-000066666", "-000077777", "-000099999",
              "0000555555", "0000666666", "0000777777", "0000888888", "0000999999", "-000555555", "-000666666", "-000777777", "-000999999",
              "0005555555", "0006666666", "0007777777", "0008888888", "0009999999", "-005555555", "-006666666", "-007777777", "-009999999",
              "0055555555", "0066666666", "0077777777", "0088888888", "0099999999", "-055555555", "-066666666", "-077777777", "-099999999",
              "0555555555", "0666666666", "0777777777", "0888888888", "0999999999", "-555555555", "-666666666", "-777777777", "-999999999",
             "-2147483426",        "101", "-2147483389",       "103", "-2147483352", "105", "-2147483315",
               "0000001234567890",  "0000001234567890", "-0000001234567890",
                "000001234567890",   "000001234567890",  "-000001234567890",
                 "00001234567890",    "00001234567890",   "-00001234567890",
                  "0001234567890",     "0001234567890",    "-0001234567890",
                   "001234567890",      "001234567890",     "-001234567890",
                    "01234567890",       "01234567890",      "-01234567890",
                     "1234567890",        "1234567890",       "-1234567890",
             };
  for(const std::string& st : strint_list) {
    T expected_value;
    strtod_from_string(st, expected_value);
    T result_value;
    auto result = fast_float::from_chars(st.data(), st.data() + st.size(),
                                      result_value);
    if (result.ec != std::errc()) {
      printf("parsing %.*s\n", int(st.size()), st.data());
      std::cerr << " I could not parse " << std::endl;
      return false;
    }
    if(result.ptr != st.data() + st.size()) {
      printf("parsing %.*s\n", int(st.size()), st.data());
      std::cerr << " Did not get to the end " << std::endl;
      return false;
    }
    if(result_value != expected_value) {
      printf("parsing %.*s\n", int(st.size()), st.data());
      std::cerr << "expected value : " << to_string(expected_value) << std::endl;
      std::cerr << "result   value : " << to_string(result_value) << std::endl;
      std::cerr << " Mismatch " << std::endl;
      return false;
    }

  }
  return true;

}


int main() {
#if defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)  || defined(sun) || defined(__sun)
  std::cout << "Warning: msys/cygwin or solaris detected." << std::endl;
#endif
  std::cout << "32 bits checks" << std::endl;
  Assert(partow_test<float>());
  Assert(test<float>());
  
  std::cout << "64 bits checks" << std::endl;
  Assert(partow_test<double>());
  Assert(test<double>());

  std::cout << "All ok" << std::endl;
  return EXIT_SUCCESS;
}
