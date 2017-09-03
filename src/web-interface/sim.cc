#include "sim.h"

#include <simlib/filesystem.h>
// #include <sim/jobs.h>
// #include <simlib/debug.h>
// #include <simlib/logger.h>
// #include <simlib/time.h>

using std::string;
using std::unique_ptr;
using std::vector;

server::HttpResponse Sim::handle(CStringView _client_ip,
	server::HttpRequest req)
{
	client_ip = std::move(_client_ip);
	request = std::move(req);
	resp = server::HttpResponse(server::HttpResponse::TEXT);

	stdlog(request.target);

	// TODO: this is pretty bad-looking
	auto hard_error500 = [&] {
		resp.status_code = "500 Internal Server Error";
		resp.headers["Content-Type"] = "text/html; charset=utf-8";
		resp.content = "<!DOCTYPE html>"
				"<html lang=\"en\">"
				"<head><title>500 Internal Server Error</title></head>"
				"<body>"
					"<center>"
						"<h1>500 Internal Server Error</h1>"
						"<p>Try to reload the page in a few seconds.</p>"
						"<button onclick=\"history.go(0)\">Reload</button>"
						"</center>"
				"</body>"
			"</html>";
	};

	try {
		STACK_UNWINDING_MARK;
		try {
			STACK_UNWINDING_MARK;

			url_args = RequestURIParser {request.target};
			StringView next_arg = url_args.extractNextArg();

			// Reset state
			page_template_began = false;
			notifications.clear();
			session_is_open = false;
			form_validation_error = false;

			// Check CSRF token
			if (request.method == server::HttpRequest::POST) {
				// If no session is open, load value from cookie to pass
				// verification
				if (not session_open())
					session_csrf_token = request.getCookie("csrf_token");

				if (request.form_data.get("csrf_token") != session_csrf_token) {
					error403();
					goto cleanup;
				}
			}

			if (next_arg == "kit")
				static_file();

			// else if (next_arg == "c")
				// contest_handle();

			else if (next_arg == "s")
				submission_handle();

			else if (next_arg == "u")
				users_handle();

			else if (next_arg == "")
				main_page();

			else if (next_arg == "api")
				api_handle();

			// else if (next_arg == "p")
				// problemset_handle();

			else if (next_arg == "login")
				login();

			else if (next_arg == "jobs")
				jobs_handle();

			// else if (next_arg == "file")
				// contest_file();

			else if (next_arg == "logout")
				logout();

			else if (next_arg == "signup")
				sign_up();

			else if (next_arg == "logs")
				view_logs();

			else
				error404();

		cleanup:
			page_template_end();

			// Make sure that the session is closed
			session_close();

		} catch (const std::exception& e) {
			ERRLOG_CATCH(e);
			error500();

		} catch (...) {
			ERRLOG_CATCH();
			error500();
		}

	} catch (const std::exception& e) {
		ERRLOG_CATCH(e);
		// We cannot use error500() because it will probably throw
		hard_error500();

	} catch (...) {
		ERRLOG_CATCH();
		// We cannot use error500() because it will probably throw
		hard_error500();
	}

	return std::move(resp);
}

void Sim::main_page() {
	STACK_UNWINDING_MARK;

	page_template("Main page");
	append("<div style=\"text-align: center\">"
			"<img src=\"/kit/img/SIM-logo.png\" width=\"260\" height=\"336\" "
				"alt=\"\">"
			"<p style=\"font-size: 30px\">Welcome to SIM</p>"
			"<hr>"
			"<p>SIM is an open source platform for carrying out algorithmic "
				"contests</p>"
		"</div>");
}

void Sim::static_file() {
	STACK_UNWINDING_MARK;

	string file_path = concat_tostr("static",
			abspath(decodeURI(StringView{request.target}
				.substring(1, request.target.find('?')))));
	// Extract path (ignore query)
	D(stdlog(file_path);)

	// Get file stat
	struct stat attr;
	if (stat(file_path.c_str(), &attr) != -1) {
		// Extract time of last modification
		auto it = request.headers.find("if-modified-since");
		resp.headers["last-modified"] = date("%a, %d %b %Y %H:%M:%S GMT",
			attr.st_mtime);
		resp.setCache(true, 100 * 24 * 60 * 60); // 100 days

		// If "If-Modified-Since" header is set and its value is not lower than
		// attr.st_mtime
		struct tm client_mtime;
		if (it && strptime(it->second.c_str(),
				"%a, %d %b %Y %H:%M:%S GMT", &client_mtime) != nullptr &&
			timegm(&client_mtime) >= attr.st_mtime)
		{
			resp.status_code = "304 Not Modified";
			return;
		}
	}

	resp.content_type = server::HttpResponse::FILE;
	resp.content = std::move(file_path);
}

void Sim::view_logs() {
	STACK_UNWINDING_MARK;

	if (!session_open() || session_user_type > UserType::ADMIN)
		return error403();

	page_template("Logs", "body{padding-left:20px}");

	append("<h2>Server's log:</h2>"
		"<pre id=\"web\" class=\"logs\"></pre>"

		"<h2>Server's error log:</h2>"
		"<pre id=\"web_err\" class=\"logs\"></pre>"

		"<h2>Job server's log:</h2>"
		"<pre id=\"jobs\" class=\"logs\"></pre>"

		"<h2>Job server's error log:</h2>"
		"<pre id=\"jobs_err\" class=\"logs\"></pre>"

		"<script>"
			"new Logs('web', $('#web')).monitor_scroll();"
			"new Logs('web_err', $('#web_err')).monitor_scroll();"
			"new Logs('jobs', $('#jobs')).monitor_scroll();"
			"new Logs('jobs_err', $('#jobs_err')).monitor_scroll();"
		"</script>");
}
