#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <iterator>

namespace fs = std::filesystem;

static uint32_t crc32_table[256];
static bool crc32_table_ready = false;

static void crc32_init() {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (int j = 0; j < 8; j++)
            c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        crc32_table[i] = c;
    }
    crc32_table_ready = true;
}

static uint32_t crc32(const uint8_t* data, size_t len) {
    if (!crc32_table_ready) crc32_init();
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < len; i++)
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFu;
}

struct ZipEntry {
    std::string arcname;
    std::string fullpath;
    bool is_directory;
    uint32_t crc;
    uint32_t comp_size;
    uint32_t uncomp_size;
    uint32_t local_header_offset;
};

static std::vector<uint8_t> read_file(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    return std::vector<uint8_t>((std::istreambuf_iterator<char>(f)),
                                 std::istreambuf_iterator<char>());
}

static bool is_excluded_dir(const std::string& name) {
    static const char* excluded[] = {"build", "build-core", ".git", ".vscode", ".gemini", "scratch", "__pycache__"};
    for (auto e : excluded)
        if (name == e) return true;
    return false;
}

static bool is_excluded_ext(const fs::path& p) {
    std::string ext = p.extension().string();
    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
    static const char* excluded[] = {"exe", "obj", "o", "a", "pdb", "ilk", "log", "tmp"};
    for (auto e : excluded)
        if (ext == e) return true;
    return false;
}

static bool is_excluded_file(const fs::path& p) {
    if (is_excluded_ext(p)) return true;
    std::string s = p.string();
    if (s.size() >= 7 && s.substr(s.size() - 7) == ".gosave") return true;
    return false;
}

static std::string to_posix(const fs::path& p) {
    std::string s = p.string();
    for (auto& c : s)
        if (c == '\\') c = '/';
    return s;
}

static void write_u16(std::vector<uint8_t>& buf, uint16_t v) {
    buf.push_back(v & 0xFF);
    buf.push_back((v >> 8) & 0xFF);
}

static void write_u32(std::vector<uint8_t>& buf, uint32_t v) {
    buf.push_back(v & 0xFF);
    buf.push_back((v >> 8) & 0xFF);
    buf.push_back((v >> 16) & 0xFF);
    buf.push_back((v >> 24) & 0xFF);
}

static void write_local_header(std::vector<uint8_t>& out, const std::string& name, uint32_t crc,
                                uint32_t comp_size, uint32_t uncomp_size) {
    write_u32(out, 0x04034b50);
    write_u16(out, 20);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0x21);
    write_u32(out, crc);
    write_u32(out, comp_size);
    write_u32(out, uncomp_size);
    write_u16(out, static_cast<uint16_t>(name.size()));
    write_u16(out, 0);
    for (char c : name) out.push_back(static_cast<uint8_t>(c));
}

static void write_central_dir(std::vector<uint8_t>& out, const ZipEntry& e) {
    write_u32(out, 0x02014b50);
    write_u16(out, 20);
    write_u16(out, 20);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0x21);
    write_u32(out, e.crc);
    write_u32(out, e.comp_size);
    write_u32(out, e.uncomp_size);
    write_u16(out, static_cast<uint16_t>(e.arcname.size()));
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u32(out, 0);
    write_u32(out, e.local_header_offset);
    for (char c : e.arcname) out.push_back(static_cast<uint8_t>(c));
}

static void write_end_central_dir(std::vector<uint8_t>& out, uint32_t count,
                                    uint32_t cd_offset, uint32_t cd_size) {
    write_u32(out, 0x06054b50);
    write_u16(out, 0);
    write_u16(out, 0);
    write_u16(out, static_cast<uint16_t>(count));
    write_u16(out, static_cast<uint16_t>(count));
    write_u32(out, cd_size);
    write_u32(out, cd_offset);
    write_u16(out, 0);
}

static void collect_entries(const fs::path& root, std::vector<ZipEntry>& entries) {
    static const char* include_dirs[] = {"include", "src", "assets", "tests", "docs", "scripts", "saves"};
    static const char* include_files[] = {"CMakeLists.txt", "README.md", "HOW_TO_RUN.md", "Project - Go game.pdf"};

    for (auto fname : include_files) {
        fs::path fp = root / fname;
        if (!fs::exists(fp)) continue;
        ZipEntry e;
        e.arcname = "GoGame/" + std::string(fname);
        e.fullpath = fp.string();
        e.is_directory = false;
        e.crc = 0;
        e.comp_size = 0;
        e.uncomp_size = 0;
        entries.push_back(e);
    }

    for (auto dname : include_dirs) {
        fs::path dp = root / dname;
        if (!fs::exists(dp)) continue;

        if (fs::is_directory(dp) && fs::is_empty(dp)) {
            ZipEntry e;
            e.arcname = "GoGame/" + std::string(dname) + "/";
            e.fullpath = dp.string();
            e.is_directory = true;
            entries.push_back(e);
            continue;
        }

        std::vector<fs::path> dirs_to_visit;
        dirs_to_visit.push_back(dp);

        while (!dirs_to_visit.empty()) {
            fs::path cur = dirs_to_visit.back();
            dirs_to_visit.pop_back();

            std::vector<fs::directory_entry> items;
            for (auto& entry : fs::directory_iterator(cur))
                items.push_back(entry);

            for (auto& entry : items) {
                std::string name = entry.path().filename().string();
                if (entry.is_directory()) {
                    if (is_excluded_dir(name)) continue;
                    dirs_to_visit.push_back(entry.path());
                } else if (entry.is_regular_file()) {
                    if (is_excluded_file(entry.path())) continue;

                    std::string rel = to_posix(fs::relative(entry.path(), root));
                    std::string arcname = "GoGame/" + rel;

                    auto data = read_file(entry.path());
                    uint32_t file_crc = crc32(data.data(), data.size());

                    ZipEntry ze;
                    ze.arcname = arcname;
                    ze.fullpath = entry.path().string();
                    ze.is_directory = false;
                    ze.crc = file_crc;
                    ze.comp_size = static_cast<uint32_t>(data.size());
                    ze.uncomp_size = static_cast<uint32_t>(data.size());
                    entries.push_back(ze);
                }
            }
        }
    }
}

static void write_zip(const fs::path& output, std::vector<ZipEntry>& entries) {
    std::vector<uint8_t> file_data;

    for (auto& e : entries) {
        size_t header_start = file_data.size();
        write_local_header(file_data, e.arcname, e.crc, e.comp_size, e.uncomp_size);
        e.local_header_offset = static_cast<uint32_t>(header_start);

        if (!e.is_directory) {
            auto file_bytes = read_file(e.fullpath);
            for (auto b : file_bytes) file_data.push_back(b);
        }
    }

    uint32_t cd_offset = static_cast<uint32_t>(file_data.size());

    std::vector<uint8_t> cd_data;
    for (auto& e : entries)
        write_central_dir(cd_data, e);

    for (auto b : cd_data) file_data.push_back(b);

    uint32_t cd_size = static_cast<uint32_t>(cd_data.size());
    write_end_central_dir(file_data, static_cast<uint32_t>(entries.size()), cd_offset, cd_size);

    std::ofstream out(output, std::ios::binary);
    out.write(reinterpret_cast<const char*>(file_data.data()), file_data.size());
    out.close();
}

int main() {
    fs::path root = fs::current_path();
    fs::path output = root / "GoGame_Submission.zip";

    std::vector<ZipEntry> entries;
    collect_entries(root, entries);

    write_zip(output, entries);

    std::cout << "Created submission zip at: " << output.string() << "\n";
    std::cout << "Entries: " << entries.size() << "\n";
    std::cout << "File size: " << fs::file_size(output) << " bytes\n";
    return 0;
}
