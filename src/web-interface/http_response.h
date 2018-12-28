#pragma once

#include "http_headers.h"

#include <simlib/time.h>

namespace server {

class HttpResponse {
public:
	enum ContentType : uint8_t { TEXT, FILE, FILE_TO_REMOVE } content_type;
	InplaceBuff<100> status_code;
	HttpHeaders headers, cookies;
	InplaceBuff<4096> content;

	explicit HttpResponse(ContentType con_type = TEXT,
			const std::string& stat_code = "200 OK")
			: content_type(con_type), status_code(stat_code), headers(),
			cookies(), content() {}

	HttpResponse(const HttpResponse&) = default;
	HttpResponse(HttpResponse&&) noexcept = default;
	HttpResponse& operator=(const HttpResponse&) = default;
	HttpResponse& operator=(HttpResponse&&) = default;

	~HttpResponse() {}

	void setCookie(const std::string& name, const std::string& val,
			time_t expire = -1, const std::string& path = "",
			const std::string& domain ="", bool http_only = false,
			bool secure = false);

	void setCache(bool to_public, uint max_age, bool must_revalidate) {
		headers["expires"] = date("%a, %d %b %Y %H:%M:%S GMT",
			time(nullptr) + max_age);
		headers["cache-control"] =
			concat_tostr((to_public ? "public" : "private"),
				(must_revalidate ? "; must-revalidate" : ""),
				"; max-age=", max_age);
	}

	StringView getCookie(StringView name) const noexcept {
		StringView cookie = cookies.get(name);
		return cookie.substr(0, cookie.find(';'));
	}
};

} // namespace server