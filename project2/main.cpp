//    Project 2
//
//    The left mouse button does rotation
//    The middle mouse button does scaling
//    The user interface allows:
//        1. The axes to be turned on and off
//        2. Debugging to be turned on and off
//        3. Depth cueing to be turned on and off
//        4. The projection to be changed
//        5. The transformations to be reset
//        6. The program to quit


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>


#pragma warning(disable:4996)
#include "glew.h"


#include "glut.h"

#include "cessna.hpp"

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"



// title of these windows:
const char *WINDOW_TITLE = "Project 2";

// the escape key:
#define ESCAPE        0x1b

// initial window size:
const int INITIAL_WINDOW_SIZE = 1800;


// multiplication factors for input interaction:
const float ANGLE_FACTOR = 1.0f;
const float SCALE_FACTOR = 0.005f;


// minimum allowable scale factor:
const float SCALE_FACTOR_MINIMUM = 0.05f;


// animation cycle time
const int MS_IN_THE_ANIMATION_CYCLE = 800;


// active mouse buttons (or them together):
const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which view perspective
enum ViewPerspective {
    OUTSIDE,
    INSIDE
};


// which button:
enum ButtonVals {
    RESET,
    QUIT
};

// window background color (rgba):
const GLfloat BACKCOLOR[] = { 0., 0., 0., 1. };


// line width for the axes:
const GLfloat AXES_WIDTH   = { 3. };




// non-constant global variables:
int        ActiveButton;            // current button that is down
GLuint    AxesList;                // list to hold the axes
int        AxesOn;                    // != 0 means to draw the axes
int        DebugOn;                // != 0 means to print debugging info
GLuint    BoxList;                // object display list
GLuint  CessnaList;               // helicopter display list
GLuint  CessnaWireList;           // wireframe helicopter display list
GLuint  CessnaPropellerList;
GLuint  BladeList;              // helicopter blade display list
GLuint  ObjList;                // new object display list
int        MainWindow;                // window id for main graphics window
float    Scale;                    // scaling factor
int        WhichColor;                // index into Colors[ ]
int     WhichViewPerspective;   // OUTSIDE or INSIDE
int        Xmouse, Ymouse;            // mouse values
float    Xrot, Yrot;                // rotation angles in degrees
float   Time;                   // current time elapsed
float   TimeCycle;              // current time in animation cycle
bool    Frozen;                 // sets whether the scene is frozen




// MARK: - Function prototypes

void    Animate();
void    Display();
void    FunkyTargetThingy();
void    createCessnaWireframe();
//void    CESSNAshade();
void    createCessnaPropeller();
void    DoAxesMenu(int);
void    DoColorMenu(int);
void    DoDebugMenu(int);
void    DoMainMenu(int);
void    DoRasterString(float, float, float, char const *);
void    DoStrokeString(float, float, float, float, char const *);
float    ElapsedSeconds();
void    InitGraphics();
void    InitLists();
void    InitMenus();
void    Keyboard(unsigned char, int, int);
void    MouseButton(int, int, int, int);
void    MouseMotion(int, int);
void    Reset();


void    Axes(float);
float   Dot(float v1[3], float v2[3]);
void    Cross(float v1[3], float v2[3], float vout[3]);
float   Unit(float vin[3], float vout[3]);


// MARK: - Function definitions

// main program:
int main(int argc, char *argv[]) {

    glutInit(&argc, argv);
    InitGraphics();
    InitLists();
    Reset();
    InitMenus();
    glutSetWindow(MainWindow);
    glutMainLoop();

    return 0;
}


void Animate() {
    int ms = glutGet(GLUT_ELAPSED_TIME);
    Time = (float)ms / (float)1000;
    ms %= MS_IN_THE_ANIMATION_CYCLE;
    TimeCycle = (float)ms / (float)MS_IN_THE_ANIMATION_CYCLE;    // [0., 1.]
    
    
    // force a call to Display() next time it is convenient:
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}


// draw the complete scene:

void makeShadingFlat() {
    glShadeModel(GL_SMOOTH);
}

void eraseBackground() {
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void centerViewport() {
    GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
    GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
    GLsizei v = vx < vy ? vx : vy;            // minimum dimension
    GLint xl = (vx - v) / 2;
    GLint yb = (vy - v) / 2;
    glViewport(xl, yb,  v, v);
}

void Display() {
    if (DebugOn) {
        fprintf(stderr, "Display\n");
    }
        
    // set which window we want to do the graphics into:
    glutSetWindow(MainWindow);
    
    eraseBackground();
    makeShadingFlat();
    centerViewport();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, 1, 0.1, 1000);
        
    // place the objects into the scene:
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // eye (my eyes?), center (i am looking at this?), up (?)
    if (WhichViewPerspective == INSIDE) {
        gluLookAt(0, 1.8, -5,     0, 0, 10,     0, 0, 10);
    } else {
        gluLookAt(11, 7, 9,     0, 0, 1.6,     0, 1, 0);
                
        // rotate the scene:
        glRotatef((GLfloat)Yrot, 0, 1, 0);
        glRotatef((GLfloat)Xrot, 1., 0, 0);
        
        // uniformly scale the scene:
        if (Scale < SCALE_FACTOR_MINIMUM) {
            Scale = SCALE_FACTOR_MINIMUM;
        }
        
        glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
    }
    
    // possibly draw the axes:
    if (AxesOn) {
        GLfloat const red[3] = {1,0,0};
        glColor3fv(red);
        glCallList(AxesList);
    }
    
    glEnable(GL_NORMALIZE);

    glCallList(CessnaList);
    glCallList(CessnaWireList);

    
    glPushMatrix();
    
    // draw nose propeller
    
    glTranslatef(0.,0.,7.5);
    glScalef(5., 5., 5.);
    glRotatef(360.*TimeCycle, 0., 0., 1.);
    glColor3f(1., 1., 1.);
    glCallList(CessnaPropellerList);
    
    glPopMatrix();
    glPushMatrix();
    
    // draw left propeller
    glTranslatef(-10., 3., 0);
    glScalef(3, 3, 3);
    glRotatef(-2*360.*TimeCycle, 0., 1., 0.);
    glRotatef(90., 0., 0., 0.);
    glCallList(CessnaPropellerList);
    
    glPopMatrix();
    glPushMatrix();
    // draw right propeller
    glTranslatef(10., 3., 0.);
    glScalef(3, 3, 3);
    glRotatef(2*360.*TimeCycle, 0., 1., 0.);
    glRotatef(-90., 0., 0., 0.);
    glCallList(CessnaPropellerList);
    
    glPopMatrix();
    
    FunkyTargetThingy();
 
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0., 100., 0., 100.);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glutSwapBuffers();
    glFlush();
}


void DoAxesMenu(int id) {
    AxesOn = id;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}


void DoDebugMenu(int id) {
    DebugOn = id;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}


// main menu callback:
void DoMainMenu(int id) {
    switch (id) {
        case RESET:
            Reset();
            break;
            
        case QUIT:
            glutSetWindow(MainWindow);
            glFinish();
            glutDestroyWindow(MainWindow);
            exit(0);
            break;
            
        default:
            fprintf(stderr, "Don't know what to do with Menu %d\n", id);
    }
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}


void DoPerspMenu(int id) {
    WhichViewPerspective = id;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}



// return the number of seconds since the start of the program:

float ElapsedSeconds() {
    // get # of milliseconds since the start of the program:
    int ms = glutGet(GLUT_ELAPSED_TIME);
    
    // convert it to seconds:
    return (float)ms / 1000.f;
}


// initialize the glui window:

void InitMenus() {
    glutSetWindow(MainWindow);
    
    int axesmenu = glutCreateMenu(DoAxesMenu);
    glutAddMenuEntry("Off",  0);
    glutAddMenuEntry("On",   1);
    
    int debugmenu = glutCreateMenu(DoDebugMenu);
    glutAddMenuEntry("Off",  0);
    glutAddMenuEntry("On",   1);
    
    int perspmenu = glutCreateMenu(DoPerspMenu);
    glutAddMenuEntry("Outside", OUTSIDE);
    glutAddMenuEntry("Inside", INSIDE);
    
    glutCreateMenu(DoMainMenu);
    glutAddSubMenu(  "Axes",          axesmenu);
    glutAddSubMenu(  "View",          perspmenu);
    glutAddMenuEntry("Reset",         RESET);
    glutAddSubMenu(  "Debug",         debugmenu);
    glutAddMenuEntry("Quit",          QUIT);
    
    // attach the pop-up menu to the right mouse button:
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}



// initialize the glut and OpenGL libraries:
//    also setup display lists and callback functions

void InitGraphics() {
    // request the display modes:
    // ask for red-green-blue-alpha color, double-buffering, and z-buffering:
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    
    // set the initial window configuration:
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(INITIAL_WINDOW_SIZE, INITIAL_WINDOW_SIZE);
    
    // open the window and set its title:
    MainWindow = glutCreateWindow(WINDOW_TITLE);
    glutSetWindowTitle(WINDOW_TITLE);
    
    // set the framebuffer clear values:
    glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);
    
    
    glutSetWindow(MainWindow);
    glutDisplayFunc(Display);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion);
    glutPassiveMotionFunc(NULL);
    glutEntryFunc(NULL);
    glutSpecialFunc(NULL);
    glutSpaceballMotionFunc(NULL);
    glutSpaceballRotateFunc(NULL);
    glutSpaceballButtonFunc(NULL);
    glutButtonBoxFunc(NULL);
    glutDialsFunc(NULL);
    glutTabletMotionFunc(NULL);
    glutTabletButtonFunc(NULL);
    glutMenuStateFunc(NULL);
    glutTimerFunc(-1, NULL, 0);
    glutIdleFunc(Animate);
    
    // init glew (a window must be open to do this):
    
#ifdef WIN32
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "glewInit Error\n");
    }
    else
        fprintf(stderr, "GLEW initialized OK\n");
    fprintf( stderr, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif
    
}

void initAxes() {
    AxesList = glGenLists(1);
    glNewList(AxesList, GL_COMPILE);
    glLineWidth(AXES_WIDTH);
    Axes(1.5);
    glLineWidth(1.);
    glEndList();
}

void InitLists() {
    glutSetWindow(MainWindow);
    
    createCessnaWireframe();
    createCessnaPropeller();

    initAxes();
}

void setColor(float r, float g, float b) {
    glColor3f(r, g, b);
}

void createCessnaWireframe() {
    CessnaWireList = glGenLists(1);
    glNewList(CessnaWireList, GL_COMPILE);
    
    int i;
    struct edge *ep;
    struct point *p0, *p1;

    glPushMatrix();
    glRotatef(-7., 0., 1., 0.);
    glTranslatef( 0., -1., 0. );
    glRotatef(  97.,   0., 1., 0. );
    glRotatef( -15.,   0., 0., 1. );
    glBegin( GL_LINES );

    // red
    setColor(1, 0, 0);

    for (i=0, ep = CESSNAedges; i < CESSNAnedges; i++, ep++) {
        p0 = &CESSNApoints[ ep->p0 ];
        p1 = &CESSNApoints[ ep->p1 ];
        glVertex3f( p0->x, p0->y, p0->z );
        glVertex3f( p1->x, p1->y, p1->z );
    }
    
    glEnd();
    glPopMatrix();
    
    glEndList();
}

//void CESSNAshade() {
//
//    CessnaList = glGenLists(1);
//    glNewList(CessnaList, GL_COMPILE);
//
//    int i;
//    struct tri *tp;
//    float p01[3], n[3];
////    struct edge *ep;
//    struct point *p0, *p1;
//
//    glPushMatrix();
//    glRotatef(-7., 0., 1., 0.);
//    glTranslatef( 0., -1., 0. );
//    glRotatef(  97.,   0., 1., 0. );
//    glRotatef( -15.,   0., 0., 1. );
//    glBegin(GL_TRIANGLES);
//
//
//    for (i=0, tp = CESSNAtris; i < CESSNAntris; i++, tp++) {
//        p0 = &CESSNApoints[ tp->p0 ];
//        p1 = &CESSNApoints[ tp->p1 ];
//
//        /* fake "lighting" from above:            */
//
//        p01[0] = p1->x - p0->x;
//        p01[1] = p1->y - p0->y;
//        p01[2] = p1->z - p0->z;
////        Unit( n, n );
//        n[1] = fabs( n[1] );
//        n[1] += .25;
//        if( n[1] > 1. )
//            n[1] = 1.;
//        glColor3f( 0., n[1], 0. );
//
//        glVertex3f( p0->x, p0->y, p0->z );
//        glVertex3f( p1->x, p1->y, p1->z );
//    }
//    glEnd( );
//    glPopMatrix( );
//
//    glEndList();
//}

void createCessnaPropeller() {
    // propeller parameters:

    #define PROPELLER_RADIUS     1.0
    #define PROPELLER_WIDTH      0.4
    
    CessnaPropellerList = glGenLists(1);
    glNewList(CessnaPropellerList, GL_COMPILE);

            // draw the cessna propeller with radius PROPELLER_RADIUS and
            //    width PROPELLER_WIDTH centered at (0.,0.,0.) in the XY plane

            glBegin( GL_TRIANGLES );
                setColor(1, 1, 1);
                glVertex2f(  PROPELLER_RADIUS,  PROPELLER_WIDTH/2. );
                glVertex2f(  0., 0. );
                glVertex2f(  PROPELLER_RADIUS, -PROPELLER_WIDTH/2. );

                glVertex2f( -PROPELLER_RADIUS, -PROPELLER_WIDTH/2. );
                glVertex2f(  0., 0. );
                glVertex2f( -PROPELLER_RADIUS,  PROPELLER_WIDTH/2. );
    
    
                glTranslatef(50,50,50);
            glEnd( );

    
    glEndList();
}


void FunkyTargetThingy() {
    glTranslatef(0, 1, 15.);
    glRotatef(90., 90, 0, -5);
    glScalef(2,2,2);
    glBegin(GL_LINE_LOOP);
    
    for (int y = 0; y < 200; y++) {
        float deg = y / 10.;
        if (y < 40) glColor3f(0, y/80., 128/255.);
        else if (y < 80) glColor3f(0, 128/255., (80-y)/80.);
        else if (y < 120) glColor3f((y-80.)/80., 128/255., 0);
        else glColor3f(128/255., (160-y)/80., 0);
        glVertex3f(cosf(deg)/2., y/200.*sinf(Time), sinf(deg)/2.);
        glVertex3f(cosf(deg), y/80.*sinf(Time), sinf(deg));
    }
    
    glEnd();

}



// the keyboard callback:
void Keyboard(unsigned char c, int x, int y) {
    if (DebugOn)
        fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );
    
    switch (c) {
        case 'q': case 'Q': case ESCAPE:
            DoMainMenu(QUIT);    // will not return here
            break;                // happy compiler
            
        case 'f': case 'F':
            Frozen = !Frozen;
            if (Frozen) glutIdleFunc(NULL);
            else glutIdleFunc(Animate);
            break;
            
        default:
            fprintf(stderr, "Don't know what to do with keyboard: '%c' (0x%0x)\n", c, c);
    }
    
    // force a call to Display():
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}


// called when the mouse button transitions down or up:
void MouseButton(int button, int state, int x, int y) {
    int b = 0;            // LEFT, MIDDLE, or RIGHT
    
    if (DebugOn)
        fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);
    
    
    // get the proper button bit mask:
    switch (button) {
        case GLUT_LEFT_BUTTON:
            b = LEFT;
            break;
            
        case GLUT_MIDDLE_BUTTON:
            b = MIDDLE;
            break;
            
        case GLUT_RIGHT_BUTTON:
            b = RIGHT;
            break;
            
        default:
            b = 0;
            fprintf(stderr, "Unknown mouse button: %d\n", button);
    }
    
    
    // button down sets the bit, up clears the bit:
    if (state == GLUT_DOWN) {
        Xmouse = x;
        Ymouse = y;
        ActiveButton |= b;
    } else {
        ActiveButton &= ~b;
    }
}


// called when the mouse moves while a button is down:
void MouseMotion(int x, int y) {
    if (DebugOn)
        fprintf(stderr, "MouseMotion: %d, %d\n", x, y);
    
    
    int dx = x - Xmouse;        // change in mouse coords
    int dy = y - Ymouse;
    
    if (ActiveButton & LEFT) {
        Xrot += (ANGLE_FACTOR*dy);
        Yrot += (ANGLE_FACTOR*dx);
    }
    
    
    if (ActiveButton & MIDDLE) {
        Scale += SCALE_FACTOR * (float) (dx - dy);
        
        // keep object from turning inside-out or disappearing:
        
        if (Scale < SCALE_FACTOR_MINIMUM)
            Scale = SCALE_FACTOR_MINIMUM;
    }
    
    Xmouse = x;            // new current position
    Ymouse = y;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}


// reset the transformations and the colors:
void Reset() {
    ActiveButton = 0;
    AxesOn = 0;
    DebugOn = 0;
    Scale  = 1.0;
    Xrot = Yrot = 0.;
    Frozen = 0;
}


static int x_order[] = {
    1, 2, -3, 4
};

static int z_order[] = {
    1, 2, 3, 4, -5, 6
};

static int y_order[] = {
    1, 2, 3, -2, 4
};

static float xx[] = {
    0.f, 1.f, 0.f, 1.f
};

static float xy[] = {
    -.5f, .5f, .5f, -.5f
};

static float yx[] = {
    0.f, 0.f, -.5f, .5f
};

static float yy[] = {
    0.f, .6f, 1.f, 1.f
};

static float zx[] = {
    1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[] = {
    .5f, .5f, -.5f, -.5f, 0.f, 0.f
};

// fraction of the length to use as height of the characters:
const float LENFRAC = 0.10f;

// fraction of length to use as start location of the characters:
const float BASEFRAC = 1.10f;

//    Draw a set of 3D axes:
//    (length is the axis length in world coordinates)
void Axes(float length) {
    // Draw axis lines
    glBegin(GL_LINE_STRIP);
    glVertex3f(length, 0., 0.);
    glVertex3f(0., 0., 0.);
    glVertex3f(0., length, 0.);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex3f(0., 0., 0.);
    glVertex3f(0., 0., length);
    glEnd();
    
    
    // Set length fractions
    float fact = LENFRAC * length;
    float base = BASEFRAC * length;
    
    // Draw stroke X
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 4; i++) {
        int j = x_order[i];
        if (j < 0) {
            glEnd();
            glBegin(GL_LINE_STRIP);
            j = -j;
        }
        j--;
        glVertex3f(base + fact*xx[j], fact*xy[j], 0.0);
    }
    glEnd();
    
    // Draw stroke Y
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 5; i++) {
        int j = y_order[i];
        if (j < 0) {
            glEnd();
            glBegin(GL_LINE_STRIP);
            j = -j;
        }
        j--;
        glVertex3f(fact*yx[j], base + fact*yy[j], 0.0);
    }
    glEnd();
    
    // Draw stroke Z
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < 6; i++) {
        int j = z_order[i];
        if (j < 0) {
            glEnd();
            glBegin(GL_LINE_STRIP);
            j = -j;
        }
        j--;
        glVertex3f(0.0, fact*zy[j], base + fact*zx[j]);
    }
    glEnd();
    
}
