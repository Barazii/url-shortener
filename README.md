# URL Shortener (C++)

A minimal SQLite-backed URL shortener implemented in modern C++ (C++20). Based on an original [C version](https://github.com/dexter-xD/project-box/tree/main/url-shortener); redesigned with RAII, exceptions, and <random> for secure code generation.

## Features
- Generates unique 7-character codes (62 charset)
- Stores mappings with creation timestamp
- Expands short code back to original URL
- Simple interactive CLI
- Unit test example with CTest

## Build
```bash
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

## Run
```bash
./url_shortener
```
Commands:
- `shorten <url>`: returns a short code
- `expand <code>`: prints original URL
- `quit` or `exit`: leave program

Database file `urls.db` is created in working directory.

## Test
```bash
cd build
ctest --output-on-failure
```

## Design Notes
- `Shortener` encapsulates the sqlite3* handle; no manual close needed.
- Random codes via thread-local `std::mt19937_64` seeded by `std::random_device`.
- Avoids copying large strings by using `std::string_view` for inputs.
- Defensive checks for URL length and code length.
- Could be extended with prepared statement caching, async IO, or HTTP interface.

## Future Improvements
- Add an HTTP server endpoint (e.g. using Boost.Beast)
- Add deletion / stats (hits count)
- Implement rate limiting
- Use stronger randomness (e.g. `std::random_device` every char) for higher entropy
- Provide migration tool and config options
