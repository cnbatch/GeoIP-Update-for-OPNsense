#include "geoip_file.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <memory>
#include <vector>
#include <algorithm>
#include <curl/curl.h>
#include <archive.h>
#include <archive_entry.h>

using namespace std;
namespace fs = std::filesystem;

struct curl_global_resource
{
	curl_global_resource()
	{
		curl_global_init(CURL_GLOBAL_ALL);
	}
	~curl_global_resource()
	{
		curl_global_cleanup();
	}
};

typedef size_t (*call_back_function_ptr)(char*, size_t, size_t, void*);
CURLcode download_geoip_block(const std::string &url, void *write_to, call_back_function_ptr call_back);
map<string, string> unpack_archive(shared_ptr<archive> current_archive_ptr);

bool access_or_create_folder(const std::string &folder_name, bool slience_mode)
{
	fs::path target_folder = folder_name;
	if (!fs::exists(target_folder))
	{
		if (!slience_mode) cout << target_folder << " not exist\n";
		if (access_or_create_folder(target_folder.parent_path(), slience_mode))
		{
			if (access(target_folder.parent_path().c_str(), W_OK) != 0)
			{
				if (!slience_mode) cout << target_folder.parent_path() << " cannot write (no permission)\n";
				return false;
			}

			if (fs::create_directory(target_folder))
			{
				if (!slience_mode) cout << target_folder << " created\n";
			}
			else
			{
				if (!slience_mode) cout << target_folder << " cannot create\n";
				return false;
			}
		}
		else return false;
	}

	return true;
}

bool download_geoip_block_file(const std::string &url, shared_ptr<FILE> temp_file)
{
	auto res = download_geoip_block(url, temp_file.get(),
		[](char *ptr, size_t size, size_t nmemb, void *stream) -> size_t { return fwrite(ptr, size, nmemb, (FILE *)stream); });
	fseek(temp_file.get(), 0, SEEK_END);
	return CURLE_OK == res;
}

vector<char> download_geoip_block_memory(const std::string &url)
{
	vector<char> received_data(512 * 1024);
	auto call_back = [](char *ptr, size_t size, size_t nmemb, void *userdata) -> size_t
	{
		vector<char> &received_data = *reinterpret_cast<vector<char> *>(userdata);
		received_data.insert(received_data.end(), ptr, ptr + nmemb);
		return size * nmemb;
	};
	download_geoip_block(url, &received_data, call_back);

	return received_data;
}

CURLcode download_geoip_block(const std::string &url, void *write_to, call_back_function_ptr call_back)
{
	curl_global_resource cgr;
	auto deleter = [](CURL *handle) { if (handle) curl_easy_cleanup(handle); };
	unique_ptr<CURL, decltype(deleter)> curl_handle(curl_easy_init(), deleter);
	curl_easy_setopt(curl_handle.get(), CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl_handle.get(), CURLOPT_WRITEFUNCTION, call_back);
	curl_easy_setopt(curl_handle.get(), CURLOPT_WRITEDATA, write_to);
	curl_easy_setopt(curl_handle.get(), CURLOPT_URL, url.c_str());
	return curl_easy_perform(curl_handle.get());
}


map<string, string> unpack_archive(vector<char> &received_data)
{
	shared_ptr<archive> current_archive_sptr(archive_read_new(),
		[](archive *the_archive) { if (the_archive) archive_read_free(the_archive); });

	archive *current_archive = current_archive_sptr.get();
	archive_read_support_filter_all(current_archive);
	archive_read_support_format_all(current_archive);
	if (archive_read_open_memory(current_archive, received_data.data(), received_data.size()) != ARCHIVE_OK)
	{
		return map<string, string>();
	}

	return unpack_archive(current_archive_sptr);
}

map<string, string> unpack_archive(vector<char> &&received_data)
{
	return unpack_archive(received_data);
}

map<string, string> unpack_archive(shared_ptr<FILE> temp_file)
{
	shared_ptr<archive> current_archive_sptr(archive_read_new(),
		[](archive *the_archive) { if (the_archive) archive_read_free(the_archive); });

	archive *current_archive = current_archive_sptr.get();
	archive_read_support_filter_all(current_archive);
	archive_read_support_format_all(current_archive);
	if (archive_read_open_FILE(current_archive, temp_file.get()) != ARCHIVE_OK)
	{
		return map<string, string>();
	}

	return unpack_archive(current_archive_sptr);
}

map<string, string> unpack_archive(shared_ptr<archive> current_archive_ptr)
{
	map<string, string> filename_content_map;
	archive_entry *entry;
	archive *current_archive = current_archive_ptr.get();
	while (archive_read_next_header(current_archive, &entry) == ARCHIVE_OK)
	{
		auto file_size = archive_entry_size(entry);
		if (file_size <= 0)
			continue;

		string filepath_in_zip = archive_entry_pathname(entry);
		size_t country_pos = filepath_in_zip.find("/country/");
		if (country_pos == string::npos)
			continue;

		size_t iso_start = country_pos + 9; // length of "/country/"
		size_t iso_end = filepath_in_zip.find("/", iso_start);
		if (iso_end == string::npos)
			continue;

		string iso_code = filepath_in_zip.substr(iso_start, iso_end - iso_start);
		if (iso_code.length() != 2)
			continue;

		string filename = filepath_in_zip.substr(iso_end + 1);
		string file_type;

		if (filename.find("ipv4") != string::npos)
			file_type = "IPv4";
		else if (filename.find("ipv6") != string::npos)
			file_type = "IPv6";
		else
			continue;

		vector<char> buffer(file_size + 1, 0);
		auto data_size = archive_read_data(current_archive, &buffer[0], file_size);
		if (data_size <= 0)
			continue;

		string content(buffer.data());
		string processed_content;
		istringstream iss(content);
		string line;

		while (getline(iss, line))
		{
			if (line.empty() || line[0] == '#')
				continue;
			processed_content += line + "\n";
		}

		transform(iso_code.begin(), iso_code.end(), iso_code.begin(),
			[](unsigned char c) -> unsigned char { return toupper(c); });

		string output_filename = iso_code + "-" + file_type;
		filename_content_map[output_filename] = processed_content;
	}
	return filename_content_map;
}

