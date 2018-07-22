#include "config_file.h"

#include <fstream>

namespace config {
    void ConfigFile::save() {
        if (!m_changed) return;

        std::ofstream file(m_path, std::ios::out);
        if (!file.good()) {
            m_log->error("ConfigFile::save(): unable to open file " + m_path.string());
            return;
        }

        file << serialize();
    }
}  // namespace config
