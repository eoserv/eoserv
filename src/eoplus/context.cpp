/* eoplus/context.cpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "context.hpp"

#include "../eoplus.hpp"

#include "../util.hpp"
#include "../util/variant.hpp"

#include <deque>
#include <functional>
#include <stdexcept>
#include <string>

namespace EOPlus
{
	static int recursive_depth = 0;
	static const int max_recursion = 100;

	Context::Context(const Quest* quest)
		: quest(quest)
		, state(0)
		, finished(false)
	{ }

	const State* Context::GetState() const
	{
		return this->state;
	}

	std::string Context::StateName() const
	{
		return this->state_name;
	}

	void Context::SetState(const std::string& state, bool do_actions)
	{
		std::string state_id = util::lowercase(state);

		this->state_name = state_id;

		if (state_id == "end" || state_id == "done")
		{
			this->state = 0;
			this->finished = true;
		}

		auto it = this->quest->states.find(state_id);

		if (it == this->quest->states.end())
		{
			if (state_id == "end" || state_id == "done")
				return;
			else
				throw Runtime_Error("Unknown quest state: " + state_id);
		}

		this->finished = (state_id == "end");
		this->state = &it->second;
		this->BeginState(state, *this->state);

		if (do_actions)
			this->DoActions();
	}

	void Context::DoActions()
	{
		if (!this->state)
		{
			if (this->finished)
				return;
			else
				throw std::runtime_error("No state selected");
		}

		if (++recursive_depth > max_recursion)
		{
			--recursive_depth;
			throw std::runtime_error("Quest action recursion too deep");
		}

		try
		{
			auto do_action = [&](const EOPlus::Action& action)
			{
				if (this->DoAction(action))
				{
					// *this may not be valid here
					--recursive_depth;
					return;
				}
			};

			bool last_cond = false;

			UTIL_FOREACH(this->state->actions, action)
			{
				if (action.cond == EOPlus::Action::ElseIf || action.cond == EOPlus::Action::Else)
				{
					if (last_cond)
						continue;
				}

				if (action.cond == EOPlus::Action::If || action.cond == EOPlus::Action::ElseIf)
				{
					last_cond = this->CheckRule(action.cond_expr);

					if (last_cond)
						do_action(action);
				}
				else
				{
					do_action(action);
				}
			}
		}
		catch (...)
		{
			--recursive_depth;
			throw;
		}

		--recursive_depth;
		this->CheckRules();
	}

	const Rule* Context::GetGoal() const
	{
		if (!this->state || this->state->rules.empty())
			return 0;

		return &this->state->rules[this->state->goal_rule];
	}

	bool Context::Finished() const
	{
		return this->finished;
	}

	bool Context::QueryRule(std::string rule) const
	{
		return this->QueryRule(rule, [](const std::deque<util::variant>&) { return true; });
	}

	bool Context::QueryRule(std::string rule, std::function<bool(const std::deque<util::variant>&)> arg_check) const
	{
		if (!this->state)
			return false;

		UTIL_FOREACH(this->state->rules, check_rule)
		{
			if (check_rule.expr.function == rule && arg_check(check_rule.expr.args))
				return true;
		}

		return false;
	}

	bool Context::TriggerRule(std::string rule)
	{
		return this->TriggerRule(rule, [](const std::deque<util::variant>&) { return true; });
	}

	bool Context::TriggerRule(std::string rule, std::function<bool(const std::deque<util::variant>&)> arg_check)
	{
		if (!this->state)
			return false;

		UTIL_FOREACH(this->state->rules, check_rule)
		{
			if (check_rule.expr.function == rule && arg_check(check_rule.expr.args))
			{
				this->DoAction(check_rule.action);
				// *this may not be valid here
				return true;
			}
		}

		return false;
	}

	bool Context::CheckRules()
	{
		if (!this->state)
		{
			if (this->finished)
				return false;
			else
				throw std::runtime_error("No state selected");
		}

		if (++recursive_depth > max_recursion)
		{
			--recursive_depth;
			throw std::runtime_error("Quest action recursion too deep");
		}

		try
		{
			UTIL_FOREACH(this->state->rules, rule)
			{
				if (this->CheckRule(rule.expr))
				{
					if (this->DoAction(rule.action))
					{
						// *this may not be valid here
						--recursive_depth;
						return true;
					}
				}
			}
		}
		catch (...)
		{
			--recursive_depth;
			throw;
		}

		--recursive_depth;

		return false;
	}

	Context::~Context()
	{

	}
}
