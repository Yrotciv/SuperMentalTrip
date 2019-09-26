// SuperMentalTrip.cpp : Questo file contiene la funzione 'main', in cui inizia e termina l'esecuzione del programma.
//

#include <iostream>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define WINDOW_WIDTH 1366
#define WINDOW_HEIGHT 768
#define TIMER_PERIOD 25
#define SCALE_SPEED 1.020 //the scale amount applied to the octagons in every frame
#define OCTAGON_DELAY 1250 //the amount of miliseconds passed between octagons
#define COLOR_CHANGE true //true for repeatedly color change, false for no color change
typedef struct {
	float r, g, b;
} color_t;

typedef struct {
	color_t color;	 //randomly generated r,g,b color
	float scale;	 //scale of the octagon, which increases in time
	int missingPart; //the empty part of the octagon so that player can pass through
} octagon_t;

typedef struct {
	bool isStarted, //checks if the game is started or is over
		pause,		//checks if the game is paused
		animate;
} game_control_t;

typedef struct {
	int current,
		max = -1;
} score_t;

score_t score; //keeps current and the maximum score
int width, height;
game_control_t game;
octagon_t octagons[4];
int input = 0;	   //the screen is splitted into 6 parts and as players uses arrow keys, the value of this variable changes accordingly
int timerCount = 0;//counts how many times timer function runned
color_t background;//background color

float maxScale; //after this scale, the scale of the octagons will be initialized to initialScale
float initialScale; //

float rotation;	   //rotation of all objects, octagons and players, which changes in time
float rotateSpeed;//rotation speed, which randomly switch between 1 and 3

float scale;	  //scale of the whole screen, used in the initial animation and in game in general
void initializeGlobals()
{
	input = 0;
	scale = 1;
	score.current = 0;
	game.isStarted = true;
	game.pause = false;
	game.animate = false;
	rotateSpeed = (rand() % 100) / 50.0 + 1;
	rotation = 0;
	float frameNeeded = OCTAGON_DELAY / TIMER_PERIOD;
	float scaleMultiplier = powf(SCALE_SPEED, frameNeeded);
	float octagonScales[4];
	octagonScales[3] = 0.2;
	for (int i = 2; i >= 0; i--)
		octagonScales[i] = octagonScales[i + 1] / scaleMultiplier;
	initialScale = octagonScales[2];
	maxScale = octagonScales[3] * pow(scaleMultiplier, 3);
	for (int i = 0; i < 4; i++)
	{
		octagons[i].color = { rand() % 100 / 200.0f + 0.5f, rand() % 100 / 200.0f + 0.5f,rand() % 100 / 200.0f + 0.5f };
		octagons[i].scale = octagonScales[i];
		octagons[i].missingPart = rand() % 6;
	}
	background = { rand() % 100 / 300.0f, rand() % 100 / 300.0f ,rand() % 100 / 300.0f };
}

//
// to draw circle, center at (x,y)
//  radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
	float angle;

	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI * i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void drawString(const char* string)
{
	glPushMatrix();
	while (*string)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *string++);
	glPopMatrix();
}

void displayBackground()
{
	glColor4f(0, 0, 0, 0.1);
	for (int i = 0; i < 4; i++)
	{
		glPushMatrix();
		glRotatef(i * 90.0 + rotation, 0, 0, 1);
		glBegin(GL_TRIANGLES);
		glVertex2f(0, 0);
		glVertex2f(-1500 / sqrt(4), -1500);
		glVertex2f(1500 / sqrt(4), -1500);
		glEnd();
		glPopMatrix();
	}
}

void displayHexagons()
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (j == octagons[i].missingPart)
				continue;
			glColor3f(octagons[i].color.r, octagons[i].color.g, octagons[i].color.b);
			glPushMatrix();
			glScalef(octagons[i].scale, octagons[i].scale, 0);
			glRotatef(j * 45.0 + rotation, 0, 0, 1);
			glTranslatef(0, -100 * sqrt(3), 0);
			glRectf(-75, 0, 75, -5);
			glPopMatrix();
		}
	}
}

void displayPlayer()
{
	glPushMatrix();
	glColor3f(1, 1, 1);
	glRotatef(input * 45.0 + rotation, 0, 0, 1);
	glTranslatef(0, -200, 0);
	circle(0, 0, 5);
	glPopMatrix();
}

void displayUI()
{
	glPushMatrix();
	//if (!game.isStarted)
	if (false)
	{
		glColor4f(0, 0, 0, 0.7);
		glRectf(-300, -100, 300, 100);
		glColor3f(1, 1, 1);
		glTranslatef(-150, 0, 0);
		glScalef(0.3, 0.3, 0);
		drawString("F1 per iniziare!");
		glTranslatef(60, -100, 0);
		glScalef(0.5, 0.5, 0);
		drawString("Frecce < > per muoversi");

		if (score.max != -1)
		{
			char str[100];
			glTranslatef(150, -200, 0);
			sprintf_s(str, "Record: %d", score.max);
			drawString(str);
			glTranslatef(0, -150, 0);
			sprintf_s(str, "Ultimo punteggio: %d", score.current);
			drawString(str);
		}

	}
	else
	{
		glTranslatef(-290, 280, 0);
		glScalef(0.1, 0.1, 0);
		if (!game.pause)
			drawString("F2 pausa");
		else
			drawString("F2 start");
		char str[100];
		glPopMatrix();
		glPushMatrix();
		sprintf_s(str, "Punteggio: %d", score.current);
		glTranslatef(230, 280, 0);
		glScalef(0.1, 0.1, 0);
		drawString(str);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(230, -280, 0);
		glScalef(0.1, 0.1, 0);
		//drawString("F1 reset");
	}
	glPopMatrix();

}

//
// To display onto window using OpenGL commands
//
void display()
{
	glClearColor(background.r, background.g, background.b, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glLoadIdentity();
	glScalef(scale, scale, 0);
	if (game.animate)
		glRotatef(rotation, 0, 0, 1);

	displayBackground();
	displayHexagons();
	displayPlayer();
	displayUI();

	glutSwapBuffers();
}

//
// key function for ASCII charachters ESC
//
void ASCIIKeyDown(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
}

void ASCIIKeyUp(unsigned char key, int x, int y)
{
}


//
// Special Key like F1, F2, F3, Arrow Keys, Page UP, ...
//
void SpecialKeyDown(int key, int x, int y)
{
	if (game.isStarted && !game.pause)
	{
		switch (key) {
		case GLUT_KEY_LEFT:
			input = (input + 7) % 8;
			break;
		case GLUT_KEY_RIGHT:
			input = (input + 1) % 8;
			break;
		}
	}
	//if (key == GLUT_KEY_F1)
	//{
	//	initializeGlobals();
	//	game.animate = true;
	//	scale = 9;

	//}
	else if (key == GLUT_KEY_F2)
		game.pause = !game.pause;
}


void SpecialKeyUp(int key, int x, int y)
{
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void reshape(int w, int h)
{
	width = w;
	height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void onTimer(int v) {
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	//initial animation of the game
	if (game.animate)
	{
		timerCount++;
		scale -= 0.1;
		rotation += 9;
		if (scale <= 1)
		{
			scale = 1;
			rotation = 0;
			game.animate = false;
			game.isStarted = true;
		}
	}
	if (game.isStarted && !game.pause)
	{
		timerCount++;
		rotation += rotateSpeed;
		for (int i = 0; i < 4; i++)
		{
			if (fabs(octagons[i].scale - 1.130) < 0.01)
				if (input != octagons[i].missingPart)
				{
					game.isStarted = false;
					if (score.current > score.max)
						score.max = score.current;
				}
				else
					score.current++;
			octagons[i].scale *= SCALE_SPEED;
			if (octagons[i].scale >= maxScale)
			{
				octagons[i].scale = initialScale;
			}
		}
		if (timerCount % 25 == 0 && COLOR_CHANGE)
		{
			for (int i = 0; i < 4; i++)
			{
				octagons[i].color = { rand() % 100 / 200.0f + 0.5f, rand() % 100 / 200.0f + 0.5f,rand() % 100 / 200.0f + 0.5f };
			}
			background = { rand() % 100 / 300.0f, rand() % 100 / 300.0f ,rand() % 100 / 300.0f };
			rotateSpeed = (rand() % 100) / 20.0 - 2.5f;
		}
		else if (timerCount % 50 == 0)
		{
			int rnd = rand() % 60 - 30;
			if (rnd == 0)
				rnd = 180;
			rotation += rnd;
			timerCount = 0;
		}
	}
	glutPostRedisplay();
}

void main(int argc, char* argv[])
{
	initializeGlobals();
	srand(time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("SuperMentalTrip");

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	//
	// keyboard registration
	//
	glutKeyboardFunc(ASCIIKeyDown);
	glutKeyboardUpFunc(ASCIIKeyUp);

	glutSpecialFunc(SpecialKeyDown);
	glutSpecialUpFunc(SpecialKeyUp);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);

	glutMainLoop();
}