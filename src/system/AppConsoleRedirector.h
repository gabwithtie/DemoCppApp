#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <streambuf>

class AppConsoleRedirector : public std::streambuf {
public:
    AppConsoleRedirector() {
        // Backup the original buffer
        old_buf = std::cout.rdbuf(this);
    }

    ~AppConsoleRedirector() {
        // Restore the original buffer on exit
        std::cout.rdbuf(old_buf);
    }

protected:
    // This is called whenever a character is output to std::cout
    virtual int_type overflow(int_type v) override {
        if (v != EOF) {
            char c = static_cast<char>(v);
            buffer += c;
            if (c == '\n') {
                // Add to your actual app log (replace with your log logic)
                log_lines.push_back(buffer);
                buffer.clear();
            }
        }
        return v;
    }

public:
    std::vector<std::string> log_lines;
private:
    std::streambuf* old_buf;
    std::string buffer;
};