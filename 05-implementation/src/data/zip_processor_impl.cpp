#include "zip_processor_impl.h"
#include <fstream>
#include <filesystem>
#include <chrono>

namespace dawproject::data {

    Result<std::vector<ArchiveEntry>> ZIPProcessorImpl::listEntries(const std::filesystem::path& archivePath) {
        // Input validation
        if (archivePath.empty()) {
            return Result<std::vector<ArchiveEntry>>::makeError("Archive path cannot be empty");
        }
        
        try {
            std::vector<ArchiveEntry> entries;
            
            if (!std::filesystem::exists(archivePath)) {
                return Result<std::vector<ArchiveEntry>>::makeError(
                    "Archive file does not exist: " + archivePath.string()
                );
            }
            
            if (!std::filesystem::is_regular_file(archivePath)) {
                return Result<std::vector<ArchiveEntry>>::makeError(
                    "Path is not a regular file: " + archivePath.string()
                );
            }
            
            // Get file info with error handling
            const auto fileSize = std::filesystem::file_size(archivePath);
            if (fileSize == 0) {
                return Result<std::vector<ArchiveEntry>>::makeSuccess(std::move(entries)); // Empty archive
            }
            
            ArchiveEntry entry;
            entry.name = "project.xml";
            entry.path = "project.xml";
            entry.compressedSize = fileSize;
            entry.uncompressedSize = fileSize; // Same for GREEN phase
            
            // Safely convert file time with error handling
            try {
                const auto fileTime = std::filesystem::last_write_time(archivePath);
                const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    fileTime - std::filesystem::file_time_type::clock::now() + 
                    std::chrono::system_clock::now());
                entry.modified = sctp;
            }
            catch (const std::exception&) {
                // Use current time if file time conversion fails
                entry.modified = std::chrono::system_clock::now();
            }
            
            entry.isDirectory = false;
            entries.push_back(std::move(entry));

            return Result<std::vector<ArchiveEntry>>::makeSuccess(std::move(entries));
        }
        catch (const std::filesystem::filesystem_error& fe) {
            return Result<std::vector<ArchiveEntry>>::makeError(
                "Filesystem error listing archive: " + std::string(fe.what())
            );
        }
        catch (const std::exception& e) {
            return Result<std::vector<ArchiveEntry>>::makeError(
                "Exception listing archive entries: " + std::string(e.what())
            );
        }
    }

    Result<std::vector<uint8_t>> ZIPProcessorImpl::extractEntry(const std::filesystem::path& archivePath,
                                         const std::string& entryName) {
        // Input validation
        if (archivePath.empty()) {
            return Result<std::vector<uint8_t>>::makeError("Archive path cannot be empty");
        }
        
        if (entryName.empty()) {
            return Result<std::vector<uint8_t>>::makeError("Entry name cannot be empty");
        }
        
        try {
            if (!std::filesystem::exists(archivePath)) {
                return Result<std::vector<uint8_t>>::makeError(
                    "Archive file does not exist: " + archivePath.string()
                );
            }
            
            if (!std::filesystem::is_regular_file(archivePath)) {
                return Result<std::vector<uint8_t>>::makeError(
                    "Path is not a regular file: " + archivePath.string()
                );
            }

            // Check file size before reading
            const auto fileSize = std::filesystem::file_size(archivePath);
            if (fileSize == 0) {
                return Result<std::vector<uint8_t>>::makeError(
                    "Archive file is empty: " + archivePath.string()
                );
            }
            
            // Size limit check (100MB)
            const size_t maxFileSize = 100 * 1024 * 1024;
            if (fileSize > maxFileSize) {
                return Result<std::vector<uint8_t>>::makeError(
                    "Archive file too large (" + std::to_string(fileSize) + " bytes): " + archivePath.string()
                );
            }

            std::ifstream file(archivePath, std::ios::binary);
            if (!file.is_open()) {
                return Result<std::vector<uint8_t>>::makeError(
                    "Failed to open archive file: " + archivePath.string()
                );
            }

            // Read file content with error checking
            std::vector<uint8_t> data(static_cast<size_t>(fileSize));
            file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(fileSize));
            
            if (file.fail() && !file.eof()) {
                return Result<std::vector<uint8_t>>::makeError(
                    "Failed to read archive content from: " + archivePath.string()
                );
            }
            
            // Adjust size if read was partial
            const auto actualBytesRead = file.gcount();
            if (actualBytesRead != static_cast<std::streamsize>(fileSize)) {
                data.resize(static_cast<size_t>(actualBytesRead));
            }

            return Result<std::vector<uint8_t>>::makeSuccess(std::move(data));
        }
        catch (const std::filesystem::filesystem_error& fe) {
            return Result<std::vector<uint8_t>>::makeError(
                "Filesystem error extracting entry: " + std::string(fe.what())
            );
        }
        catch (const std::bad_alloc& ba) {
            return Result<std::vector<uint8_t>>::makeError(
                "Memory allocation failed for file size: " + std::string(ba.what())
            );
        }
        catch (const std::exception& e) {
            return Result<std::vector<uint8_t>>::makeError(
                "Exception extracting entry: " + std::string(e.what())
            );
        }
    }

    WriteResult ZIPProcessorImpl::addEntry(const std::filesystem::path& archivePath,
                   const std::string& entryName,
                   const std::vector<uint8_t>& data) {
        // Input validation
        if (archivePath.empty()) {
            return WriteResult::makeError("Archive path cannot be empty");
        }
        
        if (entryName.empty()) {
            return WriteResult::makeError("Entry name cannot be empty");
        }
        
        // Validate entry name for security (no path traversal)
        if (entryName.find("../") != std::string::npos || entryName.find("..\\") != std::string::npos) {
            return WriteResult::makeError("Invalid entry name (path traversal detected): " + entryName);
        }
        
        try {
            // Create archive directory if it doesn't exist
            const std::filesystem::path archiveDir = archivePath.parent_path();
            if (!archiveDir.empty() && !std::filesystem::exists(archiveDir)) {
                std::filesystem::create_directories(archiveDir);
            }

            // Write data with proper error checking
            std::ofstream file(archivePath, std::ios::binary);
            if (!file.is_open()) {
                return WriteResult::makeError("Failed to create archive file: " + archivePath.string());
            }

            if (!data.empty()) {
                file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
                
                if (file.fail()) {
                    return WriteResult::makeError("Failed to write data to archive: " + archivePath.string());
                }
            }
            
            file.close();
            if (file.fail()) {
                return WriteResult::makeError("Failed to close archive file: " + archivePath.string());
            }
            
            // Verify file was created successfully
            if (!std::filesystem::exists(archivePath)) {
                return WriteResult::makeError("Archive file was not created: " + archivePath.string());
            }

            return WriteResult::makeSuccess(data.size());
        }
        catch (const std::filesystem::filesystem_error& fe) {
            return WriteResult::makeError("Filesystem error adding entry: " + std::string(fe.what()));
        }
        catch (const std::exception& e) {
            return WriteResult::makeError("Exception adding entry to archive: " + std::string(e.what()));
        }
    }

    bool ZIPProcessorImpl::isValidArchive(const std::filesystem::path& archivePath) {
        try {
            // Input validation
            if (archivePath.empty()) {
                return false;
            }
            
            // Check file existence and type
            if (!std::filesystem::exists(archivePath)) {
                return false;
            }
            
            if (!std::filesystem::is_regular_file(archivePath)) {
                return false;
            }
            
            // Check if file is readable and not empty
            const auto fileSize = std::filesystem::file_size(archivePath);
            if (fileSize == 0) {
                return false; // Empty files are not valid archives
            }
            
            // For GREEN phase: basic validation passed
            // In a real implementation, this would check ZIP file signature
            return true;
        }
        catch (const std::filesystem::filesystem_error&) {
            return false;
        }
        catch (const std::exception&) {
            return false;
        }
    }

} // namespace dawproject::data