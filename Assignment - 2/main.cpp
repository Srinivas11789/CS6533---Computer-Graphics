#include "glsupport.h"
#include <GL/freeglut.h>
#include "geometrymaker.h"
#include "matrix4.h"
#include "quat.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define NULL 0

// Buffer Objects 
GLuint vertexBO, vertexBO1;
GLuint indexBO, indexBO1;

// Program Handle for the Shader Programs
GLuint program;

// Attributes for Position and Color
GLuint positionAttribute;
GLuint normalAttribute;

//Uniform Location Variables

GLuint modelviewMatrixUniformLocation;
GLuint projectionMatrixUniformLocation;
GLuint normalMatrixUniformLocation;
GLuint colorUniformLocation;

//Indices
int numIndices;

// Vertex Structure for Position and Normal
struct VertexPN {
	Cvec3f p;
	Cvec3f n;
	VertexPN() {}
	VertexPN(float x, float y, float z, float nx, float ny, float nz) : p(x, y, z), n(nx, ny, nz) {}

	VertexPN& operator = (const GenericVertex& v) {
		p = v.pos;
		n = v.normal;
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
	void Draw(GLuint positionAttribute, GLuint normalAttribute, string type) {
		// bind buffer objects and draw
		if (type == "cube") {
			vertex = vertexBO;
			index = indexBO;
		}
		else if (type == "sphere") {
			vertex = vertexBO1;
			index = indexBO1;
		}
		else {
			cout << "No Type Matched";
		}
		glBindBuffer(GL_ARRAY_BUFFER, vertex);
		glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, p));
		glEnableVertexAttribArray(positionAttribute);
		glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, n));
		glEnableVertexAttribArray(normalAttribute);
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

	void Draw(Matrix4 eyeInverse, GLuint positionAttribute, GLuint normalAttribute,
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
		geometry.Draw(positionAttribute, normalAttribute, type);
	}
};

// Cube Function to Draw the Cube from the Geometry Maker Header File
void cube() {

	    // Cube Parameters
	    int ibLen, vbLen;
		getCubeVbIbLen(vbLen, ibLen);
		std::vector<VertexPN> vtx(vbLen);
		std::vector<unsigned short> idx(ibLen);
		makeCube(2, vtx.begin(), idx.begin());

		glGenBuffers(1, &vertexBO);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx.size(), vtx.data(), GL_STATIC_DRAW);
		glGenBuffers(1, &indexBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx.size(), idx.data(), GL_STATIC_DRAW);
		numIndices = idx.size();
		
	}

void sphere() {

	//Sphere Parameters
	int ibLen1, vbLen1;
	getSphereVbIbLen(10, 10, vbLen1, ibLen1);
	std::vector<VertexPN> vtx1(vbLen1);
	std::vector<unsigned short> idx1(ibLen1);
	makeSphere(1.0, 10, 10, vtx1.begin(), idx1.begin());
	glGenBuffers(1, &vertexBO1);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx1.size(), vtx1.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &indexBO1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx1.size(), idx1.data(), GL_STATIC_DRAW);
	numIndices = idx1.size();
}

// Rendering Function which creates the Eye Matrix and maintains each object parameters
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
	glClearDepth(0.0f);
	glEnable(GL_DEPTH_TEST);
	glUniform3f(colorUniformLocation, 0.0, 0.0, 5.0);
	
	//Eye Matrix Component
	Matrix4 eye;
	eye = eye.makeTranslation(Cvec3(0.0, 0.0, 9.0));
	eye = inv(eye);

	string type = "cube";
	Entity *parent;
	parent = new Entity();
	parent->transform.rotation = Quat::makeYRotation((GLfloat)glutGet(GLUT_ELAPSED_TIME) / 50.0f) * Quat::makeXRotation((GLfloat)glutGet(GLUT_ELAPSED_TIME) / 50.0f);
	parent->transform.position = Cvec3(0.0, 0.0, 0.0);
	parent->transform.scale = Cvec3(0.8, 0.8, 0.8);
	parent->parent = NULL;
	parent->Draw( eye, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalMatrixUniformLocation, type);

	glUniform3f(colorUniformLocation, 5.0, 0.0, 0.0);
	Entity *child;
	child = new Entity();
	child->transform.rotation = Quat::makeZRotation(60.0f);
	child->transform.position = Cvec3(0.0, 0.0, 3.0);
	child->transform.scale = Cvec3(0.7, 0.7, 0.7);
	child->parent = parent;
	child->Draw(eye, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalMatrixUniformLocation, type);

	glUniform3f(colorUniformLocation, 0.0, 5.0, 0.0);
	Entity *child1;
	child1 = new Entity();
	child1->transform.rotation = Quat::makeXRotation(40.0f);
	child1->transform.position = Cvec3(0.0, 0.0, 3.0);
	child1->transform.scale = Cvec3(0.5, 0.5, 0.5);
	child1->parent = child;
	child1->Draw(eye, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalMatrixUniformLocation, type);
	
	type = "sphere";
	glUniform3f(colorUniformLocation, 0.6, 0.4, 0.2);
	Entity *child2;
	child2 = new Entity();
	child2->transform.rotation = Quat::makeXRotation((GLfloat)glutGet(GLUT_ELAPSED_TIME) / 10.0f) * Quat::makeYRotation((GLfloat)glutGet(GLUT_ELAPSED_TIME) / 10.0f);
	child2->transform.position = Cvec3(0.0, 3.0, 3.0);
	child2->transform.scale = Cvec3(0.4, 0.4, 0.4);
	child2->parent = parent;
	child2->Draw(eye, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalMatrixUniformLocation, type);

	type = "sphere";
	glUniform3f(colorUniformLocation, 0.6, 0.4, 0.5);
	Entity *child3;
	child3 = new Entity();
	child3->transform.rotation = Quat::makeXRotation((GLfloat)glutGet(GLUT_ELAPSED_TIME) / 10.0f) * Quat::makeYRotation((GLfloat)glutGet(GLUT_ELAPSED_TIME) / 10.0f);
	child3->transform.position = Cvec3(0.0, -3.0, -3.0);
	child3->transform.scale = Cvec3(0.4, 0.4, 0.4);
	child3->parent = parent;
	child3->Draw(eye, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalMatrixUniformLocation, type);

	type = "sphere";
	glUniform3f(colorUniformLocation, 0.6, 0.2, 0.3);
	Entity *child4;
	child4 = new Entity();
	child4->transform.rotation = Quat::makeXRotation((GLfloat)glutGet(GLUT_ELAPSED_TIME) / 10.0f) * Quat::makeYRotation((GLfloat)glutGet(GLUT_ELAPSED_TIME) / 10.0f);
	child4->transform.position = Cvec3(0.0, 2.0, 2.0);
	child4->transform.scale = Cvec3(0.4, 0.4, 0.4);
	child4->parent = child2;
	child4->Draw(eye, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalMatrixUniformLocation, type);

	type = "sphere";
	glUniform3f(colorUniformLocation, 0.7, 1.0, 0.2);
	Entity *child5;
	child5 = new Entity();
	child5->transform.rotation = Quat::makeXRotation((GLfloat)glutGet(GLUT_ELAPSED_TIME) / 10.0f) * Quat::makeYRotation((GLfloat)glutGet(GLUT_ELAPSED_TIME) / 10.0f);
	child5->transform.position = Cvec3(0.0, 2.0, 2.0);
	child5->transform.scale = Cvec3(0.4, 0.4, 0.4);
	child5->parent = child3;
	child5->Draw(eye, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalMatrixUniformLocation, type);

	// Attributes Disable

	glDisableVertexAttribArray(positionAttribute);
	glDisableVertexAttribArray(normalAttribute);

    glutSwapBuffers();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glFlush();
}

void init() {

	// Initialization with BackGround Color

	glClearDepth(0.0f);
	glClearColor(0.2f, 0.3f, 0.3f, 0.0f);

	// Creating an Handle for Shader Programs 

	program = glCreateProgram();
	readAndCompileShader(program, "vertex.glsl", "fragment.glsl");
	glUseProgram(program);

	// Declaring the Position Attribute and Normal Attribute

	positionAttribute = glGetAttribLocation(program, "position");
	normalAttribute = glGetAttribLocation(program, "normal");

	// Getting the Uniform Locations

	modelviewMatrixUniformLocation = glGetUniformLocation(program, "modelViewMatrix");
	projectionMatrixUniformLocation = glGetUniformLocation(program, "projectionMatrix");
	normalMatrixUniformLocation = glGetUniformLocation(program, "normalMatrix");
	colorUniformLocation = glGetUniformLocation(program, "uColor");

	// Drawing the Cube using the Geometry Maker Header File
	cube();
	sphere();

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
    glutInitWindowSize(500, 500);
	glutCreateWindow("CS-6533 - 3D Object Modeling - Srinivas Piskala Ganesh Babu");

	glDepthFunc(GL_GREATER);

	// Property Enabling
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
	glReadBuffer(GL_BACK);

    glewInit();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    
    init();
    glutMainLoop();
    return 0;
}