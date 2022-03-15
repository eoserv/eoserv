/* eoplus/context.hpp
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EOPLUS_CONTEXT_HPP_INCLUDED
#define EOPLUS_CONTEXT_HPP_INCLUDED

#include "fwd/context.hpp"

#include "../fwd/eoplus.hpp"

#include "../util/variant.hpp"

#include <deque>
#include <functional>
#include <string>

namespace EOPlus
{
	class Context
	{
		private:
			const Quest* quest;
			const State* state;
			std::string state_name;
			bool finished;

		protected:
			virtual void BeginState(const std::string& name, const State& state) = 0;
			virtual bool DoAction(const Action& action) = 0;
			virtual bool CheckRule(const Expression& expr) = 0;

		public:
			Context(const Quest* quest);

			const State* GetState() const;
			std::string StateName() const;

			void SetState(const std::string& state, bool do_actions = true);

			void DoActions();

			const Rule* GetGoal() const;

			bool Finished() const;

			bool QueryRule(std::string rule) const;
			bool QueryRule(std::string rule, std::function<bool(const std::deque<util::variant>&)> arg_check) const;

			bool TriggerRule(std::string rule);
			bool TriggerRule(std::string rule, std::function<bool(const std::deque<util::variant>&)> arg_check);

			bool CheckRules();

			virtual ~Context();
	};
}

#endif // EOPLUS_CONTEXT_HPP_INCLUDED
