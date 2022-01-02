#include "TextureBuilder.h"
#include "Model_3DS.h"
#include "GLTexture.h"
#include <glut.h>
#include <iostream>
#define DEG2RAD(a) (a * 0.0174532925)
using namespace std;

int WIDTH = 1280;
int HEIGHT = 720;
void handleCollision();

GLuint tex;
char title[] = "Run Forest Run";

// 3D Projection Options
GLdouble fovy = 45.0;
GLdouble aspectRatio = (GLdouble)WIDTH / (GLdouble)HEIGHT;
GLdouble zNear = 0.1;
GLdouble zFar = 100;

class Vector3f
{
public:
	float x, y, z;

	Vector3f(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f)
	{
		x = _x;
		y = _y;
		z = _z;
	}

	Vector3f operator+(Vector3f v)
	{
		return Vector3f(x + v.x, y + v.y, z + v.z);
	}

	Vector3f operator-(Vector3f v)
	{
		return Vector3f(x - v.x, y - v.y, z - v.z);
	}

	Vector3f operator*(float n)
	{
		return Vector3f(x * n, y * n, z * n);
	}

	Vector3f operator/(float n)
	{
		return Vector3f(x / n, y / n, z / n);
	}

	Vector3f unit()
	{
		return *this / sqrt(x * x + y * y + z * z);
	}

	Vector3f cross(Vector3f v)
	{
		return Vector3f(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
	}
};

class Camera
{
public:
	Vector3f eye, center, up;

	Camera(float eyeX = 23.5f, float eyeY = 6.5f, float eyeZ = 0.0f, float centerX = 22.5f, float centerY = 6.2f, float centerZ = 0.0f, float upX = -0.3f, float upY = 0.956561f, float upZ = 0.0f)
	{
		eye = Vector3f(eyeX, eyeY, eyeZ);
		center = Vector3f(centerX, centerY, centerZ);
		up = Vector3f(upX, upY, upZ);
	}

	void moveX(float d)
	{
		Vector3f right = up.cross(center - eye).unit();
		eye = eye + right * d;
		center = center + right * d;
	}

	void moveY(float d)
	{
		eye = eye + up.unit() * d;
		center = center + up.unit() * d;
	}

	void moveZ(float d)
	{
		Vector3f view = (center - eye).unit();
		eye = eye + view * d;
		center = center + view * d;
	}

	void rotateX(float a)
	{
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + up * sin(DEG2RAD(a));
		up = view.cross(right);
		center = eye + view;
	}

	void rotateY(float a)
	{
		Vector3f view = (center - eye).unit();
		Vector3f right = up.cross(view).unit();
		view = view * cos(DEG2RAD(a)) + right * sin(DEG2RAD(a));
		right = view.cross(up);
		center = eye + view;
	}

	void look()
	{
		gluLookAt(
			eye.x, eye.y, eye.z,
			center.x, center.y, center.z,
			up.x, up.y, up.z);
	}
};

Camera camera;

// Model Variables
Model_3DS model_player;
Model_3DS model_tree;
Model_3DS model_bridge;
Model_3DS model_treetunnel;
Model_3DS model_logs;
Model_3DS model_stones;
Model_3DS model_bush;
Model_3DS model_redpowerup;
Model_3DS model_orangepowerup;
Model_3DS model_car;

// Textures
GLTexture tex_ground;
GLTexture tex_bridge;
GLTexture tex_desert;

float bush1x = 0.0;
float bush2x = -40.0;
float tree1x = -20.0;
float tree2x = -60.0;
float bridgex = -80.0;
float bush3x = -80.0;
float tree3x = -100.0;
float bush4x = -120.0;
float tree4x = -140.0;
float bush5x = -150.0;
float playery = 0;
float redpowerupx = -70.0;
float orangepowerupx = -50.0;
bool jump = false;
int jumpTimer = 0;
int score = 0;
int LevelTimer = 178;
int lives = 5;
bool Level2 = true;
bool firstview = false;
bool gameOver = false;
//=======================================================================
// Lighting Configuration Function
//=======================================================================
void InitLightSource()
{
	// Enable Lighting for this OpenGL Program
	glEnable(GL_LIGHTING);

	// Enable Light Source number 0
	// OpengL has 8 light sources
	glEnable(GL_LIGHT0);

	// Define Light source 0 ambient light
	GLfloat ambient[] = { 0.1f, 0.1f, 0.1, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

	// Define Light source 0 diffuse light
	GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);

	// Define Light source 0 Specular light
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	// Finally, define light source 0 position in World Space
	GLfloat light_position[] = { 0.0f, 10.0f, 0.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

void setupCamera()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90, 1200 / 800, 0.0001, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera.look();
}

//=======================================================================
// Material Configuration Function
//======================================================================
void InitMaterial()
{
	// Enable Material Tracking
	glEnable(GL_COLOR_MATERIAL);

	// Sich will be assigneet Material Properties whd by glColor
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	// Set Material's Specular Color
	// Will be applied to all objects
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);

	// Set Material's Shine value (0->128)
	GLfloat shininess[] = { 96.0f };
	glMaterialfv(GL_FRONT, GL_SHININESS, shininess);
}

//=======================================================================
// OpengGL Configuration Function
//=======================================================================
void myInit(void)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();

	gluPerspective(fovy, aspectRatio, zNear, zFar);
	//*******************************************************************************************//
	// fovy:			Angle between the bottom and top of the projectors, in degrees.			 //
	// aspectRatio:		Ratio of width to height of the clipping plane.							 //
	// zNear and zFar:	Specify the front and back clipping planes distances from camera.		 //
	//*******************************************************************************************//

	glMatrixMode(GL_MODELVIEW);

	glLoadIdentity();
	setupCamera();
	//gluLookAt(Eye.x, Eye.y, Eye.z, At.x, At.y, At.z, Up.x, Up.y, Up.z);
	//*******************************************************************************************//
	// EYE (ex, ey, ez): defines the location of the camera.									 //
	// AT (ax, ay, az):	 denotes the direction where the camera is aiming at.					 //
	// UP (ux, uy, uz):  denotes the upward orientation of the camera.							 //
	//*******************************************************************************************//

	InitLightSource();

	InitMaterial();

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
}

void print1(float x, float y, char* string)
{
	int len, i;
	len = 20;
	glRasterPos2f(x, y);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, string[i]);
	}
}

void Timer(int value)
{
	LevelTimer--;
	if (jump && !firstview) {
		jumpTimer++;
		if (jumpTimer == 6) {
			playery = 0;
			jump = false;
			jumpTimer = 0;
		}
	}
	if (jump && firstview) {
		jumpTimer++;
		camera.eye.x = 18.0442;
		camera.eye.y = 5.55441;
		camera.eye.z = 0;
		camera.center.x = 17.1286;
		camera.center.y = 5.15244;
		camera.center.z = 0;
		camera.up.x = -0.401961;
		camera.up.y = 0.915657;
		camera.up.z = 0;
		if (jumpTimer == 6) {
			playery = 0;
			jump = false;
			jumpTimer = 0;
			camera.eye.x = 18.8482;
			camera.eye.y = 3.72311;
			camera.eye.z = 0;
			camera.center.x = 17.9325;
			camera.center.y = 3.32113;
			camera.center.z = 0;
			camera.up.x = -0.401981;
			camera.up.y = 0.915648;
			camera.up.z = 0;
		}
	}
	if (LevelTimer == 0) {
		LevelTimer = 178;
		Level2 = !Level2;
		bush1x = 0.0;
		bush2x = -40.0;
		tree1x = -20.0;
		tree2x = -60.0;
		bridgex = -80.0;
		orangepowerupx = -50.0;
		redpowerupx = -70.0;
		bush3x = -80.0;
		tree3x = -100.0;
		bush4x = -120.0;
		tree4x = -140.0;
		bush5x = -150.0;
	}
	score += 5;
	bush1x += 1.0;
	bush2x += 1.0;
	tree1x += 1.0;
	tree2x += 1.0;
	bridgex += 1.0;
	bush3x += 1.0;
	tree3x += 1.0;
	bush4x += 1.0;
	tree4x += 1.0;
	bush5x += 1.0;
	redpowerupx += 1.0;
	orangepowerupx += 1.0;
	handleCollision();
	if(!gameOver)
		glutPostRedisplay();
	if (Level2)
		glutTimerFunc(100, Timer, 0);
	else
		glutTimerFunc(200, Timer, 0);
}

//=======================================================================
// Render Ground Function
//=======================================================================
void RenderGround(int level)
{
	glDisable(GL_LIGHTING);	// Disable lighting 

	glColor3f(0.6, 0.6, 0.6);	// Dim the ground texture a bit

	glEnable(GL_TEXTURE_2D);	// Enable 2D texturing
	if (level == 1) {
		glBindTexture(GL_TEXTURE_2D, tex_ground.texture[0]);
	}// Bind the ground texture
	else {
		glBindTexture(GL_TEXTURE_2D, tex_desert.texture[0]);
	}

	glPushMatrix();
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);	// Set quad normal direction.
	glTexCoord2f(0, 0);		// Set tex coordinates ( Using (0,0) -> (5,5) with texture wrapping set to GL_REPEAT to simulate the ground repeated grass texture).
	glVertex3f(-20, 0, -10);
	glTexCoord2f(5, 0);
	glVertex3f(20, 0, -10);
	glTexCoord2f(5, 5);
	glVertex3f(20, 0, 10);
	glTexCoord2f(0, 5);
	glVertex3f(-20, 0, 10);
	glEnd();
	glPopMatrix();

	glEnable(GL_LIGHTING);	// Enable lighting again for other entites coming throung the pipeline.

	glColor3f(1, 1, 1);	// Set material back to white instead of grey used for the ground texture.
}

void RenderBridge(float pos) {

	glPushMatrix();
	glTranslatef(pos, -0.3, 1.5);
	glScalef(1, 1, 0.7);
	glRotatef(90.f, 0, 1, 0);
	glRotatef(-0.5, 0, 0, 1);
	model_bridge.Draw();
	glPopMatrix();

}

void RenderLogs(float pos) {
	glPushMatrix();
	glTranslatef(pos, 0, 0);
	glScalef(0.5, 0.5, 0.5);
	model_logs.Draw();
	glPopMatrix();
}

void RenderBush(float pos) {
	glPushMatrix();
	glTranslatef(pos, 0, 0);
	glScalef(0.5, 1, 0.8);
	model_bush.Draw();
	glPopMatrix();
}

void RenderStones(float pos) {
	glPushMatrix();
	glTranslatef(pos, 0, 0.8);
	glScalef(0.07, 0.07, 0.07);
	model_stones.Draw();
	glPopMatrix();
}

void RenderPlayer() {
	glPushMatrix();
	glTranslatef(19, playery, 0);
	glScalef(0.002, 0.002, 0.002);
	glRotatef(-90, 0, 1, 0);
	model_player.Draw();
	glPopMatrix();
}

void RenderCar(float pos) {
	glPushMatrix();
	glTranslatef(pos, 0.75, 7);
	glScalef(0.3, 0.3, 0.3);
	glRotatef(8, 1, 0, 0);
	model_car.Draw();
	glPopMatrix();
}

void RenderRedPowerUp(float pos) {
	glPushMatrix();
	glTranslatef(pos, 5, 0);
	glScalef(0.15, 0.175, 0.15);
	model_redpowerup.Draw();
	glPopMatrix();
}

void RenderOrangePowerUp(float pos) {
	glPushMatrix();
	glTranslatef(pos, 5, 0);
	glScalef(0.15, 0.175, 0.15);
	model_orangepowerup.Draw();
	glPopMatrix();
}

//=======================================================================
// Display Function
//=======================================================================
void myDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setupCamera();

	if (!Level2) {
		GLfloat lightIntensity[] = { 0.7, 0.7, 0.7, 1.0f };
		GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

		// Draw Ground
		RenderGround(1);
		//model_powerup.Draw();

		RenderPlayer();

		RenderBridge(bridgex);

		RenderBush(bush1x);
		RenderBush(bush2x);
		RenderLogs(tree1x);
		RenderLogs(tree2x);
		RenderBush(bush4x);
		RenderLogs(tree3x);
		RenderLogs(tree4x);


		RenderOrangePowerUp(orangepowerupx);


		//sky box
		glPushMatrix();

		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glTranslated(50, 0, 0);
		glRotated(90, 1, 0, 1);
		glBindTexture(GL_TEXTURE_2D, tex);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 100, 100, 100);
		gluDeleteQuadric(qobj);


		glPopMatrix();

		glColor3d(0, 1, 0);
		char* sc[20];
		sprintf((char*)sc, "Score: %d", score);
		print1(-3, 12, (char*)sc);

		glColor3d(0, 1, 0);
		char* sc2[20];
		sprintf((char*)sc2, "Lives: %d", lives);
		print1(-3, 10, (char*)sc2);

		if (lives == 0) {
			glColor3d(0, 1, 0);
			char* sc3[20];
			sprintf((char*)sc3, "Game Over!");
			print1(0, 14, (char*)sc3);
			gameOver = true;
		}

		glutSwapBuffers();
	}
	else {
		GLfloat lightIntensity[] = { 0.01, 0.01, 0.01, 1.0f };
		GLfloat lightPosition[] = { 0.0f, 100.0f, 0.0f, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
		glLightfv(GL_LIGHT0, GL_AMBIENT, lightIntensity);

		//Draw Ground
		RenderGround(2);
		//Draw Player
		RenderPlayer();
		RenderStones(bush1x);
		RenderCar(tree1x);
		RenderStones(bush2x);
		RenderCar(tree2x);
		RenderStones(bush3x);
		RenderCar(tree3x);
		RenderStones(bush4x);
		RenderCar(tree4x);
		RenderStones(bush5x);
		RenderRedPowerUp(redpowerupx);
		
		//sky box
		glPushMatrix();

		GLUquadricObj* qobj;
		qobj = gluNewQuadric();
		glTranslated(50, 0, 0);
		glRotated(90, 1, 0, 1);
		glBindTexture(GL_TEXTURE_2D, tex);
		gluQuadricTexture(qobj, true);
		gluQuadricNormals(qobj, GL_SMOOTH);
		gluSphere(qobj, 100, 100, 100);
		gluDeleteQuadric(qobj);


		glPopMatrix();

		glColor3d(0, 1, 0);
		char* sc[20];
		sprintf((char*)sc, "Score: %d", score);
		print1(-3, 12, (char*)sc);

		glColor3d(0, 1, 0);
		char* sc2[20];
		sprintf((char*)sc2, "Lives: %d", lives);
		print1(-3, 10, (char*)sc2);

		if (lives == 0) {
			glColor3d(0, 1, 0);
			char* sc3[20];
			sprintf((char*)sc3, "Game Over!");
			print1(0, 14, (char*)sc3);
			gameOver = true;
		}

		glutSwapBuffers();
	}
}

//=======================================================================
// Keyboard Function
//=======================================================================
void myKeyboard(unsigned char button, int x, int y)
{
	float d = 1.0;
	switch (button)
	{


	case ' ':
		playery = 2.5;
		jump = true;
		break;
	case '1': //1st Person View
		firstview = true;
		camera.eye.x = 18.8482;
		camera.eye.y = 3.72311;
		camera.eye.z = 0;
		camera.center.x = 17.9325;
		camera.center.y = 3.32113;
		camera.center.z = 0;
		camera.up.x = -0.401981;
		camera.up.y = 0.915648;
		camera.up.z = 0;
		break;
	case '3': //3rd Person View
		firstview = false;
		camera.eye.x = 23.5;
		camera.eye.y = 6.5;
		camera.eye.z = 0;
		camera.center.x = 22.5;
		camera.center.y = 6.2;
		camera.center.z = 0;
		camera.up.x = -0.3;
		camera.up.y = 0.956561;
		camera.up.z = 0.0;
		break;
	case 'f':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'r':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 'w': //Camera Up
		camera.moveY(d);
		break;
	case 's': //Camera Down
		camera.moveY(-d);
		break;
	case 'a': //Camera Left
		camera.moveX(d);
		break;
	case 'd': //Camera Right
		camera.moveX(-d);
		break;
	case 'q': //Zoom In
		camera.moveZ(d);
		break;
	case 'e': //Zoom Out
		camera.moveZ(-d);
		break;
	case 27:
		exit(0);
		break;
	default:
		break;
	}
	//cout << "Eye x: " << camera.eye.x << " y: " << camera.eye.y << " z: " << camera.eye.z << "\n Up x: " << camera.up.x << " y: " << camera.up.y << " z: " << camera.up.z << "\n Center x: " << camera.center.x << " y: " << camera.center.y << " z: " << camera.center.z;

	glutPostRedisplay();
}

void Special(int key, int x, int y)
{
	float a = 1.0;

	switch (key)
	{
	case GLUT_KEY_UP: //Camera Angle Up
		camera.rotateX(a);
		break;
	case GLUT_KEY_DOWN: //Camera Angle Down
		camera.rotateX(-a);
		break;
	case GLUT_KEY_LEFT: //Camera Angle Left
		camera.rotateY(a);
		break;
	case GLUT_KEY_RIGHT: //Camera Angle Right
		camera.rotateY(-a);
		break;
	}
	glutPostRedisplay();
}

//=======================================================================
// Assets Loading Function
//=======================================================================
void LoadAssets()
{
	// Loading Model files
	model_player.Load("models/player/playerworking.3ds");
	model_tree.Load("models/tree/tree1.3ds");
	model_bush.Load("models/bush/bush.3ds");
	model_orangepowerup.Load("models/powerup/orangepowerup.3ds");
	model_redpowerup.Load("models/powerup/redpowerup.3ds");
	model_bridge.Load("models/bridge/stonebridge.3DS");
	model_logs.Load("models/log/logs.3ds");
	model_stones.Load("models/stones/stones.3ds");
	model_car.Load("models/car/car2.3ds");

	// Loading texture files
	tex_ground.Load("Textures/ground.bmp");
	tex_desert.Load("textures/desert.bmp");
	loadBMP(&tex, "Textures/blu-sky-3.bmp", true);
}

//=======================================================================
// Main Function
//=======================================================================
void main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(WIDTH, HEIGHT);

	glutInitWindowPosition(100, 150);

	glutCreateWindow(title);

	glutDisplayFunc(myDisplay);

	glutKeyboardFunc(myKeyboard);

	/*glutMotionFunc(myMotion);

	glutMouseFunc(myMouse);

	glutReshapeFunc(myReshape);*/
	glutSpecialFunc(Special);

	glutTimerFunc(0, Timer, 0);


	myInit();

	LoadAssets();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}

void deductLives() {
	if (lives > 0){
		lives--; 
		score -= 50;
	}
}

void handleCollision() {
	if (bush1x >= 18.5 && bush1x <= 20 && playery != 2.5) {
		cout << "Collision";
		deductLives();
		bush1x += 1.5;
	}
	if (bush2x >= 18.5 && bush2x <= 20 && playery != 2.5) {
		cout << "Collision";
		deductLives();
		bush2x += 1.5;
	}
	if (bush3x >= 18.5 && bush3x <= 20 && playery != 2.5 && Level2) {
		cout << "Collision";
		deductLives();
		bush3x += 1.5;
	}
	if (bush4x >= 18.5 && bush4x <= 20 && playery != 2.5) {
		cout << "Collision";
		deductLives();
		bush4x += 1.5;
	}
	if (bush5x >= 18.5 && bush5x <= 20 && playery != 2.5 && Level2) {
		cout << "Collision";
		deductLives();
		bush5x += 1.5;
	}
	if (tree1x >= 18.5 && tree1x <= 20 && playery != 2.5) {
		cout << "Collision";
		deductLives();
		tree1x += 1.5;
	}
	if (tree2x >= 18.5 && tree2x <= 20 && playery != 2.5) {
		cout << "Collision";
		deductLives();
		tree2x += 1.5;
	}
	if (tree3x >= 18.5 && tree3x <= 20 && playery != 2.5) {
		cout << "Collision";
		deductLives();
		tree3x += 1.5;
	}
	if (tree4x >= 18.5 && tree4x <= 20 && playery != 2.5) {
		cout << "Collision";
		deductLives();
		tree4x += 1.5;

	}
	if (bridgex >= 17 && bridgex <= 21 && playery == 2.5 && !Level2) {
		cout << "Collision";
		deductLives();
		bridgex += 4;
	}
}