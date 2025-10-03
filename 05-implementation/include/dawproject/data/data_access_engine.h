#pragma once

#include <memory>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>
#include <optional>
#include <unordered_map>

namespace dawproject::data {

    // Forward declarations
    class IProjectReader;
    class IProjectWriter;
    class IXMLProcessor;
    class IZIPProcessor;

    // Data Transfer Objects
    struct ProjectInfo {
        std::string title;
        std::string artist;
        std::string album;
        std::string genre;
        double tempo;
        std::string timeSignature;
        std::string key;
        std::chrono::system_clock::time_point created;
        std::chrono::system_clock::time_point modified;
        
        bool isValid() const;
        std::vector<std::string> getValidationErrors() const;
    };

    enum class TrackType {
        Audio,
        Instrument,
        Group,
        Return
    };

    struct TrackInfo {
        std::string id;
        std::string name;
        TrackType type;
        std::string color;
        double volume;
        double pan;
        bool muted;
        bool soloed;
        int orderIndex;
        
        // Track-specific properties
        std::optional<std::string> instrumentId;  // For instrument tracks
        std::optional<std::string> audioFile;     // For audio tracks
        
        bool isValid() const;
    };

    struct ClipInfo {
        std::string id;
        std::string name;
        std::string trackId;
        double startTime;
        double duration;
        double playbackRate;
        double fadeInTime;
        double fadeOutTime;
        
        // Content reference
        std::optional<std::string> contentId;
        std::optional<std::string> audioFile;
        
        bool isValid() const;
    };

    struct ValidationResult {
        bool isValid;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        std::string context;
        
        void addError(const std::string& message);
        void addWarning(const std::string& message);
        void merge(const ValidationResult& other);
    };

    template<typename T>
    struct Result {
        bool success;
        T value;
        std::string errorMessage;
        int errorCode;
        
        static Result<T> makeSuccess(T&& value) {
            return {true, std::forward<T>(value), "", 0};
        }
        
        static Result<T> makeSuccess(const T& value) {
            return {true, value, "", 0};
        }
        
        static Result<T> makeError(const std::string& message, int code = -1) {
            return {false, T{}, message, code};
        }
        
        operator bool() const { return success; }
    };

    using WriteResult = Result<size_t>;

    struct ArchiveEntry {
        std::string name;
        std::string path;
        uint64_t compressedSize;
        uint64_t uncompressedSize;
        std::chrono::system_clock::time_point modified;
        bool isDirectory;
        
        bool isValid() const { return !name.empty(); }
    };

    // Primary data access interface
    class IDataAccessEngine {
    public:
        virtual ~IDataAccessEngine() = default;
        
        // Project file operations
        virtual Result<ProjectInfo> loadProjectInfo(
            const std::filesystem::path& path) = 0;
        virtual Result<std::vector<TrackInfo>> loadTracks(
            const std::filesystem::path& path) = 0;
        virtual Result<std::vector<ClipInfo>> loadClips(
            const std::filesystem::path& path, 
            const std::string& trackId = "") = 0;
        
        virtual WriteResult saveProject(
            const ProjectInfo& project, 
            const std::vector<TrackInfo>& tracks,
            const std::vector<ClipInfo>& clips,
            const std::filesystem::path& path) = 0;
        
        // File validation
        virtual ValidationResult validateFile(
            const std::filesystem::path& path) = 0;
        virtual bool isValidProjectFile(
            const std::filesystem::path& path) = 0;
        
        // Streaming support
        virtual std::unique_ptr<IProjectReader> createReader(
            const std::filesystem::path& path) = 0;
        virtual std::unique_ptr<IProjectWriter> createWriter(
            const std::filesystem::path& path) = 0;
    };

    // Streaming reader interface
    class IProjectReader {
    public:
        virtual ~IProjectReader() = default;
        
        virtual bool open() = 0;
        virtual void close() = 0;
        virtual bool isOpen() const = 0;
        
        virtual Result<ProjectInfo> readProjectInfo() = 0;
        virtual Result<TrackInfo> readNextTrack() = 0;
        virtual Result<ClipInfo> readNextClip() = 0;
        
        virtual bool hasMoreTracks() const = 0;
        virtual bool hasMoreClips() const = 0;
        virtual size_t getTrackCount() const = 0;
        virtual size_t getClipCount() const = 0;
    };

    // Streaming writer interface
    class IProjectWriter {
    public:
        virtual ~IProjectWriter() = default;
        
        virtual bool open() = 0;
        virtual void close() = 0;
        virtual bool isOpen() const = 0;
        
        virtual WriteResult writeProjectInfo(
            const ProjectInfo& info) = 0;
        virtual WriteResult writeTrack(
            const TrackInfo& track) = 0;
        virtual WriteResult writeClip(
            const ClipInfo& clip) = 0;
        
        virtual WriteResult finalize() = 0;
    };

    // XML Document abstraction
    struct XMLDocument {
        std::string rootElementName;
        std::unordered_map<std::string, std::string> attributes;
        std::string textContent;
        std::vector<XMLDocument> children;
        
        // Helper methods
        XMLDocument* findChild(const std::string& name);
        const XMLDocument* findChild(const std::string& name) const;
        std::vector<XMLDocument*> findChildren(const std::string& name);
        std::string getAttribute(const std::string& name, 
                               const std::string& defaultValue = "") const;
        void setAttribute(const std::string& name, const std::string& value);
    };

    // XML processing interface
    class IXMLProcessor {
    public:
        virtual ~IXMLProcessor() = default;
        
        // Document operations
        virtual Result<XMLDocument> loadDocument(
            const std::filesystem::path& path) = 0;
        virtual Result<XMLDocument> parseDocument(
            const std::string& xmlContent) = 0;
        virtual WriteResult saveDocument(
            const XMLDocument& doc, 
            const std::filesystem::path& path) = 0;
        
        // Validation
        virtual ValidationResult validateXML(
            const std::filesystem::path& path) = 0;
        virtual ValidationResult validateAgainstSchema(
            const XMLDocument& doc, 
            const std::filesystem::path& schemaPath) = 0;
    };

    // ZIP archive interface
    class IZIPProcessor {
    public:
        virtual ~IZIPProcessor() = default;
        
        // Archive operations
        virtual Result<std::vector<ArchiveEntry>> listEntries(
            const std::filesystem::path& archivePath) = 0;
        virtual Result<std::vector<uint8_t>> extractEntry(
            const std::filesystem::path& archivePath,
            const std::string& entryName) = 0;
        virtual WriteResult addEntry(
            const std::filesystem::path& archivePath,
            const std::string& entryName,
            const std::vector<uint8_t>& data) = 0;
        
        // Validation
        virtual bool isValidArchive(
            const std::filesystem::path& archivePath) = 0;
    };

} // namespace dawproject::data