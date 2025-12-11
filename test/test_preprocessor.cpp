#include <iostream>
#include <vector>
#include <string>
#include <phonemize/preprocessor/num2word.h>
#include <phonemize/preprocessor/text2sentences.h>

using namespace phonemize::preprocessor;

int main() {
    std::string num = num2words::convert(-17.17);
    std::cout << num << "\n";

    std::string num2 = num2words::convert<num2words::ConversionMode::ORDINAL>("16");
    std::cout << num2 << "\n";

    return 0;
}
