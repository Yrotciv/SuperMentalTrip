/*
				Computer Graphics - Final Project

		Students:	Filippo Lapide
					Vittoriofranco Vagge

		Project name: SuperMentalTrip

*/


#include <iostream>
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "audiere.h"
using namespace audiere;

// Funzioni audio
AudioDevicePtr device(OpenDevice());
OutputStreamPtr stream(OpenSound(device, "../data/Race_Car.mp3", true));
OutputStreamPtr gate(OpenSound(device, "../data/gate.mp3", false));
OutputStreamPtr over(OpenSound(device, "../data/over.mp3", false));


#define WINDOW_WIDTH 1366	//larghezza schermata di gioco
#define WINDOW_HEIGHT 768 	//altezza schermata di gioco
#define TIMER_PERIOD 25 	//periodo che regola il tempo di scalatura degli esagoni
#define SCALE_SPEED 1.020 	//l'aumento di scala applicata agli ottagoni in ogni fotogramma
#define OCTAGON_DELAY 1250 	//la quantità di millisecondi che trascorre tra ogni ottagono
#define COLOR_CHANGE true 	//true per cambiare colore, false per esecusione monocromata di default
typedef struct {
	float r, g, b;
} color_t;

typedef struct {
	color_t color;	 //generatore randomico di colori r,g,b
	float scale;	 //scalatura dell'ottagono, che aumenta al trascorrere del tempo
	int missingPart; //la parte mancante dell'ottagono così da permettere il giocatore di attraversarlo
} octagon_t;

typedef struct {
	bool isStarted, //controlla se il gioco sia iniziato o finito
		pause,		//controlla se il gioco sia in pausa
		animate;
} game_control_t;

typedef struct {
	int current,	//punteggio corrente
		max = -1;	//miglior punteggio
} score_t;

score_t score;			//mantiene il punteggio attuale e il migliore
int width, height;		//variabili utili per ridimensionare lo schermo 
game_control_t game;	//controlla lo stato del gioco
octagon_t octagons[4];	//numero massimo di ottagoni che compaiono a schermo 
int input = 0;	   		//lo schermo è diviso in 8 parti e il giocatore usa i tasti delle frecce per spostarsi modificando il valore di questa variabile
int timerCount = 0;		//tiene il conto di quante volte è stata eseguita la funzione del timer
color_t background;		//colore del background

float maxScale; 	//oltre questo valore la scalatura dell'ottagono sarà inizializzata al valore initialScale
float initialScale; //

float rotation;		//rotazione di tutti gli oggetti,ottagono e giocatore, i quali cambiano nel tempo
float rotateSpeed;	//velocità di rotazione, che cambia casualmente tra 1 e 3

float scale;	//scala dell'intero schermo, usata per l'animazione iniziale e per il gioco in generale

void initializeGlobals()	//funzione di inizializzazione
{
	input = 0;
	scale = 1;
	score.current = 0;
	game.isStarted = false;
	game.pause = false;
	game.animate = false;
	rotateSpeed = (rand() % 100) / 50.0 + 1;
	rotation = 0;
	gate->setVolume(0.5f);
	stream->setVolume(0.3f);
	over->setVolume(0.5f);
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

void circle(int x, int y, int r)	//funzione per disegnare il giocatore centrato a (x,y) con raggio r
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

void drawString(const char* string)	//font stringhe
{
	glPushMatrix();
	while (*string)
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *string++);
	glPopMatrix();
}

void displayBackground()		//funzione che regola la suddivisione e la rotazione dello sfondo
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

void displayOctagons() //funzione per disegnare gli Octagons, se si vuole cambiare forma QUI
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
	circle(0, 0, 7);
	glPopMatrix();
}

void displayUI()
{
	glPushMatrix();
	if (!game.isStarted)
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
		drawString("F1 reset");
	}
	glPopMatrix();

}

//
// Funzioni per disegnare scritte e UI in OPENGL
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
	displayOctagons();
	displayPlayer();
	displayUI();

	glutSwapBuffers();
}

//
// Tasto ESC per uscire dal gioco 
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
// Tasto < > per muovere l'input, F2 per mettere in pausa, F1 per restart
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
	if (key == GLUT_KEY_F1)
	{
		initializeGlobals();
		game.animate = true;
		scale = 9;

	}
	else if (key == GLUT_KEY_F2)
		game.pause = !game.pause;
}


void SpecialKeyUp(int key, int x, int y)
{
}

//
// Funzione chiamata quando cambiano le dimensioni della finestra
// w : nuova width della finestra in pixels.
// h : nuova height della finestra in pixels.
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

	//Animazione iniziale del gioco
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
					over->play();
					game.isStarted = false;
					if (score.current > score.max)
						score.max = score.current;
				}
				else {
					score.current++;
					if (gate->isPlaying()) gate->reset();
					else gate->play();
				}
			octagons[i].scale *= SCALE_SPEED;
			if (octagons[i].scale >= maxScale)
			{
				octagons[i].scale = initialScale;
			}
		}
		if (timerCount % 25 == 0 && COLOR_CHANGE)	//Cambio colore degli elementi della UI
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
	//Inizializzazione
	initializeGlobals();
	srand(time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("SuperMentalTrip");
	// Funzioni resize finestra
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	// Funzioni tastiera
	glutKeyboardFunc(ASCIIKeyDown);
	glutKeyboardUpFunc(ASCIIKeyUp);
	// Funzioni audio
	if (!device) {
		return;
	}
	stream->setRepeat(true);
	stream->play();

	glutSpecialFunc(SpecialKeyDown);
	glutSpecialUpFunc(SpecialKeyUp);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);

	glutMainLoop();
}