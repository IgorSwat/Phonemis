#include <iostream>
#include <vector>
#include <string>
#include <phonemis/preprocessor/num2word.h>
#include <phonemis/preprocessor/tools.h>

using namespace phonemis::preprocessor;

int main() {
    std::string num = num2words::convert(101);
    std::cout << num << "\n";

    std::string num2 = num2words::convert<num2words::ConversionMode::ORDINAL>("16");
    std::cout << num2 << "\n";

    std::string num3 = verbalize_numbers("It was a 61st aniversary of 215th battle of Division 303. I like it in -2.61 degree. But It costed only 2$ so worthed.");
    std::cout << num3 << "\n";

    return 0;
}
