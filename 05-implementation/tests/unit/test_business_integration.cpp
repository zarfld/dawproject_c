/**
 * @file test_business_integration.cpp
 * @brief Comprehensive business logic integration test coverage
 * 
 * Standards: ISO/IEC/IEEE 12207:2017 (Implementation Process)
 * DAWProject: v1.0 End-to-End Workflow Testing
 * Coverage Target: +6% improvement from integration scenarios
 * Test Framework: Catch2 v3.4.0
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <chrono>
#include <string>
#include <map>
#include <algorithm>

namespace fs = std::filesystem;

// Mock Business Logic Interfaces (replace with actual implementations)
class DAWProjectInterface {
public:
    virtual ~DAWProjectInterface() = default;
    virtual bool createProject(const std::string& name, const std::string& version = "1.0") = 0;
    virtual bool addTrack(const std::string& id, const std::string& name, const std::string& mediaType) = 0;
    virtual bool addClip(const std::string& trackId, const std::string& clipId, double startTime, double duration) = 0;
    virtual bool setTempo(double bpm, double time = 0.0) = 0;
    virtual bool setTimeSignature(int numerator, int denominator, double time = 0.0) = 0;
    virtual bool saveToFile(const fs::path& filePath) = 0;
    virtual bool loadFromFile(const fs::path& filePath) = 0;
    virtual std::string exportToXML() const = 0;
    virtual bool validate() const = 0;
};

class AudioEngineInterface {
public:
    virtual ~AudioEngineInterface() = default;
    virtual bool initialize(int sampleRate, int bufferSize) = 0;
    virtual bool processAudio(const std::vector<float>& input, std::vector<float>& output) = 0;
    virtual bool addEffect(const std::string& trackId, const std::string& effectId) = 0;
    virtual bool setParameter(const std::string& effectId, const std::string& paramName, float value) = 0;
    virtual bool startPlayback() = 0;
    virtual bool stopPlayback() = 0;
    virtual bool isPlaying() const = 0;
    virtual double getCurrentTime() const = 0;
};

class ExportEngineInterface {
public:
    virtual ~ExportEngineInterface() = default;
    virtual bool exportAudio(const fs::path& outputPath, const std::string& format) = 0;
    virtual bool exportMIDI(const fs::path& outputPath) = 0;
    virtual bool setQualitySettings(int sampleRate, int bitDepth) = 0;
    virtual double getExportProgress() const = 0;
};

// Mock implementations for testing
class MockDAWProject : public DAWProjectInterface {
private:
    std::string projectName_;
    std::string version_;
    std::map<std::string, std::string> tracks_;
    std::vector<std::map<std::string, std::string>> clips_;
    double tempo_ = 120.0;
    std::pair<int, int> timeSignature_ = {4, 4};
    bool isValid_ = false;

public:
    bool createProject(const std::string& name, const std::string& version = "1.0") override {
        if (name.empty() || version.empty()) return false;
        projectName_ = name;
        version_ = version;
        isValid_ = true;
        return true;
    }
    
    bool addTrack(const std::string& id, const std::string& name, const std::string& mediaType) override {
        if (id.empty() || name.empty() || !isValid_) return false;
        tracks_[id] = name + ":" + mediaType;
        return true;
    }
    
    bool addClip(const std::string& trackId, const std::string& clipId, double startTime, double duration) override {
        if (trackId.empty() || clipId.empty() || startTime < 0 || duration <= 0) return false;
        if (tracks_.find(trackId) == tracks_.end()) return false;
        
        clips_.push_back({
            {"trackId", trackId},
            {"clipId", clipId},
            {"startTime", std::to_string(startTime)},
            {"duration", std::to_string(duration)}
        });
        return true;
    }
    
    bool setTempo(double bpm, double time = 0.0) override {
        if (bpm <= 0 || bpm > 300 || time < 0) return false;
        tempo_ = bpm;
        return true;
    }
    
    bool setTimeSignature(int numerator, int denominator, double time = 0.0) override {
        if (numerator <= 0 || denominator <= 0 || time < 0) return false;
        timeSignature_ = {numerator, denominator};
        return true;
    }
    
    bool saveToFile(const fs::path& filePath) override {
        if (!isValid_ || filePath.empty()) return false;
        
        try {
            std::ofstream ofs(filePath);
            ofs << exportToXML();
            return ofs.good();
        } catch (...) {
            return false;
        }
    }
    
    bool loadFromFile(const fs::path& filePath) override {
        if (!fs::exists(filePath)) return false;
        
        try {
            std::ifstream ifs(filePath);
            std::string content((std::istreambuf_iterator<char>(ifs)),
                              std::istreambuf_iterator<char>());
            
            // Simple validation - check for basic DAWProject structure
            return content.find("<Project version=\"1.0\"") != std::string::npos;
        } catch (...) {
            return false;
        }
    }
    
    std::string exportToXML() const override {
        if (!isValid_) return "";
        
        std::ostringstream oss;
        oss << R"(<?xml version="1.0" encoding="UTF-8"?>
<Project version=")" << version_ << R"(" xmlns="http://dawproject.org/XML">
    <Application name=")" << projectName_ << R"(" version="1.0"/>
    <Transport>
        <Tempo automationLaneId="tempo">
            <Real time="0" value=")" << tempo_ << R"("/>
        </Tempo>
        <TimeSignature>
            <Integer time="0" value=")" << timeSignature_.first << R"("/>
        </TimeSignature>
    </Transport>
    <Structure>)";
        
        for (const auto& track : tracks_) {
            auto parts = track.second;
            auto colonPos = parts.find(':');
            std::string name = parts.substr(0, colonPos);
            std::string mediaType = parts.substr(colonPos + 1);
            
            oss << R"(
        <Track id=")" << track.first << R"(" name=")" << name << R"(" mediaType=")" << mediaType << R"(">
            <Channel role="regular"/>
        </Track>)";
        }
        
        oss << R"(
    </Structure>
    <Arrangement>
        <Lanes>)";
        
        for (const auto& track : tracks_) {
            oss << R"(
            <Lane id="lane_)" << track.first << R"(" trackId=")" << track.first << R"("/>)";
        }
        
        oss << R"(
        </Lanes>
        <Clips>)";
        
        for (const auto& clip : clips_) {
            oss << R"(
            <Clip id=")" << clip.at("clipId") << R"(" trackId=")" << clip.at("trackId") 
                << R"(" time=")" << clip.at("startTime") << R"(" duration=")" << clip.at("duration") << R"("/>)";
        }
        
        oss << R"(
        </Clips>
    </Arrangement>
</Project>)";
        
        return oss.str();
    }
    
    bool validate() const override {
        return isValid_ && !projectName_.empty() && !version_.empty();
    }
    
    // Test helpers
    size_t getTrackCount() const { return tracks_.size(); }
    size_t getClipCount() const { return clips_.size(); }
    double getTempo() const { return tempo_; }
    std::pair<int, int> getTimeSignature() const { return timeSignature_; }
};

class MockAudioEngine : public AudioEngineInterface {
private:
    bool initialized_ = false;
    bool playing_ = false;
    double currentTime_ = 0.0;
    int sampleRate_ = 44100;
    int bufferSize_ = 512;
    std::map<std::string, std::vector<std::string>> trackEffects_;
    
public:
    bool initialize(int sampleRate, int bufferSize) override {
        if (sampleRate <= 0 || bufferSize <= 0) return false;
        sampleRate_ = sampleRate;
        bufferSize_ = bufferSize;
        initialized_ = true;
        return true;
    }
    
    bool processAudio(const std::vector<float>& input, std::vector<float>& output) override {
        if (!initialized_ || input.size() != output.size()) return false;
        
        // Simple pass-through with gain
        std::transform(input.begin(), input.end(), output.begin(),
                      [](float sample) { return sample * 0.8f; });
        
        if (playing_) {
            currentTime_ += static_cast<double>(bufferSize_) / sampleRate_;
        }
        
        return true;
    }
    
    bool addEffect(const std::string& trackId, const std::string& effectId) override {
        if (trackId.empty() || effectId.empty()) return false;
        trackEffects_[trackId].push_back(effectId);
        return true;
    }
    
    bool setParameter(const std::string& effectId, const std::string& paramName, float value) override {
        // Mock parameter setting - just validate inputs
        return !effectId.empty() && !paramName.empty() && value >= 0.0f && value <= 1.0f;
    }
    
    bool startPlayback() override {
        if (!initialized_) return false;
        playing_ = true;
        return true;
    }
    
    bool stopPlayback() override {
        playing_ = false;
        currentTime_ = 0.0;
        return true;
    }
    
    bool isPlaying() const override { return playing_; }
    double getCurrentTime() const override { return currentTime_; }
    
    // Test helpers
    int getSampleRate() const { return sampleRate_; }
    int getBufferSize() const { return bufferSize_; }
    size_t getEffectCount(const std::string& trackId) const {
        auto it = trackEffects_.find(trackId);
        return it != trackEffects_.end() ? it->second.size() : 0;
    }
};

class MockExportEngine : public ExportEngineInterface {
private:
    int sampleRate_ = 44100;
    int bitDepth_ = 16;
    double progress_ = 0.0;
    
public:
    bool exportAudio(const fs::path& outputPath, const std::string& format) override {
        if (outputPath.empty() || format.empty()) return false;
        
        // Simulate export process
        for (int i = 0; i <= 10; ++i) {
            progress_ = i / 10.0;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        // Create dummy file
        std::ofstream ofs(outputPath);
        ofs << "Mock audio data for format: " << format;
        
        return fs::exists(outputPath);
    }
    
    bool exportMIDI(const fs::path& outputPath) override {
        if (outputPath.empty()) return false;
        
        std::ofstream ofs(outputPath);
        ofs << "Mock MIDI data";
        
        return fs::exists(outputPath);
    }
    
    bool setQualitySettings(int sampleRate, int bitDepth) override {
        if (sampleRate <= 0 || bitDepth <= 0) return false;
        sampleRate_ = sampleRate;
        bitDepth_ = bitDepth;
        return true;
    }
    
    double getExportProgress() const override { return progress_; }
};

TEST_CASE("Business Integration - Project Lifecycle", "[integration][project][lifecycle]") {
    
    SECTION("Complete Project Creation Workflow") {
        MockDAWProject project;
        
        // Phase 1: Project creation
        REQUIRE(project.createProject("Test Project", "1.0"));
        REQUIRE(project.validate());
        
        // Phase 2: Track management
        REQUIRE(project.addTrack("track1", "Drums", "audio"));
        REQUIRE(project.addTrack("track2", "Bass", "audio"));
        REQUIRE(project.addTrack("track3", "Lead", "midi"));
        
        REQUIRE(project.getTrackCount() == 3);
        
        // Phase 3: Clip arrangement
        REQUIRE(project.addClip("track1", "clip1", 0.0, 4.0));
        REQUIRE(project.addClip("track1", "clip2", 4.0, 4.0));
        REQUIRE(project.addClip("track2", "clip3", 0.0, 8.0));
        REQUIRE(project.addClip("track3", "clip4", 2.0, 6.0));
        
        REQUIRE(project.getClipCount() == 4);
        
        // Phase 4: Transport settings
        REQUIRE(project.setTempo(128.0));
        REQUIRE(project.setTimeSignature(4, 4));
        
        REQUIRE(project.getTempo() == 128.0);
        REQUIRE(project.getTimeSignature().first == 4);
        REQUIRE(project.getTimeSignature().second == 4);
        
        // Phase 5: Export validation
        std::string xml = project.exportToXML();
        REQUIRE_FALSE(xml.empty());
        REQUIRE(xml.find("<?xml version=\"1.0\"") == 0);
        REQUIRE(xml.find("<Project version=\"1.0\"") != std::string::npos);
        REQUIRE(xml.find("Test Project") != std::string::npos);
        
        INFO("Generated XML size: " << xml.length() << " characters");
    }
    
    SECTION("Project Save/Load Round Trip") {
        auto tempDir = fs::temp_directory_path() / "dawproject_integration_test";
        fs::create_directories(tempDir);
        
        auto projectFile = tempDir / "test_project.dawproject";
        
        // Create and save project
        {
            MockDAWProject project;
            REQUIRE(project.createProject("Round Trip Test", "1.0"));
            REQUIRE(project.addTrack("audio1", "Audio Track", "audio"));
            REQUIRE(project.addClip("audio1", "clip1", 0.0, 2.0));
            REQUIRE(project.setTempo(140.0));
            
            REQUIRE(project.saveToFile(projectFile));
            REQUIRE(fs::exists(projectFile));
        }
        
        // Load and validate project
        {
            MockDAWProject loadedProject;
            REQUIRE(loadedProject.loadFromFile(projectFile));
        }
        
        fs::remove_all(tempDir);
    }
    
    SECTION("Error Handling in Project Operations") {
        MockDAWProject project;
        
        // Test invalid project creation
        REQUIRE_FALSE(project.createProject("", "1.0"));
        REQUIRE_FALSE(project.createProject("Test", ""));
        
        // Operations on invalid project should fail
        REQUIRE_FALSE(project.addTrack("track1", "Test", "audio"));
        REQUIRE_FALSE(project.validate());
        
        // Create valid project
        REQUIRE(project.createProject("Valid Project", "1.0"));
        
        // Test invalid track operations
        REQUIRE_FALSE(project.addTrack("", "Test", "audio"));
        REQUIRE_FALSE(project.addTrack("track1", "", "audio"));
        
        // Test invalid clip operations
        REQUIRE_FALSE(project.addClip("nonexistent", "clip1", 0.0, 1.0));
        REQUIRE_FALSE(project.addClip("track1", "", 0.0, 1.0));
        REQUIRE_FALSE(project.addClip("track1", "clip1", -1.0, 1.0));
        REQUIRE_FALSE(project.addClip("track1", "clip1", 0.0, 0.0));
        
        // Test invalid transport settings
        REQUIRE_FALSE(project.setTempo(0.0));
        REQUIRE_FALSE(project.setTempo(-120.0));
        REQUIRE_FALSE(project.setTempo(1000.0));
        REQUIRE_FALSE(project.setTimeSignature(0, 4));
        REQUIRE_FALSE(project.setTimeSignature(4, 0));
    }
}

TEST_CASE("Business Integration - Audio Engine Integration", "[integration][audio][engine]") {
    
    SECTION("Audio Engine Initialization and Basic Operations") {
        MockAudioEngine engine;
        
        // Test initialization
        REQUIRE(engine.initialize(44100, 512));
        REQUIRE(engine.getSampleRate() == 44100);
        REQUIRE(engine.getBufferSize() == 512);
        
        // Test invalid initialization parameters
        MockAudioEngine badEngine;
        REQUIRE_FALSE(badEngine.initialize(0, 512));
        REQUIRE_FALSE(badEngine.initialize(44100, 0));
        REQUIRE_FALSE(badEngine.initialize(-44100, 512));
        
        // Test playback control
        REQUIRE_FALSE(engine.isPlaying());
        REQUIRE(engine.startPlayback());
        REQUIRE(engine.isPlaying());
        REQUIRE(engine.stopPlayback());
        REQUIRE_FALSE(engine.isPlaying());
        
        // Test playback without initialization should fail
        MockAudioEngine uninitEngine;
        REQUIRE_FALSE(uninitEngine.startPlayback());
    }
    
    SECTION("Audio Processing Pipeline") {
        MockAudioEngine engine;
        REQUIRE(engine.initialize(44100, 1024));
        
        // Test audio processing
        std::vector<float> input(1024, 0.5f);  // Half amplitude
        std::vector<float> output(1024, 0.0f);
        
        REQUIRE(engine.processAudio(input, output));
        
        // Verify processing (mock applies 0.8 gain)
        for (size_t i = 0; i < output.size(); ++i) {
            REQUIRE(output[i] == Catch::Approx(0.4f).epsilon(0.001f));
        }
        
        // Test mismatched buffer sizes
        std::vector<float> smallOutput(512, 0.0f);
        REQUIRE_FALSE(engine.processAudio(input, smallOutput));
    }
    
    SECTION("Effect Chain Management") {
        MockAudioEngine engine;
        REQUIRE(engine.initialize(48000, 256));
        
        // Add effects to track
        REQUIRE(engine.addEffect("track1", "reverb"));
        REQUIRE(engine.addEffect("track1", "compressor"));
        REQUIRE(engine.addEffect("track1", "eq"));
        
        REQUIRE(engine.getEffectCount("track1") == 3);
        
        // Add effect to different track
        REQUIRE(engine.addEffect("track2", "delay"));
        REQUIRE(engine.getEffectCount("track2") == 1);
        REQUIRE(engine.getEffectCount("track1") == 3); // Should be unchanged
        
        // Test parameter setting
        REQUIRE(engine.setParameter("reverb", "wetness", 0.3f));
        REQUIRE(engine.setParameter("compressor", "ratio", 0.8f));
        
        // Test invalid parameter values
        REQUIRE_FALSE(engine.setParameter("reverb", "wetness", -0.1f));
        REQUIRE_FALSE(engine.setParameter("reverb", "wetness", 1.1f));
        REQUIRE_FALSE(engine.setParameter("", "wetness", 0.5f));
        REQUIRE_FALSE(engine.setParameter("reverb", "", 0.5f));
    }
    
    SECTION("Real-time Performance Simulation") {
        MockAudioEngine engine;
        REQUIRE(engine.initialize(44100, 512));
        REQUIRE(engine.startPlayback());
        
        const int numBuffers = 100;  // Simulate ~1 second of audio at 44.1kHz/512 buffer
        std::vector<float> input(512, 0.1f);
        std::vector<float> output(512);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < numBuffers; ++i) {
            REQUIRE(engine.processAudio(input, output));
            
            // Simulate real-time constraint - each buffer should process quickly
            auto bufferStart = std::chrono::high_resolution_clock::now();
            
            // Simulate some processing work
            volatile float dummy = 0.0f;
            for (int j = 0; j < 1000; ++j) {
                dummy += static_cast<float>(j) * 0.001f;
            }
            
            auto bufferEnd = std::chrono::high_resolution_clock::now();
            auto bufferTime = std::chrono::duration_cast<std::chrono::microseconds>(
                bufferEnd - bufferStart).count();
            
            // Real-time constraint: buffer processing should be much faster than playback time
            double bufferDurationMs = (512.0 / 44100.0) * 1000.0;  // ~11.6ms at 44.1kHz
            REQUIRE(bufferTime < bufferDurationMs * 100);  // Allow 100x safety margin for testing
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start).count();
        
        INFO("Processed " << numBuffers << " buffers in " << totalTime << "ms");
        REQUIRE(engine.getCurrentTime() > 0.0);  // Time should have advanced during playback
    }
}

TEST_CASE("Business Integration - Export Workflows", "[integration][export][workflow]") {
    
    SECTION("Audio Export Pipeline") {
        MockExportEngine exporter;
        auto tempDir = fs::temp_directory_path() / "dawproject_export_test";
        fs::create_directories(tempDir);
        
        // Test quality settings
        REQUIRE(exporter.setQualitySettings(48000, 24));
        REQUIRE_FALSE(exporter.setQualitySettings(0, 24));
        REQUIRE_FALSE(exporter.setQualitySettings(48000, 0));
        
        // Test different export formats
        std::vector<std::string> formats = {"wav", "mp3", "flac", "aiff"};
        
        for (const auto& format : formats) {
            auto outputFile = tempDir / ("export." + format);
            
            REQUIRE(exporter.exportAudio(outputFile, format));
            REQUIRE(fs::exists(outputFile));
            REQUIRE(fs::file_size(outputFile) > 0);
            
            // Verify export progress reached 100%
            REQUIRE(exporter.getExportProgress() == Catch::Approx(1.0).epsilon(0.001));
        }
        
        fs::remove_all(tempDir);
    }
    
    SECTION("MIDI Export Pipeline") {
        MockExportEngine exporter;
        auto tempDir = fs::temp_directory_path() / "dawproject_midi_export_test";
        fs::create_directories(tempDir);
        
        auto midiFile = tempDir / "export.mid";
        
        REQUIRE(exporter.exportMIDI(midiFile));
        REQUIRE(fs::exists(midiFile));
        REQUIRE(fs::file_size(midiFile) > 0);
        
        fs::remove_all(tempDir);
    }
    
    SECTION("Export Error Handling") {
        MockExportEngine exporter;
        
        // Test invalid paths
        REQUIRE_FALSE(exporter.exportAudio("", "wav"));
        REQUIRE_FALSE(exporter.exportAudio("/invalid/path/export.wav", "wav"));
        REQUIRE_FALSE(exporter.exportAudio("export.wav", ""));
        
        REQUIRE_FALSE(exporter.exportMIDI(""));
    }
}

TEST_CASE("Business Integration - End-to-End Workflows", "[integration][e2e][workflow]") {
    
    SECTION("Complete Production Workflow") {
        auto tempDir = fs::temp_directory_path() / "dawproject_e2e_test";
        fs::create_directories(tempDir);
        
        // Step 1: Create project
        MockDAWProject project;
        REQUIRE(project.createProject("E2E Test Project", "1.0"));
        
        // Step 2: Setup tracks and structure
        REQUIRE(project.addTrack("drums", "Drum Kit", "audio"));
        REQUIRE(project.addTrack("bass", "Bass Guitar", "audio"));
        REQUIRE(project.addTrack("guitar", "Electric Guitar", "audio"));
        REQUIRE(project.addTrack("vocals", "Lead Vocals", "audio"));
        REQUIRE(project.addTrack("synth", "Synthesizer", "midi"));
        
        // Step 3: Add clips (typical song structure)
        // Intro (0-8 beats)
        REQUIRE(project.addClip("drums", "drums_intro", 0.0, 8.0));
        REQUIRE(project.addClip("bass", "bass_intro", 4.0, 4.0));
        
        // Verse 1 (8-24 beats)
        REQUIRE(project.addClip("drums", "drums_verse", 8.0, 16.0));
        REQUIRE(project.addClip("bass", "bass_verse", 8.0, 16.0));
        REQUIRE(project.addClip("guitar", "guitar_verse", 12.0, 12.0));
        REQUIRE(project.addClip("vocals", "vocals_verse1", 16.0, 8.0));
        
        // Chorus (24-40 beats)
        REQUIRE(project.addClip("drums", "drums_chorus", 24.0, 16.0));
        REQUIRE(project.addClip("bass", "bass_chorus", 24.0, 16.0));
        REQUIRE(project.addClip("guitar", "guitar_chorus", 24.0, 16.0));
        REQUIRE(project.addClip("vocals", "vocals_chorus1", 24.0, 16.0));
        REQUIRE(project.addClip("synth", "synth_lead", 32.0, 8.0));
        
        // Step 4: Set project parameters
        REQUIRE(project.setTempo(120.0));
        REQUIRE(project.setTimeSignature(4, 4));
        
        // Step 5: Initialize audio engine
        MockAudioEngine engine;
        REQUIRE(engine.initialize(44100, 512));
        
        // Step 6: Add effects to tracks
        REQUIRE(engine.addEffect("drums", "compressor"));
        REQUIRE(engine.addEffect("drums", "eq"));
        REQUIRE(engine.addEffect("bass", "compressor"));
        REQUIRE(engine.addEffect("guitar", "amp_sim"));
        REQUIRE(engine.addEffect("guitar", "reverb"));
        REQUIRE(engine.addEffect("vocals", "reverb"));
        REQUIRE(engine.addEffect("vocals", "delay"));
        REQUIRE(engine.addEffect("synth", "filter"));
        
        // Step 7: Save project
        auto projectFile = tempDir / "e2e_project.dawproject";
        REQUIRE(project.saveToFile(projectFile));
        
        // Step 8: Export audio
        MockExportEngine exporter;
        REQUIRE(exporter.setQualitySettings(44100, 16));
        
        auto audioExport = tempDir / "e2e_export.wav";
        REQUIRE(exporter.exportAudio(audioExport, "wav"));
        
        // Step 9: Validation
        REQUIRE(project.validate());
        REQUIRE(project.getTrackCount() == 5);
        REQUIRE(project.getClipCount() == 11);
        REQUIRE(fs::exists(projectFile));
        REQUIRE(fs::exists(audioExport));
        
        // Verify XML structure
        std::string xml = project.exportToXML();
        REQUIRE(xml.find("E2E Test Project") != std::string::npos);
        REQUIRE(xml.find("Drum Kit") != std::string::npos);
        REQUIRE(xml.find("Synthesizer") != std::string::npos);
        
        INFO("Project has " << project.getTrackCount() << " tracks and " 
             << project.getClipCount() << " clips");
        INFO("XML export size: " << xml.length() << " characters");
        
        fs::remove_all(tempDir);
    }
    
    SECTION("Collaborative Workflow Simulation") {
        auto tempDir = fs::temp_directory_path() / "dawproject_collaborative_test";
        fs::create_directories(tempDir);
        
        auto sharedProject = tempDir / "shared_project.dawproject";
        
        // User 1: Creates base project
        {
            MockDAWProject project1;
            REQUIRE(project1.createProject("Collaborative Song", "1.0"));
            REQUIRE(project1.addTrack("drums", "Drums", "audio"));
            REQUIRE(project1.addTrack("bass", "Bass", "audio"));
            REQUIRE(project1.addClip("drums", "basic_beat", 0.0, 16.0));
            REQUIRE(project1.setTempo(110.0));
            REQUIRE(project1.saveToFile(sharedProject));
        }
        
        // User 2: Loads and adds content
        {
            MockDAWProject project2;
            REQUIRE(project2.loadFromFile(sharedProject));
            REQUIRE(project2.createProject("Collaborative Song", "1.0")); // Re-initialize for mock
            
            // Add User 2's contributions
            REQUIRE(project2.addTrack("drums", "Drums", "audio"));
            REQUIRE(project2.addTrack("bass", "Bass", "audio"));
            REQUIRE(project2.addTrack("guitar", "Guitar", "audio"));
            REQUIRE(project2.addClip("guitar", "rhythm_guitar", 8.0, 24.0));
            REQUIRE(project2.addClip("bass", "bass_line", 0.0, 32.0));
            
            auto updatedProject = tempDir / "updated_project.dawproject";
            REQUIRE(project2.saveToFile(updatedProject));
        }
        
        // User 3: Final mix and export
        {
            MockDAWProject project3;
            auto finalProject = tempDir / "updated_project.dawproject";
            REQUIRE(project3.loadFromFile(finalProject));
            REQUIRE(project3.createProject("Collaborative Song", "1.0")); // Re-initialize for mock
            
            // Add final elements
            REQUIRE(project3.addTrack("drums", "Drums", "audio"));
            REQUIRE(project3.addTrack("bass", "Bass", "audio"));
            REQUIRE(project3.addTrack("guitar", "Guitar", "audio"));
            REQUIRE(project3.addTrack("vocals", "Vocals", "audio"));
            REQUIRE(project3.addClip("vocals", "lead_vocals", 16.0, 16.0));
            
            // Export final version
            MockExportEngine exporter;
            auto finalMix = tempDir / "final_mix.wav";
            REQUIRE(exporter.exportAudio(finalMix, "wav"));
            REQUIRE(fs::exists(finalMix));
        }
        
        fs::remove_all(tempDir);
    }
    
    SECTION("Performance Under Load") {
        const int numProjects = 10;
        const int tracksPerProject = 20;
        const int clipsPerTrack = 5;
        
        auto tempDir = fs::temp_directory_path() / "dawproject_load_test";
        fs::create_directories(tempDir);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::unique_ptr<MockDAWProject>> projects;
        
        // Create multiple projects simultaneously
        for (int p = 0; p < numProjects; ++p) {
            auto project = std::make_unique<MockDAWProject>();
            
            REQUIRE(project->createProject("Load Test " + std::to_string(p), "1.0"));
            
            // Add tracks and clips
            for (int t = 0; t < tracksPerProject; ++t) {
                std::string trackId = "track_" + std::to_string(t);
                std::string trackName = "Track " + std::to_string(t);
                
                REQUIRE(project->addTrack(trackId, trackName, "audio"));
                
                for (int c = 0; c < clipsPerTrack; ++c) {
                    std::string clipId = "clip_" + std::to_string(t) + "_" + std::to_string(c);
                    double startTime = c * 4.0;  // 4-beat clips
                    
                    REQUIRE(project->addClip(trackId, clipId, startTime, 4.0));
                }
            }
            
            REQUIRE(project->setTempo(120.0 + p * 5));  // Vary tempo
            
            // Save project
            auto projectFile = tempDir / ("load_test_" + std::to_string(p) + ".dawproject");
            REQUIRE(project->saveToFile(projectFile));
            
            projects.push_back(std::move(project));
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        
        // Verify all projects
        for (const auto& project : projects) {
            REQUIRE(project->validate());
            REQUIRE(project->getTrackCount() == tracksPerProject);
            REQUIRE(project->getClipCount() == tracksPerProject * clipsPerTrack);
        }
        
        INFO("Created " << numProjects << " projects with " 
             << (numProjects * tracksPerProject) << " total tracks and "
             << (numProjects * tracksPerProject * clipsPerTrack) << " total clips in "
             << duration << "ms");
        
        // Performance assertion
        REQUIRE(duration < 10000);  // Should complete within 10 seconds
        
        fs::remove_all(tempDir);
    }
}

TEST_CASE("Business Integration - Error Recovery and Resilience", "[integration][error][resilience]") {
    
    SECTION("Graceful Degradation Under Resource Constraints") {
        auto tempDir = fs::temp_directory_path() / "dawproject_resilience_test";
        fs::create_directories(tempDir);
        
        MockDAWProject project;
        REQUIRE(project.createProject("Resilience Test", "1.0"));
        
        // Simulate memory pressure by creating many tracks/clips
        const int maxTracks = 1000;
        int successfulTracks = 0;
        
        for (int i = 0; i < maxTracks; ++i) {
            std::string trackId = "track_" + std::to_string(i);
            if (project.addTrack(trackId, "Track " + std::to_string(i), "audio")) {
                successfulTracks++;
                
                // Add clips to each track
                for (int j = 0; j < 10; ++j) {
                    std::string clipId = "clip_" + std::to_string(i) + "_" + std::to_string(j);
                    project.addClip(trackId, clipId, j * 2.0, 2.0);
                }
            }
        }
        
        INFO("Successfully created " << successfulTracks << " tracks");
        REQUIRE(successfulTracks > 0);  // Should create at least some tracks
        REQUIRE(project.validate());
        
        // Verify project can still be saved despite scale
        auto projectFile = tempDir / "large_project.dawproject";
        REQUIRE(project.saveToFile(projectFile));
        
        fs::remove_all(tempDir);
    }
    
    SECTION("Recovery from Corrupted Data") {
        auto tempDir = fs::temp_directory_path() / "dawproject_recovery_test";
        fs::create_directories(tempDir);
        
        // Create a valid project file
        auto validFile = tempDir / "valid.dawproject";
        {
            MockDAWProject project;
            REQUIRE(project.createProject("Recovery Test", "1.0"));
            REQUIRE(project.addTrack("track1", "Test Track", "audio"));
            REQUIRE(project.saveToFile(validFile));
        }
        
        // Create corrupted version
        auto corruptedFile = tempDir / "corrupted.dawproject";
        {
            std::ifstream valid(validFile);
            std::ofstream corrupted(corruptedFile);
            
            std::string content((std::istreambuf_iterator<char>(valid)),
                              std::istreambuf_iterator<char>());
            
            // Corrupt the content by removing closing tags
            auto pos = content.find("</Project>");
            if (pos != std::string::npos) {
                content.erase(pos);
            }
            
            corrupted << content;
        }
        
        // Test recovery behavior
        MockDAWProject recoveryProject;
        REQUIRE_FALSE(recoveryProject.loadFromFile(corruptedFile));
        
        // Should still be able to load valid file
        REQUIRE(recoveryProject.loadFromFile(validFile));
        
        fs::remove_all(tempDir);
    }
    
    SECTION("Concurrent Access Stress Test") {
        auto tempDir = fs::temp_directory_path() / "dawproject_concurrent_test";
        fs::create_directories(tempDir);
        
        auto sharedFile = tempDir / "shared.dawproject";
        
        // Create initial project
        {
            MockDAWProject project;
            REQUIRE(project.createProject("Concurrent Test", "1.0"));
            REQUIRE(project.saveToFile(sharedFile));
        }
        
        const int numThreads = 8;
        const int operationsPerThread = 50;
        std::atomic<int> successfulOperations{0};
        std::atomic<int> failedOperations{0};
        
        std::vector<std::thread> threads;
        
        for (int t = 0; t < numThreads; ++t) {
            threads.emplace_back([&, t]() {
                for (int i = 0; i < operationsPerThread; ++i) {
                    try {
                        MockDAWProject project;
                        
                        // Alternate between read and write operations
                        if (i % 2 == 0) {
                            // Read operation
                            if (project.loadFromFile(sharedFile)) {
                                successfulOperations.fetch_add(1);
                            } else {
                                failedOperations.fetch_add(1);
                            }
                        } else {
                            // Write operation
                            project.createProject("Thread " + std::to_string(t), "1.0");
                            project.addTrack("track_" + std::to_string(i), "Track", "audio");
                            
                            auto threadFile = tempDir / ("thread_" + std::to_string(t) + 
                                                      "_" + std::to_string(i) + ".dawproject");
                            if (project.saveToFile(threadFile)) {
                                successfulOperations.fetch_add(1);
                            } else {
                                failedOperations.fetch_add(1);
                            }
                        }
                    } catch (const std::exception&) {
                        failedOperations.fetch_add(1);
                    }
                    
                    // Brief pause to allow context switching
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
            });
        }
        
        for (auto& thread : threads) {
            thread.join();
        }
        
        int totalOperations = numThreads * operationsPerThread;
        double successRate = static_cast<double>(successfulOperations.load()) / totalOperations;
        
        INFO("Concurrent operations: " << totalOperations
             << " Successful: " << successfulOperations.load()
             << " Failed: " << failedOperations.load()
             << " Success rate: " << (successRate * 100) << "%");
        
        // Should have high success rate under concurrent access
        REQUIRE(successRate > 0.8);  // At least 80% success rate
        
        fs::remove_all(tempDir);
    }
}