#pragma once

#include <variant>
#include "Util/XXML/Lexer.hpp"

namespace Invasion::Util::XXML
{
	class Scope;

    struct Value
    {
        std::variant<
            NarrowString,
            double,
            bool,
            MutableArray<Value>,
            Shared<Scope> 
        > data;

        Value() = default;

        Value(const NarrowString& str) : data(str) {}
        Value(double num) : data(num) {}
        Value(bool boolean) : data(boolean) {}
        Value(const MutableArray<Value>& array) : data(array) {}
        Value(const Shared<Scope>& scope) : data(scope) {}
    };


    class Variable
    {

    public:

        NarrowString name;
        Value value;
    };

    class Scope
    {

    public:

        UnorderedMap<NarrowString, Variable> variables;
        UnorderedMap<NarrowString, Shared<Scope>> namespaces;

        bool Exists(const NarrowString& path)
        {
            auto components = SplitPath(path);
            return Exists(components);
        }

        template <typename T>
        T Get(const NarrowString& path)
        {
            auto components = SplitPath(path);
            Shared<Value> value = GetValue(components);

            if (!value)
                throw std::runtime_error(NarrowString("Value not found at path: ") + path);

            if (auto result = GetValueAs<T>(*value))
                return *result;

            throw std::runtime_error(NarrowString("Value at path '") + path + "' is not of the requested type");
        }

    private:
        
        MutableArray<NarrowString> SplitPath(const NarrowString& path)
        {
            MutableArray<NarrowString> components;

            size_t start = 0;
            size_t end = 0;

            while ((end = path.find('.', start)) != NarrowString::NullPosition)
            {
                components += path.SubString<char>(start, end - start);
                start = end + 1;
            }

            components += path.SubString<char>(start);

            return components;
        }

        bool Exists(const MutableArray<NarrowString>& components, size_t index = 0)
        {
            if (index >= components.Length())
                return false;

            const auto& key = components[index];

            auto nsIt = namespaces.find(key);

            if (nsIt != namespaces.end())
            {
                if (index + 1 == components.Length())
                    return true;

                return nsIt->second->Exists(components, index + 1);
            }

            auto varIt = variables.find(key);

            if (varIt != variables.end())
            {
                if (index + 1 == components.Length())
                    return true; 

                if (auto scopePtr = std::get_if<Shared<Scope>>(&varIt->second.value.data))
                    return (*scopePtr)->Exists(components, index + 1);

                return false;
            }

            return false;
        }

        Shared<Value> GetValue(const MutableArray<NarrowString>& components, size_t index = 0)
        {
            if (index >= components.Length())
                return nullptr;

            const auto& key = components[index];

            auto nsIt = namespaces.find(key);

            if (nsIt != namespaces.end())
            {
                if (index + 1 == components.Length())
                    return std::make_shared<Value>(nsIt->second);
                
                return nsIt->second->GetValue(components, index + 1);
            }

            auto varIt = variables.find(key);

            if (varIt != variables.end())
            {
                if (index + 1 == components.Length())
                    return std::make_shared<Value>(varIt->second.value);

                if (auto scopePtr = std::get_if<Shared<Scope>>(&varIt->second.value.data))
                    return (*scopePtr)->GetValue(components, index + 1);

                return nullptr;
            }

            return nullptr;
        }

        template <typename T>
        std::optional<T> GetValueAs(Value& value)
        {
            if constexpr (std::is_same_v<T, NarrowString>)
            {
                if (auto ptr = std::get_if<NarrowString>(&value.data))
                    return *ptr;
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                if (auto ptr = std::get_if<double>(&value.data))
                    return *ptr;
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                if (auto ptr = std::get_if<bool>(&value.data))
                    return *ptr;
            }
            else if constexpr (std::is_same_v<T, MutableArray<Value>>)
            {
                if (auto ptr = std::get_if<MutableArray<Value>>(&value.data))
                    return *ptr;
            }
            else if constexpr (std::is_same_v<T, Shared<Scope>>)
            {
                if (auto ptr = std::get_if<Shared<Scope>>(&value.data))
                    return *ptr;
            }
            else
                return std::nullopt;
            
            return std::nullopt;
        }
    };

    class Parser
    {

    public:

        Shared<Scope> Parse()
        {
            current = 0;
            auto rootScope = std::make_shared<Scope>();

            while (!IsAtEnd())
                ParseStatement(rootScope);
            
            return rootScope;
        }

        static Shared<Parser> Create(const MutableArray<Token>& tokens)
        {
            Shared<Parser> result(new Parser);

            result->tokens = tokens;

            return result;
        }

    private:

        Parser() = default;

        MutableArray<Token> tokens;
        size_t current = 0;

        Token& Peek() 
        { 
            return tokens[current]; 
        }

        Token& Advance() 
        { 
            return tokens[current++]; 
        }

        bool IsAtEnd() const 
        { 
            return current >= tokens.Length() || tokens[current].type == TokenType::END_OF_FILE; 
        }

        void ParseStatement(Shared<Scope>& scope)
        {
            Token& token = Peek();

            if (token.type == TokenType::TAG_OPEN)
            
                ParseTag(scope);
            
            else if (token.type == TokenType::BRACKET_OPEN)
                ParseNamespace(scope);
            else if (token.type == TokenType::IDENTIFIER)
                ParseAssignment(scope);
            else
                Advance();
        }
        
        void ParseTag(Shared<Scope>& scope)
        {
            Consume(TokenType::TAG_OPEN, "Expected '<'");
            Token& nameToken = Consume(TokenType::IDENTIFIER, "Expected tag name");
            Consume(TokenType::ASSIGN, "Expected '=' after tag name");
            Value value = ParseValue();
            Consume(TokenType::TAG_CLOSE, "Expected '>'");

            Variable var{ nameToken.value, value };
            scope->variables[nameToken.value] = var;
        }

        void ParseNamespace(Shared<Scope>& parentScope)
        {
            Consume(TokenType::BRACKET_OPEN, "Expected '['");
            Consume(TokenType::TAG_OPEN, "Expected '<' after '['");
            Token& nameToken = Consume(TokenType::IDENTIFIER, "Expected namespace name");
            Consume(TokenType::TAG_CLOSE, "Expected '>' after namespace name");

            auto namespaceScope = std::make_shared<Scope>();

            while (Peek().type != TokenType::BRACKET_CLOSE && !IsAtEnd())
            {
                ParseStatement(namespaceScope);
            }

            Consume(TokenType::BRACKET_CLOSE, "Expected ']'");

            parentScope->namespaces[nameToken.value] = namespaceScope;
        }

        void ParseAssignment(Shared<Scope>& scope)
        {
            Token& nameToken = Consume(TokenType::IDENTIFIER, "Expected identifier");
            Consume(TokenType::ASSIGN, "Expected '=' after identifier");

            if (Peek().type == TokenType::BRACE_OPEN)
            {
                Value objValue = ParseObject();
                Variable var{ nameToken.value, objValue };
                scope->variables[nameToken.value] = var;
            }
            else
            {
                Value value = ParseValue();
                Variable var{ nameToken.value, value };
                scope->variables[nameToken.value] = var;
            }
        }

        Value ParseValue()
        {
            Token& token = Peek();

            if (token.type == TokenType::STRING_LITERAL)
            {
                Advance();
                return token.value;
            }
            else if (token.type == TokenType::NUMBER)
            {
                Advance();
                return std::stod(std::string(token.value));
            }
            else if (token.type == TokenType::BOOLEAN)
            {
                Advance();
                return token.value == "true";
            }
            else if (token.type == TokenType::BRACKET_OPEN)
            {
                return ParseArray();
            }
            else
            {
                throw std::runtime_error("Expected value");
            }
        }

        Value ParseArray()
        {
            Consume(TokenType::BRACKET_OPEN, "Expected '['");

            MutableArray<Value> elements;

            if (Peek().type != TokenType::BRACKET_CLOSE)
            {
                do
                {
                    elements += ParseValue();

                    if (Peek().type != TokenType::COMMA) 
                        break;

                    Advance();
                } while (true);
            }

            Consume(TokenType::BRACKET_CLOSE, "Expected ']'");
            return elements;
        }

        Value ParseObject()
        {
            Consume(TokenType::BRACE_OPEN, "Expected '{'");

            auto objScope = std::make_shared<Scope>();

            while (Peek().type != TokenType::BRACE_CLOSE && !IsAtEnd())
                ParseAssignment(objScope);
            
            Consume(TokenType::BRACE_CLOSE, "Expected '}'");
            return objScope;
        }

        Token& Consume(TokenType expectedType, const NarrowString& errorMessage)
        {
            if (Peek().type == expectedType) 
                return Advance();

            throw std::runtime_error(errorMessage);
        }
    };
}