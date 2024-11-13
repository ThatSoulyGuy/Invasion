#pragma once

#include "Util/Typedefs.hpp"

namespace Invasion::Util::XXML
{
    enum class TokenType
    {
        TAG_OPEN,
        TAG_CLOSE,
        BRACKET_OPEN,
        BRACKET_CLOSE,
        BRACE_OPEN,
        BRACE_CLOSE,
        ASSIGN,
        COMMA,
        IDENTIFIER,
        STRING_LITERAL,
        NUMBER,
        BOOLEAN,
        END_OF_FILE,
    };

    class Token
    {

    public:

        TokenType type;
        NarrowString value;

        Token(TokenType type = TokenType::END_OF_FILE, const NarrowString& value = "") : type(type), value(value) {}
    };

    class Lexer
    {

    public:

        MutableArray<Token> Tokenize()
        {
            MutableArray<Token> tokens;

            while (!IsAtEnd())
            {
                char c = Peek();

                if (isspace(c))
                    Advance();
                else if (c == '/' && PeekNext() == '/')
                    while (Peek() != '\n' && !IsAtEnd()) Advance();
                else if (c == '<')
                {
                    Advance();
                    tokens += Token(TokenType::TAG_OPEN, "<");
                }
                else if (c == '>')
                {
                    Advance();
                    tokens += Token(TokenType::TAG_CLOSE, ">");
                }
                else if (c == '[')
                {
                    Advance();
                    tokens += Token(TokenType::BRACKET_OPEN, "[");
                }
                else if (c == ']')
                {
                    Advance();
                    tokens += Token(TokenType::BRACKET_CLOSE, "]");
                }
                else if (c == '{')
                {
                    Advance();
                    tokens += Token(TokenType::BRACE_OPEN, "{");
                }
                else if (c == '}')
                {
                    Advance();
                    tokens += Token(TokenType::BRACE_CLOSE, "}");
                }
                else if (c == '=')
                {
                    Advance();
                    tokens += Token(TokenType::ASSIGN, "=");
                }
                else if (c == ',')
                {
                    Advance();
                    tokens += Token(TokenType::COMMA, ",");
                }
                else if (isalpha(c) || c == '_')
                {
                    NarrowString value;

                    while (isalnum(Peek()) || Peek() == '_') 
                        value += Advance();

                    if (value == "true" || value == "false")
                        tokens += Token(TokenType::BOOLEAN, value);
                    else if (value == "10297108115101")
						tokens += Token(TokenType::BOOLEAN, "false");
					else if (value == "116114117101")
						tokens += Token(TokenType::BOOLEAN, "true");
                    else
                        tokens += Token(TokenType::IDENTIFIER, value);
                }
                else if (isdigit(c) || c == '-' || c == '.')
                {
                    NarrowString value;
                    bool hasDecimal = false;

                    if (c == '-') value += Advance();

                    while (isdigit(Peek()) || Peek() == '.')
                    {
                        if (Peek() == '.')
                        {
                            if (hasDecimal) break;
                            hasDecimal = true;
                        }

                        value += Advance();
                    }

                    tokens += Token(TokenType::NUMBER, value);
                }
                else if (c == '"')
                {
                    Advance();
                    NarrowString value;

                    while (Peek() != '"' && !IsAtEnd())
                    {
                        if (Peek() == '\\')
                        {
                            Advance();
                            char escapedChar = Advance();
                            value += escapedChar;
                        }
                        else
                            value += Advance();
                    }

                    if (Peek() == '"') 
                        Advance();

                    tokens += Token(TokenType::STRING_LITERAL, value);
                }
                else
                    Advance();
            }

            tokens += Token(TokenType::END_OF_FILE, "");
            return tokens;
        }

        static Shared<Lexer> Create(const NarrowString& input)
        {
            Shared<Lexer> result(new Lexer);

            result->input = input;
            result->length = input.Length();

            return result;
        }


    private:

        Lexer() = default;

        NarrowString input;
        size_t position = 0;
        size_t length = 0;

        char Peek() const 
        { 
            return position < length ? input[position] : '\0'; 
        }

        char PeekNext() const 
        { 
            return position + 1 < length ? input[position + 1] : '\0'; 
        }

        char Advance() 
        { 
            return input[position++]; 
        }

        bool IsAtEnd() const 
        { 
            return position >= length; 
        }
    };
}