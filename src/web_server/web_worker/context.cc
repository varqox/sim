#include "context.hh"
#include "simlib/string_view.hh"
#include "simlib/time.hh"
#include "src/web_server/http/response.hh"
#include "src/web_server/ui_template.hh"

#include <optional>

using web_server::http::Response;

namespace web_server::web_worker {

void Context::open_session() {
    session = std::nullopt;
    auto session_id = request.get_cookie(Session::id_cookie_name);
    if (session_id.empty()) {
        return; // Optimization (no mysql query) for empty or nonexistent cookie
    }
    Session s;
    auto stmt =
        mysql.prepare("SELECT s.csrf_token, s.user_id, u.type, u.username, s.data FROM "
                      "sessions s JOIN users u ON u.id=s.user_id WHERE s.id=? AND expires>=?");
    stmt.bind_and_execute(session_id, mysql_date());
    stmt.res_bind_all(s.csrf_token, s.user_id, s.user_type, s.username, s.data);
    if (not stmt.next()) {
        // Session expired or was deleted
        cookie_changes.set(Session::id_cookie_name, "", 0);
        return;
    }
    s.id = session_id;
    s.orig_data = s.data;
    session = s;
}

void Context::close_session() {
    assert(session.has_value());
    if (session->data != session->orig_data) {
        auto stmt = mysql.prepare("UPDATE sessions SET data=? WHERE id=?");
        stmt.bind_and_execute(session->data, session->id);
    }
    session = std::nullopt;
}

bool Context::session_has_expired() noexcept {
    return !request.get_cookie(Session::id_cookie_name).empty() and !session.has_value();
}

Response response(
    StringView status_code, decltype(Context::cookie_changes)&& cookie_changes,
    StringView content, StringView content_type) {
    auto resp = Response{Response::TEXT, status_code};
    resp.headers["content-type"] = content_type.to_string();
    resp.cookies = std::move(cookie_changes);
    resp.content = content;
    return resp;
}

Response Context::response_ok(StringView content, StringView content_type) {
    return response("200 OK", std::move(cookie_changes), content, content_type);
}

Response Context::response_json(StringView content) {
    return response(
        "200 OK", std::move(cookie_changes), content, "application/json; charset=utf-8");
}

Response Context::response_400(StringView content, StringView content_type) {
    return response("400 Bad Request", std::move(cookie_changes), content, content_type);
}

Response Context::response_403(StringView content, StringView content_type) {
    // In order not to reveal information (e.g. existence of something) only admins may see
    // error 403 and the @p content
    if (session) {
        using UT = sim::users::User::Type;
        switch (session->user_type) {
        case UT::ADMIN: {
            assert(not session_has_expired());
            return response("403 Forbidden", std::move(cookie_changes), content, content_type);
        }
        case UT::TEACHER:
        case UT::NORMAL: break;
        }
    }
    return response_404();
}

http::Response Context::response_404() {
    static constexpr CStringView session_expired_msg =
        "Your session has expired, please try to sign in and then try again.";
    auto content = session_has_expired() ? session_expired_msg : "";
    return response(
        "404 Not Found", std::move(cookie_changes), content, "text/plain; charset=utf-8");
}

Response Context::response_ui(StringView title, StringView javascript_code) {
    auto resp = response_ok("", "text/html; charset=utf-8");
    // TODO: merge *_ui_template into one function after getting rid of the old code requiring
    // split form
    begin_ui_template(
        resp,
        {
            .title = title,
            .session = session,
            .notifications = "",
        });
    resp.content.append(javascript_code);
    end_ui_template(resp);
    return resp;
}

} // namespace web_server::web_worker
