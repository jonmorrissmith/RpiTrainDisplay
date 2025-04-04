// Train Display - an RGB matrix departure board for the Raspberry Pi
//
// Display text implementation file
//
// Jon Morris Smith - April 2025
// Version 1.0
// Instructions, fixes and issues at https://github.com/jonmorrissmith/RGB_Matrix_Train_Departure_Board
//

#include "display_text.h"
#include <algorithm>
#include <utility>

//-------------------------------------------------------------------------
// FontCache implementation
//-------------------------------------------------------------------------

void FontCache::setFont(const Font& font) {
    try {
        font_ptr = &font;
        // Cache character widths
        for (int i = 0; i < 256; i++) {
            char_widths[i] = font.CharacterWidth(static_cast<char>(i));
        }
        // Cache font baseline
        baseline = font.baseline();
        height = font.height();
    } catch (const std::exception& e) {
        DEBUG_PRINT("Error in FontCache::setFont: " << e.what());
        throw std::runtime_error("Error initializing font cache: " + std::string(e.what()));
    }
}

int FontCache::getCharWidth(char c) const {
    try {
        unsigned char idx = static_cast<unsigned char>(c);
        return char_widths[idx];
    } catch (const std::exception& e) {
        DEBUG_PRINT("Error in FontCache::getCharWidth: " << e.what());
        throw std::out_of_range("Character out of range in FontCache::getCharWidth");
    }
}

int FontCache::getTextWidth(const std::string& text) const {
    try {
        int width = 0;
        for (const char& c : text) {
            width += getCharWidth(c);
        }
        return width;
    } catch (const std::exception& e) {
        DEBUG_PRINT("Error in FontCache::getTextWidth: " << e.what());
        throw std::runtime_error("Error calculating text width: " + std::string(e.what()));
    }
}

int FontCache::getBaseline(){
    return baseline;
}

int FontCache::getheight() {
    return height;
}


//-------------------------------------------------------------------------
// DisplayText implementation
//-------------------------------------------------------------------------

DisplayText::DisplayText(const std::string& t, int w, int x, int y, uint64_t v)
    : text(t), width(w), x_position(x), y_position(y), data_version(v) {}

void DisplayText::setTextAndWidth(const std::string& newText, const FontCache& fontsizes) {
    try {
        text = newText;
        width = fontsizes.getTextWidth(text);
    } catch (const std::exception& e) {
        DEBUG_PRINT("Error in DisplayText::setTextAndWidth: " << e.what());
        throw std::runtime_error("Error setting text and width: " + std::string(e.what()));
    }
}

void DisplayText::setWidth(const FontCache& fontsizes) {
    try {
        width = fontsizes.getTextWidth(text);
    } catch (const std::exception& e) {
        DEBUG_PRINT("Error in DisplayText::setWidth: " << e.what());
        throw std::runtime_error("Error setting width: " + std::string(e.what()));
    }
}

bool DisplayText::empty() {
    return text.empty();
}

DisplayText& DisplayText::operator=(const std::string& str) {
    try {
        text = str;
    } catch (const std::exception& e) {
        DEBUG_PRINT("Error in DisplayText::operator=: " << e.what());
        throw std::runtime_error("Error assigning string to DisplayText: " + std::string(e.what()));
    }
    return *this;
}

DisplayText& DisplayText::operator=(int pos) {
    x_position = pos;
    return *this;
}

DisplayText& DisplayText::operator<<(const std::string& str) {
    try {
        // More efficient string concatenation
        if (text.empty()) {
            text = str;  // Direct assignment for first string
        } else {
            // Reserve space to avoid multiple reallocations
            text.reserve(text.size() + str.size());
            text.append(str);  // More efficient than concatenation operator
        }
    } catch (const std::exception& e) {
        DEBUG_PRINT("Error in DisplayText::operator<<: " << e.what());
        throw std::runtime_error("Error in string concatenation: " + std::string(e.what()));
    }
    return *this;
}

DisplayText& DisplayText::operator<<(const char* str) {
    try {
        return *this << std::string(str);  // Reuse string version
    } catch (const std::exception& e) {
        DEBUG_PRINT("Error in DisplayText::operator<<(const char*): " << e.what());
        throw std::runtime_error("Error in string literal concatenation: " + std::string(e.what()));
    }
}

DisplayText& DisplayText::operator<<(int value) {
    try {
        return *this << std::to_string(value);  // Convert to string and reuse
    } catch (const std::exception& e) {
        DEBUG_PRINT("Error in DisplayText::operator<<(int): " << e.what());
        throw std::runtime_error("Error in integer concatenation: " + std::string(e.what()));
    }
}

DisplayText::operator int() const {
    return x_position;
}

int DisplayText::getXPosition() const {
    return x_position;
}

DisplayText& DisplayText::operator++() {
    ++x_position;
    return *this;
}

DisplayText DisplayText::operator++(int) {
    DisplayText old = *this;
    ++(*this);
    return old;
}

DisplayText& DisplayText::operator--() {
    --x_position;
    return *this;
}

DisplayText DisplayText::operator--(int) {
    DisplayText old = *this;
    --(*this);
    return old;
}

void DisplayText::reset() {
    text.clear();
    width = 0;
    x_position = 0;
    y_position = 0;
    data_version = 0;
}

void DisplayText::dump(const std::string &name) {
    std::cout << "Name: " << name;
    std::cout << ". Width: " << width;
    std::cout << ", x_position: " << x_position;
    std::cout << ", y_position: " << y_position;
    std::cout << ", data_version: " << data_version << "." << std::endl;
}

void DisplayText::fulldump(const std::string &name) {
    std::cout << "Name: " << name;
    std::cout << ". text: " << text;
    std::cout << ", Width: " << width;
    std::cout << ", x_position: " << x_position;
    std::cout << ", y_position: " << y_position;
    std::cout << ", data_version: " << data_version << "." << std::endl;
}

bool DisplayText::operator<(const DisplayText& other) const {
    return x_position < other.x_position;
}

bool DisplayText::operator>(const DisplayText& other) const {
    return x_position > other.x_position;
}

bool DisplayText::operator<=(const DisplayText& other) const {
    return x_position <= other.x_position;
}

bool DisplayText::operator>=(const DisplayText& other) const {
    return x_position >= other.x_position;
}

bool DisplayText::operator==(const DisplayText& other) const {
    return x_position == other.x_position;
}

bool DisplayText::operator!=(const DisplayText& other) const {
    return x_position != other.x_position;
}

//-------------------------------------------------------------------------
// Global operator implementations
//-------------------------------------------------------------------------

// Comparison operators between DisplayText and int
bool operator<(const DisplayText& lhs, const int& rhs) {
    return lhs.getXPosition() < rhs;
}

bool operator<(const int& lhs, const DisplayText& rhs) {
    return lhs < rhs.getXPosition();
}

bool operator>(const DisplayText& lhs, const int& rhs) {
    return lhs.getXPosition() > rhs;
}

bool operator>(const int& lhs, const DisplayText& rhs) {
    return lhs > rhs.getXPosition();
}

bool operator<=(const DisplayText& lhs, const int& rhs) {
    return lhs.getXPosition() <= rhs;
}

bool operator<=(const int& lhs, const DisplayText& rhs) {
    return lhs <= rhs.getXPosition();
}

bool operator>=(const DisplayText& lhs, const int& rhs) {
    return lhs.getXPosition() >= rhs;
}

bool operator>=(const int& lhs, const DisplayText& rhs) {
    return lhs >= rhs.getXPosition();
}

bool operator==(const DisplayText& lhs, const int& rhs) {
    return lhs.getXPosition() == rhs;
}

bool operator==(const int& lhs, const DisplayText& rhs) {
    return lhs == rhs.getXPosition();
}

bool operator!=(const DisplayText& lhs, const int& rhs) {
    return lhs.getXPosition() != rhs;
}

bool operator!=(const int& lhs, const DisplayText& rhs) {
    return lhs != rhs.getXPosition();
}

// Output stream operator
std::ostream& operator<<(std::ostream& os, const DisplayText& dt) {
    try {
        os << dt.text;
    } catch (const std::exception& e) {
        DEBUG_PRINT("Error in operator<< for DisplayText: " << e.what());
        throw std::runtime_error("Error in stream output: " + std::string(e.what()));
    }
    return os;
}

// Input stream operator
std::istream& operator>>(std::istream& stream, DisplayText& input) {
    try {
        stream >> input.text;
    } catch (const std::exception& e) {
        DEBUG_PRINT("Error in operator>> for DisplayText: " << e.what());
        throw std::runtime_error("Error in stream input: " + std::string(e.what()));
    }
    return stream;
}

// Addition operators
DisplayText operator+(const DisplayText& dt, int offset) {
    DisplayText result = dt;
    result.x_position += offset;
    return result;
}

DisplayText operator+(int offset, const DisplayText& dt) {
    return dt + offset;
}

// Subtraction operator
DisplayText operator-(const DisplayText& dt, int offset) {
    DisplayText result = dt;
    result.x_position -= offset;
    return result;
}
