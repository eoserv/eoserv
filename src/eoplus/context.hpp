
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef EOPLUS_CONTEXT_HPP_INCLUDED
#define EOPLUS_CONTEXT_HPP_INCLUDED

#include "fwd/context.hpp"

#include "util.hpp"

#include "../fwd/eoplus.hpp"

#include <deque>
#include <memory>
#include <string>

namespace EOPlus
{
	class Context
	{
		private:
			const Quest* quest;
			const State* state;
			bool finished;

		protected:
			virtual void BeginState(const std::string& name, const State& state) = 0;
			virtual bool DoAction(const Action& action) = 0;
			virtual bool CheckRule(const Rule& rule) = 0;

		public:
			Context(const Quest* quest);

			const State* GetState() const;
			void SetState(const std::string& state, bool do_actions = true);

			const Rule* GetGoal() const;

			bool Finished() const;

			bool QueryRule(std::string rule) const;
			bool QueryRule(std::string rule, std::function<bool(const std::deque<util::variant>&)> arg_check) const;

			bool TriggerRule(std::string rule);
			bool TriggerRule(std::string rule, std::function<bool(const std::deque<util::variant>&)> arg_check);

			void CheckRules();

			virtual ~Context();
	};
}

#endif // EOPLUS_CONTEXT_HPP_INCLUDED
