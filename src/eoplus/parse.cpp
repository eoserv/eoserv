/* eoplus/parse.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "parse.hpp"

#include <deque>
#include <functional>
#include <map>
#include <string>
#include <utility>

namespace EOPlus
{
	Parser_Token_Server_Base::Parser_Token_Server_Base()
		: line(1)
		, reject_line(1)
	{ }

	Token Parser_Token_Server_Base::GetToken(unsigned int allow)
	{
		Token t;
		int newlines = 0;

		while (true)
		{
			if (!this->token_buffer.empty())
			{
				t = this->token_buffer.top();
				this->token_buffer.pop();
			}
			else
			{
				t = this->xGetToken();
			}

			if (t.type == Token::NewLine)
			{
				++this->line;

				if ((Token::NewLine & allow) == Token::NewLine)
					break;

				++newlines;
			}
			else
			{
				break;
			}
		}

		t.newlines = newlines;

		if ((t.type & allow) != t.type)
		{
			this->PutBack(t);
			return Token();
		}

		return t;
	}

	void Parser_Token_Server_Base::PutBack(Token t)
	{
		this->reject_token = t;
		this->reject_line = this->line;
		this->line -= t.newlines;

		if (t.type == Token::NewLine)
			--this->line;

		while (t.newlines-- > 0)
			this->token_buffer.push(Token(Token::NewLine));

		this->token_buffer.push(t);
	}

	Parser_Token_Server_Base::~Parser_Token_Server_Base()
	{ }

	static std::string op_to_string(unsigned char h)
	{
		switch (h)
		{
			case UOP1('('):          return "(";
			case UOP1(')'):          return ")";
			case UOP1('&'):          return "&";
			case UOP2('&', '&'):     return "&&";
			case UOP1('|'):          return "|";
			case UOP2('|', '|'):     return "||";
			case UOP1('='):          return "=";
			case UOP2('=', '='):     return "==";
			case UOP2('!', '='):     return "!=";
			case UOP1('<'):          return "<";
			case UOP2('<', '='):     return "<=";
			case UOP1('>'):          return ">";
			case UOP2('>', '='):     return ">=";
			case UOP1('+'):          return "+";
			case UOP1('-'):          return "-";
			case UOP1('*'):          return "*";
			case UOP1('/'):          return "/";
			case UOP1('%'):          return "%";
			case UOP1('~'):          return "~";
			case UOP1('!'):          return "!";
			case UOP2('-', UOP_ALT): return "-";
			default:                 return "(unknown)";
		}
	}

	static std::string cut_string(std::string s)
	{
		if (s.length() > 16)
			return s.substr(0, 13) + "...";

		return s;
	}

	static std::string token_got_string(Token t)
	{
		switch (t.type)
		{
			case Token::Invalid:    return "invalid token";
			case Token::Identifier: return "identifier '" + std::string(t.data) + "'";
			case Token::String:     return "string '" + cut_string(t.data) + "'";
			case Token::Integer:    return "integer '" + std::string(t.data) + "'";
			case Token::Float:      return "float '" + std::string(t.data) + "'";
			case Token::Boolean:    return "boolean '" + std::string(bool(t.data) ? "true" : "false") + "'";
			case Token::Operator:   return "operator '" + op_to_string(int(t.data)) + "'";
			case Token::Symbol:     return "symbol '" + std::string(t.data) + "'";
			case Token::NewLine:    return "new-line";
			case Token::EndOfFile:  return "end of file";
		}

		return "unknown token";
	}

#define PARSER_ERROR(s) throw Parser_Error((s), this->tok->RejectLine());
#define PARSER_ERROR_GOT(s) throw Parser_Error(std::string((s)) + " Got: " + token_got_string((this->tok->RejectToken())), this->tok->RejectLine());

	unsigned int Parser::ParseVersion()
	{
		typedef unsigned int ui;
		unsigned int version = 0;
		Token t;

		if (this->GetToken(t, Token::Integer))
		{
			version += ui(int(t.data)) << 16;

			if (this->GetTokenIf(t, [](const Token& t) { return std::string(t.data) == "."; }, Token::Symbol))
			{
				if (this->GetToken(t, Token::Integer))
					version |= ui(int(t.data)) & 0xFFFF;
				else
					PARSER_ERROR_GOT("Expected number after '.' in version.");
			}
		}
		else
		{
			PARSER_ERROR_GOT("Expected number after 'version'");
		}

		return version;
	}

	std::deque<Scope> Parser::ParseScopes()
	{
		static const std::map<std::string, Scope::Type> scope_id_map{
			{"character", Scope::Character},
			{"npc", Scope::NPC},
			{"map", Scope::Map},
			{"world", Scope::World}
		};

		std::deque<Scope> scopes;
		Token t;

		while (this->GetToken(t, Token::Identifier))
		{
			auto it = scope_id_map.find(std::string(t.data));

			if (it == scope_id_map.end())
			{
				this->tok->PutBack(t);
				return scopes;
			}

			Scope s;
			s.type = it->second;

			scopes.push_back(s);

			if (!this->GetTokenIf(t, [](const Token& t) { return std::string(t.data) == "."; }, Token::Symbol))
				PARSER_ERROR_GOT("Expected '.' after scope-identifier.");
		}

		return scopes;
	}

	Expression Parser::ParseExpression()
	{
		Expression expression;
		Token t;

		if (this->GetToken(t, Token::Identifier))
		{
			if (std::string(t.data) == "goto")
			{
				Token tt;

				if (this->GetToken(tt, Token::Identifier))
				{
					expression.function = "setstate";
					expression.args = {std::string(tt.data)};
				}
				else
				{
					PARSER_ERROR_GOT("Expected identifier after 'goto'.");
				}
			}
			else
			{
				expression.scopes = this->ParseScopes();
				expression.function = std::string(t.data);

				if (this->GetTokenIf(t, [](const Token& t) { return int(t.data) == UOP1('('); }, Token::Operator))
				{
					if (this->GetTokenIf(t, [](const Token& t) { return int(t.data) == UOP1(')'); }, Token::Operator))
					{
						return expression;
					}

					while (this->GetToken(t, Token::String | Token::Float | Token::Integer))
					{
						expression.args.push_back(t.data);

						if (this->GetTokenIf(t, [](const Token& t)
						{
							return ((t.type == Token::Symbol && std::string(t.data) == ",")
							     || (t.type == Token::Operator && int(t.data) == UOP1(')')));
						}, Token::Symbol | Token::Operator))
						{
							if (int(t.data) == UOP1(')'))
								return expression;
						}
						else
						{
							PARSER_ERROR_GOT("Expected ',' or ')' after function-argument in function-argument-list.");
						}
					}

					PARSER_ERROR_GOT("Expected function-argument in function-argument-list.");
				}
				else
				{
					PARSER_ERROR_GOT("Expected '(' after function-name.");
				}
			}
		}

		return expression;
	}

	std::string Parser::ParseQuestName()
	{
		Token t;

		if (this->GetToken(t, Token::String))
			return std::string(t.data);

		PARSER_ERROR_GOT("Expected string after 'questname'.");
	}

	std::string Parser::ParseDesc()
	{
		Token t;

		if (this->GetToken(t, Token::String))
			return std::string(t.data);

		PARSER_ERROR_GOT("Expected string after 'desc'.");
	}

	Action Parser::ParseAction()
	{
		Action action;
		action.cond = Action::None;
		action.expr = this->ParseExpression();
		return action;
	}

	Rule Parser::ParseRule()
	{
		Rule rule;
		rule.expr = this->ParseExpression();
		rule.action = this->ParseAction();
		return rule;
	}

	void Parser::ParseRuleActionBlock(std::deque<Rule>& rules, std::deque<Action>& actions, std::function<bool(std::string)> custom_handler, std::function<void(std::string)> error_handler)
	{
		Token t;

		while (this->GetToken(t, Token::Identifier | Token::Symbol))
		{
			if (t.type == Token::Identifier)
			{
				if (std::string(t.data) == "rule")
				{
					rules.push_back(this->ParseRule());
				}
				else if (std::string(t.data) == "action")
				{
					actions.push_back(this->ParseAction());
				}
				else if (std::string(t.data) == "if")
				{
					Rule rule = this->ParseRule();
					Action action;
					action.cond = Action::If;
					action.cond_expr = rule.expr;
					action.expr = rule.action.expr;
					actions.push_back(action);
				}
				else if (std::string(t.data) == "elseif" || std::string(t.data) == "elif")
				{
					Rule rule = this->ParseRule();
					Action action;
					action.cond = Action::ElseIf;
					action.cond_expr = rule.expr;
					action.expr = rule.action.expr;
					actions.push_back(action);
				}
				else if (std::string(t.data) == "else")
				{
					Action action = this->ParseAction();
					action.cond = Action::Else;
					actions.push_back(action);
				}
				else
				{
					if (!custom_handler(std::string(t.data)))
					{
						this->tok->PutBack(t);
						break;
					}
				}
			}
			else if (t.type == Token::Symbol)
			{
				if (std::string(t.data) == "}")
				{
					return;
				}
				else if (std::string(t.data) == ";")
				{
					continue;
				}
				else
				{
					this->tok->PutBack(t);
					break;
				}
			}
		}

		// PARSER_ERROR_GOT("Expected rule-action-block entry (rule/action) or closing brace '}'.")
		error_handler("rule/action/if/elseif/else");
	}

	Info Parser::ParseMain()
	{
		bool has_questname = false;
		bool has_version = false;
		Info info;
		Token t;

		while (this->GetToken(t, Token::Identifier | Token::Symbol))
		{
			if (t.type == Token::Identifier)
			{
				if (std::string(t.data) == "questname")
				{
					if (has_questname)
						PARSER_ERROR("Main-block can only contain one questname.");

					has_questname = true;
					info.name = ParseQuestName();
				}
				else if (std::string(t.data) == "version")
				{
					if (has_version)
						PARSER_ERROR("Main-block can only contain one version.");

					has_version = true;
					info.version = ParseVersion();
				}
				else if (std::string(t.data) == "hidden")
				{
					if (info.hidden != Info::NotHidden)
						PARSER_ERROR("Main-block can only contain one hidden attribute.");

					info.hidden = Info::Hidden;
				}
				else if (std::string(t.data) == "hidden_end")
				{
					if (info.hidden != Info::NotHidden)
						PARSER_ERROR("Main-block can only contain one hidden attribute.");

					info.hidden = Info::HiddenEnd;
				}
				else if (std::string(t.data) == "disabled")
				{
					if (info.disabled)
						PARSER_ERROR("Main-block can only contain one disabled attribute.");

					info.disabled = true;
				}
				else
				{
					break;
				}
			}
			else if (t.type == Token::Symbol)
			{
				if (std::string(t.data) == "}")
					return info;
				else
					break;
			}
		}

		PARSER_ERROR_GOT("Expected main-block entry (questname/version/hidden/hidden_end/disabled) or closing brace '}'.");
	}

	std::pair<std::string, State> Parser::ParseStateBlock()
	{
		Token t;

		if (this->GetToken(t, Token::Identifier))
		{
			std::string name = t.data;

			if (this->GetToken(t, Token::Symbol) && std::string(t.data) == "{")
			{
				bool has_desc = false;
				State state;

				ParseRuleActionBlock(state.rules, state.actions, [&](std::string entry) -> bool
				{
					if (entry == "desc")
					{
						if (has_desc)
							PARSER_ERROR("State can only contain one description.");

						has_desc = true;
						state.desc = this->ParseDesc();
						return true;
					}
					else if (entry == "goal")
					{
						Rule rule = this->ParseRule();
						state.rules.push_back(rule);
						state.goal_rule = state.rules.size() - 1;
						return true;
					}
					else
					{
						return false;
					}
				},
				[&](std::string expected)
				{
					PARSER_ERROR_GOT("Expected state-block entry (" + expected + "/goal/desc) or closing brace '}' in state '" + name + "'.");
				});

				state.name = name;
				return std::pair<std::string, State>(name, state);
			}

			PARSER_ERROR_GOT("Expected opening brace '{' after state-name.");
		}

		PARSER_ERROR_GOT("Expected state-name after 'state'.");
	}

	Info Parser::ParseMainBlock()
	{
		Token t;

		if (this->GetToken(t, Token::Symbol) && std::string(t.data) == "{")
			return ParseMain();

		PARSER_ERROR_GOT("Expected opening brace '{' after 'main'.");
	}

	void Parser::ParseGlobal()
	{
		Token t;

		while (this->GetToken(t, Token::Identifier | Token::EndOfFile))
		{
			if (t.type == Token::Identifier)
			{
				if (std::string(t.data) == "main")
				{
					if (quest.has_info)
						PARSER_ERROR("Quest can only contain one main-block.");

					quest.info = this->ParseMainBlock();
					quest.has_info = true;
				}
				else if (std::string(t.data) == "state")
				{
					auto state = this->ParseStateBlock();

					if (!this->quest.states.insert(state).second)
						PARSER_ERROR("State '" + state.first + "' already exists");
				}
				else
				{
					break;
				}
			}
			else if (t.type == Token::EndOfFile)
			{
				if (!quest.has_info)
					PARSER_ERROR("No main-block in quest file.");

				return;
			}
		}

		PARSER_ERROR_GOT("Expected block-identifier (main/state) in global scope.");
	}
}
