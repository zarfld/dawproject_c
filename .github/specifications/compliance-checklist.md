# Pre-Implementation Compliance Checklist

**Purpose**: Prevent specification violations and architectural errors before writing any code

## 📋 **Before Writing ANY Requirements**

### **Specification Validation** ✅
- [ ] Read complete DAWProject v1.0 specification in `.github/specifications/dawproject-v1.0-specification.md`
- [ ] Review official Project.xsd and MetaData.xsd schemas from bitwig/dawproject
- [ ] Understand universal format principles (DAW-agnostic, standards-based)
- [ ] Identify system boundaries (what is our responsibility vs DAW responsibility)

### **Requirements Engineering Check** ✅
- [ ] Verify requirement aligns with DAWProject universal format principle
- [ ] Confirm requirement doesn't violate format boundaries
- [ ] Check that acceptance criteria are testable with available tools
- [ ] Validate traceability references exist (StR-XXX must be real)

## 🏗️ **Before Writing ANY Code**

### **Architecture Review** ✅
- [ ] Review existing abstractions in `.github/build-instructions/architecture-patterns.md`
- [ ] Check XMLProcessor interfaces before creating XML parsing
- [ ] Verify DataAccessEngine patterns for data operations
- [ ] Confirm ZIP container handling approach

### **Standards Compliance** ✅
- [ ] Validate approach against IEEE/ISO/IEC standards in copilot instructions
- [ ] Ensure TDD methodology (Red-Green-Refactor)
- [ ] Check separation of concerns (no hardcoded business logic)
- [ ] Verify proper abstraction layers

## 🧪 **Before Writing ANY Tests**

### **Test Strategy Validation** ✅
- [ ] Reference DAWProject standard examples for test data
- [ ] Use standard-compliant XML format in test generation
- [ ] Validate test XML against official XSD schemas
- [ ] Ensure tests verify interoperability, not just internal correctness

## 🔄 **During Implementation**

### **Continuous Validation** ✅
- [ ] Run tests after each change (TDD cycle)
- [ ] Validate generated XML against schemas
- [ ] Check architectural compliance
- [ ] Maintain traceability documentation

## ❌ **Red Flags - STOP and Review**

### **Specification Violations**
- Creating custom XML format instead of DAWProject v1.0 standard
- Adding proprietary extensions not in official specification
- Implementing DAW-specific behavior instead of universal format

### **Architecture Violations**
- Hardcoding XML parsing in business logic
- Bypassing existing XMLProcessor abstractions
- Creating circular dependencies
- Mixing concerns (parsing + business logic)

### **Requirements Issues**
- Requirements that violate universal format principle
- Broken traceability chains
- Untestable acceptance criteria
- Requirements outside system boundaries

## 🎯 **Success Criteria**

### **Compliant Implementation Must**
- Generate XML that validates against official Project.xsd/MetaData.xsd
- Be compatible with official bitwig/dawproject tools
- Follow all IEEE/ISO/IEC standards requirements
- Maintain clean architectural abstractions
- Have complete requirements traceability
- Pass all automated compliance checks

---

**⚠️ MANDATORY**: If ANY checkbox fails, STOP and resolve before proceeding. These violations cause architectural debt and standards non-compliance.