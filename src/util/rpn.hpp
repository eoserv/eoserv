
/* $Id$
 * EOSERV is released under the zlib license.
 * See LICENSE.txt for more info.
 */

#ifndef UTIL_RPN_HPP_INCLUDED
#define UTIL_RPN_HPP_INCLUDED

#include <stack>
#include <string>
#include <unordered_map>

#include "variant.hpp"

namespace util
{

static const double rpn_cmp_epsilon = 0.0001;
static const double rpn_cmp_epsilon_2 = rpn_cmp_epsilon / 2.0;

std::stack<util::variant> rpn_parse(std::string expr);
double rpn_eval(std::stack<util::variant>, std::unordered_map<std::string, double> vars);

}

#endif // UTIL_RPN_HPP_INCLUDED
