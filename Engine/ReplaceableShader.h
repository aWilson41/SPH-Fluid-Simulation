#pragma once
#include <list>
#include <map>
#include <vector>
#include <string>

// Each line in a shader has "dependencies". Things the line requires to be executed.
// In this way it's like an AST of operations. But I cheat and use whole strings instead to
// make things easier. This way lines can be reconfigured/swapped on the fly. As well as
// produce a variation of shaders.

enum NodeType
{
	NONE = 0, // Invalid
	INATTRIB = 1,
	OUTATTRIB = 2,
	STRUCT = 3,
	UNIFORM = 4,
	FUNCTION = 5,
	OPERATION = 6
};

// One graph of nodes can represent 1-N shaders. This is also beneficial because nodes are toggleable.
class ASTNode
{
public:
	ASTNode(NodeType type, std::string str)
	{
		ASTNode::type = type;
		ASTNode::str = str;
	}

	virtual std::string getString() { return str; };

	void addRequiredNode(ASTNode* node) { requiredNodes.push_back(node); }

public:
	std::string str = "";
	bool active = false;
	NodeType type = NONE;

	std::vector<ASTNode*> requiredNodes; // Needs these nodes before it to execute
};

class ASTShaderGraph
{
public:
	// Gets all possible shaders
	std::vector<std::string> getShaderSet();

	void SetNumberInputPorts(unsigned int n)
	{
		inputNodes.resize(n);
	}
	void AddInputPort(ASTNode* inputNode)
	{
		inputNodes.push_back(inputNode);
	}
	void AddOutputPort(ASTNode* outputNode)
	{
		outputNodes.push_back(outputNode);
	}

protected:
	std::vector<ASTNode*> inputNodes;
	std::vector<ASTNode*> outputNodes;
};

// Basic vertex + fragment shader graph (assumes output of vec4 color)
class ShaderProgramGraph : public ASTShaderGraph
{
public:
	ShaderProgramGraph()
	{
		ASTNode* outputNode = new ASTNode(NONE, "root");
		AddOutputPort(outputNode);
	}
};

class PolyDataFragmentShaderGraph : public ShaderProgramGraph
{
public:
	PolyDataFragmentShaderGraph()
	{
		// Vertex Shader Graph

		// VS Uniforms
		ASTNode* mvpMatrix_Uniform_VSNode = new ASTNode(UNIFORM, "uniform mat4 mvpMatrix;");

		// VS Input Attributes
		ASTNode* pos_InAttrib_VSNode = new ASTNode(INATTRIB, "layout(location = 0) in vec3 inVPos;");
		ASTNode* normal_InAttrib_VSNode = new ASTNode(INATTRIB, "layout(location = 1) in vec3 inVNormal;");
		ASTNode* texCoord_InAttrib_VSNode = new ASTNode(INATTRIB, "layout(location = 2) in vec2 inVTexCoord;");
		ASTNode* color_InAttrib_VSNode = new ASTNode(INATTRIB, "layout(location = 3) in vec3 inVColor;");

		// VS Output Attributes
		ASTNode* normal_OutAttrib_VSNode = new ASTNode(OUTATTRIB, "smooth out vec3 inFNormal;");
		ASTNode* texCoord_OutAttrib_VSNode = new ASTNode(OUTATTRIB, "smooth out vec2 inFTexCoord;");
		ASTNode* color_OutAttrib_VSNode = new ASTNode(OUTATTRIB, "smooth out vec3 inFColor;");

		// VS Operations
		ASTNode* posToWorld_Operation_VSNode = new ASTNode(OPERATION, "gl_Position = mvpMatrix * vec4(inVPos, 1.0);");
		posToWorld_Operation_VSNode->addRequiredNode(mvpMatrix_Uniform_VSNode);
		posToWorld_Operation_VSNode->addRequiredNode(pos_InAttrib_VSNode);
		ASTNode* forwardNormal_Operation_VSNode = new ASTNode(OPERATION, "inFNormal = inVNormal;");
		forwardNormal_Operation_VSNode->addRequiredNode(normal_InAttrib_VSNode);
		forwardNormal_Operation_VSNode->addRequiredNode(normal_OutAttrib_VSNode);
		ASTNode* forwardTexCoord_Operation_VSNode = new ASTNode(OPERATION, "inFTexCoord = inVTexCoord;");
		forwardTexCoord_Operation_VSNode->addRequiredNode(texCoord_InAttrib_VSNode);
		forwardTexCoord_Operation_VSNode->addRequiredNode(texCoord_OutAttrib_VSNode);
		ASTNode* forwardColor_Operation_VSNode = new ASTNode(OPERATION, "inFColor = inVColor;");
		forwardColor_Operation_VSNode->addRequiredNode(color_InAttrib_VSNode);
		forwardColor_Operation_VSNode->addRequiredNode(color_OutAttrib_VSNode);


		// Fragment Shader Graph

		// FS Structs
		ASTNode* material_Struct_FSNode = new ASTNode(STRUCT,
			"struct Material {\n"
			"	vec3 diffuseColor;\n"
			"	vec3 ambientColor;\n"
			"};");

		// FS Uniforms
		ASTNode* material_Uniform_FSNode = new ASTNode(UNIFORM, "uniform Material mat;");
		material_Uniform_FSNode->addRequiredNode(material_Struct_FSNode);
		ASTNode* dirLight_Uniform_FSNode = new ASTNode(UNIFORM, "uniform vec3 lightDir;");
		ASTNode* tex_Uniform_FSNode = new ASTNode(UNIFORM, "uniform sampler2D tex;");

		// FS Input Attributes
		ASTNode* normal_InAttrib_FSNode = new ASTNode(INATTRIB, "in vec3 inFNormal;");
		normal_InAttrib_FSNode->addRequiredNode(forwardNormal_Operation_VSNode);
		ASTNode* texCoord_InAttrib_FSNode = new ASTNode(INATTRIB, "in vec2 inFTexCoord;");
		texCoord_InAttrib_FSNode->addRequiredNode(forwardTexCoord_Operation_VSNode);
		ASTNode* color_InAttrib_FSNode = new ASTNode(INATTRIB, "in vec3 inFColor;");
		color_InAttrib_FSNode->addRequiredNode(forwardColor_Operation_VSNode);

		// FS Output Attributes
		ASTNode* color_OutAttrib_FSNode = new ASTNode(OUTATTRIB, "out vec4 outFColor;");

		// FS Operations
		ASTNode* texColor_Operation_FSNode = new ASTNode(OPERATION, "vec4 texColor = texture2D(tex, inFTexCoord);");
		texColor_Operation_FSNode->addRequiredNode(tex_Uniform_FSNode);
		texColor_Operation_FSNode->addRequiredNode(texCoord_InAttrib_FSNode);
		ASTNode* diffuseRadiance_Operation_FSNode = new ASTNode(OPERATION, "float diffuseRadiance = dot(lightDir, inFNormal);");
		diffuseRadiance_Operation_FSNode->addRequiredNode(dirLight_Uniform_FSNode);
		diffuseRadiance_Operation_FSNode->addRequiredNode(normal_InAttrib_FSNode);

		ASTNode* forwardColor_Operation_FSNode = new ASTNode(OPERATION, "fragColor = inFColor;");
		forwardColor_Operation_FSNode->addRequiredNode(color_OutAttrib_FSNode);
		forwardColor_Operation_FSNode->addRequiredNode(color_InAttrib_FSNode);
		ASTNode* ambientColor_Operation_FSNode = new ASTNode(OPERATION, "fragColor = vec4(mat.ambientColor, 1.0f);");
		ambientColor_Operation_FSNode->addRequiredNode(color_OutAttrib_FSNode);
		ambientColor_Operation_FSNode->addRequiredNode(material_Struct_FSNode);
		ASTNode* diffuseAmbient_Operation_FSNode = new ASTNode(OPERATION, "fragColor = clamp(diffuseRadiance * mat.diffuseColor + mat.ambientColor, 0.0f, 1.0f);");
		diffuseAmbient_Operation_FSNode->addRequiredNode(color_OutAttrib_FSNode);
		diffuseAmbient_Operation_FSNode->addRequiredNode(diffuseRadiance_Operation_FSNode);
		diffuseAmbient_Operation_FSNode->addRequiredNode(material_Struct_FSNode);
		ASTNode* diffuseColor_Operation_FSNode = new ASTNode(OPERATION, "fragColor = clamp(diffuseRadiance * inFColor, 0.0f, 1.0f);");
		diffuseColor_Operation_FSNode->addRequiredNode(color_OutAttrib_FSNode);
		diffuseColor_Operation_FSNode->addRequiredNode(diffuseRadiance_Operation_FSNode);
		diffuseColor_Operation_FSNode->addRequiredNode(color_InAttrib_FSNode);
		ASTNode* texColor_Operation_FSNode = new ASTNode(OPERATION, "fragColor = texColor;");
		texColor_Operation_FSNode->addRequiredNode(color_OutAttrib_FSNode);
		texColor_Operation_FSNode->addRequiredNode(texColor_Operation_FSNode);
		ASTNode* diffuseTexColor_Operation_FSNode = new ASTNode(OPERATION, "fragColor = clamp(diffuseRadiance * vec3(texColor), 0.0f, 1.0f);");
		diffuseTexColor_Operation_FSNode->addRequiredNode(color_OutAttrib_FSNode);
		diffuseTexColor_Operation_FSNode->addRequiredNode(diffuseRadiance_Operation_FSNode);
		diffuseTexColor_Operation_FSNode->addRequiredNode(texColor_Operation_FSNode);
	}
};