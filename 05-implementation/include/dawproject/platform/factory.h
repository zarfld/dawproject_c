#pragma once

/**
 * @file factory.h
 * @brief Platform Factory Interface (DES-C-004)
 * 
 * Factory interface for creating platform-specific implementations of
 * the platform abstraction interfaces.
 */

#include "interfaces.h"
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdint>

namespace dawproject::platform {

    /**
     * @brief Platform exception class for platform-specific errors
     */
    class PlatformException : public std::exception {
    public:
        explicit PlatformException(const std::string& message,
                                 const std::string& context = "",
                                 const PlatformError& platformError = {})
            : message_(message), context_(context), platformError_(platformError) {}

        const char* what() const noexcept override {
            return message_.c_str();
        }

        const std::string& getContext() const { return context_; }
        const PlatformError& getPlatformError() const { return platformError_; }

    private:
        std::string message_;
        std::string context_;
        PlatformError platformError_;
    };

    /**
     * @brief Factory for creating platform-specific implementations
     * 
     * This factory provides a unified interface for creating platform abstraction
     * implementations. It uses the singleton pattern and lazy initialization
     * for optimal performance.
     */
    class PlatformFactory {
    public:
        /**
         * @brief Get singleton factory instance
         * @return Reference to factory instance
         */
        static PlatformFactory& getInstance();

        /**
         * @brief Create file system implementation
         * @return Unique pointer to platform-specific file system
         */
        virtual std::unique_ptr<IFileSystem> createFileSystem() = 0;

        /**
         * @brief Create threading implementation
         * @return Unique pointer to platform-specific threading
         */
        virtual std::unique_ptr<IThreading> createThreading() = 0;

        /**
         * @brief Create memory manager implementation
         * @return Unique pointer to platform-specific memory manager
         */
        virtual std::unique_ptr<IMemoryManager> createMemoryManager() = 0;

        /**
         * @brief Get system information
         * @return System information structure
         */
        virtual SystemInfo getSystemInfo() = 0;

        /**
         * @brief Get current platform
         * @return Platform enumeration value
         */
        virtual Platform getCurrentPlatform() = 0;

        /**
         * @brief Get current architecture
         * @return Architecture enumeration value
         */
        virtual Architecture getCurrentArchitecture() = 0;

        /**
         * @brief Check if system is little endian
         * @return true if little endian, false if big endian
         */
        virtual bool isLittleEndian() = 0;

        /**
         * @brief Get environment variable value
         * @param name Environment variable name
         * @return Variable value or empty string if not found
         */
        virtual std::string getEnvironmentVariable(const std::string& name) = 0;

        /**
         * @brief Set environment variable value
         * @param name Environment variable name
         * @param value Variable value
         */
        virtual void setEnvironmentVariable(const std::string& name, 
                                          const std::string& value) = 0;

    public:
        virtual ~PlatformFactory() = default;

        // Prevent copying
        PlatformFactory(const PlatformFactory&) = delete;
        PlatformFactory& operator=(const PlatformFactory&) = delete;

    protected:
        PlatformFactory() = default;

    private:
        static std::unique_ptr<PlatformFactory> instance_;
        static std::once_flag initFlag_;
        
        static void initialize();
        static std::unique_ptr<PlatformFactory> createPlatformSpecificFactory();
    };

    // Convenience namespace for common file system operations
    namespace fs {
        /**
         * @brief Check if path exists
         * @param path Path to check
         * @return true if path exists
         */
        bool exists(const std::filesystem::path& path);

        /**
         * @brief Create directories recursively
         * @param path Directory path to create
         * @return true if directories were created or already exist
         */
        bool createDirectories(const std::filesystem::path& path);

        /**
         * @brief Read entire file into byte vector
         * @param path Path to file
         * @return Vector containing file contents
         * @throws PlatformException on read errors
         */
        std::vector<uint8_t> readAllBytes(const std::filesystem::path& path);

        /**
         * @brief Write byte vector to file
         * @param path Path to file
         * @param data Data to write
         * @throws PlatformException on write errors
         */
        void writeAllBytes(const std::filesystem::path& path, 
                          const std::vector<uint8_t>& data);

        /**
         * @brief Read entire file as text string
         * @param path Path to file
         * @return String containing file contents
         * @throws PlatformException on read errors
         */
        std::string readAllText(const std::filesystem::path& path);

        /**
         * @brief Write text string to file
         * @param path Path to file
         * @param text Text to write
         * @throws PlatformException on write errors
         */
        void writeAllText(const std::filesystem::path& path, 
                         const std::string& text);
    }

} // namespace dawproject::platform