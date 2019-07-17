#include "PolyDataMapper.h"
#include "Camera.h"
#include "Material.h"
#include "PolyData.h"
#include "Renderer.h"
#include "ReplaceableShader.h"
#include "Shaders.h"

PolyDataMapper::PolyDataMapper()
{
	// Initialize the master shader layout. All the possible structs, uniforms, attributes, etc
	// Vertex Shader
	//vertexShader = new ReplaceableShader();
	//// Uniforms
	//vertexShader->addUniform("mvpMatrix", "uniform mat4 mvpMatrix;");

	//// In vertex attributes
	//vertexShader->addInAttribute("inVPos", "in vec3 inVPos;");
	//vertexShader->addInAttribute("inVColor", "in vec3 inVColor;");
	//vertexShader->addInAttribute("inVNormal", "in vec3 inVNormal;");
	//vertexShader->addInAttribute("inVTexCoord", "in vec2 inVTexCoord;");

	//// Out vertex attributes/In fragment attributes
	//vertexShader->addOutAttribute("inFColor", "smooth out vec2 inFTexCoord;");
	//vertexShader->addOutAttribute("inFNormal", "smooth out vec3 inFNormal;");
	//vertexShader->addOutAttribute("inFTexCoord", "smooth out vec3 inFColor;");


	//// Fragment Shader
	//fragmentShader = new ReplaceableShader();
	//// Structs
	//fragmentShader->addStruct("Material", "struct Material { vec3 diffuseColor; vec3 ambientColor; }");

	//// Uniforms
	//fragmentShader->addUniform("mat", "uniform Material mat;");
	//fragmentShader->addUniform("tex", "uniform sample2D tex;");
	//fragmentShader->addUniform("lightDir", "uniform vec3 lightDir;");

	//// In fragment attributes
	//fragmentShader->addInAttribute("inFColor", "smooth in vec3 inFColor;");
	//fragmentShader->addInAttribute("inFNormal", "smooth in vec3 inFNormal;");
	//fragmentShader->addInAttribute("inFTexCoord", "smooth in vec2 inFTexCoord;");

	//// Out fragment attributes
	//fragmentShader->addOutAttribute("fColor", "out vec4 fColor;");
}

PolyDataMapper::~PolyDataMapper()
{
	/*if (vertexShader)
		delete vertexShader;
	if (fragmentShader)
		delete fragmentShader;*/
	glUseProgram(0);
	if (vaoID != -1)
		glDeleteVertexArrays(1, &vaoID);
	if (vboID != -1)
		glDeleteBuffers(1, &vboID);
	if (iboID != -1)
		glDeleteBuffers(1, &iboID);
}

void PolyDataMapper::update()
{
	// Poly data must have vertex data to be visually mapped
	if (polyData->getVertexData() == nullptr)
		return;

	updateInfo();
	// If no shader was set don't map anything
	if (shaderProgram == nullptr)
		return;

	glUseProgram(shaderProgram->getProgramID());
	const GLuint shaderID = shaderProgram->getProgramID();

	// If the vbo hasn't been created yet allocate it's space
	if (vboID == -1)
	{
		// Generate the vao
		glGenVertexArrays(1, &vaoID);
		glBindVertexArray(vaoID);
		// Gen and allocate space for vbo
		glGenBuffers(1, &vboID);
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferData(GL_ARRAY_BUFFER, vboSize, NULL, GL_DYNAMIC_DRAW);	
	}
	// If it has already been created, check if we should udpate size
	else
	{
		// Get the current vbo size
		GLint bufferSize;
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

		// If the size has changed reallocate
		if (vboSize != bufferSize)
			glBufferData(GL_ARRAY_BUFFER, vboSize, NULL, GL_DYNAMIC_DRAW);
	}
	// If we have index data
	if (hasIndices)
	{
		// If it hasn't been created yet
		iboSize = sizeof(GLuint) * polyData->getIndexCount();
		if (iboID == -1)
		{
			glGenBuffers(1, &iboID);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, iboSize, polyData->getIndexData(), GL_DYNAMIC_DRAW);
		}
		// If it was already created
		else
		{
			// Get the current ibo size
			GLint bufferSize;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
			glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

			// If the size has changed reallocate
			if (iboSize != bufferSize)
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, iboSize, NULL, GL_DYNAMIC_DRAW);
		}
	}
	updateBuffer();

	// Verify the desired representation (we can't map cells with more elements to one of fewer ie: can't represent point with triangle)
	representation = MathHelp::clamp(representation, POINT, polyData->getCellType());
}
void PolyDataMapper::updateInfo()
{
	// We'll determine what shader to use based on what data is available and what the user wants to use
	hasNormals = (polyData->getNormalData() != nullptr && useNormals);
	hasTexCoords = (polyData->getTexCoordData() != nullptr && useTexCoords);
	hasScalars = (polyData->getScalarData() != nullptr && useScalars);
	hasIndices = (polyData->getIndexData() != nullptr && useIndex);

	// Determine size of gpu mem to allocate
	const GLuint numPts = polyData->getPointCount();
	vboSize = sizeof(GLfloat) * 3 * numPts; // Position
	if (hasNormals)
		vboSize += sizeof(GLfloat) * 3 * numPts; // Normals
	if (hasTexCoords)
		vboSize += sizeof(GLfloat) * 2 * numPts; // Tex coords
	if (hasScalars)
		vboSize += sizeof(GLfloat) * 3 * numPts; // Scalars

	// If force shader is on then user of class must set the shader
	if (forceShader)
		return;

	// Put each into a separate bit to produce a unique number for each decision
	UINT flag = static_cast<UINT>(hasScalars) |
		(static_cast<UINT>(hasTexCoords) * 2) |
		(static_cast<UINT>(hasNormals) * 4);
	switch (flag)
	{
		// No normals, no texCoords, no scalars (000)
	case 0: shaderProgram = Shaders::getShader("Point Shader");
		break;
		// No normals, no texCoords, has scalars (001)
	case 1: shaderProgram = Shaders::getShader("Color Shader");
		break;
		// No normals, has texCoords, no scalars (010)
	case 2: shaderProgram = Shaders::getShader("Tex3 Shader");
		break;
		// No normals, has texCoords, has scalars (011)
			//case 3: // There is no shader for this currently. Not that useful
			//	break;
		// Has normals, no texCoords, no scalars (100)
	case 4: shaderProgram = Shaders::getShader("Normal Shader");
		break;
		// Has normals, no texCoords, has scalars (101)
	case 5: shaderProgram = Shaders::getShader("NormalColor Shader");
		break;
		// Has normals, has texCoords, no scalars (110)
	case 6: shaderProgram = Shaders::getShader("NormalTex Shader");
		break;
		// Has normals, has texCoords, has scalars (111)
			//case 7: // There is no shader for this currently. Not that useful
			//	break;
	default:
		shaderProgram = nullptr;
		break;
	};
}
void PolyDataMapper::updateBuffer()
{
	glBindVertexArray(vaoID);

	const GLfloat* vertexData = polyData->getVertexData();
	const GLfloat* normalData = polyData->getNormalData();
	const GLfloat* texCoordData = polyData->getTexCoordData();
	const GLfloat* scalarData = polyData->getScalarData();
	const GLuint shaderID = shaderProgram->getProgramID();
	const GLint numPts = polyData->getPointCount();

	if (vboID != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vboID);

		// Load positional data
		GLint size = sizeof(GLfloat) * 3 * numPts;
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertexData);
		// Set it's location and access scheme in vao
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void*)0);

		GLint offset = size;

		// If it has normal data
		if (hasNormals)
		{
			size = sizeof(GLfloat) * 3 * numPts;
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, normalData);
			// Set it's location and access scheme in vao
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void*)offset);
			offset += size;
		}
		if (hasTexCoords)
		{
			size = sizeof(GLfloat) * 2 * numPts;
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, texCoordData);
			// Set it's location and access scheme in vao
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)offset);
			offset += size;
		}
		if (hasScalars)
		{
			size = sizeof(GLfloat) * 3 * numPts;
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, scalarData);
			// Set it's location and access scheme in vao
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void*)offset);
			//offset += size;
		}
	}
	// Update index buffer if it has one and buffer has been created
	if (iboID != -1)
	{
		if (hasIndices)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(GLuint) * polyData->getIndexCount(), polyData->getIndexData());
		}
	}

	glBindVertexArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void PolyDataMapper::draw(Renderer* ren)
{
	if (polyData == nullptr || vaoID == -1)
		return;

	// Save the polygon mode
	GLint polyMode;
	glGetIntegerv(GL_POLYGON_MODE, &polyMode);

	// Set the polygon mode needed
	if (representation == TRIANGLE)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else if (representation == LINE)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else if (representation == POINT)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glPointSize(pointSize);
	}

	// If the currently bound shader is diff bind the new one
	GLuint programId = shaderProgram->getProgramID();
	glUseProgram(programId);

	// Set the uniforms
	glm::mat4 mvp = ren->getCamera()->proj * ren->getCamera()->view * model;
	glm::vec3 tmp = glm::normalize(glm::vec3(0.0f, 1.0f, 1.0f));
	GLuint lightDirLocation = glGetUniformLocation(programId, "lightDir");
	if (lightDirLocation != -1)
		glUniform3fv(lightDirLocation, 1, &tmp[0]);
	GLuint mvpMatrixLocation = glGetUniformLocation(programId, "mvp_matrix");
	if (mvpMatrixLocation != -1)
		glUniformMatrix4fv(mvpMatrixLocation, 1, GL_FALSE, &mvp[0][0]);
	glm::vec3 diffuse = glm::vec3(0.7f, 0.7f, 0.7f);
	glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
	if (material != nullptr)
	{
		diffuse = material->getDiffuse();
		ambient = material->getAmbient();
	}
	GLuint diffuseColorLocation = glGetUniformLocation(programId, "mat.diffuseColor");
	if (diffuseColorLocation != -1)
		glUniform3fv(diffuseColorLocation, 1, &diffuse[0]);
	GLuint ambientColorLocation = glGetUniformLocation(programId, "mat.ambientColor");
	if (ambientColorLocation != -1)
		glUniform3fv(ambientColorLocation, 1, &ambient[0]);

	glBindVertexArray(vaoID);
	if (polyData->getIndexData() != nullptr && useIndex)
		glDrawElements(GL_TRIANGLES, polyData->getIndexCount(), GL_UNSIGNED_INT, (void*)0);
	else
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(polyData->getPointCount()));
	glBindVertexArray(0);

	// Set the poly mode back to what it was
	glPolygonMode(GL_FRONT_AND_BACK, polyMode);
}