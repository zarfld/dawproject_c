# External DAWProject Specification Authority

**⚠️ CRITICAL**: This document contains ONLY references to external authoritative sources. NO agent should ever modify external specifications internally.

## 🎯 **Authoritative External Sources**

### **Primary Specification Authority**
- **Repository**: https://github.com/bitwig/dawproject
- **HTML Reference**: https://htmlpreview.github.io/?https://github.com/bitwig/dawproject/blob/main/Reference.html
- **Project Schema**: https://raw.githubusercontent.com/bitwig/dawproject/main/Project.xsd
- **Metadata Schema**: https://raw.githubusercontent.com/bitwig/dawproject/main/MetaData.xsd

### **Version Control**
- **Current Version**: 1.0
- **Specification Owner**: Bitwig GmbH and PreSonus Software Ltd.
- **Last Updated**: Check GitHub repository for latest commits

## 📋 **External Reference Usage Rules**

### **✅ Allowed Operations**
- Reference external URLs in requirements
- Link to specific schema sections  
- Quote exact specification text with attribution
- Validate our implementation against external schemas

### **❌ FORBIDDEN Operations**
- Copy external schemas into our repository
- Modify or "improve" external specifications
- Create derivative versions of official schemas
- Internalize external documentation content

## 🔗 **Required External References for Requirements**

### **For XML Schema Compliance**
All requirements MUST reference:
```
XML Schema Authority: 
- Project.xsd: https://raw.githubusercontent.com/bitwig/dawproject/main/Project.xsd
- MetaData.xsd: https://raw.githubusercontent.com/bitwig/dawproject/main/MetaData.xsd
```

### **For Format Specification**
All requirements MUST reference:
```
Format Specification Authority:
- HTML Reference: https://htmlpreview.github.io/?https://github.com/bitwig/dawproject/blob/main/Reference.html
- Repository: https://github.com/bitwig/dawproject
```

### **For Implementation Validation**
All requirements MUST reference:
```
Reference Implementation:
- Java Implementation: https://github.com/bitwig/dawproject/tree/main/src/main/java/com/bitwig/dawproject
- Test Files: https://github.com/bitwig/dawproject/tree/main/test-projects
```

## 🧪 **Validation Against External Authority**

### **Schema Validation Requirements**
Every generated XML MUST:
1. Validate against https://raw.githubusercontent.com/bitwig/dawproject/main/Project.xsd
2. Validate against https://raw.githubusercontent.com/bitwig/dawproject/main/MetaData.xsd
3. Be loadable by official Java reference implementation

### **Interoperability Requirements**
Every implementation MUST:
1. Generate files readable by Bitwig Studio
2. Generate files readable by PreSonus Studio One
3. Read files created by reference implementation

## 📚 **External Test Data Sources**

### **Official Test Projects**
Use ONLY external test data from:
```
Test Data Authority:
- Official Examples: https://github.com/bitwig/dawproject/tree/main/test-projects
- Reference Files: https://github.com/bitwig/dawproject/tree/main/examples
```

### **Validation Test Strategy**
1. Download test files from external authority
2. Validate our parser against external test files
3. Generate test files and validate against external schemas
4. NEVER create custom test data that might not match standard

## ⚖️ **Legal and Standards Compliance**

### **Intellectual Property**
- DAWProject format is owned by Bitwig GmbH and PreSonus Software Ltd.
- Schemas and specifications remain external property
- Our implementation follows but does not copy external IP

### **Standards Authority**
- External repository is SOLE authority for format definition
- Any specification conflicts defer to external authority
- Version updates come ONLY from external source

---

**🛡️ ENFORCEMENT RULE**: Any agent attempting to modify external specifications or internalize external schemas violates this architecture and must be stopped immediately.