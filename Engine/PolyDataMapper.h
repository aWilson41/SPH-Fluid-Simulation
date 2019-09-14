#pragma once
#include "AbstractMapper.h"
#include "Types.h"

class PhongMaterial;
class PolyData;
class ShaderProgram;

// Does rendering and pre rendering operations (mapping input to visual representation)
// Generalized class, makes no assumptions about data provided too it at the cost of some optimization
class PolyDataMapper : public AbstractMapper
{
public:
	PolyDataMapper();
	~PolyDataMapper();

public:
	PolyData* getInput() { return polyData; }
	ShaderProgram* getShaderProgram() { return shaderProgram; }
	PhongMaterial* getMaterial() { return material; }
	glm::mat4 getModelMatrix() { return model; }
	CellType getPolyRepresentation() { return representation; }
	GLfloat getPointSize() { return pointSize; }
	GLuint getShaderProgramID() override;

	void setInput(PolyData* input) { polyData = input; }
	void setShaderProgram(ShaderProgram* shaderProgram)
	{
		PolyDataMapper::shaderProgram = shaderProgram;
		useCustomShader = true;
	}
	void setMaterial(PhongMaterial* material) { PolyDataMapper::material = material; }
	void setModelMatrix(glm::mat4 model) { PolyDataMapper::model = model; }
	void setPolyRepresentation(CellType representation) { PolyDataMapper::representation = representation; }
	void setPointSize(GLfloat pointSize) { PolyDataMapper::pointSize = pointSize; }
	void setUseNormals(bool val) { useNormals = val; }
	void setUseTexCoords(bool val) { useTexCoords = val; }
	void setUseScalars(bool val) { useScalars = val; }
	void setUseIndex(bool val) { useIndex = val; }

	std::string getMapperName() override { return "PolyDataMapper"; }

	void update() override;

	void useShader(std::string shaderGroup) override;

	void draw(Renderer* ren) override;

protected:
	// Updates internal info about the data to be processed
	void updateInfo();
	void updateBuffer();

protected:
	PolyData* polyData = nullptr;
	PhongMaterial* material = nullptr;
	glm::mat4 model = glm::mat4(1.0f);

	GLuint vboID = -1;
	GLint vboSize = -1;
	GLuint iboID = -1;
	GLint iboSize = -1;
	GLuint vaoID = -1;

	CellType representation = TRIANGLE;
	GLfloat pointSize = 4.0f;

	ShaderProgram* shaderProgram = nullptr;

	// Use normals, scalars, and indices when available
	// User set values
	bool useNormals = true;
	bool useTexCoords = false;
	bool useScalars = true;
	bool useIndex = true;

	// Internally determined
	bool hasNormals = false;
	bool hasTexCoords = false;
	bool hasScalars = false;
	bool hasIndices = false;
};