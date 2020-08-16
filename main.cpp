#include <iostream>
#include <fstream>
#include <filesystem>
#include <memory>
#include <chrono>
#include <unistd.h>
#include "geoip_file.hpp"

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    auto start = chrono::system_clock::now();
    bool parse_in_memory = true;
    bool slience_mode = false;

    if (argc > 1)
    {
        int ch;
        string application_name = "updategeoip";
        while ((ch = getopt(argc, argv, "fhms")) != -1)
        {
            switch (ch)
            {
            case 'f':
                parse_in_memory = false;
                break;
            case 'm':
                parse_in_memory = true;
                break;
            case 's':
                slience_mode = true;
                break;
            case 'h':
            case '?':
            default:
                cout << "Usage: \n";
                cout << application_name << " [-option]\n";
                cout << application_name << " <without parameter>: the same as \"" << application_name << " -m -s\"\n";
                cout << application_name << " -m\t" "Download and Parse GeoIP file in memory directly (faster)\n";
                cout << application_name << " -f\t" "Download and Parse GeoIP file in /tmp (save memory)\n";
                cout << application_name << " -s\t" "Slience mode (no stdout message)\n";
                return 0;
            }
        }
    }

    if (!fs::exists("/usr/local/share/GeoIP/alias"))
    {
        if (!fs::create_directory("/usr/local/share/GeoIP/alias"))
        {
            if(!slience_mode)
                cerr << "Cannot access '/usr/local/share/GeoIP/alias'.\n\n";
            return -1;
        }
    }

    string json_record = R"({"address_count":{0},"file_count":{1},"timestamp":"{2}","locations_filename":"GeoLite2-Country-Locations-en.csv","address_sources":{"IPv4":"GeoLite2-Country-Blocks-IPv4.csv","IPv6":"GeoLite2-Country-Blocks-IPv6.csv"}})";
    string url = "https://github.com/herrbischoff/country-ip-blocks/archive/master.zip";
    map<string, string> filename_content_map;
    if (parse_in_memory)
    {
        if (slience_mode)
        {
            filename_content_map = unpack_archive(download_geoip_block_memory(url));
        }
        else
        {
            vector<char> downloaded_zip_file = download_geoip_block_memory(url);
            if (downloaded_zip_file.size())
                filename_content_map = unpack_archive(downloaded_zip_file);
            else
            {
                cerr << "Cannot download file from Github.\n\n";
                return -2;
            }
        }
    }
    else
    {
        shared_ptr<FILE> temp_file(tmpfile(), [](FILE *this_file) { fclose(this_file); });
        if (download_geoip_block_file(url, temp_file))
        {
            rewind(temp_file.get());
            filename_content_map = unpack_archive(temp_file);
        }
        else
        {
            if (!slience_mode)
                cerr << "Cannot download and save file.\n\n";
            return -2;
        }
    }


    int address_count = 0;
    int file_count = 0;

    for (auto &filename_content : filename_content_map)
    {
        fs::path destination_file = "/usr/local/share/GeoIP/alias/" + filename_content.first;
        ofstream output_file(destination_file, ios::trunc);
        output_file << filename_content.second;
        address_count += count(begin(filename_content.second), end(filename_content.second), '/');
        file_count++;
    }

    time_t current_time = time(nullptr);
    char time_str[20]{};
    if (strftime(time_str, sizeof(time_str), "%FT%T", localtime(&current_time)))
    {
        json_record.replace(json_record.find("{0}"), 3, to_string(address_count));
        json_record.replace(json_record.find("{1}"), 3, to_string(file_count));
        json_record.replace(json_record.find("{2}"), 3, time_str);

        ofstream output_file("/usr/local/share/GeoIP/alias.stats", ios::trunc);
        output_file << json_record;
    }

    auto end = chrono::system_clock::now();
    chrono::duration<double> diff = end - start;
    if (!slience_mode)
        cout << "Completed in " << diff.count() << " s\n\n";

    return 0;
}
