#include "Shortener.hpp"
#include <stdexcept>
#include <chrono>
#include <cstdio>

namespace {
    constexpr std::string_view kChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"; // 62
}

Shortener::Shortener(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK) {
        std::string msg = "Can't open database: ";
        msg += sqlite3_errmsg(db_);
        throw std::runtime_error(msg);
    }
    try {
        ensureSchema();
    } catch (...) {
        sqlite3_close(db_);
        db_ = nullptr;
        throw; // rethrow
    }
}

Shortener::Shortener(Shortener&& other) noexcept : db_(other.db_) {
    other.db_ = nullptr;
}

Shortener& Shortener::operator=(Shortener&& other) noexcept {
    if (this != &other) {
        if (db_) sqlite3_close(db_);
        db_ = other.db_;
        other.db_ = nullptr;
    }
    return *this;
}

Shortener::~Shortener() {
    if (db_) {
        sqlite3_close(db_);
    }
}

void Shortener::ensureSchema() {
    const char* sql = "CREATE TABLE IF NOT EXISTS urls ("\
                      "code TEXT PRIMARY KEY,"\
                      "url TEXT NOT NULL,"\
                      "created INTEGER NOT NULL"\
                      ");";
    char* err = nullptr;
    if (sqlite3_exec(db_, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = "SQL error: ";
        if (err) {
            msg += err;
            sqlite3_free(err);
        }
        throw std::runtime_error(msg);
    }
}

std::string Shortener::generateCode() {
    // Use thread-local random engine seeded once
    thread_local std::mt19937_64 rng{std::random_device{}()};
    std::uniform_int_distribution<int> dist(0, static_cast<int>(kChars.size()) - 1);
    std::string code;
    code.reserve(ShortCodeLen - 1);
    for (std::size_t i = 0; i < ShortCodeLen - 1; ++i) {
        auto idx = static_cast<std::size_t>(dist(rng));
        code.push_back(kChars[idx]);
    }
    return code;
}

bool Shortener::codeExists(std::string_view code) const {
    const char* sql = "SELECT 1 FROM urls WHERE code = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return true; // conservative: assume exists prevents reuse if error
    }
    sqlite3_bind_text(stmt, 1, code.data(), static_cast<int>(code.size()), SQLITE_STATIC);
    int rc = sqlite3_step(stmt);
    bool exists = (rc == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

std::optional<std::string> Shortener::shorten(std::string_view url) {
    if (url.empty() || url.size() > MaxUrlLen) {
        return std::nullopt;
    }
    constexpr int kMaxTries = 10;
    std::string code;
    for (int tries = 0; tries < kMaxTries; ++tries) {
        code = generateCode();
        if (!codeExists(code)) break;
        if (tries == kMaxTries - 1) return std::nullopt; // give up
    }

    const char* sql = "INSERT INTO urls (code, url, created) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, code.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, url.data(), static_cast<int>(url.size()), SQLITE_STATIC);
    auto created = static_cast<sqlite3_int64>(std::chrono::system_clock::now().time_since_epoch()/std::chrono::seconds(1));
    sqlite3_bind_int64(stmt, 3, created);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
    sqlite3_finalize(stmt);
    return code;
}

std::optional<std::string> Shortener::expand(std::string_view code) const {
    if (code.size() != ShortCodeLen - 1) return std::nullopt; // fast reject
    const char* sql = "SELECT url FROM urls WHERE code = ?";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }
    sqlite3_bind_text(stmt, 1, code.data(), static_cast<int>(code.size()), SQLITE_STATIC);

    std::optional<std::string> result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* txt = sqlite3_column_text(stmt, 0);
        if (txt) result = reinterpret_cast<const char*>(txt);
    }
    sqlite3_finalize(stmt);
    return result;
}
