/* eoplus/parse.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EOPLUS_PARSE_HPP_INCLUDED
#define EOPLUS_PARSE_HPP_INCLUDED

#include "fwd/parse.hpp"

#include "../eoplus.hpp"

#include <algorithm>
#include <deque>
#include <functional>
#include <memory>
#include <stack>
#include <string>
#include <utility>

namespace EOPlus
{
	class Parser_Token_Server_Base
	{
		private:
			std::stack<Token> token_buffer;
			int line;

			Token reject_token;
			int reject_line;

		protected:
			virtual Token xGetToken() = 0;

		public:
			Parser_Token_Server_Base();

			Token GetToken(unsigned int allow = 0xFFFFFFFF);
			void PutBack(Token);

			Token RejectToken() const { return this->reject_token; }
			int RejectLine() const { return std::max(this->line, this->reject_line); }

			int Line() const { return this->line; }

			virtual ~Parser_Token_Server_Base();
	};

	template <class IT> class Parser_Token_Server : public Parser_Token_Server_Base
	{
		public:
			typedef IT iterator_type;

		private:
			IT it;
			IT end;

		protected:
			Token xGetToken()
			{
				if (it == end)
					return Token();

				return *(it++);
			}

		public:
			Parser_Token_Server(IT begin, IT end)
				: it(begin)
				, end(end)
			{ }
	};

	class Parser
	{
		private:
			Quest& quest;
			std::unique_ptr<Parser_Token_Server_Base> tok;

			bool GetToken(Token& t, unsigned int allow = 0xFFFFFFFF)
			{
				t = this->tok->GetToken(allow);
				return bool(t);
			}

			bool GetTokenIf(Token& t, std::function<bool(const Token&)> f, unsigned int allow = 0xFFFFFFFF)
			{
				t = this->tok->GetToken(allow);

				if (t && !f(t))
				{
					this->tok->PutBack(t);
					t = Token();
				}

				return bool(t);
			}

		public:
			template <class IT> Parser(Quest& quest, IT begin, IT end)
				: quest(quest)
				, tok(new Parser_Token_Server<IT>(begin, end))
			{ }

			unsigned int ParseVersion();
			std::string ParseQuestName();
			std::string ParseDesc();

			std::string ParseStateDesc();

			std::deque<Scope> ParseScopes();
			Expression ParseExpression();

			Action ParseAction();
			Rule ParseRule();

			void ParseRuleActionBlock(std::deque<Rule>& rules, std::deque<Action>& actions, std::function<bool(std::string)> custom_handler, std::function<void(std::string)> error_handler);
			Info ParseMain();

			std::pair<std::string, State> ParseStateBlock();
			Info ParseMainBlock();

			void ParseGlobal();
	};
}

#endif // EOPLUS_PARSE_HPP_INCLUDED
