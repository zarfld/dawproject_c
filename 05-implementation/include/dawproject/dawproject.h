#pragma once

/**
 * @file dawproject.h
 * @brief High-Level DAW Project API (US-001)
 * 
 * Provides simple, intuitive API for loading and working with DAW Project files
 * as specified in User Story US-001: Load DAW Project File
 * 
 * Traceability: US-001, REQ-F-001, REQ-F-002
 */

#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <dawproject/data/data_access_engine.h>

namespace dawproject {

    // Forward declarations
    class DawProjectImpl;

    /**
     * @brief Exception thrown when DAW Project operations fail
     * 
     * Provides clear error messages for malformed or missing files
     * as required by US-001 acceptance criteria.
     */
    class DawProjectException : public std::exception {
    public:
        explicit DawProjectException(const std::string& message);
        explicit DawProjectException(const std::string& message, const std::filesystem::path& filePath);
        
        const char* what() const noexcept override;
        const std::string& getMessage() const noexcept;
        const std::filesystem::path& getFilePath() const noexcept;
        
    private:
        std::string message_;
        std::filesystem::path filePath_;
        mutable std::string whatMessage_;
    };

    /**
     * @brief File not found exception for missing DAW Project files
     */
    class FileNotFoundException : public DawProjectException {
    public:
        explicit FileNotFoundException(const std::filesystem::path& filePath);
    };

    /**
     * @brief Project metadata accessible through object-oriented interface
     * 
     * Corresponds to US-001 requirement: "All project elements accessible 
     * through object-oriented interface"
     */
    class ProjectMetadata {
    public:
        ProjectMetadata(const data::ProjectInfo& info);
        
        const std::string& getTitle() const;
        const std::string& getArtist() const;
        const std::string& getAlbum() const;
        const std::string& getGenre() const;
        double getTempo() const;
        const std::string& getTimeSignature() const;
        const std::string& getKey() const;
        
    private:
        data::ProjectInfo info_;
    };

    /**
     * @brief Track information with properties
     */
    class Track {
    public:
        Track(const data::TrackInfo& info);
        
        const std::string& getId() const;
        const std::string& getName() const;
        data::TrackType getType() const;
        const std::string& getColor() const;
        double getVolume() const;
        double getPan() const;
        bool isMuted() const;
        bool isSoloed() const;
        
    private:
        data::TrackInfo info_;
    };

    /**
     * @brief High-level DAW Project interface
     * 
     * Provides simple API for loading DAW Project files as specified in US-001:
     * "As a DAW developer, I want to load a DAW Project file with simple API calls"
     * 
     * Usage example:
     * ```cpp
     * try {
     *     auto project = DawProject::load("myproject.dawproject");
     *     std::cout << "Title: " << project->getMetadata().getTitle() << std::endl;
     *     
     *     for (const auto& track : project->getTracks()) {
     *         std::cout << "Track: " << track.getName() << std::endl;
     *     }
     * } catch (const DawProjectException& e) {
     *     std::cerr << "Error: " << e.what() << std::endl;
     * }
     * ```
     */
    class DawProject {
    public:
        /**
         * @brief Load DAW Project file with single function call
         * 
         * Implements US-001 acceptance criteria:
         * - Simple API: Load project with single function call
         * - Error Handling: Clear error messages for malformed or missing files
         * - Memory Management: Automatic resource cleanup using RAII
         * 
         * @param filePath Path to .dawproject file to load
         * @return Unique pointer to loaded project (RAII for automatic cleanup)
         * @throws FileNotFoundException if file doesn't exist
         * @throws DawProjectException for malformed files or other errors
         */
        static std::unique_ptr<DawProject> load(const std::string& filePath);
        
        /**
         * @brief Load DAW Project from filesystem path
         * 
         * Overload accepting std::filesystem::path for convenience
         */
        static std::unique_ptr<DawProject> load(const std::filesystem::path& filePath);
        
        /**
         * @brief Get project metadata
         * 
         * Implements US-001 requirement: "I can access project.getMetadata().getTitle()"
         * 
         * @return Project metadata with title, artist, tempo, etc.
         */
        const ProjectMetadata& getMetadata() const;
        
        /**
         * @brief Get all tracks in the project
         * 
         * Implements US-001 requirement: "I can iterate through project.getTracks()"
         * 
         * @return Vector of tracks with names and properties
         */
        const std::vector<Track>& getTracks() const;
        
        /**
         * @brief Save DAW Project to file
         * 
         * Implements US-002 acceptance criteria:
         * - Simple API: Save project with single function call
         * - Error Handling: Clear error messages for invalid or unwritable files
         * - Data Integrity: All project elements are written and verifiable on reload
         * - Performance: Save 32-track project within 30 seconds on standard hardware
         * 
         * @param filePath Path to save .dawproject file to
         * @throws DawProjectException for file system errors or write permissions
         */
        void save(const std::string& filePath) const;
        
        /**
         * @brief Save DAW Project to filesystem path
         * 
         * Overload accepting std::filesystem::path for convenience
         * 
         * @param filePath Path to save .dawproject file to  
         * @throws DawProjectException for file system errors or write permissions
         */
        void save(const std::filesystem::path& filePath) const;
        
        /**
         * @brief Check if project was loaded successfully
         */
        bool isValid() const;
        
        /**
         * @brief Get the file path this project was loaded from
         */
        const std::filesystem::path& getFilePath() const;
        
        // Destructor (RAII - automatic resource cleanup)
        ~DawProject();
        
    private:
        // Private constructor - use load() factory method
        explicit DawProject(std::unique_ptr<DawProjectImpl> impl);
        
        // Pimpl idiom for implementation details
        std::unique_ptr<DawProjectImpl> impl_;
    };

} // namespace dawproject