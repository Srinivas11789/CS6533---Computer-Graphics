#include "glsupport.h"
#include <GL/freeglut.h>
#include "geometrymaker.h"
#include "matrix4.h"
#include "quat.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <algorithm> 
#define NULL 0

// Buffer Objects - Cube, Sphere, Plane, Model 1 2 3 
GLuint vertexBO, vertexBO1, vertexBO2, vertexBO3, vertexBO4, vertexBO5;
GLuint indexBO, indexBO1, indexBO2, indexBO3, indexBO4, indexBO5;

// Program Handle for the Shader Programs
GLuint program;

// Attributes for Position and Color
GLuint positionAttribute;
GLuint normalAttribute;
GLuint texCoordAttribute;
GLuint binormalAttribute;
GLuint tangentAttribute;

//Uniform Location Variables

// Matrix Uniforms
GLuint modelviewMatrixUniformLocation;
GLuint projectionMatrixUniformLocation;
GLuint normalMatrixUniformLocation;
// Light Position Uniforms
GLuint lightDirectionUniformLocation0;
GLuint lightDirectionUniformLocation1;
GLuint lightDirectionUniformLocation2;
// Light Color Uniforms
GLuint lightColorUniform0;
GLuint lightColorUniform1;
GLuint lightColorUniform2;
// Specular Light Uniforms
GLuint SpecularLightUniform0;
GLuint SpecularLightUniform1;
GLuint SpecularLightUniform2;
// Uniform to shift between VaryingNormal and TexureNormal
GLuint text_off_uniform;

//Texture
//Diffuse
GLuint diffuseTexture;
GLuint diffuseTexture1;
GLuint diffuseTexture2;
//Specular
GLuint specularTexture;
GLuint specularTexture1;
GLuint specularTexture2;
//Normal
GLuint normalTexture;
GLuint normalTexture1;
GLuint normalTexture2;

// Texture Uniform Locations
GLuint specularUniformLocation;
GLuint specularUniformLocation1;
GLuint diffuseTextureUniformLocation;
GLuint diffuseTextureUniformLocation1;
GLuint normalTextureUniformLocation;

//Indices
int numIndices;

//Mouse Movements - Arcball
int screen_width = 500;
int screen_height = 500;
int last_mx = 0, last_my = 0, cur_mx = 0, cur_my = 0;
int arcball_on = false;
float angle = 0;
Quat arc;

// Matrix Declarations
Matrix4 arcMatrix;
Matrix4 eye;
Matrix4 projectionMatrix;

// Vertex Structure for Position and Normal
struct VertexPNTBTG {
	Cvec3f p, n, b, tg;
	Cvec2f t;
	VertexPNTBTG() {}
	VertexPNTBTG(float x, float y, float z, float nx, float ny, float nz) : p(x, y, z), n(nx, ny, nz) {}
	VertexPNTBTG& operator = (const GenericVertex& v) {
		p = v.pos;
		n = v.normal;
		t = v.tex;
		b = v.binormal;
		tg = v.tangent;
		return *this;
	}
};

// Transformation Structure with object Matrix creation Function - Translation, Rotation and Scaling
struct Transform {
	Quat rotation;
	Cvec3 scale;
	Cvec3 position;
	Transform() : scale(1.0f, 1.0f, 1.0f) {
	}
	Matrix4 createMatrix() {
		Matrix4 object;
		Matrix4 quatrotationMatrix = quatToMatrix(rotation);
		Matrix4 obj_scale = obj_scale.makeScale(scale);
		Matrix4 obj_trans = obj_trans.makeTranslation(position);
		object = object * obj_trans * quatrotationMatrix * obj_scale;
		return object;
	}
};

// Geometry Structure which consists of function DRAW - which binds buffer objects and issues draw call
struct Geometry {
	GLuint vertex;
	GLuint index;
	void Draw(GLuint positionAttribute, GLuint normalAttribute, GLuint textureAttribute, GLuint binormalAttribute, GLuint tangentAttribute, string type) {
		// bind buffer objects and draw
		if (type == "cube") {
			vertex = vertexBO;
			index = indexBO;
		}
		else if (type == "sphere") {
			vertex = vertexBO1;
			index = indexBO1;
		}
		else if (type == "model") {
			vertex = vertexBO2;
			index = indexBO2;
		}
		else if (type == "model2") {
			vertex = vertexBO4;
			index = indexBO4;
		}
		else if (type == "model3") {
			vertex = vertexBO5;
			index = indexBO5;
		}
		else if (type == "plane") {
			vertex = vertexBO3;
			index = indexBO3;
		}
		else {
			cout << "No Type Matched";
		}
		glBindBuffer(GL_ARRAY_BUFFER, vertex);
		glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPNTBTG), (void*)offsetof(VertexPNTBTG, p));
		glEnableVertexAttribArray(positionAttribute);
		glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPNTBTG), (void*)offsetof(VertexPNTBTG, n));
		glEnableVertexAttribArray(normalAttribute);
		glVertexAttribPointer(textureAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPNTBTG), (void*)offsetof(VertexPNTBTG, t));
		glEnableVertexAttribArray(textureAttribute);
//		glVertexAttribPointer(binormalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPNTBTG), (void*)offsetof(VertexPNTBTG, b));
//		glEnableVertexAttribArray(binormalAttribute);
//		glVertexAttribPointer(tangentAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPNTBTG), (void*)offsetof(VertexPNTBTG, tg));
//		glEnableVertexAttribArray(tangentAttribute);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index);
		glDrawElements(GL_TRIANGLES, numIndices , GL_UNSIGNED_SHORT, 0);
	}
};

// Entity Structure for every object transforms them to Eye and then Projection co-ord systems
// Maintains the Hierarchy
struct Entity {
	Transform transform;
	Geometry geometry;
	Entity *parent;
	Matrix4 m;

	void Draw(Matrix4 eyeInverse, GLuint positionAttribute, GLuint normalAttribute, GLuint textureAttribute, GLuint binormalAttribute, GLuint tangentAttribute,
		GLuint modelviewMatrixUniformLocation, GLuint normalMatrixUniformLocation, string type) {
		// create modelview matrix
		// Hierarchy Parent Setup Component
		if (parent == NULL) {
			m = transform.createMatrix();
			m = eyeInverse * m;
		} 
		else {
			m = parent->m * transform.createMatrix();
		}
		// create normal matrix
		Matrix4 invm = inv(m);
		invm(0, 3) = invm(1, 3) = invm(2, 3) = 0;
		Matrix4 n = transpose(invm);
		// set the model view and normal matrices to the uniforms locations
		GLfloat glmatrix[16];
		m.writeToColumnMajorMatrix(glmatrix);
		glUniformMatrix4fv(modelviewMatrixUniformLocation, 1, false, glmatrix);
		GLfloat normalglmatrix[16];
		n.writeToColumnMajorMatrix(normalglmatrix);
		glUniformMatrix4fv(normalMatrixUniformLocation, 1, false, normalglmatrix);
		//Projection Matrix Component
		Matrix4 projectionMatrix;
		projectionMatrix = projectionMatrix.makeProjection(45.0, 1.0, -0.1, 100.0);
		GLfloat glmatrixProjection[16];
		projectionMatrix.writeToColumnMajorMatrix(glmatrixProjection);
		glUniformMatrix4fv(projectionMatrixUniformLocation, 1, false, glmatrixProjection);
		// Geometry Structure Draw Call
		geometry.Draw(positionAttribute, normalAttribute, textureAttribute, binormalAttribute, tangentAttribute, type);
	}
};

// Cube Function to Draw the Cube from the Geometry Maker Header File
void cube() {

	    // Cube Parameters
	    int ibLen, vbLen;
		getCubeVbIbLen(vbLen, ibLen);
		std::vector<VertexPNTBTG> vtx(vbLen);
		std::vector<unsigned short> idx(ibLen);
		makeCube(2, vtx.begin(), idx.begin());

		glGenBuffers(1, &vertexBO);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPNTBTG) * vtx.size(), vtx.data(), GL_STATIC_DRAW);
		glGenBuffers(1, &indexBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx.size(), idx.data(), GL_STATIC_DRAW);
		numIndices = idx.size();
		
	}

//Sphere
void sphere() {

	//Sphere Parameters
	int ibLen1, vbLen1;
	getSphereVbIbLen(10, 10, vbLen1, ibLen1);
	std::vector<VertexPNTBTG> vtx1(vbLen1);
	std::vector<unsigned short> idx1(ibLen1);
	makeSphere(1.0, 10, 10, vtx1.begin(), idx1.begin());
	glGenBuffers(1, &vertexBO1);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPNTBTG) * vtx1.size(), vtx1.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &indexBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx1.size(), idx1.data(), GL_STATIC_DRAW);
	numIndices = idx1.size();
}

//Plane
void plane() {

	int ibLen2, vbLen2;
	getPlaneVbIbLen(vbLen2,ibLen2);
	std::vector<VertexPNTBTG> vtx2(vbLen2);
	std::vector<unsigned short> idx2(ibLen2);
	makePlane(7, vtx2.begin(), idx2.begin());
	glGenBuffers(1, &vertexBO3);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPNTBTG) * vtx2.size(), vtx2.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &indexBO3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx2.size(), idx2.data(), GL_STATIC_DRAW);
	numIndices = idx2.size();

}

void calculateFaceTangent(const Cvec3f &v1, const Cvec3f &v2, const Cvec3f &v3, const Cvec2f &texCoord1, const Cvec2f &texCoord2,
	const Cvec2f &texCoord3, Cvec3f &tangent, Cvec3f &binormal) {
	Cvec3f side0 = v1 - v2;
	Cvec3f side1 = v3 - v1;
	Cvec3f normal = cross(side1, side0);
	normalize(normal);
	float deltaV0 = texCoord1[1] - texCoord2[1];
	float deltaV1 = texCoord3[1] - texCoord1[1];
	tangent = side0 * deltaV1 - side1 * deltaV0;
	normalize(tangent);
	float deltaU0 = texCoord1[0] - texCoord2[0];
	float deltaU1 = texCoord3[0] - texCoord1[0];
	binormal = side0 * deltaU1 - side1 * deltaU0;
	normalize(binormal);
	Cvec3f tangentCross = cross(tangent, binormal);
	if (dot(tangentCross, normal) < 0.0f) {
		tangent = tangent * -1;
	}
}

// Tiny Object Loading - Changes Upto the Normal Maps - Function used by Monk and Batman Model
void loadObjFile(const std::string &fileName, std::vector<VertexPNTBTG> &outVertices, std::vector<unsigned short> &outIndices) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str(), NULL, true);
	if (ret) {
		for (int i = 0; i < shapes.size(); i++) {
			for (int j = 0; j < shapes[i].mesh.indices.size(); j++) {
				unsigned int vertexOffset = shapes[i].mesh.indices[j].vertex_index * 3;
				unsigned int normalOffset = shapes[i].mesh.indices[j].normal_index * 3;
				unsigned int texOffset = shapes[i].mesh.indices[j].texcoord_index * 2;
				VertexPNTBTG v;
				v.p[0] = attrib.vertices[vertexOffset];
				v.p[1] = attrib.vertices[vertexOffset + 1];
				v.p[2] = attrib.vertices[vertexOffset + 2];
				v.n[0] = attrib.normals[normalOffset];
				v.n[1] = attrib.normals[normalOffset + 1];
				v.n[2] = attrib.normals[normalOffset + 2];
				v.t[0] = attrib.texcoords[texOffset];
				v.t[1] = 1.0 - attrib.texcoords[texOffset + 1];
				outVertices.push_back(v);
				outIndices.push_back(outVertices.size() - 1);
			}
		}
		for (int i = 0; i < outVertices.size(); i += 3) {
				Cvec3f tangent;
				Cvec3f binormal;
				calculateFaceTangent(outVertices[i].p, outVertices[i + 1].p, outVertices[i + 2].p,
					outVertices[i].t, outVertices[i + 1].t, outVertices[i + 2].t, tangent, binormal);
				outVertices[i].tg = tangent;
				outVertices[i + 1].tg = tangent;
				outVertices[i + 2].tg = tangent;
				outVertices[i].b = binormal;
				outVertices[i + 1].b = binormal;
				outVertices[i + 2].b = binormal;
			}
	}
	else {
		std::cout << err << std::endl;
		assert(false);
	}
}

// Function Used By the LUCY Object - Without the textures containing Only the Lights
void loadObjFile1(const std::string &fileName, std::vector<VertexPNTBTG> &outVertices, std::vector<unsigned
	short> &outIndices) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str(), NULL, true);
	if (ret) {
		for (int i = 0; i < attrib.vertices.size(); i += 3) {
			VertexPNTBTG v;
			v.p[0] = attrib.vertices[i];
			v.p[1] = attrib.vertices[i + 1];
			v.p[2] = attrib.vertices[i + 2];
			v.n[0] = attrib.normals[i];
			v.n[1] = attrib.normals[i + 1];
			v.n[2] = attrib.normals[i + 2];
			outVertices.push_back(v);
		}
		for (int i = 0; i < shapes.size(); i++) {
			for (int j = 0; j < shapes[i].mesh.indices.size(); j++) {
				outIndices.push_back(shapes[i].mesh.indices[j].vertex_index);
			}
		}
	}
	else {
		std::cout << err << std::endl;
		assert(false);
	}
}

// Tiny Object Loading
void loadObjFile2(const std::string &fileName, std::vector<VertexPNTBTG> &outVertices, std::vector<unsigned short> &outIndices) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str(), NULL, true);
	if (ret) {
		for (int i = 0; i < shapes.size(); i++) {
			for (int j = 0; j < shapes[i].mesh.indices.size(); j++) {
				unsigned int vertexOffset = shapes[i].mesh.indices[j].vertex_index * 3;
				unsigned int normalOffset = shapes[i].mesh.indices[j].normal_index * 3;
				unsigned int texOffset = shapes[i].mesh.indices[j].texcoord_index * 2;
				VertexPNTBTG v;
				v.p[0] = attrib.vertices[vertexOffset];
				v.p[1] = attrib.vertices[vertexOffset + 1];
				v.p[2] = attrib.vertices[vertexOffset + 2];
				v.n[0] = attrib.normals[normalOffset];
				v.n[1] = attrib.normals[normalOffset + 1];
				v.n[2] = attrib.normals[normalOffset + 2];
				v.t[0] = attrib.texcoords[texOffset];
				v.t[1] = 1.0 - attrib.texcoords[texOffset + 1];
				outVertices.push_back(v);
				outIndices.push_back(outVertices.size() - 1);
			}
		}
	}
	else {
		std::cout << err << std::endl;
		assert(false);
	}
}
// Model Creation - Monk and Batman
void model(string type) {
	std::vector<VertexPNTBTG> meshVertices;
	std::vector<unsigned short> meshIndices;
	loadObjFile(type, meshVertices, meshIndices);
	glGenBuffers(1, &vertexBO2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPNTBTG) * meshVertices.size(), meshVertices.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &indexBO2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * meshIndices.size(), meshIndices.data(), GL_STATIC_DRAW);
	numIndices = meshIndices.size();
}

void model2(string type) {

	std::vector<VertexPNTBTG> meshVertices2;
	std::vector<unsigned short> meshIndices2;
	loadObjFile(type, meshVertices2, meshIndices2);
	glGenBuffers(1, &vertexBO5);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBO5);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPNTBTG) * meshVertices2.size(), meshVertices2.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &indexBO5);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO5);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * meshIndices2.size(), meshIndices2.data(), GL_STATIC_DRAW);
	numIndices = meshIndices2.size();

}


// Model Creation - Lucy 

void model1(string type) {

	std::vector<VertexPNTBTG> meshVertices1;
	std::vector<unsigned short> meshIndices1;
	loadObjFile1(type, meshVertices1, meshIndices1);
	glGenBuffers(1, &vertexBO4);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBO4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPNTBTG) * meshVertices1.size(), meshVertices1.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &indexBO4);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO4);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * meshIndices1.size(), meshIndices1.data(), GL_STATIC_DRAW);
	numIndices = meshIndices1.size();

}

// Converting the Light Position to Eye Space
Cvec4 Light_Coord(Cvec3 c) {
	return (inv(eye) * Cvec4(c[0], c[1], c[2], 1.0));
}

// Rendering Function which creates the Eye Matrix and maintains each object parameters
void display(void) {

    glClear(GL_COLOR_BUFFER_BIT);
	glClearDepth(0.0f);
	glEnable(GL_DEPTH_TEST);

	// Varying Time Variable to Rotate the Light
	float time = glutGet(GLUT_ELAPSED_TIME);

	//Eye Matrix Component
	eye = arcMatrix * eye.makeTranslation(Cvec3(0.0, 0.0, 9.0));

	// Light Position in the Eye Space
	Cvec4 light_pos1 = Light_Coord(Cvec3((float)sin(time / 1000.0f) * 10, 4.0, -3.0));
	Cvec4 light_pos2 = Light_Coord(Cvec3(-3.0, 5.0, 1.0));
	Cvec4 light_pos3 = Light_Coord(Cvec3(2.0, 3.0, (float)sin(time / 1000.0f) * 10));
	Cvec4 light_pos4 = Light_Coord(Cvec3(3.0, 3.0, -6.0));
	/*
	// The LUCY Object Rendering
	// LUCY Object Lighting using Varying Normal - No Texture
	glUniform3f(lightDirectionUniformLocation0, light_pos1[0], light_pos1[1], light_pos1[2]);
	glUniform3f(lightColorUniform0, 1.0, 0.5, 0.0);
	glUniform3f(SpecularLightUniform0, 1.0, 1.0, 1.0);
	glUniform3f(lightDirectionUniformLocation1, light_pos2[0], light_pos2[1], light_pos2[2]);
	glUniform3f(lightColorUniform1, 1.0, 0.4, 0.0);
	glUniform3f(SpecularLightUniform1, 1.0, 1.0, 1.0);
	glUniform3f(lightDirectionUniformLocation2, light_pos3[0], light_pos3[1], light_pos3[2]);
	glUniform3f(lightColorUniform2, 1.0, 1.0, 0.0);
	glUniform3f(SpecularLightUniform2, 1.0, 1.0, 1.0);
	glUniform1i(text_off_uniform, 1);
	
	
	// Lucy Object and Draw Call

	string type2 = "model2";
	Entity *parent1;
	parent1 = new Entity();
	parent1->transform.rotation = Quat::makeYRotation(0.0f);
	parent1->transform.position = Cvec3(1.0, -2.0, 1.0);
	parent1->transform.scale = Cvec3(1.0, 1.0, 1.0);
	parent1->parent = NULL;
	parent1->Draw(inv(eye), positionAttribute, normalAttribute, texCoordAttribute, binormalAttribute, tangentAttribute, modelviewMatrixUniformLocation, normalMatrixUniformLocation, type2);
	*/

	
	// The Monk Object Rendering

	// Light Uniform Setting for Monk Model - texture Normal
	glUniform3f(lightDirectionUniformLocation0, light_pos1[0], light_pos1[1], light_pos1[2]);
	glUniform3f(lightColorUniform0, 1.0, 1.0, 1.0);
	glUniform3f(SpecularLightUniform0, 1.0, 1.0, 1.0);
	glUniform3f(lightDirectionUniformLocation1, light_pos2[0], light_pos2[1], light_pos2[2]);
	glUniform3f(lightColorUniform1, 1.0, 1.0, 1.0);
	glUniform3f(SpecularLightUniform1, 1.0, 1.0, 1.0);
	glUniform3f(lightDirectionUniformLocation2, light_pos3[0], light_pos3[1], light_pos3[2]);
	glUniform3f(lightColorUniform2, 1.0, 1.0, 1.0);
	glUniform3f(SpecularLightUniform2, 1.0, 1.0, 1.0);
	glUniform1i(text_off_uniform, 1);

	//Monk Model Texture
	glUniform1i(diffuseTextureUniformLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture);
	//glUniform1i(specularUniformLocation, 1);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, specularTexture);
	//glUniform1i(normalTextureUniformLocation, 2);
	//glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, normalTexture);
	//glUniform1i(text_off_uniform, 0);
	
	// Monk Model Object and Draw
	string type = "model";
	Entity *parent;
	parent = new Entity();
	parent->transform.rotation = Quat::makeYRotation(50.0f)*Quat::makeXRotation(15.3f)*Quat::makeZRotation(-43.0f);;
	parent->transform.position = Cvec3(1.5, -1.2, 0.0);
	parent->transform.scale = Cvec3(1.0, 1.0, 1.0);
	parent->parent = NULL;
	parent->Draw(inv(eye), positionAttribute, normalAttribute, texCoordAttribute, binormalAttribute, tangentAttribute, modelviewMatrixUniformLocation, normalMatrixUniformLocation, type);

    // The Batman Object Rendering
	// Batman Object Lighting - texture Normal
	glUniform3f(lightDirectionUniformLocation0, light_pos1[0], light_pos1[1], light_pos1[2]);
	glUniform3f(lightColorUniform0, 1.0, 1.0, 1.0);
	glUniform3f(SpecularLightUniform0, 1.0, 1.0, 1.0);
	glUniform3f(lightDirectionUniformLocation1, light_pos2[0], light_pos2[1], light_pos2[2]);
	glUniform3f(lightColorUniform1, 1.0, 1.0, 1.0);
	glUniform3f(SpecularLightUniform1, 1.0, 1.0, 1.0);
	glUniform3f(lightDirectionUniformLocation2, light_pos4[0], light_pos4[1], light_pos4[2]);
	glUniform3f(lightColorUniform2, 1.0, 1.0, 1.0);
	glUniform3f(SpecularLightUniform2, 1.0, 1.0, 1.0);
	glUniform1i(text_off_uniform, 1);

	//Batman Texture
	glUniform1i(diffuseTextureUniformLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture2);
	//glUniform1i(specularUniformLocation1, 1);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, specularTexture1);
	//glUniform1i(normalTextureUniformLocation, 2);
	//glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, normalTexture1);

	// Batman Object and Draw Call
	string type3 = "model3";
	Entity *parent2;
	parent2 = new Entity();
	//Quat::makeYRotation(180.0f)*Quat::makeXRotation(0.0f)*Quat::makeZRotation(-4.0f);
	parent2->transform.rotation = Quat::makeYRotation(180.0f)*Quat::makeXRotation(2.3f)*Quat::makeZRotation(-5.3f);
		//Quat::makeYRotation(90.0f)*Quat::makeXRotation(20.0f);
	parent2->transform.position = Cvec3(-1.5, -1.9, 0.0);
	parent2->transform.scale = Cvec3(0.4, 0.4, 0.4);
	parent2->parent = NULL;
	parent2->Draw(inv(eye), positionAttribute, normalAttribute, texCoordAttribute, binormalAttribute, tangentAttribute, modelviewMatrixUniformLocation, normalMatrixUniformLocation, type3);
	

	// Floor Plane Rendering
	// Floor Light - texture Normal
	glUniform3f(lightDirectionUniformLocation0, light_pos1[0], light_pos1[1], light_pos1[2]);
	glUniform3f(lightColorUniform0, 1.0, 1.0, 1.0);
	glUniform3f(SpecularLightUniform0, 1.0, 1.0, 1.0);
	glUniform3f(lightDirectionUniformLocation1, light_pos2[0], light_pos2[1], light_pos2[2]);
	glUniform3f(lightColorUniform1, 1.0, 0.0, 0.0);
	glUniform3f(SpecularLightUniform1, 1.0, 1.0, 1.0);
	glUniform3f(lightDirectionUniformLocation2, light_pos3[0], light_pos3[1], light_pos3[2]);
	glUniform3f(lightColorUniform2, 0.0, 0.0, 1.0);
	glUniform3f(SpecularLightUniform2, 1.0, 1.0, 1.0);

	// Floor Texture
	glUniform1i(diffuseTextureUniformLocation, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, diffuseTexture1);
//	glUniform1i(specularUniformLocation, 1);
//	glActiveTexture(GL_TEXTURE1);
//	glBindTexture(GL_TEXTURE_2D, specularTexture2);
//	glUniform1i(normalTextureUniformLocation, 2);
//	glActiveTexture(GL_TEXTURE2);
//	glBindTexture(GL_TEXTURE_2D, normalTexture2);
	glUniform1i(text_off_uniform, 1);

	// Floor Object and Draw Call
	string type1 = "plane";
	Entity *plane1;
	plane1 = new Entity();
	plane1->transform.rotation = Quat::makeYRotation(0.0f);
	plane1->transform.position = Cvec3(0.0, -2.0, 0.0);
	plane1->transform.scale = Cvec3(1.0, 1.0, 1.0);
	plane1->parent = NULL;
	plane1->Draw(inv(eye), positionAttribute, normalAttribute, texCoordAttribute, binormalAttribute, tangentAttribute, modelviewMatrixUniformLocation, normalMatrixUniformLocation, type1);
	
	// Attributes Disable

	glDisableVertexAttribArray(positionAttribute);
	glDisableVertexAttribArray(normalAttribute);
	glDisableVertexAttribArray(texCoordAttribute);
	glDisableVertexAttribArray(tangentAttribute);
	glDisableVertexAttribArray(binormalAttribute);

    glutSwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();
}

// Key A and D to Move the Scene about the Y Axis
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
		angle = angle - 1;
		arc = Quat::makeYRotation(angle);
		break;
	case 'd':
		angle = angle + 1;
		arc = Quat::makeYRotation(angle);
		break;
	case 'w':
		angle = angle + 1;
		arc = Quat::makeXRotation(angle);
		break;
	case 's':
		angle = angle - 1;
		arc = Quat::makeXRotation(angle);
		break;
	case 'q':
		angle = angle + 1;
		arc = Quat::makeZRotation(angle);
		break;
	case 'e':
		angle = angle - 1;
		arc = Quat::makeZRotation(angle);
		break;
	}
	arcMatrix = quatToMatrix(arc);
}

// Half Cooked ArcBall - Rotating in Only Y Direction using Mouse
// Mouse Input to get the Initial Position of the input from the Mouse
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		arcball_on = true;
		last_mx = cur_mx = x;
		last_my = cur_my = y;
	}
	else {
		arcball_on = false;
	}
}

// Mouse Input to get the Final Position of the input from the Mouse
void mouseMove(int x, int y) {
	if (arcball_on) {  // if left button is pressed
		cur_mx = x;
		cur_my = y;
		float v1_x = last_mx;
		float v1_y = last_my;
		float v2_x = cur_mx;
		float v2_y = cur_my;
		angle = angle + 1;
		arc = Quat::makeYRotation(angle);
		arcMatrix = quatToMatrix(arc);
	}
}

void init() {

	// Initialization with BackGround Color

	glClearDepth(0.0f);
	glClearColor(0.2f, 0.3f, 0.3f, 0.0f);

	// Creating an Handle for Shader Programs 

	program = glCreateProgram();
	readAndCompileShader(program, "vertex.glsl", "fragment.glsl");
	glUseProgram(program);

	// Declaring the Attributes (Position, Normal, TexCoord, Binormal, Tangent)

	positionAttribute = glGetAttribLocation(program, "position");
	normalAttribute = glGetAttribLocation(program, "normal");
	texCoordAttribute = glGetAttribLocation(program, "texCoord");
//	binormalAttribute = glGetAttribLocation(program, "binormal");
//	tangentAttribute = glGetAttribLocation(program, "tangent");

	// Getting the Uniform Locations

	modelviewMatrixUniformLocation = glGetUniformLocation(program, "modelViewMatrix");
	projectionMatrixUniformLocation = glGetUniformLocation(program, "projectionMatrix");
	normalMatrixUniformLocation = glGetUniformLocation(program, "normalMatrix");

	//Diffuse Lighting ---> Using 3 Lights in the Scene

	lightDirectionUniformLocation0 = glGetUniformLocation(program, "lights[0].lightPosition");
	lightColorUniform0 = glGetUniformLocation(program, "lights[0].lightColor");
	SpecularLightUniform0 = glGetUniformLocation(program, "lights[0].specularLightColor");
	
	lightDirectionUniformLocation1 = glGetUniformLocation(program, "lights[1].lightPosition");
	lightColorUniform1 = glGetUniformLocation(program, "lights[1].lightColor");
	SpecularLightUniform1 = glGetUniformLocation(program, "lights[1].specularLightColor");
	
	lightDirectionUniformLocation2 = glGetUniformLocation(program, "lights[2].lightPosition");
	lightColorUniform2 = glGetUniformLocation(program, "lights[2].lightColor");
	SpecularLightUniform2 = glGetUniformLocation(program, "lights[2].specularLightColor");

	//Texture Uniform Locations

	diffuseTextureUniformLocation = glGetUniformLocation(program, "diffuseTexture");
	specularUniformLocation = glGetUniformLocation(program, "specularTexture");
	normalTextureUniformLocation = glGetUniformLocation(program, "normalTexture");

	// To Emulate the Texture present for the Object : Executing Respective Code in the Fragment Shaders
	text_off_uniform = glGetUniformLocation(program, "text_off");

	// --------->  Drawing the Plane and the Models

	//MODEL 1 ---> MONK Object with all Textures

	model("fblen.obj");
	diffuseTexture = loadGLTexture("f_new.jpg");
//	specularTexture = loadGLTexture("Monk_S1.tga");
//	normalTexture = loadGLTexture("Monk_N1.tga");

	// MODEL 2 ---> Batman Object with all Textures

	model2("bblend.obj");
	diffuseTexture2 = loadGLTexture("b_new.jpg");
//	specularTexture1 = loadGLTexture("Batman_S.tga");
//	normalTexture1 = loadGLTexture("Batman_N.tga");

	// FLOOR --> The Floor constructed using a Plane from the Geometry Make containing the Textures

	plane();
	diffuseTexture1 = loadGLTexture("floor_D.tga");
//	specularTexture2 = loadGLTexture("floor_S.tga");
//	normalTexture2 = loadGLTexture("floor_N.tga");

	//MODEL 3 ---> LUCY Object without any Textures

	model1("lucy.obj");

}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

int main(int argc, char **argv) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    // Window Initialization
	glutInitWindowSize(screen_width, screen_height);
	glutCreateWindow("CS-6533 - 3D Object Modeling - Srinivas Piskala Ganesh Babu");

	// Property Enabling
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
	glReadBuffer(GL_BACK);

	// Function Calls
    glewInit();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMove);

    init();
    glutMainLoop();
    return 0;
}