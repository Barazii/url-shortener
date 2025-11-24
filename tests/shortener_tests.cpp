#include "Shortener.hpp"
#include <cassert>
#include <iostream>

int main() {
    try {
        Shortener s{"test_urls.db"};
        auto codeOpt = s.shorten("https://example.com/");
        assert(codeOpt && codeOpt->size() == Shortener::ShortCodeLen - 1);
        auto urlOpt = s.expand(*codeOpt);
        assert(urlOpt && *urlOpt == "https://example.com/");
        std::cout << "All tests passed\n";
    } catch (const std::exception& ex) {
        std::cerr << "Test failure: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
