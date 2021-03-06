#include "Grader_eval.h"
#include "Self_eval.h"
#include "Eval_item.h"
#include "Submission.h"
#include "auth/User.h"
#include "../Session.h"
#include "../common/exceptions.h"
#include "../common/format.h"
#include "../common/stringify.h"
#include "../common/paths.h"
#include "../common/util.h"

#include <Wt/Dbo/Impl.h>
#include <Wt/Json/Value.h>

#include <algorithm>

namespace J = Wt::Json;

DBO_INSTANTIATE_TEMPLATES(Grader_eval)

Grader_eval::Grader_eval(const dbo::ptr<Self_eval>& self_eval,
                         const dbo::ptr<User>& grader)
        : Abstract_evaluation(0),
          self_eval_(self_eval),
          grader_(grader),
          status_(Status::editing)
{ }

Grader_eval::Grader_eval(const dbo::ptr<Self_eval>& self_eval,
                         const dbo::ptr<User>& grader,
                         double score,
                         const std::string& explanation)
        : Abstract_evaluation(score, explanation),
          self_eval_(self_eval),
          grader_(grader),
          status_(Status::ready)
{ }

dbo::ptr<Grader_eval> Grader_eval::get_for(
        const dbo::ptr<Self_eval>& self_eval,
        Session& session)
{
    auto result = self_eval->grader_eval();

    if (result)
        return result;
    else
        return session.addNew<Grader_eval>(self_eval, session.user());
}

std::string Grader_eval::plain_score_string() const
{
    switch (status()) {
        case Status::ready:
            return Abstract_evaluation::plain_score_string();
        case Status::held_back:
            return "[held back]";
        case Status::editing:
            return "[editing]";
        case Status::regrade:
            return "[regrade]";
    }
}

Score_owner Grader_eval::score_owner(Viewing_context const& cxt) const
{
    WString score, owner;

    if (is_ready() || cxt.viewer->can_grade()) {
        score = plain_score_string();

        if (! grader()->can_grade())
            owner = "auto";

        else if (cxt.viewer->can_grade() || grader()->can_admin())
            owner = grader()->nice_name(false);
    }

    return {score, owner};
}

const Wt::Dbo::ptr<Eval_item>& Grader_eval::eval_item() const
{
    return self_eval()->eval_item();
}

const Wt::Dbo::ptr<Submission>& Grader_eval::submission() const
{
    return self_eval()->submission();
}

bool Grader_eval::can_view(dbo::ptr<User> const& user) const {
    return user->can_grade() ||
            (self_eval()->submission()->can_view(user) && status() == Status::ready);
}

void Grader_eval::check_can_view(dbo::ptr<User> const& user) const
{
    if (user->can_grade()) return;

    if (!self_eval()->submission()->can_view(user))
        throw Resource_not_found();

    if (status() != Status::ready)
        throw Access_check_failed("That isn’t ready yet.");
}

std::string Grader_eval::rest_uri() const {
    return api::paths::Submissions_1_evals_2_grader(self_eval()->submission()->id(),
                                                    self_eval()->eval_item()->sequence());
}

Wt::Json::Object Grader_eval::to_json(Viewing_context const& cxt) const {
    J::Object result = Abstract_evaluation::to_json();

    result["uri"]           = J::Value(rest_uri());
    result["grader"]        = J::Value(score_owner(cxt).owner);
    result["status"]        = J::Value(stringify(status()));

    return result;
}

namespace rc = std::regex_constants;

static std::regex const editing_re("editing", rc::icase);
static std::regex const held_back_re("held_back", rc::icase);
static std::regex const ready_re("ready", rc::icase);
static std::regex const regrade_re("regrade", rc::icase);

char const* Enum<Grader_eval::Status>::show(Grader_eval::Status role)
{
    switch (role) {
    case Grader_eval::Status::editing:
        return "editing";
    case Grader_eval::Status::held_back:
        return "held_back";
    case Grader_eval::Status::ready:
        return "ready";
    case Grader_eval::Status::regrade:
        return "regrade";
    }
}

Grader_eval::Status Enum<Grader_eval::Status>::read(std::string_view role)
{
    auto match = [=] (auto re) {
        return std::regex_match(role.begin(), role.end(), re);
    };

    if (match(editing_re))   return Grader_eval::Status::editing;
    if (match(held_back_re)) return Grader_eval::Status::held_back;
    if (match(ready_re))     return Grader_eval::Status::ready;
    if (match(regrade_re))   return Grader_eval::Status::ready;

    throw std::invalid_argument{"Could not parse role"};
}

namespace Wt::Dbo {

char const*
sql_value_traits<Grader_eval_status, void>::type(
        SqlConnection* conn, int size)
{
    return repr_trait::type(conn, size);
}

void
sql_value_traits<Grader_eval_status, void>::bind(
        const Grader_eval_status& v,
        SqlStatement* statement,
        int column,
        int size)
{
    int value = static_cast<int>(v);
    repr_trait::bind(value, statement, column, size);
}

bool
sql_value_traits<Grader_eval_status, void>::read(
        Grader_eval_status& v,
        SqlStatement* statement,
        int column,
        int size)
{
    int value;
    bool result = repr_trait::read(value, statement, column, size);
    v = static_cast<Grader_eval_status>(value);
    return result;
}


} // end namespace Wt::Dbo
