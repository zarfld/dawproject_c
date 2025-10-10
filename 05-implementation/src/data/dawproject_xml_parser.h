#pragma once

#include <dawproject/data/data_access_engine.h>
#include <memory>

namespace dawproject::data {

/**
 * @brief DAWProject v1.0 XML parser interface
 * 
 * Provides specialized parsing for DAWProject standard-compliant XML files.
 * Follows the official specification from bitwig/dawproject repository.
 * 
 * This interface separates DAWProject-specific XML parsing from generic XML processing,
 * following the architecture patterns defined in .github/build-instructions/architecture-patterns.md
 */
class IDawProjectXMLParser {
public:
    virtual ~IDawProjectXMLParser() = default;

    /**
     * @brief Parse Project.xml to extract project information
     * @param doc XML document loaded from project.xml
     * @return Result containing ProjectInfo or error message
     * 
     * Expected XML structure (DAWProject v1.0 standard):
     * <Project version="1.0">
     *   <Application name="..." version="..."/>
     *   <Transport>
     *     <Tempo ... value="120.0" .../>
     *     <TimeSignature numerator="4" denominator="4" .../>
     *   </Transport>
     * </Project>
     */
    virtual Result<ProjectInfo> parseProjectInfo(const XMLDocument& doc) = 0;

    /**
     * @brief Parse track information from Project.xml Structure section
     * @param doc XML document containing DAWProject structure
     * @return Result containing vector of TrackInfo or error message
     * 
     * Expected XML structure (DAWProject v1.0 standard):
     * <Project version="1.0">
     *   <Structure>
     *     <Track contentType="notes" loaded="true" id="..." name="...">
     *       <Channel audioChannels="2" role="regular" ...>
     *         <Volume ... value="1.0" .../>
     *         <Pan ... value="0.5" .../>
     *       </Channel>
     *     </Track>
     *   </Structure>
     * </Project>
     */
    virtual Result<std::vector<TrackInfo>> parseTracks(const XMLDocument& doc) = 0;

    /**
     * @brief Parse metadata from MetaData.xml
     * @param doc XML document loaded from metadata.xml
     * @return Result containing metadata fields
     * 
     * Expected XML structure (DAWProject v1.0 standard):
     * <MetaData>
     *   <Title>Project Title</Title>
     *   <Artist>Artist Name</Artist>
     *   <Album>Album Name</Album>
     *   <Genre>Genre</Genre>
     * </MetaData>
     */
    virtual Result<ProjectMetadata> parseMetadata(const XMLDocument& doc) = 0;

    /**
     * @brief Validate that XML document conforms to DAWProject v1.0 specification
     * @param doc XML document to validate
     * @return ValidationResult with compliance status and any violations
     */
    virtual ValidationResult validateDAWProjectCompliance(const XMLDocument& doc) = 0;
};

/**
 * @brief DAWProject XML generator interface
 * 
 * Generates standard-compliant DAWProject v1.0 XML documents.
 * All output must validate against official Project.xsd and MetaData.xsd schemas.
 */
class IDawProjectXMLGenerator {
public:
    virtual ~IDawProjectXMLGenerator() = default;

    /**
     * @brief Generate Project.xml document
     * @param info Project information
     * @param tracks Track list
     * @return XMLDocument with DAWProject v1.0 compliant structure
     */
    virtual XMLDocument generateProject(const ProjectInfo& info, 
                                       const std::vector<TrackInfo>& tracks) = 0;

    /**
     * @brief Generate MetaData.xml document
     * @param info Project information containing metadata
     * @return XMLDocument with DAWProject v1.0 compliant metadata structure
     */
    virtual XMLDocument generateMetadata(const ProjectInfo& info) = 0;
};

/**
 * @brief Factory function to create DAWProject XML parser
 * @return Unique pointer to parser implementation
 */
std::unique_ptr<IDawProjectXMLParser> createDawProjectXMLParser();

/**
 * @brief Factory function to create DAWProject XML generator
 * @return Unique pointer to generator implementation
 */
std::unique_ptr<IDawProjectXMLGenerator> createDawProjectXMLGenerator();

} // namespace dawproject::data