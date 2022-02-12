#pragma once

#include <iosfwd>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace parse {

namespace token_type {
struct Number {  // Лексема «число»
    int value;
};

struct Id {  // Лексема «идентификатор»
    std::string value;
};

struct Char {  // Лексема «символ»
    char value;
};

struct String {  // Лексема «строковая константа»
    std::string value;
};

struct Class {};    // Лексема «class»
struct Return {};   // Лексема «return»
struct If {};       // Лексема «if»
struct Else {};     // Лексема «else»
struct Def {};      // Лексема «def»
struct Newline {};  // Лексема «конец строки»
struct Print {};    // Лексема «print»
struct Indent {};  // Лексема «увеличение отступа», соответствует двум пробелам
struct Dedent {};       // Лексема «уменьшение отступа»
struct Eof {};          // Лексема «конец файла»
struct And {};          // Лексема «and»
struct Or {};           // Лексема «or»
struct Not {};          // Лексема «not»
struct Eq {};           // Лексема «==»
struct NotEq {};        // Лексема «!=»
struct LessOrEq {};     // Лексема «<=»
struct GreaterOrEq {};  // Лексема «>=»
struct None {};         // Лексема «None»
struct True {};         // Лексема «True»
struct False {};        // Лексема «False»
}  // namespace token_type

using TokenBase =
    std::variant<token_type::Number, token_type::Id, token_type::Char, token_type::String, token_type::Class,
                 token_type::Return, token_type::If, token_type::Else, token_type::Def, token_type::Newline,
                 token_type::Print, token_type::Indent, token_type::Dedent, token_type::And, token_type::Or,
                 token_type::Not, token_type::Eq, token_type::NotEq, token_type::LessOrEq, token_type::GreaterOrEq,
                 token_type::None, token_type::True, token_type::False, token_type::Eof>;

struct Token : TokenBase {
    using TokenBase::TokenBase;

    template <typename T>
    [[nodiscard]] bool Is() const {
        return std::holds_alternative<T>(*this);
    }

    template <typename T>
    [[nodiscard]] const T& As() const {
        return std::get<T>(*this);
    }

    template <typename T>
    [[nodiscard]] const T* TryAs() const {
        return std::get_if<T>(this);
    }
};

bool operator==(const Token& left, const Token& right);
bool operator!=(const Token& left, const Token& right);

std::ostream& operator<<(std::ostream& os, const Token& token);

class LexerError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

/*!
 * @brief Provides the wrapper to handle indents during the parsing.
 *
 * @details Stores current indent while parsing the line.
 * Cuts the indents spaces from line for the simplified further processing.
 *
 */
class IndentParser {
public:  // Constructor
    explicit IndentParser(std::istream& input);

public:  // Methods
    [[nodiscard]] int GetIndent() const;
    [[nodiscard]] std::stringstream& GetParsedStream();
    [[nodiscard]] bool IsInputStreamEmpty() const;

    void ParseNextLine();

private:
    [[nodiscard]] bool EmptyLine(std::string_view line) const;

private:  // Fields
    std::istream& input_;
    std::stringstream line_;

    int lines_count_{0};
    int indent_{0};
};

class Lexer {
public:  // Constructor
    explicit Lexer(std::istream& input);

public:  // Methods
    /*!
     * @brief Gets reference to the current token
     * @return Reference to the current token, EOF in case the stream is over
     */
    [[nodiscard]] const Token& CurrentToken() const;

    /*!
     * @brief Gets reference to the next token
     * @return Reference to the current token, EOF in case the stream is over
     */
    Token NextToken();

    /*!
     * @brief Checks if token has assumed type
     * @tparam TokenType Assumed token type
     * @return Reference to the token of specified type
     * @throws LexerError In case if token type is differ from expected
     */
    template <typename TokenType>
    const TokenType& Expect() const {
        using namespace std::string_literals;

        if (current_.Is<TokenType>())
            return current_.As<TokenType>();

        throw LexerError("Token is has unexpected type:" + std::string(typeid(TokenType).name()));
    }

    /*!
     *  @brief Checks if token has assumed type and value
     *  @param value Expected value of token
     *  @tparam TokenType Assumed token type
     *  @tparam ValueType Assumed value type
     *  @throws LexerError In case token type or value is differ from expected
     */
    template <typename TokenType, typename ValueType>
    void Expect(const ValueType& value) const {
        if (!current_.Is<TokenType>() || current_.As<TokenType>().value != value)
            throw LexerError("Token is has unexpected type or value");
    }

    /*!
     *  @brief Gets reference to the next token if it has expected type
     *  @tparam TokenType Assumed token type
     *  @return Reference to the next token
     *  @throws LexerError In case token type is differ from expected
     */
    template <typename TokenType>
    const TokenType& ExpectNext() {
        NextToken();
        return Expect<TokenType>();
    }

    /*!
     *  @brief Gets reference to the next token if it has expected type and value
     *  @tparam TokenType Assumed token type
     *  @tparam ValueType Assumed token value
     *  @return Reference to the next token
     *  @throws LexerError In case token type or value is differ from expected
     */
    template <typename TokenType, typename ValueType>
    void ExpectNext(const ValueType& value) {
        NextToken();
        Expect<TokenType>(value);
    }

private:  // Methods
    /// @brief Reads the whole line converting it to the sequence of tokens
    Token NextTokenImpl();

    /// @brief Reads the identifiers
    Token ReadId(std::stringstream& ss);

    /// @brief Reads the numbers
    Token ReadNumber(std::stringstream& ss);

    /// @brief Reads the string in ""
    Token ReadString(std::stringstream& ss, char last_symbol);

private:  // Fields
    int indent_{0};
    IndentParser indent_reader_;
    Token current_;
};

}  // namespace parse