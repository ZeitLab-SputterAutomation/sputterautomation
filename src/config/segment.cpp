#include "segment.h"

#include "logging/logging.h"

namespace config {
    std::shared_ptr<Segment> Segment::get_segment(const std::string &name, bool create_missing) {
        std::scoped_lock<std::mutex> lock{m_mutex};

        if (name.empty()) return shared_from_this();

        auto tokens = util::split(name, '.', false);
        if (!tokens) {
            logging::get_log("main")->error("Segment::get_segment(): encountered empty segment name in '{0}'", name);
            return nullptr;
        }

        std::shared_ptr<Segment> current_segment = shared_from_this();
        for (const auto &seg : *tokens) {
            if (current_segment->m_children.count(seg) < 1) {
                // Create new segment if seg is not found and we are allowed to
                if (!create_missing) return nullptr;

                current_segment->m_children.insert(std::make_pair(seg, std::make_shared<Segment>(seg)));
            }

            // Advance down the tree
            current_segment = current_segment->m_children[seg];
        }

        return current_segment;
    }

    std::string Segment::do_serialize(const std::string &chain, int tablevel) const {
        std::string s;

        // tab handles indentation of settings and tab_root the indentation of "our" comment and name
        const std::string tab(tablevel * CONFIG_SPACES_PER_INDENT_LEVEL, ' ');
        const std::string tab_root((tablevel > 0) ? (tablevel - 1) * CONFIG_SPACES_PER_INDENT_LEVEL : 0, ' ');

        // Is there a comment for our segment name?
        if (!m_comment.empty()) s += tab_root + "# " + m_comment + "\n";

        // Skip the root segment name (ie. empty segment name)
        if (!m_name.empty()) {
            s += tab_root + "[";

            if (!chain.empty()) s += chain + ".";

            s += m_name + "]\n";
        }

        // Write all settings
        int empty_line = 0;
        for (const auto &setting : m_settings) {
            if (!setting.second.comment.empty()) {
                s += tab + "# " + setting.second.comment + "\n";

                // For all settings with comments we write an empty line after the setting
                empty_line++;
            }

            s += tab + setting.first + " = ";

            // TODO: handle dates correctly
            switch (setting.second.value_type) {
            case ValueType::number:
                s += setting.second.value + "\n";
                break;
            case ValueType::string:
                s += "\"" + setting.second.value + "\"\n";
                break;
            case ValueType::date:
                break;

            // We handle additional cases like strings
            default:
                logging::get_log("main")->warn(
                    "Segment::serialize(): encountered setting with unknown serialization, defaulting to string");
                s += "\"" + setting.second.value + "\"\n";
                break;
            }

            if (empty_line > 0) s += "\n";

            empty_line--;
        }

        // If empty_line is 0 here it means that we already wrote an empty line last, so we don't need a new one before
        // the segment name
        if (empty_line < 0) s += "\n";

        // Now serialize all children too
        for (const auto &child : m_children) {
            // No seperator for root segment
            std::string chain_temp = chain;
            if (!chain_temp.empty()) chain_temp += ".";

            s += child.second->do_serialize(chain_temp + m_name, tablevel + 1) + "\n";
        }

        // Remove the last newline
        if(!s.empty()) s.pop_back();

        return s;
    }
}  // namespace config
