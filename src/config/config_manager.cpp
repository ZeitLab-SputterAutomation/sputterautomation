#include "config_manager.h"

#include <fstream>
#include <cctype>

#include "util/util.h"
#include "util/to_string.h"

namespace config {
    std::shared_ptr<ConfigFile> ConfigManager::get_config(const std::string &name) {
        std::scoped_lock<std::mutex> lock{m_mutex};

        auto it = m_configs.find(name);
        if (it == m_configs.end()) return nullptr;

        return it->second;
    }

    void ConfigManager::close_all() {
        std::scoped_lock<std::mutex> lock{m_mutex};

        for (auto &config : m_configs) {
            if (config.second) config.second->save();
        }

        m_configs.clear();
    }

    bool ConfigManager::load(const std::string &file, const std::string &identifier) {
        std::scoped_lock<std::mutex> lock{m_mutex};

        std::ifstream input(file);
        if (!input.is_open()) {
            m_log->error("ConfigManager::load(): unable to open config file {0}", file);
            return false;
        }

        auto conf = std::make_shared<ConfigFile>(file);
        if (!parse(input, conf)) {
            return false;
        }

        if (m_configs.count(identifier) > 0) {
            m_log->warn("ConfigManager::load(): config file with id {0} already loaded", identifier);
        }
        m_configs[identifier] = conf;

        return true;
    }

    // TODO: support "a.b = 5" in file, so that segment a setting b is 5
    // FIXME: multiline comments should not be trimmed
    // FIXME: ignore trailing comments after key-value-pairs
    bool ConfigManager::parse(std::istream &stream, std::shared_ptr<ConfigFile> conf) {
        int line_number = 0;
        std::string line;
        std::string comment;

        auto current_segment = conf->m_root = std::make_shared<Segment>();

        while (util::safe_getline(stream, line)) {
            line_number++;

            // Trim whitespace charaters
            util::trim_both(line);

            auto it = line.begin();
            auto end = line.end();

            // Line is empty
            if (it == end) continue;

            // Line is a comment
            if (*it == '#') {
                ++it;

                util::trim_left(it, end);

                // Handle multiline comments properly
                if (!comment.empty()) comment += "\n #";

                comment += std::string{it, end};
                continue;
            }

            // Segment
            if (*it == '[') {
                ++it;

                util::trim_left(it, end);

                // Error: Empty segment name?
                if (*it == ']' || it == end) {
                    m_log->error("Error in file {0}({1}): empty segment name, got '{2}'", conf->m_path.string(),
                                 line_number, line);

                    return false;
                }

                // Closing ']' should be the last character now
                std::string segment_name{it, std::prev(end)};
                util::trim_all(segment_name, [](unsigned char c) { return isspace(c); });

                if (segment_name.empty()) {
                    m_log->error("Error in file {0}({1}): missing closing ']' in segment name, got '{2}'",
                                 conf->m_path.string(), line_number, line);

                    return false;
                }

                // Only alphanumeric characters and '.' are allowed for the segment names
                if (std::any_of(segment_name.begin(), segment_name.end(),
                                [](unsigned char c) { return !(std::isalnum(c) || c == '.'); })) {
                    m_log->error(
                        "Error in file {0}({1}): only alphanumeric characters and '.' allowed in segment name, got '{2}'",
                        conf->m_path.string(), line_number, line);

                    return false;
                }

                // Load the segment (chain)
                current_segment = conf->m_root->get_segment(segment_name);

                if (!current_segment) {
                    m_log->error("Error in file {0}({1}): an error occured in get_segment()", conf->m_path.string(),
                                 line_number);

                    return false;
                }

                if (!current_segment->get_comment().empty()) {
                    m_log->warn("Warning in file {0}({1}): overwriting an already set comment; old comment was '{2}'",
                                conf->m_path.string(), line_number, current_segment->get_comment());
                }
                current_segment->set_comment(comment);
                comment.clear();

                continue;
            }  // Segment

            // Key-value pair
            std::string value;

            auto equal_pos = line.find('=');

            if (equal_pos == std::string::npos) {
                m_log->error("Error in file {0}({1}): missing equal sign, got '{2}'", conf->m_path.string(),
                             line_number, line);

                return false;
            } else if (equal_pos == line.length()) {
                m_log->error("Error in file {0}({1}): empty value, got '{2}'", conf->m_path.string(), line_number,
                             line);

                return false;
            }

            std::string key = line.substr(0, equal_pos);
            if (key.empty()) {
                m_log->error("Error in file {0}({1}): empty key, got '{2}'", conf->m_path.string(), line_number, line);

                return false;
            }
            util::trim_both(key);

            bool in_string = false;
            ValueType type = ValueType::number;

            // TODO: parse datetime

            size_t i = equal_pos + 1;
            for (; i < line.length(); i++) {
                unsigned char c = line[i];

                if (c == '"') {
                    if (in_string) {
                        ++i;
                        in_string = false;
                        break;
                    } else {
                        in_string = true;
                        type = ValueType::string;
                        continue;
                    }
                }

                if (isspace(c) && !in_string) continue;

                if (!(isdigit(c) || c == '.') && !in_string) {
                    m_log->error("Error in file {0}({1}): non-numeric character outside string, got '{2}'",
                                 conf->m_path.string(), line_number, line);

                    return false;
                }

                value += c;
            }

            if (in_string) {
                m_log->error("Error in file {0}({1}): missing closing \", got '{2}'", conf->m_path.string(),
                             line_number, line);

                return false;
            }

            if (i != line.length()) {
                m_log->error("Error in file {0}({1}): non-space character after string ended, got '{2}'",
                             conf->m_path.string(), line_number, line);

                return false;
            }

            current_segment->m_settings[key] = Segment::Setting{value, comment, type};
            comment.clear();
        }

        return true;
    }

}  // namespace config
