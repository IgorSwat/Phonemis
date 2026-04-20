#include <phonemis/base/config.h>
#include <phonemis/base/pipeline.h>
#include <phonemis/utils/conversions.h>
#include <phonemis/utils/io.h>
#include <phonemis/utils/strings.h>

#include <iostream>
#include <string>
#include <vector>

void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [options] <text>\n"
              << "Options:\n"
              << "  --lexicon <path>     Path to lexicon file\n"
              << "  --model <path>       Path to neural model file\n"
              << "  --tagger <path>      Path to tagger data file\n"
              << "  --lang <lang_code>   Language code (e.g., en-us, en-gb)\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    phonemis::Config config;
    std::string text_to_process;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--lexicon" && i + 1 < argc) {
            config.phonemizer.lexicon_filepath = argv[++i];
        } else if (arg == "--model" && i + 1 < argc) {
            config.phonemizer.nn_model_filepath = argv[++i];
        } else if (arg == "--tagger" && i + 1 < argc) {
            config.tagger = {
                .data_filepath = argv[++i],
            };
        } else if (arg == "--lang" && i + 1 < argc) {
            config.lang = argv[++i];
            config.phonemizer.lang = config.lang;
        } else if (i == argc - 1) {
            text_to_process = arg;
        } else {
            std::cerr << "Unknown or incomplete option: " << arg << std::endl;
            print_usage(argv[0]);
            return 1;
        }
    }

    if (text_to_process.empty()) {
        std::cerr << "Error: No input text provided." << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    try {
        phonemis::Pipeline pipeline(config);
        std::u32string result = pipeline.process(text_to_process);

        std::cout << "\n\033[1;32mPhonemization Result:\033[0m" << std::endl;
        std::cout << "\033[1;34mInput:  \033[0m" << text_to_process << std::endl;
        std::cout << "\033[1;36mOutput: \033[0m" << phonemis::utils::conversions::u32_to_utf8(result) << std::endl;
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "\033[1;31mError:\033[0m " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
