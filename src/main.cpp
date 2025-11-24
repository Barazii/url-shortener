#include "Shortener.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

// Simple CLI: commands
// shorten <url>
// expand <code>
// quit

int main() {
    try {
        Shortener shortener{"urls.db"};
        std::cout << "URL Shortener - Commands: shorten <url>, expand <code>, quit" << std::endl;
        std::string line;
        while (true) {
            std::cout << "> " << std::flush;
            if (!std::getline(std::cin, line)) break;
            std::istringstream iss(line);
            std::string cmd;
            iss >> cmd;
            if (cmd == "quit" || cmd == "exit") {
                break;
            } else if (cmd == "shorten") {
                std::string url;
                iss >> url; // simplistic; could parse rest of line
                if (url.empty()) {
                    std::cout << "Missing URL" << std::endl;
                    continue;
                }
                if (auto code = shortener.shorten(url)) {
                    std::cout << "Short code: " << *code << std::endl;
                } else {
                    std::cout << "Error creating short URL" << std::endl;
                }
            } else if (cmd == "expand") {
                std::string code;
                iss >> code;
                if (code.empty()) {
                    std::cout << "Missing code" << std::endl;
                    continue;
                }
                if (auto url = shortener.expand(code)) {
                    std::cout << "URL: " << *url << std::endl;
                } else {
                    std::cout << "Code not found" << std::endl;
                }
            } else if (!cmd.empty()) {
                std::cout << "Unknown command" << std::endl;
            }
        }
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
