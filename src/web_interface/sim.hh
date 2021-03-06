#pragma once

#include "http_request.hh"
#include "http_response.hh"

#include <sim/constants.hh>
#include <sim/contest_file_permissions.hh>
#include <sim/contest_permissions.hh>
#include <sim/cpp_syntax_highlighter.hh>
#include <sim/mysql.hh>
#include <sim/problem_permissions.hh>
#include <sim/user.hh>
#include <simlib/http/response.hh>
#include <simlib/request_uri_parser.hh>
#include <utime.h>

// Every object is independent, objects can be used in multi-thread program
// as long as one is not used by two threads simultaneously
class Sim final {
	/* ============================== General ============================== */

	MySQL::Connection mysql =
	   MySQL::make_conn_with_credential_file(".db.config");
	CStringView client_ip; // TODO: put in request?
	server::HttpRequest request;
	server::HttpResponse resp;
	RequestUriParser url_args{""};
	CppSyntaxHighlighter cpp_syntax_highlighter;

	/**
	 * @brief Sets headers to make a redirection
	 * @details Does not clear response headers and contents
	 *
	 * @param location URL address where to redirect
	 */
	void redirect(std::string location) {
		STACK_UNWINDING_MARK;

		resp.status_code = "302 Moved Temporarily";
		resp.headers["Location"] = std::move(location);
	}

	/// Sets resp.status_code to @p status_code and resp.content to
	///  @p response_body
	void set_response(StringView status_code, StringView response_body = {}) {
		STACK_UNWINDING_MARK;

		resp.status_code = status_code;
		resp.content = response_body;

#ifdef DEBUG
		if (response_body.find(notifications) == StringView::npos) {
			THROW("Omitted notifications in the error message: ",
			      response_body);
		}

		notifications.clear();
#endif
	}

	/* ================================ API ================================ */

	void api_error400(StringView response_body = {}) {
		set_response("400 Bad Request", response_body);
	}

	void api_error403(StringView response_body = {}) {
		set_response("403 Forbidden", response_body);
	}

	void api_error404(StringView response_body = {}) {
		set_response("404 Not Found", response_body);
	}

	// api.cc
	void api_handle();

	void api_logs();

	// jobs_api.cc
	void api_jobs();

	void api_job();

	void api_job_restart(JobType job_type, StringView job_info);

	void api_job_cancel();

	void api_job_download_log();

	void api_job_download_uploaded_package(std::optional<uint64_t> file_id,
	                                       JobType job_type);

	void api_job_download_uploaded_statement(std::optional<uint64_t> file_id,
	                                         JobType job_type, StringView info);

	// jobs_api.cc
	void api_problems();

	void api_problem();

	void api_problem_add_or_reupload_impl(bool reuploading);

	void api_problem_add(sim::problem::OverallPermissions overall_perms);

	void api_statement_impl(uint64_t problem_file_id, StringView problem_label,
	                        StringView simfile);

	void api_problem_statement(StringView problem_label, StringView simfile,
	                           sim::problem::Permissions perms);

	void api_problem_download(StringView problem_label,
	                          sim::problem::Permissions perms);

	void api_problem_rejudge_all_submissions(sim::problem::Permissions perms);

	void api_problem_reset_time_limits(sim::problem::Permissions perms);

	void api_problem_reupload(sim::problem::Permissions perms);

	void api_problem_edit(sim::problem::Permissions perms);

	void api_problem_edit_tags(sim::problem::Permissions perms);

	void api_problem_delete(sim::problem::Permissions perms);

	void api_problem_merge_into_another(sim::problem::Permissions perms);

	void api_problem_change_statement(sim::problem::Permissions perms);

	void
	api_problem_attaching_contest_problems(sim::problem::Permissions perms);

	// users_api.cc
	void api_users();

	void api_user();

	void api_user_add();

	void api_user_edit();

	void api_user_delete();

	void api_user_merge_into_another();

	void api_user_change_password();

	// submissions_api.cc
	void append_submission_status(SubmissionStatus initial_status,
	                              SubmissionStatus full_status,
	                              bool show_full_status);

	void api_submissions();

	void api_submission();

	void api_submission_add();

	void api_submission_rejudge();

	void api_submission_change_type();

	void api_submission_delete();

	void api_submission_source();

	void api_submission_download();

	// contests_api.cc
	void api_contests();

	void api_contest();

	void api_contest_round(StringView contest_round_id);

	void api_contest_problem(StringView contest_problem_id);

	void api_contest_create(sim::contest::OverallPermissions overall_perms);

	void api_contest_clone(sim::contest::OverallPermissions overall_perms);

	void api_contest_edit(StringView contest_id,
	                      sim::contest::Permissions perms, bool is_public);

	void api_contest_delete(StringView contest_id,
	                        sim::contest::Permissions perms);

	void api_contest_round_create(StringView contest_id,
	                              sim::contest::Permissions perms);

	void api_contest_round_clone(StringView contest_id,
	                             sim::contest::Permissions perms);

	void api_contest_round_edit(uintmax_t contest_round_id,
	                            sim::contest::Permissions perms);

	void api_contest_round_delete(uintmax_t contest_round_id,
	                              sim::contest::Permissions perms);

	void api_contest_problem_statement(StringView problem_id);

	void api_contest_problem_add(uintmax_t contest_id,
	                             uintmax_t contest_round_id,
	                             sim::contest::Permissions perms);

	void
	api_contest_problem_rejudge_all_submissions(StringView contest_problem_id,
	                                            sim::contest::Permissions perms,
	                                            StringView problem_id);

	void api_contest_problem_edit(StringView contest_problem_id,
	                              sim::contest::Permissions perms);

	void api_contest_problem_delete(StringView contest_problem_id,
	                                sim::contest::Permissions perms);

	void api_contest_ranking(
	   sim::contest::Permissions perms,
	   StringView submissions_query_id_name, // TODO: change to id_kind
	   StringView query_id);

	// contest_users_api.cc

	void api_contest_users();

	void api_contest_user();

	void api_contest_user_add(StringView contest_id);

	void api_contest_user_change_mode(StringView contest_id,
	                                  StringView user_id);

	void api_contest_user_expel(StringView contest_id, StringView user_id);

	// contest_entry_token_api.cc

	void api_contest_entry_token();

	void api_contest_entry_token_add(StringView contest_id);

	void api_contest_entry_token_regen(StringView contest_id);

	void api_contest_entry_token_delete(StringView contest_id);

	void api_contest_entry_token_short_add(StringView contest_id);

	void api_contest_entry_token_short_regen(StringView contest_id);

	void api_contest_entry_token_short_delete(StringView contest_id);

	void api_contest_entry_token_use_to_enter_contest(uintmax_t contest_id);

	// contest_files_api.cc

	void api_contest_files();

	void api_contest_file();

	void api_contest_file_add();

	void api_contest_file_download(StringView contest_file_id,
	                               sim::contest_file::Permissions perms);

	void api_contest_file_edit(StringView contest_file_id,
	                           sim::contest_file::Permissions perms);

	void api_contest_file_delete(StringView contest_file_id,
	                             sim::contest_file::Permissions perms);

	/* ============================== Session ============================== */

	bool session_is_open = false;
	sim::User::Type session_user_type = sim::User::Type::NORMAL;
	InplaceBuff<SESSION_ID_LEN + 1> session_id;
	InplaceBuff<SESSION_CSRF_TOKEN_LEN + 1> session_csrf_token;
	InplaceBuff<32> session_user_id;
	decltype(sim::User::username) session_username;
	InplaceBuff<4096> session_data;

	/**
	 * @brief Creates session and opens it
	 * @details First closes old session and then creates and opens new one
	 *
	 * @param user_id Id of user to which session will belong
	 * @param temporary_session Whether create temporary or persistent session
	 *
	 * @errors Throws an exception in any case of error
	 */
	void session_create_and_open(StringView user_id, bool temporary_session);

	/// Destroys session (removes from database, etc.)
	void session_destroy();

	/// Opens session, returns true if opened successfully, false otherwise
	bool session_open();

	/// Closes session (even if it throws, the session is closed)
	void session_close();

	/* =========================== Page template =========================== */

	bool page_template_began = false;
	InplaceBuff<1024> notifications;

	// Appends
	void page_template(StringView title, StringView styles = {},
	                   StringView scripts = {});

	void page_template_end();

	template <class... Args>
	void add_notification(StringView css_classes, Args&&... message) {
		notifications.append("<pre class=\"", css_classes, "\">",
		                     std::forward<Args>(message)..., "</pre>\n");
	}

	template <class... Args>
	Sim& append(Args&&... args) {
		resp.content.append(std::forward<Args>(args)...);
		return *this;
	}

	void error_page_template(StringView status, StringView code,
	                         StringView message);

	// 403
	void error403() {
		error_page_template(
		   "403 Forbidden", "403",
		   "Sorry, but you're not allowed to see anything here.");
	}

	// 404
	void error404() {
		error_page_template("404 Not Found", "404", "Page not found");
	}

	// 500
	void error500() {
		error_page_template("500 Internal Server Error", "500",
		                    "Internal Server Error");
	}

	// 501
	void error501() {
		error_page_template("501 Not Implemented", "501",
		                    "This feature has not been implemented yet...");
	}

	/* ============================== Forms ============================== */

	bool form_validation_error = false;

	/// Returns true if and only if the field @p name exists and its value is no
	/// longer than max_size
	template <class T>
	bool form_validate(T& var, const std::string& name,
	                   StringView name_to_print, size_t max_size = -1) {
		STACK_UNWINDING_MARK;

		auto val_opt = request.form_data.get_opt(name);
		if (not val_opt) {
			form_validation_error = true;
			add_notification("error", "Invalid ", html_escape(name_to_print));
			return false;
		}
		if (val_opt->size() > max_size) {
			form_validation_error = true;
			add_notification("error", html_escape(name_to_print),
			                 " cannot be longer than ", max_size, " bytes");
			return false;
		}

		var = *val_opt;
		return true;
	}

	/// Validates field and (if not blank) checks it by comp
	template <class T, class Checker,
	          typename =
	             std::enable_if_t<!std::is_convertible<Checker, size_t>::value>>
	bool form_validate(T& var, const std::string& name,
	                   StringView name_to_print, Checker&& check,
	                   StringView error_msg = {}, size_t max_size = -1) {
		STACK_UNWINDING_MARK;

		if (form_validate(var, name, name_to_print, max_size)) {
			if (string_length(var) == 0 or check(var)) {
				return true;
			}

			form_validation_error = true;
			if (error_msg.empty()) {
				add_notification(
				   "error",
				   html_escape(concat(name_to_print, " validation error")));
			} else {
				add_notification("error", html_escape(error_msg));
			}
		}

		return false;
	}

	/// Like validate() but also validate not blank
	template <class T>
	bool form_validate_not_blank(T& var, const std::string& name,
	                             StringView name_to_print,
	                             size_t max_size = -1) {
		STACK_UNWINDING_MARK;

		auto val_opt = request.form_data.get_opt(name);
		if (not val_opt) {
			form_validation_error = true;
			add_notification("error", "Invalid ", html_escape(name_to_print));
			return false;
		}
		if (val_opt->empty()) {
			form_validation_error = true;
			add_notification("error", html_escape(name_to_print),
			                 " cannot be blank");
			return false;
		}
		if (val_opt->size() > max_size) {
			form_validation_error = true;
			add_notification("error", html_escape(name_to_print),
			                 " cannot be longer than ", max_size, " bytes");
			return false;
		}

		var = *val_opt;
		return true;
	}

	/// Validates field and checks it by comp
	template <class T, class Checker,
	          typename =
	             std::enable_if_t<!std::is_convertible<Checker, size_t>::value>>
	bool form_validate_not_blank(T& var, const std::string& name,
	                             StringView name_to_print, Checker&& check,
	                             StringView error_msg = {},
	                             size_t max_size = -1) {
		STACK_UNWINDING_MARK;

		if (form_validate_not_blank(var, name, name_to_print, max_size)) {
			if (check(var)) {
				return true;
			}

			form_validation_error = true;
			if (error_msg.empty()) {
				add_notification(
				   "error",
				   html_escape(concat(name_to_print, ": invalid value")));
			} else {
				add_notification("error", html_escape(error_msg));
			}
		}

		return false;
	}

	/// @brief Sets @p var to path of the uploaded file (its temporary location)
	///  or sets an error if no file as @p name was submitted. To obtain the
	// users's filename of the uploaded file use: request.form_data.get(name)
	template <class T>
	bool form_validate_file_path_not_blank(T& var, const std::string& name,
	                                       StringView name_to_print) {
		STACK_UNWINDING_MARK;

		auto const& files = request.form_data.files;
		auto it = files.find(name);
		if (it == files.end()) {
			form_validation_error = true;
			add_notification("error", html_escape(name_to_print),
			                 " has to be submitted as a file");
			return false;
		}

		var = it->second;
		return true;
	}

	/* ============================== Users ============================== */
public:
	enum class UserPermissions : uint {
		NONE = 0,
		VIEW = 1 << 0,
		EDIT = 1 << 1,
		CHANGE_PASS = 1 << 2,
		ADMIN_CHANGE_PASS = 1 << 3,
		MAKE_ADMIN = 1 << 4,
		MAKE_TEACHER = 1 << 5,
		MAKE_NORMAL = 1 << 6,
		DELETE = 1 << 7,
		MERGE = 1 << 8,
		// Overall
		VIEW_ALL = 1 << 9,
		ADD_USER = 1 << 10,
		ADD_ADMIN = 1 << 11,
		ADD_TEACHER = 1 << 12,
		ADD_NORMAL = 1 << 13
	};

private:
	friend DECLARE_ENUM_UNARY_OPERATOR(UserPermissions, ~);
	friend DECLARE_ENUM_OPERATOR(UserPermissions, |);
	friend DECLARE_ENUM_OPERATOR(UserPermissions, &);

	UserPermissions users_perms = UserPermissions::NONE;
	InplaceBuff<32> users_uid;

	UserPermissions users_get_overall_permissions() noexcept;

	/**
	 * @brief Returns a set of operation the viewer is allowed to do over the
	 *   user
	 *
	 * @param user_id uid of (viewed) user
	 * @param utype type of (viewed) user
	 *
	 * @return ORed permissions flags
	 */
	Sim::UserPermissions users_get_permissions(StringView user_id,
	                                           sim::User::Type utype) noexcept;

	// Queries MySQL
	UserPermissions users_get_permissions(StringView user_id);

	// Return true if the submitted password is correct, false otherwise
	bool check_submitted_password(StringView password_field_name = "password");

	/* Pages */

	void login();

	void logout();

	void sign_up();

	// @brief Main User handler
	void users_handle();

	void users_user();

	/* ============================= Job queue ============================= */
public:
	enum class JobPermissions : uint {
		NONE = 0,
		// Overall
		VIEW_ALL = 1, // allowed to view all the jobs
		// Job specific
		VIEW = 2, // allowed to view the job
		DOWNLOAD_LOG = 4,
		DOWNLOAD_UPLOADED_PACKAGE = 8,
		CANCEL = 16,
		RESTART = 32,
		DOWNLOAD_UPLOADED_STATEMENT = 64,
	};

private:
	friend DECLARE_ENUM_UNARY_OPERATOR(JobPermissions, ~);
	friend DECLARE_ENUM_OPERATOR(JobPermissions, |);
	friend DECLARE_ENUM_OPERATOR(JobPermissions, &);
	friend DECLARE_ENUM_ASSIGN_OPERATOR(JobPermissions, |=);

	InplaceBuff<32> jobs_jid;
	JobPermissions jobs_perms = JobPermissions::NONE;

	JobPermissions jobs_get_overall_permissions() noexcept;

	// Session must be open to access the jobs
	JobPermissions jobs_get_permissions(std::optional<StringView> creator_id,
	                                    JobType job_type,
	                                    JobStatus job_status) noexcept;

	// Used to get granted permissions to the problem jobs
	JobPermissions jobs_granted_permissions_problem(StringView problem_id);

	// Used to get granted permissions to the submission jobs
	JobPermissions
	jobs_granted_permissions_submission(StringView submission_id);

	/// Main Jobs handler
	void jobs_handle();

	/* ============================== Problems ============================== */
	InplaceBuff<32> problems_pid;
	uint64_t problems_file_id{};

	/// Main Problems handler
	void problems_handle();

	void problems_problem();

	/* ============================== Contest ============================== */

	/// Main Contests handler
	void contests_handle();

	void contests_contest(StringView contest_id);

	void contests_contest_round(StringView contest_round_id);

	void contests_contest_problem(StringView contest_problem_id);

	void enter_contest();

	/* =========================== Contest users =========================== */
public:
	enum class ContestUserPermissions : uint {
		NONE = 0,
		// Overall
		ACCESS_API = 1,
		ADD_CONTESTANT = 2,
		ADD_MODERATOR = 4,
		ADD_OWNER = 8,
		// Contest user specific
		MAKE_CONTESTANT = 1 << 4,
		MAKE_MODERATOR = 1 << 5,
		MAKE_OWNER = 1 << 6,
		EXPEL = 1 << 7
	};

private:
	friend DECLARE_ENUM_UNARY_OPERATOR(ContestUserPermissions, ~);
	friend DECLARE_ENUM_OPERATOR(ContestUserPermissions, |);
	friend DECLARE_ENUM_OPERATOR(ContestUserPermissions, &);

	// Returns only the overall permissions
	ContestUserPermissions contest_user_get_overall_permissions(
	   std::optional<sim::ContestUser::Mode> viewer_mode) noexcept;

	ContestUserPermissions contest_user_get_permissions(
	   std::optional<sim::ContestUser::Mode> viewer_mode,
	   std::optional<sim::ContestUser::Mode> user_mode) noexcept;

	// Returns (viewer mode, perms), queries MySQL
	std::pair<std::optional<sim::ContestUser::Mode>,
	          Sim::ContestUserPermissions>
	contest_user_get_overall_permissions(StringView contest_id);

	// Returns (viewer mode, perms, user's mode), queries MySQL
	std::tuple<std::optional<sim::ContestUser::Mode>,
	           Sim::ContestUserPermissions,
	           std::optional<sim::ContestUser::Mode>>
	contest_user_get_permissions(StringView contest_id, StringView user_id);

	/* ============================= Submissions =============================
	 */
public:
	enum class SubmissionPermissions : uint {
		NONE = 0,
		// Overall
		VIEW_ALL = 1,
		// Submission specific
		VIEW = 2,
		VIEW_SOURCE = 4,
		VIEW_FINAL_REPORT = 8,
		VIEW_RELATED_JOBS = 1 << 4,
		CHANGE_TYPE = 1 << 5,
		REJUDGE = 1 << 6,
		DELETE = 1 << 7
	};

private:
	friend DECLARE_ENUM_UNARY_OPERATOR(SubmissionPermissions, ~);
	friend DECLARE_ENUM_OPERATOR(SubmissionPermissions, |);
	friend DECLARE_ENUM_OPERATOR(SubmissionPermissions, &);

	SubmissionPermissions submissions_get_overall_permissions() noexcept;

	SubmissionPermissions submissions_get_permissions(
	   std::optional<uintmax_t> submission_owner, SubmissionType stype,
	   std::optional<sim::ContestUser::Mode> cu_mode,
	   decltype(sim::Problem::owner) problem_owner) noexcept;

	StringView submissions_sid;
	uint64_t submissions_file_id{};
	SubmissionLanguage submissions_slang{};
	SubmissionPermissions submissions_perms = SubmissionPermissions::NONE;

	// Pages
	void submissions_handle();

	/* =========================== Contest files =========================== */

	// Pages
	void contest_file_handle();

	/* =============================== Other =============================== */

	// Pages
	void main_page();

	void static_file();

	void view_logs();

public:
	Sim() = default;
	Sim(const Sim&) = delete;
	Sim(Sim&&) = delete;
	Sim& operator=(const Sim&) = delete;
	Sim& operator=(Sim&&) = delete;
	~Sim() = default;

	/**
	 * @brief Handles request
	 * @details Takes requests, handle it and returns response.
	 *   This function is not thread-safe
	 *
	 * @param client_ip_addr IP address of the client
	 * @param req request
	 *
	 * @return response
	 */
	server::HttpResponse handle(CStringView client_ip_addr,
	                            server::HttpRequest req); // TODO: close session
};
