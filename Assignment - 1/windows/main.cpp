#include "glsupport.h"
#include <GL/freeglut.h>

GLuint program;
GLuint vertPositionVBO;
GLuint positionAttribute;
GLuint colorAttribute;
GLuint vertColorVBO;
GLuint texCoordAttribute;
GLuint vertTexCoordVBO;
GLuint emojiTexture;
GLuint diffTexture;
GLuint timeUniform;
GLuint positionUniform;
float textureOffset = 0.0;

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);

	/*int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	glUniform1f(timeUniform, (float)timeSinceStart / 1000.0f);*/

	glUniform1f(timeUniform, textureOffset);

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

	emojiTexture = loadGLTexture("lego1.png");
	diffTexture = loadGLTexture("spider.png");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.5, 0.5, 0.5, 1.0); // set the clear color
	timeUniform = glGetUniformLocation(program, "time");
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
		textureOffset += 0.02;
		break;
	case 'd':
		textureOffset -= 0.02;
		break;
	}
}

void mouse(int button, int state, int x, int y) {
		float newPositionX = (float)x / 250.0f - 1.0f;
		float newPositionY = (1.0 - (float)y / 250.0);
		glUniform2f(positionUniform, newPositionX, newPositionY);
}
void mouseMove(int x, int y) {
		float newPositionX = (float)x / 250.0f - 1.0f;
		float newPositionY = (1.0 - (float)y / 250.0);
		glUniform2f(positionUniform, newPositionX, newPositionY);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(300, 300);
    glutCreateWindow("Graphics Assignment -> 2");

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
