/**
 * @file dawproject.cpp
 * @brief Implementation of High-Level DAW Project API (US-001)
 * 
 * Implements the simple, intuitive API for loading and working with DAW Project files
 * as specified in User Story US-001.
 * 
 * Traceability: US-001, REQ-F-001, REQ-F-002
 */

#include <dawproject/dawproject.h>
#include <dawproject/data/data_access_factory.h>
#include <sstream>

namespace dawproject {

    // ==================== Exception Implementations ====================

    DawProjectException::DawProjectException(const std::string& message)
        : message_(message) {
    }

    DawProjectException::DawProjectException(const std::string& message, const std::filesystem::path& filePath)
        : message_(message), filePath_(filePath) {
    }

    const char* DawProjectException::what() const noexcept {
        if (whatMessage_.empty()) {
            if (filePath_.empty()) {
                whatMessage_ = message_;
            } else {
                whatMessage_ = message_ + " (file: " + filePath_.string() + ")";
            }
        }
        return whatMessage_.c_str();
    }

    const std::string& DawProjectException::getMessage() const noexcept {
        return message_;
    }

    const std::filesystem::path& DawProjectException::getFilePath() const noexcept {
        return filePath_;
    }

    FileNotFoundException::FileNotFoundException(const std::filesystem::path& filePath)
        : DawProjectException("File not found. Please check file path and permissions.", filePath) {
    }

    // ==================== ProjectMetadata Implementation ====================

    ProjectMetadata::ProjectMetadata(const data::ProjectInfo& info) : info_(info) {
    }

    const std::string& ProjectMetadata::getTitle() const {
        return info_.title;
    }

    const std::string& ProjectMetadata::getArtist() const {
        return info_.artist;
    }

    const std::string& ProjectMetadata::getAlbum() const {
        return info_.album;
    }

    const std::string& ProjectMetadata::getGenre() const {
        return info_.genre;
    }

    double ProjectMetadata::getTempo() const {
        return info_.tempo;
    }

    const std::string& ProjectMetadata::getTimeSignature() const {
        return info_.timeSignature;
    }

    const std::string& ProjectMetadata::getKey() const {
        return info_.key;
    }

    // ==================== Track Implementation ====================

    Track::Track(const data::TrackInfo& info) : info_(info) {
    }

    const std::string& Track::getId() const {
        return info_.id;
    }

    const std::string& Track::getName() const {
        return info_.name;
    }

    data::TrackType Track::getType() const {
        return info_.type;
    }

    const std::string& Track::getColor() const {
        return info_.color;
    }

    double Track::getVolume() const {
        return info_.volume;
    }

    double Track::getPan() const {
        return info_.pan;
    }

    bool Track::isMuted() const {
        return info_.muted;
    }

    bool Track::isSoloed() const {
        return info_.soloed;
    }

    // ==================== DawProject Implementation Details ====================

    /**
     * @brief Private implementation class (Pimpl idiom)
     * 
     * Hides implementation details and manages the low-level DataAccessEngine
     */
    class DawProjectImpl {
    public:
        explicit DawProjectImpl(const std::filesystem::path& filePath)
            : filePath_(filePath)
            , engine_(data::DataAccessFactory::createDataAccessEngine()) {
        }

        bool loadProject() {
            try {
                // Load project metadata
                auto metadataResult = engine_->loadProjectInfo(filePath_);
                if (!metadataResult.success) {
                    lastError_ = "Failed to load project metadata: " + metadataResult.errorMessage;
                    return false;
                }
                
                metadata_ = std::make_unique<ProjectMetadata>(metadataResult.value);
                
                // Load tracks
                auto tracksResult = engine_->loadTracks(filePath_);
                if (!tracksResult.success) {
                    lastError_ = "Failed to load project tracks: " + tracksResult.errorMessage;
                    return false;
                }
                
                // Convert data::TrackInfo to Track objects
                tracks_.clear();
                tracks_.reserve(tracksResult.value.size());
                for (const auto& trackInfo : tracksResult.value) {
                    tracks_.emplace_back(trackInfo);
                }
                
                valid_ = true;
                return true;
            }
            catch (const std::exception& e) {
                lastError_ = "Exception during project loading: " + std::string(e.what());
                valid_ = false;
                return false;
            }
        }

        bool isValid() const { return valid_; }
        const std::string& getLastError() const { return lastError_; }
        const std::filesystem::path& getFilePath() const { return filePath_; }
        
        const ProjectMetadata* getMetadata() const { return metadata_.get(); }
        const std::vector<Track>& getTracks() const { return tracks_; }

    private:
        std::filesystem::path filePath_;
        std::unique_ptr<data::IDataAccessEngine> engine_;
        std::unique_ptr<ProjectMetadata> metadata_;
        std::vector<Track> tracks_;
        bool valid_ = false;
        std::string lastError_;
    };

    // ==================== DawProject Implementation ====================

    DawProject::DawProject(std::unique_ptr<DawProjectImpl> impl) : impl_(std::move(impl)) {
    }

    DawProject::~DawProject() = default;

    std::unique_ptr<DawProject> DawProject::load(const std::string& filePath) {
        return load(std::filesystem::path(filePath));
    }

    std::unique_ptr<DawProject> DawProject::load(const std::filesystem::path& filePath) {
        // Input validation
        if (filePath.empty()) {
            throw DawProjectException("File path cannot be empty");
        }

        // Check if file exists
        if (!std::filesystem::exists(filePath)) {
            throw FileNotFoundException(filePath);
        }

        // Check if it's a regular file
        if (!std::filesystem::is_regular_file(filePath)) {
            throw DawProjectException("Path is not a regular file", filePath);
        }

        // Create implementation object
        auto impl = std::make_unique<DawProjectImpl>(filePath);
        
        // Attempt to load the project
        if (!impl->loadProject()) {
            throw DawProjectException("Failed to load DAW project: " + impl->getLastError(), filePath);
        }

        // Create and return the DawProject object using private constructor
        return std::unique_ptr<DawProject>(new DawProject(std::move(impl)));
    }

    const ProjectMetadata& DawProject::getMetadata() const {
        if (!impl_ || !impl_->getMetadata()) {
            throw DawProjectException("Project not loaded or metadata not available");
        }
        return *impl_->getMetadata();
    }

    const std::vector<Track>& DawProject::getTracks() const {
        if (!impl_) {
            throw DawProjectException("Project not loaded");
        }
        return impl_->getTracks();
    }

    bool DawProject::isValid() const {
        return impl_ && impl_->isValid();
    }

    const std::filesystem::path& DawProject::getFilePath() const {
        if (!impl_) {
            throw DawProjectException("Project not loaded");
        }
        return impl_->getFilePath();
    }

} // namespace dawproject