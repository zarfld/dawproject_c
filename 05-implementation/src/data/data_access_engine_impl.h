#pragma once

#include <dawproject/data/data_access_engine.h>
#include <memory>

namespace dawproject::data {

    /**
     * @brief Data Access Engine Implementation for DAW projects
     * 
     * This class orchestrates XML and ZIP processing to provide a complete
     * data access layer for DAW project files. It implements the IDataAccessEngine
     * interface by coordinating between XMLProcessor and ZIPProcessor components.
     * 
     * The implementation follows the Dependency Injection pattern, accepting
     * processor implementations through its constructor to promote testability
     * and flexibility.
     * 
     * @note This class follows RAII principles and is move-constructible
     * @note Thread-safety: This class is not thread-safe by design
     */
    class DataAccessEngineImpl : public IDataAccessEngine {
    private:
        std::unique_ptr<IXMLProcessor> xmlProcessor_;  ///< XML processing component
        std::unique_ptr<IZIPProcessor> zipProcessor_;  ///< ZIP processing component

    public:
        /**
         * @brief Construct with dependency injection
         * @param xmlProc XML processor implementation 
         * @param zipProc ZIP processor implementation
         */
        DataAccessEngineImpl(std::unique_ptr<IXMLProcessor> xmlProc, 
                           std::unique_ptr<IZIPProcessor> zipProc);
        
        /**
         * @brief Virtual destructor for proper cleanup
         */
        virtual ~DataAccessEngineImpl() = default;

        /**
         * @brief Copy constructor (deleted - unique ownership)
         */
        DataAccessEngineImpl(const DataAccessEngineImpl&) = delete;
        
        /**
         * @brief Move constructor for efficient transfers
         */
        DataAccessEngineImpl(DataAccessEngineImpl&&) = default;
        
        /**
         * @brief Copy assignment operator (deleted - unique ownership)
         */
        DataAccessEngineImpl& operator=(const DataAccessEngineImpl&) = delete;
        
        /**
         * @brief Move assignment operator 
         */
        DataAccessEngineImpl& operator=(DataAccessEngineImpl&&) = default;

        // IDataAccessEngine interface implementation
        /**
         * @brief Load project metadata and settings
         * @param path Path to the project file
         * @return Result containing ProjectInfo or error message
         */
        Result<ProjectInfo> loadProjectInfo(const std::filesystem::path& path) override;
        
        /**
         * @brief Load all tracks from the project
         * @param path Path to the project file
         * @return Result containing vector of TrackInfo or error message
         */
        Result<std::vector<TrackInfo>> loadTracks(const std::filesystem::path& path) override;
        
        /**
         * @brief Load clips, optionally filtered by track ID
         * @param path Path to the project file
         * @param trackId Optional track ID filter (empty string loads all)
         * @return Result containing vector of ClipInfo or error message  
         */
        Result<std::vector<ClipInfo>> loadClips(const std::filesystem::path& path, 
                                               const std::string& trackId = "") override;
        
        /**
         * @brief Save complete project data to file
         * @param project Project metadata
         * @param tracks Track information
         * @param clips Clip information 
         * @param path Destination file path
         * @return WriteResult with success status and bytes written or error message
         */
        WriteResult saveProject(const ProjectInfo& project, 
                              const std::vector<TrackInfo>& tracks,
                              const std::vector<ClipInfo>& clips,
                              const std::filesystem::path& path) override;
        
        /**
         * @brief Validate project file structure and content
         * @param path Path to validate
         * @return ValidationResult with validation status and details
         */
        ValidationResult validateFile(const std::filesystem::path& path) override;
        
        /**
         * @brief Quick check if file is a valid project file
         * @param path Path to check
         * @return true if valid project file, false otherwise
         */
        bool isValidProjectFile(const std::filesystem::path& path) override;
        
        /**
         * @brief Create a reader for streaming project data
         * @param path Path to the project file
         * @return Unique pointer to IProjectReader or nullptr on error
         */
        std::unique_ptr<IProjectReader> createReader(const std::filesystem::path& path) override;
        
        /**
         * @brief Create a writer for streaming project data  
         * @param path Path for the output file
         * @return Unique pointer to IProjectWriter or nullptr on error
         */
        std::unique_ptr<IProjectWriter> createWriter(const std::filesystem::path& path) override;
    };

} // namespace dawproject::data