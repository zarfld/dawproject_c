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
#include <functional>
#include <stack>
#include <map>
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
     * @brief Validation issue found during DAWProject standard compliance analysis
     * 
     * Represents a potential issue or non-standard usage in a DAWProject file
     * Used in US-004 DAWProject Standard Compliance Analysis
     */
    struct ValidationIssue {
        std::string issueName;          ///< Name/type of the validation issue
        std::string description;        ///< Description of the issue and why it matters
        std::string severity;           ///< "warning", "error", "info"
        std::vector<std::string> affectedElements; ///< List of project elements affected
        std::string recommendation;     ///< Suggested fix or best practice
        
        ValidationIssue() = default;
        ValidationIssue(const std::string& name, const std::string& desc, 
                       const std::string& sev, const std::vector<std::string>& elements,
                       const std::string& rec = "")
            : issueName(name), description(desc), severity(sev), 
              affectedElements(elements), recommendation(rec) {}
    };

    /**
     * @brief DAWProject standard compliance analysis results
     * 
     * Contains detailed analysis of a DAWProject file's compliance with the v1.0 specification
     */
    struct ComplianceAnalysis {
        std::string projectName;                    ///< Name/title of the analyzed project
        std::string dawProjectVersion;              ///< DAWProject standard version used
        bool isCompliant;                           ///< Overall compliance status
        std::vector<std::string> featuresUsed;     ///< DAWProject v1.0 features used in this project
        std::vector<ValidationIssue> validationIssues; ///< Issues found during analysis
        std::map<std::string, std::string> statistics; ///< Usage statistics (track count, etc.)
        
        ComplianceAnalysis() = default;
        ComplianceAnalysis(const std::string& name, bool compliant)
            : projectName(name), isCompliant(compliant) {}
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
         * @brief Rename a track by index
         * 
         * Implements US-003 acceptance criteria:
         * - Track Editing: Add, remove, rename, and reorder tracks
         * - Undo/Redo: All edits are undoable and redoable
         * - Thread Safety: Edits are safe in multi-threaded contexts
         * 
         * @param trackIndex Index of track to rename (0-based)
         * @param newName New name for the track
         * @throws std::out_of_range if trackIndex is invalid
         * @throws DawProjectException for other errors
         */
        void renameTrack(size_t trackIndex, const std::string& newName);
        
        /**
         * @brief Add a new track to the project
         * 
         * @param name Name for the new track
         * @param type Type of track (Audio, MIDI, etc.)
         * @return Index of the newly added track
         */
        size_t addTrack(const std::string& name, data::TrackType type);
        
        /**
         * @brief Remove a track by index
         * 
         * @param trackIndex Index of track to remove (0-based)
         * @throws std::out_of_range if trackIndex is invalid
         */
        void removeTrack(size_t trackIndex);
        
        /**
         * @brief Undo the last edit operation
         * 
         * Implements US-003 undo/redo functionality
         * 
         * @throws DawProjectException if no operations to undo
         */
        void undo();
        
        /**
         * @brief Redo the last undone operation
         * 
         * @throws DawProjectException if no operations to redo
         */
        void redo();
        
        /**
         * @brief Analyze DAWProject standard compliance
         * 
         * Implements US-004: DAWProject Standard Compliance Analysis
         * Analyzes the project for adherence to DAWProject v1.0 specification
         * 
         * @return ComplianceAnalysis containing detailed standard compliance analysis
         * @throws DawProjectException if project is not loaded
         */
        ComplianceAnalysis analyzeCompliance() const;
        
        /**
         * @brief Get detailed feature usage analysis
         * 
         * Provides detailed information about which DAWProject v1.0 features
         * are used in the current project
         * 
         * @return Vector of strings describing each feature used
         */
        std::vector<std::string> getFeatureUsage() const;
        
        /**
         * @brief Get validation issues found in the project
         * 
         * Identifies potential issues, warnings, or non-standard usage patterns
         * 
         * @return Vector of ValidationIssue objects describing potential problems
         */
        std::vector<ValidationIssue> getValidationIssues() const;
        
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