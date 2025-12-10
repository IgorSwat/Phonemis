#include <iostream>
#include <vector>
#include <string>
#include "../src/tokenizer/tokenize.h"

int main() {
    // Sample input
    // std::string text = "Mr. Smith's e-mail: john.smith@example.com isn't rock'n'roll, it's pre-1960!";
    // std::string text = "An ambiguous question is not always a bad one! But, considering the circumstances, I strongly disagree with it's intentions.";
    std::string text = "``A violence is an art of destruction''.";

    // Tokenize
    std::vector<std::string> tokens = phonemize::tokenizer::tokenize(text);

    // Print tokens
    for (const auto& token : tokens) {
        std::cout << "[" << token << "]" << std::endl;
    }

    return 0;
}
