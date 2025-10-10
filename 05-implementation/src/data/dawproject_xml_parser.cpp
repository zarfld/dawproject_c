#include "dawproject_xml_parser.h"
#include <chrono>

namespace dawproject::data {

/**
 * @brief DAWProject v1.0 XML parser implementation
 * 
 * Implements parsing according to official DAWProject specification.
 * References: .github/specifications/dawproject-v1.0-specification.md
 */
class DawProjectXMLParserImpl : public IDawProjectXMLParser {
public:
    Result<ProjectInfo> parseProjectInfo(const XMLDocument& doc) override {
        try {
            // Validate root element is <Project version="1.0">
            if (doc.rootElementName != "Project") {
                return Result<ProjectInfo>::makeError(
                    "Invalid DAWProject: Root element must be <Project>, found <" + 
                    doc.rootElementName + ">");
            }

            // Validate version attribute
            std::string version = doc.getAttribute("version");
            if (version != "1.0") {
                return Result<ProjectInfo>::makeError(
                    "Unsupported DAWProject version: " + version + " (expected 1.0)");
            }

            ProjectInfo info;
            
            // Parse Application element (required)
            const auto* appElement = doc.findChild("Application");
            if (!appElement) {
                return Result<ProjectInfo>::makeError(
                    "Missing required <Application> element in DAWProject");
            }
            
            info.title = appElement->getAttribute("name", "Untitled Project");
            info.artist = appElement->getAttribute("vendor", "Unknown Artist");

            // Parse Transport element (optional)
            const auto* transportElement = doc.findChild("Transport");
            if (transportElement) {
                // Find Tempo child
                const auto* tempoElement = transportElement->findChild("Tempo");
                if (tempoElement) {
                    std::string tempoStr = tempoElement->getAttribute("value", "120.0");
                    try {
                        info.tempo = std::stod(tempoStr);
                    } catch (const std::exception&) {
                        info.tempo = 120.0;
                    }
                }

                // Find TimeSignature child
                const auto* timeSigElement = transportElement->findChild("TimeSignature");
                if (timeSigElement) {
                    std::string numerator = timeSigElement->getAttribute("numerator", "4");
                    std::string denominator = timeSigElement->getAttribute("denominator", "4");
                    info.timeSignature = numerator + "/" + denominator;
                } else {
                    info.timeSignature = "4/4";
                }
            } else {
                // Default transport settings
                info.tempo = 120.0;
                info.timeSignature = "4/4";
            }

            // Set timestamps (XML doesn't contain creation times)
            auto now = std::chrono::system_clock::now();
            info.created = now;
            info.modified = now;

            return Result<ProjectInfo>::makeSuccess(std::move(info));
        }
        catch (const std::exception& e) {
            return Result<ProjectInfo>::makeError(
                "Exception parsing DAWProject info: " + std::string(e.what()));
        }
    }

    Result<std::vector<TrackInfo>> parseTracks(const XMLDocument& doc) override {
        try {
            std::vector<TrackInfo> tracks;

            // Find Structure element
            const auto* structureElement = doc.findChild("Structure");
            if (!structureElement) {
                // No tracks is valid (empty project)
                return Result<std::vector<TrackInfo>>::makeSuccess(std::move(tracks));
            }

            // Parse all Track elements
            auto trackElements = structureElement->findChildren("Track");
            tracks.reserve(trackElements.size());

            for (const auto* trackElement : trackElements) {
                TrackInfo track;
                
                // Parse track attributes
                track.id = trackElement->getAttribute("id", "");
                track.name = trackElement->getAttribute("name", "Untitled Track");
                track.color = trackElement->getAttribute("color", "#ffffff");
                
                // Parse contentType attribute
                std::string contentType = trackElement->getAttribute("contentType", "notes");
                // TODO: Convert contentType string to enum when we define it

                // Parse Channel element for volume/pan
                const auto* channelElement = trackElement->findChild("Channel");
                if (channelElement) {
                    // Parse Volume parameter
                    const auto* volumeElement = channelElement->findChild("Volume");
                    if (volumeElement) {
                        std::string volumeStr = volumeElement->getAttribute("value", "1.0");
                        try {
                            track.volume = std::stod(volumeStr);
                        } catch (const std::exception&) {
                            track.volume = 1.0;
                        }
                    } else {
                        track.volume = 1.0;
                    }

                    // Parse Pan parameter  
                    const auto* panElement = channelElement->findChild("Pan");
                    if (panElement) {
                        std::string panStr = panElement->getAttribute("value", "0.5");
                        try {
                            track.pan = std::stod(panStr);
                        } catch (const std::exception&) {
                            track.pan = 0.5;
                        }
                    } else {
                        track.pan = 0.5;
                    }
                } else {
                    // Default channel settings
                    track.volume = 1.0;
                    track.pan = 0.5;
                }

                tracks.push_back(std::move(track));
            }

            return Result<std::vector<TrackInfo>>::makeSuccess(std::move(tracks));
        }
        catch (const std::exception& e) {
            return Result<std::vector<TrackInfo>>::makeError(
                "Exception parsing DAWProject tracks: " + std::string(e.what()));
        }
    }

    Result<ProjectMetadata> parseMetadata(const XMLDocument& doc) override {
        try {
            // Validate root element is <MetaData>
            if (doc.rootElementName != "MetaData") {
                return Result<ProjectMetadata>::makeError(
                    "Invalid metadata: Root element must be <MetaData>, found <" + 
                    doc.rootElementName + ">");
            }

            ProjectMetadata metadata;

            // Parse metadata fields
            const auto* titleElement = doc.findChild("Title");
            if (titleElement) {
                metadata.title = titleElement->content;
            }

            const auto* artistElement = doc.findChild("Artist"); 
            if (artistElement) {
                metadata.artist = artistElement->content;
            }

            const auto* albumElement = doc.findChild("Album");
            if (albumElement) {
                metadata.album = albumElement->content;
            }

            const auto* genreElement = doc.findChild("Genre");
            if (genreElement) {
                metadata.genre = genreElement->content;
            }

            const auto* yearElement = doc.findChild("Year");
            if (yearElement) {
                metadata.year = yearElement->content;
            }

            const auto* copyrightElement = doc.findChild("Copyright");
            if (copyrightElement) {
                metadata.copyright = copyrightElement->content;
            }

            const auto* commentElement = doc.findChild("Comment");
            if (commentElement) {
                metadata.comment = commentElement->content;
            }

            return Result<ProjectMetadata>::makeSuccess(std::move(metadata));
        }
        catch (const std::exception& e) {
            return Result<ProjectMetadata>::makeError(
                "Exception parsing metadata: " + std::string(e.what()));
        }
    }

    ValidationResult validateDAWProjectCompliance(const XMLDocument& doc) override {
        ValidationResult result;
        result.isValid = true;

        // Check root element
        if (doc.rootElementName != "Project") {
            result.addError("Root element must be <Project>, found <" + doc.rootElementName + ">");
        }

        // Check version attribute
        std::string version = doc.getAttribute("version");
        if (version.empty()) {
            result.addError("Missing required 'version' attribute on <Project> element");
        } else if (version != "1.0") {
            result.addError("Invalid version '" + version + "', expected '1.0'");
        }

        // Check required Application element
        const auto* appElement = doc.findChild("Application");
        if (!appElement) {
            result.addError("Missing required <Application> element");
        } else {
            // Validate Application attributes
            if (appElement->getAttribute("name").empty()) {
                result.addError("Missing required 'name' attribute on <Application> element");
            }
            if (appElement->getAttribute("version").empty()) {
                result.addError("Missing required 'version' attribute on <Application> element");
            }
        }

        // Validate Structure if present
        const auto* structureElement = doc.findChild("Structure");
        if (structureElement) {
            auto trackElements = structureElement->findChildren("Track");
            for (const auto* trackElement : trackElements) {
                if (trackElement->getAttribute("id").empty()) {
                    result.addWarning("Track missing 'id' attribute - recommended for DAWProject compliance");
                }
            }
        }

        return result;
    }
};

/**
 * @brief DAWProject XML generator implementation
 */
class DawProjectXMLGeneratorImpl : public IDawProjectXMLGenerator {
public:
    XMLDocument generateProject(const ProjectInfo& info, 
                               const std::vector<TrackInfo>& tracks) override {
        XMLDocument doc;
        doc.rootElementName = "Project";
        doc.setAttribute("version", "1.0");

        // Create Application element
        XMLDocument appElement;
        appElement.rootElementName = "Application";
        appElement.setAttribute("name", "DAWProject C++");
        appElement.setAttribute("version", "1.0");
        doc.children.push_back(std::move(appElement));

        // Create Transport element
        XMLDocument transportElement;
        transportElement.rootElementName = "Transport";

        // Add Tempo child
        XMLDocument tempoElement;
        tempoElement.rootElementName = "Tempo";
        tempoElement.setAttribute("max", "666.0");
        tempoElement.setAttribute("min", "20.0");
        tempoElement.setAttribute("unit", "bpm");
        tempoElement.setAttribute("value", std::to_string(info.tempo));
        tempoElement.setAttribute("id", "tempo1");
        tempoElement.setAttribute("name", "Tempo");
        transportElement.children.push_back(std::move(tempoElement));

        // Add TimeSignature child
        XMLDocument timeSigElement;
        timeSigElement.rootElementName = "TimeSignature";
        
        // Parse time signature (e.g., "4/4")
        size_t slashPos = info.timeSignature.find('/');
        if (slashPos != std::string::npos) {
            std::string numerator = info.timeSignature.substr(0, slashPos);
            std::string denominator = info.timeSignature.substr(slashPos + 1);
            timeSigElement.setAttribute("numerator", numerator);
            timeSigElement.setAttribute("denominator", denominator);
        } else {
            timeSigElement.setAttribute("numerator", "4");
            timeSigElement.setAttribute("denominator", "4");
        }
        timeSigElement.setAttribute("id", "timesig1");
        transportElement.children.push_back(std::move(timeSigElement));

        doc.children.push_back(std::move(transportElement));

        // Create Structure element if we have tracks
        if (!tracks.empty()) {
            XMLDocument structureElement;
            structureElement.rootElementName = "Structure";

            for (const auto& track : tracks) {
                XMLDocument trackElement;
                trackElement.rootElementName = "Track";
                trackElement.setAttribute("contentType", "notes"); // Default to notes
                trackElement.setAttribute("loaded", "true");
                trackElement.setAttribute("id", track.id);
                trackElement.setAttribute("name", track.name);
                trackElement.setAttribute("color", track.color);

                // Add Channel child
                XMLDocument channelElement;
                channelElement.rootElementName = "Channel";
                channelElement.setAttribute("audioChannels", "2");
                channelElement.setAttribute("role", "regular");
                channelElement.setAttribute("solo", "false");
                channelElement.setAttribute("id", track.id + "_channel");

                // Add Volume parameter
                XMLDocument volumeElement;
                volumeElement.rootElementName = "Volume";
                volumeElement.setAttribute("max", "2.0");
                volumeElement.setAttribute("min", "0.0");
                volumeElement.setAttribute("unit", "linear");
                volumeElement.setAttribute("value", std::to_string(track.volume));
                volumeElement.setAttribute("id", track.id + "_vol");
                volumeElement.setAttribute("name", "Volume");
                channelElement.children.push_back(std::move(volumeElement));

                // Add Pan parameter
                XMLDocument panElement;
                panElement.rootElementName = "Pan";
                panElement.setAttribute("max", "1.0");
                panElement.setAttribute("min", "0.0");
                panElement.setAttribute("unit", "normalized");
                panElement.setAttribute("value", std::to_string(track.pan));
                panElement.setAttribute("id", track.id + "_pan");
                panElement.setAttribute("name", "Pan");
                channelElement.children.push_back(std::move(panElement));

                trackElement.children.push_back(std::move(channelElement));
                structureElement.children.push_back(std::move(trackElement));
            }

            doc.children.push_back(std::move(structureElement));
        }

        // Add empty Arrangement and Scenes elements
        XMLDocument arrangementElement;
        arrangementElement.rootElementName = "Arrangement";
        arrangementElement.setAttribute("id", "arrangement1");
        doc.children.push_back(std::move(arrangementElement));

        XMLDocument scenesElement;
        scenesElement.rootElementName = "Scenes";
        doc.children.push_back(std::move(scenesElement));

        return doc;
    }

    XMLDocument generateMetadata(const ProjectInfo& info) override {
        XMLDocument doc;
        doc.rootElementName = "MetaData";

        // Add Title element
        if (!info.title.empty()) {
            XMLDocument titleElement;
            titleElement.rootElementName = "Title";
            titleElement.content = info.title;
            doc.children.push_back(std::move(titleElement));
        }

        // Add Artist element
        if (!info.artist.empty()) {
            XMLDocument artistElement;
            artistElement.rootElementName = "Artist";
            artistElement.content = info.artist;
            doc.children.push_back(std::move(artistElement));
        }

        // Add other metadata fields as needed
        // TODO: Add Album, Genre, Year, Copyright, Comment when available in ProjectInfo

        return doc;
    }
};

// Factory implementations
std::unique_ptr<IDawProjectXMLParser> createDawProjectXMLParser() {
    return std::make_unique<DawProjectXMLParserImpl>();
}

std::unique_ptr<IDawProjectXMLGenerator> createDawProjectXMLGenerator() {
    return std::make_unique<DawProjectXMLGeneratorImpl>();
}

} // namespace dawproject::data