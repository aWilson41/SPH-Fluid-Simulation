#include "PolyDataMapper.h"
#include "Camera.h"
#include "Material.h"
#include "PolyData.h"
#include "Renderer.h"
#include "Shaders.h"

PolyDataMapper::PolyDataMapper()
{
	// Create a property map for this mapper
	// This is so a single unique key (bitset) can be produced to use in branching
	propertyMap.addProperty("Use_Normals", false);
	propertyMap.addProperty("Use_TexCoords", false);
	propertyMap.addProperty("Use_VertexColors", false);
	propertyMap.addProperty("Use_Indices", false);

	// Ultimately shaders are decided through a combination of (data properties, renderer, scene properties)

	// Shaders aren't known during GPU memory update so standardized layouts and names are used and implemented in shaders
	// In the future maybe add location assignment in the shader compiler so I don't have to worry about this
	/*std::map<std::string, unsigned int> layoutLocations;
	layoutLocations["inPos"] = 0;
	layoutLocations["inNormal"] = 1;
	layoutLocations["inTexCoord"] = 2;
	layoutLocations["inColor"] = 3;*/
}

PolyDataMapper::~PolyDataMapper()
{
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

	// If the vbo haven't been created yet
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
	// If it has already been created
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

	propertyMap.setProperty("Use_VertexColors", hasScalars);
	propertyMap.setProperty("Use_TexCoords", hasTexCoords);
	propertyMap.setProperty("Use_Normals", hasNormals);
	propertyMap.setProperty("Use_Indices", hasIndices);

	// Determine size of gpu mem to allocate
	const GLuint numPts = polyData->getPointCount();
	vboSize = sizeof(GLfloat) * 3 * numPts; // Position
	if (hasNormals)
		vboSize += sizeof(GLfloat) * 3 * numPts; // Normals
	if (hasTexCoords)
		vboSize += sizeof(GLfloat) * 2 * numPts; // Tex coords
	if (hasScalars)
		vboSize += sizeof(GLfloat) * 3 * numPts; // Scalars
}
void PolyDataMapper::updateBuffer()
{
	glBindVertexArray(vaoID);

	const GLfloat* vertexData = polyData->getVertexData();
	const GLfloat* normalData = polyData->getNormalData();
	const GLfloat* texCoordData = polyData->getTexCoordData();
	const GLfloat* scalarData = polyData->getScalarData();
	const GLint numPts = polyData->getPointCount();

	if (vboID != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vboID);

		// Load positional data
		GLint size = sizeof(GLfloat) * 3 * numPts;
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, vertexData);
		// Set it's location and access scheme in vao
		GLuint posAttribLocation = 0;// glGetAttribLocation(shaderID, "inPos");
		glEnableVertexAttribArray(posAttribLocation);
		glVertexAttribPointer(posAttribLocation, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void*)0);

		GLint offset = size;

		// If it has normal data
		if (hasNormals)
		{
			size = sizeof(GLfloat) * 3 * numPts;
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, normalData);
			// Set it's location and access scheme in vao
			GLuint normalAttribLocation = 1;// glGetAttribLocation(shaderID, "inNormal");
			glEnableVertexAttribArray(normalAttribLocation);
			glVertexAttribPointer(normalAttribLocation, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void*)offset);
			offset += size;
		}
		if (hasTexCoords)
		{
			size = sizeof(GLfloat) * 2 * numPts;
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, texCoordData);
			// Set it's location and access scheme in vao
			GLuint texCoordAttribLocation = 2;// glGetAttribLocation(shaderID, "inTexCoord");
			glEnableVertexAttribArray(texCoordAttribLocation);
			glVertexAttribPointer(texCoordAttribLocation, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, (void*)offset);
			offset += size;
		}
		if (hasScalars)
		{
			size = sizeof(GLfloat) * 3 * numPts;
			glBufferSubData(GL_ARRAY_BUFFER, offset, size, scalarData);
			// Set it's location and access scheme in vao
			GLuint scalarsAttribLocation = 3;// glGetAttribLocation(shaderID, "inScalars");
			glEnableVertexAttribArray(scalarsAttribLocation);
			glVertexAttribPointer(scalarsAttribLocation, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3, (void*)offset);
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
	GLint prevPolyMode;
	glGetIntegerv(GL_POLYGON_MODE, &prevPolyMode);

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

	if (shaderProgram == nullptr) // Or current property bits != previous
		shaderProgram = Shaders::getShader(ren, "PolyDataMapper", propertyMap.getPropertyBits());

	// If the currently bound shader is diff bind the new one
	GLuint shaderProgramId = shaderProgram->getProgramID();
	glUseProgram(shaderProgramId);

	// Set the uniforms
	glm::mat4 mvp = ren->getCamera()->proj * ren->getCamera()->view * model;
	glm::vec3 tmp = glm::normalize(glm::vec3(0.0f, 1.0f, 1.0f));
	GLuint lightDirLocation = glGetUniformLocation(shaderProgramId, "lightDir");
	if (lightDirLocation != -1)
		glUniform3fv(lightDirLocation, 1, &tmp[0]);
	GLuint mvpMatrixLocation = glGetUniformLocation(shaderProgramId, "mvp_matrix");
	if (mvpMatrixLocation != -1)
		glUniformMatrix4fv(mvpMatrixLocation, 1, GL_FALSE, &mvp[0][0]);
	glm::vec3 diffuse = glm::vec3(0.7f, 0.7f, 0.7f);
	glm::vec3 ambient = glm::vec3(0.0f, 0.0f, 0.0f);
	if (material != nullptr)
	{
		diffuse = material->getDiffuse();
		ambient = material->getAmbient();
	}
	GLuint diffuseColorLocation = glGetUniformLocation(shaderProgramId, "mat.diffuseColor");
	if (diffuseColorLocation != -1)
		glUniform3fv(diffuseColorLocation, 1, &diffuse[0]);
	GLuint ambientColorLocation = glGetUniformLocation(shaderProgramId, "mat.ambientColor");
	if (ambientColorLocation != -1)
		glUniform3fv(ambientColorLocation, 1, &ambient[0]);

	glBindVertexArray(vaoID);
	if (polyData->getIndexData() != nullptr && useIndex)
		glDrawElements(GL_TRIANGLES, polyData->getIndexCount(), GL_UNSIGNED_INT, (void*)0);
	else
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(polyData->getPointCount()));
	glBindVertexArray(0);

	// Restore poly mode
	glPolygonMode(GL_FRONT_AND_BACK, prevPolyMode);
}