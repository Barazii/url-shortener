#pragma once
#include <string>
#include <sqlite3.h>
#include <random>
#include <memory>
#include <string_view>
#include <optional>

// URL Shortener class: RAII wrapper around sqlite3 connection
// Provides shorten(url) -> code and expand(code) -> url
class Shortener {
public:
    static constexpr std::size_t MaxUrlLen = 2048; // matches original constraint
    static constexpr std::size_t ShortCodeLen = 7; // including null terminator when stored in C string

    // Open (or create) database at path. Throws std::runtime_error on failure.
    explicit Shortener(const std::string& dbPath);

    // Non-copyable (sqlite3* unique ownership)
    Shortener(const Shortener&) = delete;
    Shortener& operator=(const Shortener&) = delete;

    // Movable
    Shortener(Shortener&& other) noexcept;
    Shortener& operator=(Shortener&& other) noexcept;

    ~Shortener();

    // Generate a new short code for the given url and insert mapping.
    // Returns short code string of length ShortCodeLen-1 (7 chars) on success.
    // Returns std::nullopt if insertion failed after retry attempts.
    [[nodiscard]] std::optional<std::string> shorten(std::string_view url);

    // Lookup url for code; returns std::nullopt if not found.
    [[nodiscard]] std::optional<std::string> expand(std::string_view code) const;

private:
    sqlite3* db_ {nullptr};

    // Prepare database schema
    void ensureSchema();

    // Generate random code candidate
    [[nodiscard]] std::string generateCode();

    // Check if code already exists
    [[nodiscard]] bool codeExists(std::string_view code) const;
};
