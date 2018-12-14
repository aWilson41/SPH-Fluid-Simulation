#pragma once
#include "CellData.h"
#include "MathHelper.h"
#include "PointData.h"
#include "Types.h"
#include <vector>
// Plans:
//  Add something to specify the content of a slot. This I can do things like: Give me the first
//    available set of normals from the vertices attributes. Rather then relying on them being in certain index.
//  Make this and imageData inherit the same base dataset

// Data = structure + attributes
// structure = topology + geometry
// PointData contains the geometry
// CellData contains the topology
// Attributes of each can be used to store things like normals, colors, other scalars, etc alongside geometry of topology
// 8 slots for attributes to keep them consistent in index.

// Actual container for poly data. Can store whatever geometry (points, lines, triangles, tetrahedrons, blah, blah blah)
// Contains functions to allocate normal, tex coords, and scalars as attributes in specific slots.
// By convention slots are utilized like so:
//  normal - 0, tex coords - 1, scalars - 2
class PolyData
{
public:
	UINT getPointCount() { return points.count; }
	UINT getCellCount() { return cells.cellCount; }
	UINT getIndexCount() { return cells.indexCount; }
	CellType getCellType() { return cells.type; }

	GLfloat* getVertexData() { return points.data; }
	GLfloat* getNormalData() { return static_cast<GLfloat*>(points.attrib[0]); }
	GLfloat* getTexCoordData() { return static_cast<GLfloat*>(points.attrib[1]); }
	GLfloat* getScalarData() { return static_cast<GLfloat*>(points.attrib[2]); }
	GLuint* getIndexData() { return cells.data; }

	// Allocate vertices by cells. Resulting amount of vertices depend on cell type
	void allocateVertexData(UINT cellCount, CellType type);
	// Allocate vertices by vertex count (usually for indexing later)
	void allocateSharedVertexData(UINT vertexCount, CellType type);
	void allocateIndexData(UINT indexCount);
	void allocateNormalData();
	void allocateTexCoords();
	void allocateScalarData(UINT numComps);

protected:
	PointData points;
	CellData cells;
};