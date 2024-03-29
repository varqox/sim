#pragma once

#include "../http/response.hh"
#include "../web_worker/context.hh"

namespace web_server::users::ui {

http::Response list_users(web_worker::Context& ctx);

http::Response sign_in(web_worker::Context& ctx);

http::Response sign_up(web_worker::Context& ctx);

http::Response sign_out(web_worker::Context& ctx);

http::Response add(web_worker::Context& ctx);

http::Response edit(web_worker::Context& ctx, decltype(sim::users::User::id) user_id);

http::Response change_password(web_worker::Context& ctx, decltype(sim::users::User::id) user_id);

http::Response delete_(web_worker::Context& ctx, decltype(sim::users::User::id) user_id);

http::Response merge_into_another(web_worker::Context& ctx, decltype(sim::users::User::id) user_id);

} // namespace web_server::users::ui
