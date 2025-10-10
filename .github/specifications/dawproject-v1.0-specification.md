# DAWProject v1.0 Official Specification Reference

**Source**: https://github.com/bitwig/dawproject  
**Last Updated**: October 2025 (GitHub analysis)  
**Status**: AUTHORITATIVE - This is the official DAWProject standard

## 🎯 **Format Overview**

**DAWProject** is a universal, DAW-agnostic exchange format for Digital Audio Workstation projects.

### **Container Format**
- **File Extension**: `.dawproject`
- **Container**: ZIP archive
- **Text Encoding**: UTF-8
- **Required Files**: 
  - `project.xml` (project structure)
  - `metadata.xml` (metadata information)

### **Key Principles**
1. **Universal Format** - Not specific to any DAW
2. **DAW-Agnostic** - Can be read/written by any compatible DAW
3. **ZIP Container** - Standard zip with XML files inside
4. **Standards-Based** - Uses XML Schema validation

## 📋 **Required XML Structure**

### **1. Root Project.xml Structure**

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Project version="1.0">
  <Application name="[DAW Name]" version="[Version]"/>
  <Transport>
    <Tempo max="666.0" min="20.0" unit="bpm" value="120.0" id="tempo1" name="Tempo"/>
    <TimeSignature denominator="4" numerator="4" id="timesig1"/>
  </Transport>
  <Structure>
    <Track contentType="notes" loaded="true" id="track1" name="Track Name" color="#ffffff">
      <Channel audioChannels="2" destination="master" role="regular" solo="false" id="channel1">
        <Devices>
          <!-- Plugin devices -->
        </Devices>
        <Mute value="false" id="mute1" name="Mute"/>
        <Pan max="1.0" min="0.0" unit="normalized" value="0.5" id="pan1" name="Pan"/>
        <Volume max="2.0" min="0.0" unit="linear" value="1.0" id="vol1" name="Volume"/>
      </Channel>
    </Track>
  </Structure>
  <Arrangement id="arrangement1">
    <Lanes timeUnit="beats" id="lanes1">
      <Lanes track="track1" id="tracklanes1">
        <Clips id="clips1">
          <Clip time="0.0" duration="4.0" playStart="0.0">
            <!-- Clip content -->
          </Clip>
        </Clips>
      </Lanes>
    </Lanes>
  </Arrangement>
  <Scenes/>
</Project>
```

### **2. MetaData.xml Structure**

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<MetaData>
  <Title>Project Title</Title>
  <Artist>Artist Name</Artist>
  <Album>Album Name</Album>
  <Genre>Genre</Genre>
  <Year>2025</Year>
  <Copyright>Copyright Notice</Copyright>
  <Comment>Project description</Comment>
</MetaData>
```

## 🏗️ **Core XML Elements**

### **Project Element (Root)**
- **Required Attributes**: `version="1.0"`
- **Required Children**: `Application`, `Structure`
- **Optional Children**: `Transport`, `Arrangement`, `Scenes`

### **Application Element**
- **Required Attributes**: `name`, `version`
- **Purpose**: Identifies the DAW that created the file

### **Transport Element**
- **Children**: `Tempo`, `TimeSignature`
- **Purpose**: Global playback parameters

### **Structure Element**
- **Children**: List of `Track` elements
- **Purpose**: Defines the track/channel hierarchy

### **Track Element**
- **Attributes**: `contentType`, `loaded`, `id`, `name`, `color`
- **Children**: `Channel`, optional nested `Track` elements
- **Content Types**: `"notes"`, `"audio"`, `"automation"`, `"tracks"`

### **Channel Element**
- **Attributes**: `audioChannels`, `destination`, `role`, `solo`, `id`
- **Children**: `Devices`, `Mute`, `Pan`, `Volume`, `Sends`
- **Roles**: `"regular"`, `"master"`, `"return"`

### **Arrangement Element**
- **Children**: `Lanes`, optional `Markers`
- **Purpose**: Timeline arrangement structure

### **Lanes Element**
- **Attributes**: `timeUnit`, `track` (optional), `id`
- **Children**: Nested `Lanes`, `Clips`, `Points`, etc.
- **Time Units**: `"beats"`, `"seconds"`

## 🎵 **Content Types and Structures**

### **Notes Content**
```xml
<Clip time="0.0" duration="4.0">
  <Notes id="notes1">
    <Note time="0.0" duration="0.25" channel="0" key="60" vel="0.8" rel="0.8"/>
  </Notes>
</Clip>
```

### **Audio Content**
```xml
<Clip time="0.0" duration="8.0">
  <Audio algorithm="stretch" channels="2" duration="8.0" sampleRate="48000" id="audio1">
    <File path="audio/sample.wav"/>
  </Audio>
</Clip>
```

### **Automation Content**
```xml
<Points target="volume" unit="linear" id="automation1">
  <Point time="0.0" value="0.8"/>
  <Point time="4.0" value="1.0"/>
</Points>
```

## 🔧 **Parameters and Values**

### **Parameter Structure**
All parameters (Volume, Pan, etc.) follow this pattern:
```xml
<Volume max="2.0" min="0.0" unit="linear" value="1.0" id="vol1" name="Volume"/>
```

### **Common Units**
- `"linear"` - Linear scale (0.0 to 1.0 or specified range)
- `"normalized"` - Normalized (0.0 to 1.0)
- `"bpm"` - Beats per minute
- `"beats"` - Musical beats
- `"seconds"` - Time in seconds

## 📁 **File References**

### **Embedded Files**
```xml
<File path="audio/sample.wav"/>  <!-- Inside ZIP container -->
```

### **External Files**
```xml
<File path="audio/sample.wav" external="true"/>  <!-- Relative to .dawproject file -->
<File path="/absolute/path/sample.wav" external="true"/>  <!-- Absolute path -->
```

## ✅ **Validation Requirements**

### **Schema Validation**
- All XML must validate against `Project.xsd` and `MetaData.xsd`
- Use official schemas from bitwig/dawproject repository

### **Required Elements**
1. `<Project version="1.0">` root element
2. `<Application>` with name and version
3. `<Structure>` with at least one track for non-empty projects

### **ID Requirements**
- All elements with `id` attributes must have unique IDs within the project
- IDs are used for referencing between elements

## 🚫 **Common Violations**

### **❌ Wrong Root Element**
```xml
<!-- WRONG -->
<project>
<DAWProject>

<!-- CORRECT -->
<Project version="1.0">
```

### **❌ Missing Required Attributes**
```xml
<!-- WRONG -->
<Project>
<Application/>

<!-- CORRECT -->
<Project version="1.0">
<Application name="DAW Name" version="1.0"/>
```

### **❌ Wrong Structure Hierarchy**
```xml
<!-- WRONG -->
<project>
  <metadata>
    <tracks>

<!-- CORRECT -->
<Project version="1.0">
  <Application.../>
  <Structure>
    <Track>
      <Channel>
```

## 🎯 **Implementation Guidelines**

### **For DAW Developers**
1. **Read specification first** before implementing
2. **Use ZIP container** with project.xml and metadata.xml
3. **Validate against XSD schemas** before saving
4. **Test with official bitwig/dawproject tools** for compatibility

### **For Library Developers**
1. **Parse ZIP container** to extract XML files
2. **Validate XML structure** against official schemas  
3. **Respect universal format** - don't add proprietary extensions
4. **Use abstractions** - don't hardcode XML parsing

## 📚 **Reference Implementation**

The **official Java reference implementation** is available at:
https://github.com/bitwig/dawproject

Key classes for C++ developers to understand:
- `Project.java` - Root project structure
- `Track.java` - Track and channel definitions  
- `Arrangement.java` - Timeline arrangement
- `DawProject.java` - Save/load operations

---

**⚠️ CRITICAL**: This is the ONLY authoritative specification. Any implementation that doesn't follow this exact structure is NOT DAWProject compliant and will break interoperability.