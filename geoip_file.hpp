#pragma once
#include <string>
#include <filesystem>
#include <map>
#include <memory>
#include <vector>

bool access_or_create_folder(const std::string &folder_name, bool slience_mode);
bool download_geoip_block_file(const std::string &url, std::shared_ptr<FILE> temp_file);
std::vector<char> download_geoip_block_memory(const std::string &url);
std::map<std::string, std::string> unpack_archive(std::vector<char> &received_data);
std::map<std::string, std::string> unpack_archive(std::vector<char> &&received_data);
std::map<std::string, std::string> unpack_archive(std::shared_ptr<FILE> temp_file);