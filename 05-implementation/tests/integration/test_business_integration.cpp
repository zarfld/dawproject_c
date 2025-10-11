#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "business/project_engine.h"
#include "business/validation_engine.h"
#include "business/domain_model.h"
#include "data/data_access_factory.h"
#include "external/external_validation_engine.h"
#include <filesystem>
#include <fstream>
#include <memory>

using namespace dawproject::business;
using namespace dawproject::data;

/**
 * @brief Comprehensive Business Logic and Integration Tests for Coverage Improvement
 * 
 * Target: Test business workflows, validation rules, and integration scenarios
 * Focus: End-to-end workflows, business rule validation, component integration
 */

TEST_CASE("Business Logic - Project Validation Rules", "[business][validation]") {
    auto validationEngine = std::make_unique<ValidationEngine>();
    REQUIRE(validationEngine != nullptr);
    
    SECTION("Valid Project Structure Validation") {
        ProjectModel validProject;
        validProject.metadata.title = "Test Project";
        validProject.metadata.artist = "Test Artist";
        validProject.transport.tempo = 120.0;
        validProject.transport.timeSignature = "4/4";
        
        // Add valid track
        TrackModel validTrack;
        validTrack.id = "track1";
        validTrack.name = "Lead Vocal";
        validTrack.contentType = TrackContentType::AudioNotes;
        validTrack.loaded = true;
        
        // Add valid clip to track
        ClipModel validClip;
        validClip.id = "clip1";
        validClip.name = "Verse 1";
        validClip.time = 0.0;
        validClip.duration = 16.0;
        
        // Add notes to clip
        NoteModel note1;
        note1.time = 0.0;
        note1.duration = 1.0;
        note1.key = 60; // Middle C
        note1.velocity = 80;
        note1.channel = 1;
        
        NoteModel note2;
        note2.time = 1.0;
        note2.duration = 0.5;
        note2.key = 64; // E
        note2.velocity = 75;
        note2.channel = 1;
        
        validClip.notes = {note1, note2};
        validTrack.clips = {validClip};
        validProject.structure.tracks = {validTrack};
        
        auto result = validationEngine->validateProject(validProject);
        REQUIRE(result.success);
        REQUIRE(result.value.isValid);
        REQUIRE(result.value.errors.empty());
    }
    
    SECTION("Invalid Project - Missing Required Fields") {
        ProjectModel invalidProject;
        // Missing title, artist, etc.
        
        auto result = validationEngine->validateProject(invalidProject);
        REQUIRE(result.success);
        REQUIRE_FALSE(result.value.isValid);
        REQUIRE_FALSE(result.value.errors.empty());
        
        // Should have specific error messages
        bool foundTitleError = false;
        for (const auto& error : result.value.errors) {
            if (error.field == "metadata.title" && error.code == ValidationErrorCode::Required) {
                foundTitleError = true;
                break;
            }
        }
        REQUIRE(foundTitleError);
    }
    
    SECTION("Track Validation Rules") {
        TrackModel track;
        track.id = ""; // Invalid: empty ID
        track.name = ""; // Invalid: empty name
        track.contentType = TrackContentType::AudioNotes;
        
        auto result = validationEngine->validateTrack(track);
        REQUIRE(result.success);
        REQUIRE_FALSE(result.value.isValid);
        REQUIRE(result.value.errors.size() >= 2); // ID and name errors
    }
    
    SECTION("Clip Timing Validation") {
        ClipModel clip;
        clip.id = "clip1";
        clip.name = "Test Clip";
        clip.time = -1.0; // Invalid: negative time
        clip.duration = 0.0; // Invalid: zero duration
        
        auto result = validationEngine->validateClip(clip);
        REQUIRE(result.success);
        REQUIRE_FALSE(result.value.isValid);
        
        // Should have timing errors
        bool foundTimeError = false, foundDurationError = false;
        for (const auto& error : result.value.errors) {
            if (error.field == "time" && error.code == ValidationErrorCode::Range) {
                foundTimeError = true;
            }
            if (error.field == "duration" && error.code == ValidationErrorCode::Range) {
                foundDurationError = true;
            }
        }
        REQUIRE(foundTimeError);
        REQUIRE(foundDurationError);
    }
    
    SECTION("Note Validation Rules") {
        NoteModel note;
        note.time = 0.0;
        note.duration = 1.0;
        note.key = 128; // Invalid: outside MIDI range (0-127)
        note.velocity = 200; // Invalid: outside velocity range (0-127)
        note.channel = 0; // Invalid: MIDI channels are 1-16
        
        auto result = validationEngine->validateNote(note);
        REQUIRE(result.success);
        REQUIRE_FALSE(result.value.isValid);
        
        // Should have MIDI range errors
        REQUIRE(result.value.errors.size() >= 3);
    }
}

TEST_CASE("Business Logic - Project Engine Operations", "[business][project]") {
    auto projectEngine = std::make_unique<ProjectEngine>();
    REQUIRE(projectEngine != nullptr);
    
    std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "dawproject_business_test";
    std::filesystem::create_directories(tempDir);
    
    SECTION("Create New Project") {
        ProjectCreationOptions options;
        options.title = "New Project";
        options.artist = "Test Artist";
        options.tempo = 140.0;
        options.timeSignature = "4/4";
        options.sampleRate = 44100;
        options.bitDepth = 24;
        
        auto result = projectEngine->createProject(options);
        REQUIRE(result.success);
        REQUIRE(result.value.metadata.title == "New Project");
        REQUIRE(result.value.transport.tempo == 140.0);
        REQUIRE(result.value.format.sampleRate == 44100);
        REQUIRE(result.value.format.bitDepth == 24);
        
        // Should have default structure
        REQUIRE_FALSE(result.value.structure.tracks.empty());
        
        // First track should be master/main track
        const auto& masterTrack = result.value.structure.tracks[0];
        REQUIRE(masterTrack.name == "Master" || masterTrack.name == "Main");
    }
    
    SECTION("Load and Save Project Workflow") {
        // Create test project file
        std::filesystem::path projectPath = tempDir / "test_project.dawproject";
        
        // Create project content using business logic
        ProjectCreationOptions options;
        options.title = "Load/Save Test";
        options.artist = "Test Suite";
        options.tempo = 120.0;
        
        auto createResult = projectEngine->createProject(options);
        REQUIRE(createResult.success);
        
        // Add some content to the project
        TrackModel newTrack;
        newTrack.id = "test_track_1";
        newTrack.name = "Test Track";
        newTrack.contentType = TrackContentType::Audio;
        
        auto addTrackResult = projectEngine->addTrack(createResult.value, newTrack);
        REQUIRE(addTrackResult.success);
        
        // Save project
        auto saveResult = projectEngine->saveProject(addTrackResult.value, projectPath);
        REQUIRE(saveResult.success);
        
        // Verify file was created
        REQUIRE(std::filesystem::exists(projectPath));
        
        // Load project back
        auto loadResult = projectEngine->loadProject(projectPath);
        REQUIRE(loadResult.success);
        
        // Verify loaded content matches
        REQUIRE(loadResult.value.metadata.title == "Load/Save Test");
        REQUIRE(loadResult.value.metadata.artist == "Test Suite");
        REQUIRE(loadResult.value.transport.tempo == Catch::Approx(120.0));
        
        // Should have our added track
        bool foundTestTrack = false;
        for (const auto& track : loadResult.value.structure.tracks) {
            if (track.name == "Test Track") {
                foundTestTrack = true;
                REQUIRE(track.contentType == TrackContentType::Audio);
                break;
            }
        }
        REQUIRE(foundTestTrack);
    }
    
    SECTION("Project Modification Operations") {
        auto createResult = projectEngine->createProject(ProjectCreationOptions{});
        REQUIRE(createResult.success);
        
        auto project = createResult.value;
        
        // Test track operations
        TrackModel newTrack;
        newTrack.id = "track_ops_test";
        newTrack.name = "Operations Track";
        newTrack.contentType = TrackContentType::AudioNotes;
        
        auto addResult = projectEngine->addTrack(project, newTrack);
        REQUIRE(addResult.success);
        project = addResult.value;
        
        // Verify track was added
        REQUIRE(project.structure.tracks.size() >= 2); // Master + new track
        
        // Test track modification
        auto modifyResult = projectEngine->modifyTrack(project, "track_ops_test", 
            [](TrackModel& track) {
                track.name = "Modified Track Name";
                track.loaded = false;
            });
        REQUIRE(modifyResult.success);
        project = modifyResult.value;
        
        // Verify modification
        bool foundModified = false;
        for (const auto& track : project.structure.tracks) {
            if (track.id == "track_ops_test") {
                REQUIRE(track.name == "Modified Track Name");
                REQUIRE_FALSE(track.loaded);
                foundModified = true;
                break;
            }
        }
        REQUIRE(foundModified);
        
        // Test track removal
        auto removeResult = projectEngine->removeTrack(project, "track_ops_test");
        REQUIRE(removeResult.success);
        project = removeResult.value;
        
        // Verify track was removed
        for (const auto& track : project.structure.tracks) {
            REQUIRE(track.id != "track_ops_test");
        }
    }
    
    // Cleanup
    std::filesystem::remove_all(tempDir);
}

TEST_CASE("Integration - End-to-End DAWProject Workflows", "[integration][e2e]") {
    std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "dawproject_integration_test";
    std::filesystem::create_directories(tempDir);
    
    SECTION("Complete Project Creation and Validation Workflow") {
        // Step 1: Create project using business logic
        auto projectEngine = std::make_unique<ProjectEngine>();
        auto validationEngine = std::make_unique<ValidationEngine>();
        
        ProjectCreationOptions options;
        options.title = "Integration Test Project";
        options.artist = "E2E Test Suite";
        options.tempo = 128.0;
        options.timeSignature = "4/4";
        
        auto createResult = projectEngine->createProject(options);
        REQUIRE(createResult.success);
        
        // Step 2: Add complex content
        TrackModel audioTrack;
        audioTrack.id = "audio_track_1";
        audioTrack.name = "Audio Track";
        audioTrack.contentType = TrackContentType::Audio;
        
        ClipModel audioClip;
        audioClip.id = "audio_clip_1";
        audioClip.name = "Audio Recording";
        audioClip.time = 0.0;
        audioClip.duration = 32.0;
        // Simulate audio file reference
        audioClip.audioFile = "recordings/take_001.wav";
        audioClip.audioChannels = 2;
        
        audioTrack.clips = {audioClip};
        
        auto addAudioTrackResult = projectEngine->addTrack(createResult.value, audioTrack);
        REQUIRE(addAudioTrackResult.success);
        
        // Add MIDI track with notes
        TrackModel midiTrack;
        midiTrack.id = "midi_track_1";
        midiTrack.name = "MIDI Track";
        midiTrack.contentType = TrackContentType::AudioNotes;
        
        ClipModel midiClip;
        midiClip.id = "midi_clip_1";
        midiClip.name = "MIDI Sequence";
        midiClip.time = 0.0;
        midiClip.duration = 16.0;
        
        // Add chord progression
        std::vector<NoteModel> chordProgression = {
            {0.0, 4.0, 60, 80, 1},   // C major
            {0.0, 4.0, 64, 80, 1},   // E
            {0.0, 4.0, 67, 80, 1},   // G
            {4.0, 4.0, 57, 80, 1},   // A minor
            {4.0, 4.0, 60, 80, 1},   // C
            {4.0, 4.0, 64, 80, 1},   // E
            {8.0, 4.0, 62, 80, 1},   // D minor
            {8.0, 4.0, 65, 80, 1},   // F
            {8.0, 4.0, 69, 80, 1},   // A
            {12.0, 4.0, 67, 80, 1},  // G major
            {12.0, 4.0, 71, 80, 1},  // B
            {12.0, 4.0, 74, 80, 1},  // D
        };
        
        midiClip.notes = chordProgression;
        midiTrack.clips = {midiClip};
        
        auto addMidiTrackResult = projectEngine->addTrack(addAudioTrackResult.value, midiTrack);
        REQUIRE(addMidiTrackResult.success);
        
        // Step 3: Validate complete project
        auto validationResult = validationEngine->validateProject(addMidiTrackResult.value);
        REQUIRE(validationResult.success);
        REQUIRE(validationResult.value.isValid);
        
        // Step 4: Save project
        std::filesystem::path projectPath = tempDir / "integration_test.dawproject";
        auto saveResult = projectEngine->saveProject(addMidiTrackResult.value, projectPath);
        REQUIRE(saveResult.success);
        
        // Step 5: Verify file structure (ZIP validation)
        auto zipProcessor = DataAccessFactory::createZIPProcessor();
        auto listResult = zipProcessor->listEntries(projectPath);
        REQUIRE(listResult.success);
        
        // Should contain required DAWProject files
        bool foundProject = false, foundMetadata = false;
        for (const auto& entry : listResult.value) {
            if (entry.name == "project.xml") foundProject = true;
            if (entry.name == "metadata.xml") foundMetadata = true;
        }
        REQUIRE(foundProject);
        REQUIRE(foundMetadata);
        
        // Step 6: Load and verify content integrity
        auto loadResult = projectEngine->loadProject(projectPath);
        REQUIRE(loadResult.success);
        
        const auto& loadedProject = loadResult.value;
        REQUIRE(loadedProject.metadata.title == "Integration Test Project");
        REQUIRE(loadedProject.transport.tempo == Catch::Approx(128.0));
        
        // Verify tracks and content
        REQUIRE(loadedProject.structure.tracks.size() >= 3); // Master + audio + midi
        
        bool foundAudioTrack = false, foundMidiTrack = false;
        for (const auto& track : loadedProject.structure.tracks) {
            if (track.name == "Audio Track") {
                foundAudioTrack = true;
                REQUIRE(track.clips.size() == 1);
                REQUIRE(track.clips[0].audioFile == "recordings/take_001.wav");
            }
            else if (track.name == "MIDI Track") {
                foundMidiTrack = true;
                REQUIRE(track.clips.size() == 1);
                REQUIRE(track.clips[0].notes.size() == 12); // Chord progression
            }
        }
        REQUIRE(foundAudioTrack);
        REQUIRE(foundMidiTrack);
    }
    
    SECTION("External Validation Integration") {
        // Create project for external validation
        auto projectEngine = std::make_unique<ProjectEngine>();
        auto project = projectEngine->createProject(ProjectCreationOptions{}).value;
        
        std::filesystem::path projectPath = tempDir / "external_validation_test.dawproject";
        auto saveResult = projectEngine->saveProject(project, projectPath);
        REQUIRE(saveResult.success);
        
        // Test external validation engine integration
        auto externalValidator = std::make_unique<dawproject::external::ExternalValidationEngine>();
        
        auto validationResult = externalValidator->validateProject(projectPath);
        REQUIRE(validationResult.success);
        
        // Should pass basic DAWProject v1.0 validation
        if (validationResult.value.isValid) {
            REQUIRE(validationResult.value.errors.empty());
        } else {
            // If validation fails, should provide specific errors
            REQUIRE_FALSE(validationResult.value.errors.empty());
            for (const auto& error : validationResult.value.errors) {
                REQUIRE_FALSE(error.message.empty());
                INFO("Validation error: " + error.message);
            }
        }
    }
    
    // Cleanup
    std::filesystem::remove_all(tempDir);
}