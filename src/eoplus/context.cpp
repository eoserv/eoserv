
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#include "context.hpp"

#include <stdexcept>

#include "../util.hpp"

#include "../eoplus.hpp"

namespace EOPlus
{
	Context::Context(const Quest* quest)
		: quest(quest)
		, state(0)
		, finished(false)
	{ }

	const State* Context::GetState() const
	{
		return this->state;
	}

	void Context::SetState(const std::string& state, bool do_actions)
	{
		std::string state_id = util::lowercase(state);

		if (state_id == "end")
		{
			this->state = 0;
			this->finished = true;
		}

		auto it = this->quest->states.find(state_id);

		if (it == this->quest->states.end())
		{
			if (state_id == "end")
				return;
			else
				throw Runtime_Error("Unknown quest state: " + state_id);
		}

		this->finished = (state_id == "end");
		this->state = &it->second;
		this->BeginState(state, *this->state);

		if (do_actions)
		{
			UTIL_CFOREACH(this->state->actions, action)
			{
				if (this->DoAction(action))
					return;
			}

			this->CheckRules();
		}
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

		UTIL_CFOREACH(this->state->rules, check_rule)
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

		UTIL_CFOREACH(this->state->rules, check_rule)
		{
			if (check_rule.expr.function == rule && arg_check(check_rule.expr.args))
			{
				this->DoAction(check_rule.action);
				return true;
			}
		}

		return false;
	}

	void Context::CheckRules()
	{
		if (!this->state)
			throw std::runtime_error("No state selected");

		UTIL_CFOREACH(this->state->rules, rule)
		{
			if (this->CheckRule(rule))
			{
				if (this->DoAction(rule.action))
					return;
			}
		}
	}

	Context::~Context()
	{

	}
}
