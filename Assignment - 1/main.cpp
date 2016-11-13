#include "glsupport.h"
#include <GL/freeglut.h>

// Shaders Program Handle
GLuint program;

// Buffer Objects
GLuint vertPositionVBO;
GLuint vertTexCoordVBO;
GLuint vertColorVBO;

// Attributes
GLuint positionAttribute;
GLuint colorAttribute;
GLuint texCoordAttribute;

// Variables to hold the texture images
GLuint emojiTexture;
GLuint diffTexture;

// Uniform Variables
GLuint positionUniform;
GLuint positionUniform1;
GLuint colorUniform;

// Texture Offsets to move the Object
float xtextureOffset = 0.0;
float ytextureOffset = 0.0;
float xtextureColor = 0.0;

// Rendering Function
void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);

	//glUniform2f(positionUniform, xtextureOffset, ytextureOffset);
	glUniform2f(positionUniform1, xtextureOffset, ytextureOffset);

	glUseProgram(program);
	glBindBuffer(GL_ARRAY_BUFFER, vertPositionVBO);
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(positionAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, vertTexCoordVBO);
	glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(texCoordAttribute);

	glBindTexture(GL_TEXTURE_2D, emojiTexture);
	glUniform2f(positionUniform, -0.5, 0.0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindTexture(GL_TEXTURE_2D, diffTexture);
	glUniform2f(positionUniform, 0.5, 0.0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(positionAttribute);
	glDisableVertexAttribArray(texCoordAttribute);
    glutSwapBuffers();

}

void init() {

	program = glCreateProgram();
	readAndCompileShader(program, "vertex.glsl", "fragment.glsl");
	glUseProgram(program);

	positionAttribute = glGetAttribLocation(program, "position");
	colorAttribute = glGetAttribLocation(program, "color");
	texCoordAttribute = glGetAttribLocation(program, "texCoord");

	glGenBuffers(1, &vertPositionVBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertPositionVBO);
	GLfloat sqVerts[12] = {
		-0.7f, -0.5f,
		0.7f, 0.5f,
		0.7f, -0.5f,
		-0.7f, -0.5f,
		-0.7f, 0.5f,
		0.7f, 0.5f
	};
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), sqVerts, GL_STATIC_DRAW);

	glGenBuffers(1, &vertTexCoordVBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertTexCoordVBO);
	GLfloat sqTexCoords[12] = {
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
	};
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), sqTexCoords, GL_STATIC_DRAW);

	emojiTexture = loadGLTexture("batman.png");
	diffTexture = loadGLTexture("spider1.png");

	positionUniform = glGetUniformLocation(program, "modelPosition");
	positionUniform1 = glGetUniformLocation(program, "modelPosition1");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.5, 0.5, 0.5, 1.0); // set the clear color
	
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
		xtextureOffset += 0.02;
		break;
	case 'd':
		xtextureOffset -= 0.02;
		break;
	case 'w':
		ytextureOffset += 0.02;
		break;
	case 's':
		ytextureOffset -= 0.02;
		break;
	}
}


int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(300, 300);
    glutCreateWindow("Graphics Assignment -> 1 -> Srinivas Piskala Ganesh Babu");

    glewInit();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
 
	glutKeyboardFunc(keyboard);

    init();
    glutMainLoop();
    return 0;
}
