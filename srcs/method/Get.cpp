#include "Get.hpp"

bool	Get::make_body(Response &response, std::string &path)
{
	std::ifstream		file;
	std::stringstream	buf;

	file.open (path.c_str(), std::ifstream::in);
	if (file.is_open() == false)
		return false;
	buf << file.rdbuf();
	response.set_body (buf.str());
	buf.str("");
	file.close();
	return true;
}

void	 Get::create_response(Response &response)
{
	response.set_header_field("Content-Length", to_string(response.get_body().size()));
	if (response.get_header_field("Content-Length") != "0")
		response.set_header_field("Content-Type", "text/html; charset=UTF-8");

}

void	Get::set_autoindex(Request &request, Response &response)
{
	DIR					*dir = opendir (request.get_autoindex_path().c_str());
	struct dirent		*diread;
	std::vector<char *>	files;
	std::string			file_name;

	if (dir == NULL)
	{
		set_error_page (request.get_cycle_instance().getMainRoot() + "/" + request.get_cycle_instance().getDefaultErrorRoot(), response.get_body());
		return;
	}

	while ((diread = readdir (dir)) != NULL)
		files.push_back (diread->d_name);
	closedir (dir);
	std::string	index = "<html>\n<head>\n<title>Index of " + request.get_path() + "</title>\n</head>\n<body>\n";
	index += "<h1>Index of " + request.get_path() + "</h1>\n";
	index += "<ul>\n";

	for (std::vector<char *>::iterator it = files.begin(); it != files.end(); it++)
	{
		file_name = *it;
		index += "<li><a href=\"" + request.get_autoindex_path().substr(request.get_cycle_instance().getMainRoot().size()) + file_name + "\">" + file_name + "</a></li>\n";;
	}

	index += "</ul>\n</body>\n</html>\n";
	response.set_body (index);
}
