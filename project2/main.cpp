//    Project 2: Animate a Helicopter
//
//    The objective is to draw a helicopter and animate the blades.
//
//    The left mouse button does rotation
//    The middle mouse button does scaling
//    The user interface allows:
//        1. The axes to be turned on and off
//        2. The color of the axes to be changed
//        3. Debugging to be turned on and off
//        4. Depth cueing to be turned on and off
//        5. The projection to be changed
//        6. The transformations to be reset
//        7. The program to quit
//
//    Author:            Kyler Stole

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




// NOTE: There are a lot of good reasons to use const variables instead
// of #define's.  However, Visual C++ does not allow a const variable
// to be used as an array size or as the case in a switch() statement.  So in
// the following, all constants are const variables except those which need to
// be array sizes or cases in switch() statements.  Those are #defines.


// title of these windows:
const char *WINDOWTITLE = { "Project2" };
const char *GLUITITLE   = { "Kateryna Gnedash" };


// what the glui package defines as true and false:
//const int GLUITRUE  = { true  };
//const int GLUIFALSE = { false };


// the escape key:
#define ESCAPE        0x1b


// initial window size:
const int INIT_WINDOW_SIZE = { 600 };


// size of the box:
const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)
const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// minimum allowable scale factor:
const float MINSCALE = { 0.05f };


// animation cycle time
const int MS_IN_THE_ANIMATION_CYCLE = { 800 };


// active mouse buttons (or them together):
const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which projection:
enum Projections {
    ORTHO,
    PERSP
};


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


// fog parameters:
const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };


// non-constant global variables:
int        ActiveButton;            // current button that is down
GLuint    AxesList;                // list to hold the axes
int        AxesOn;                    // != 0 means to draw the axes
int        DebugOn;                // != 0 means to print debugging info
int        DepthCueOn;                // != 0 means to use intensity depth cueing
GLuint    BoxList;                // object display list
GLuint  CessnaList;               // helicopter display list
GLuint  CessnaWireList;           // wireframe helicopter display list
GLuint  CessnaPropellerList;
GLuint  BladeList;              // helicopter blade display list
GLuint  ObjList;                // new object display list
int        MainWindow;                // window id for main graphics window
float    Scale;                    // scaling factor
int        WhichColor;                // index into Colors[ ]
int        WhichProjection;        // ORTHO or PERSP
int     WhichViewPerspective;   // OUTSIDE or INSIDE
int        Xmouse, Ymouse;            // mouse values
float    Xrot, Yrot;                // rotation angles in degrees
float   Time;                   // current time elapsed
float   TimeCycle;              // current time in animation cycle
bool    Frozen;                 // sets whether the scene is frozen


// blade parameters:

#define BLADE_RADIUS     1.0
#define BLADE_WIDTH         0.4


// MARK: - Function prototypes

void    Animate();
void    Display();
void    drawFunkyTargetThingy();
void    createCessnaWireframe();
void    createCessnaPropeller();
void    DoAxesMenu(int);
void    DoColorMenu(int);
void    DoDepthMenu(int);
void    DoDebugMenu(int);
void    DoMainMenu(int);
void    DoProjectMenu(int);
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
void    Resize(int, int);
void    Visibility(int);

void    Axes(float);
void    HsvRgb(float[3], float [3]);
float   Dot(float v1[3], float v2[3]);
void    Cross(float v1[3], float v2[3], float vout[3]);
float   Unit(float vin[3], float vout[3]);


// MARK: - Function definitions

// main program:
int main(int argc, char *argv[]) {
    // turn on the glut package:
    // (do this before checking argc and argv since it might
    // pull some command line arguments out)
    glutInit(&argc, argv);
    
    
    // setup all the graphics stuff:
    InitGraphics();
    
    
    // create the display structures that will not change:
    InitLists();
    
    
    // init all the global variables used by Display():
    // this will also post a redisplay
    Reset();
    
    
    // setup all the user interface stuff:
    InitMenus();
    
    
    // draw the scene once and wait for some interaction:
    // (this will never return)
    glutSetWindow(MainWindow);
    glutMainLoop();
    
    
    // this is here to make the compiler happy:
    return 0;
}


// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display() from here -- let glutMainLoop() do it

void Animate() {
    // put animation stuff in here -- change some global variables
    // for Display() to find:
    int ms = glutGet(GLUT_ELAPSED_TIME);                    // milliseconds
    Time = (float)ms / (float)1000;
    ms %= MS_IN_THE_ANIMATION_CYCLE;
    TimeCycle = (float)ms / (float)MS_IN_THE_ANIMATION_CYCLE;    // [0., 1.]
    
    // force a call to Display() next time it is convenient:
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}


// draw the complete scene:

void Display() {
    if (DebugOn)
        fprintf(stderr, "Display\n");
    
    
    // set which window we want to do the graphics into:
    glutSetWindow(MainWindow);
    
    
    // erase the background:
    glDrawBuffer(GL_BACK);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    
    
    // specify shading to be flat:
    glShadeModel(GL_FLAT);
    
    
    // set the viewport to a square centered in the window:
    GLsizei vx = glutGet(GLUT_WINDOW_WIDTH);
    GLsizei vy = glutGet(GLUT_WINDOW_HEIGHT);
    GLsizei v = vx < vy ? vx : vy;            // minimum dimension
    GLint xl = (vx - v) / 2;
    GLint yb = (vy - v) / 2;
    glViewport(xl, yb,  v, v);
    
    
    // set the viewing volume:
    // remember that the Z clipping  values are actually
    // given as DISTANCES IN FRONT OF THE EYE
    // USE gluOrtho2D() IF YOU ARE DOING 2D !
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (WhichProjection == ORTHO)
        glOrtho(-3., 3.,     -3., 3.,     0.1, 1000.);
    else
        gluPerspective(90., 1.,    0.1, 1000.);
    
    
    // place the objects into the scene:
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    
    // set the eye position, look-at position, and up-vector:
    if (WhichViewPerspective == INSIDE) {
        gluLookAt(-0.4, 1.8, -4.9,     0., 0., -14.,     0., 1., 0.);
    } else {
        gluLookAt(11., 7., 9.,     0., 0., 1.6,     0., 1., 0.);
        
        
        // rotate the scene:
        glRotatef((GLfloat)Yrot, 0., 1., 0.);
        glRotatef((GLfloat)Xrot, 1., 0., 0.);
        
        
        // uniformly scale the scene:
        if (Scale < MINSCALE)
            Scale = MINSCALE;
        glScalef((GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale);
        
    }
    
    
    // set the fog parameters:
    if (DepthCueOn) {
        glFogi(GL_FOG_MODE, FOGMODE);
        glFogfv(GL_FOG_COLOR, FOGCOLOR);
        glFogf(GL_FOG_DENSITY, FOGDENSITY);
        glFogf(GL_FOG_START, FOGSTART);
        glFogf(GL_FOG_END, FOGEND);
        glEnable(GL_FOG);
    } else {
        glDisable(GL_FOG);
    }
    
    
    // possibly draw the axes:
    if (AxesOn) {
        GLfloat const red[3] = {1,0,0};
        glColor3fv(red);
        glCallList(AxesList);
    }
    
    
    // since we are using glScalef(), be sure normals get unitized:
    glEnable(GL_NORMALIZE);

    glCallList(CessnaList);
    glCallList(CessnaWireList);
//    glCallList(CessnaPropellerList);
    
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
    
    //drawFunkyTargetThingy();
    
    
    // draw some gratuitous text that just rotates on top of the scene:
    //    glDisable(GL_DEPTH_TEST);
    //    glColor3f(0., 1., 1.);
    //    DoRasterString(0., 1., 0., "Wha!!!");
    
    
    // draw some gratuitous text that is fixed on the screen:
    //
    // the projection matrix is reset to define a scene whose
    // world coordinate system goes from 0-100 in each axis
    //
    // this is called "percent units", and is just a convenience
    //
    // the modelview matrix is reset to identity as we don't
    // want to transform these coordinates
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0., 100., 0., 100.);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glColor3f(1., 1., 1.);
    DoRasterString(5., 5., 0., "Project 2");
    
    
    // swap the double-buffered framebuffers:
    glutSwapBuffers();
    
    
    // be sure the graphics buffer has been sent:
    // note: be sure to use glFlush() here, not glFinish() !
    glFlush();
}

void drawFunkyTargetThingy() {
    glTranslatef(0, 2, -10.);
    glRotatef(47., 9, 8, -5);
    glBegin(GL_TRIANGLE_STRIP);
    
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


void DoDepthMenu(int id) {
    DepthCueOn = id;
    
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
            // gracefully close out the graphics:
            // gracefully close the graphics window:
            // gracefully exit the program:
            glutSetWindow(MainWindow);
            glFinish();
            glutDestroyWindow(MainWindow);
            exit(0);
            break;
            
        default:
            fprintf(stderr, "Don't know what to do with Main Menu ID %d\n", id);
    }
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}


void DoProjectMenu(int id) {
    WhichProjection = id;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

void DoPerspMenu(int id) {
    WhichViewPerspective = id;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}

// use glut to display a string of characters using a raster font:

void DoRasterString(float x, float y, float z, char const *s) {
    glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );
    
    char c;            // one character to print
    for(; (c = *s) != '\0'; s++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}


// use glut to display a string of characters using a stroke font:

void DoStrokeString(float x, float y, float z, float ht, char const *s) {
    glPushMatrix();
    glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
    float sf = ht / (119.05f + 33.33f);
    glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
    char c;            // one character to print
    for (; (c = *s) != '\0'; s++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
    }
    glPopMatrix();
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
    
    int depthcuemenu = glutCreateMenu(DoDepthMenu);
    glutAddMenuEntry("Off",  0);
    glutAddMenuEntry("On",   1);
    
    int debugmenu = glutCreateMenu(DoDebugMenu);
    glutAddMenuEntry("Off",  0);
    glutAddMenuEntry("On",   1);
    
    int projmenu = glutCreateMenu(DoProjectMenu);
    glutAddMenuEntry("Orthographic",  ORTHO);
    glutAddMenuEntry("Perspective",   PERSP);
    
    int perspmenu = glutCreateMenu(DoPerspMenu);
    glutAddMenuEntry("Outside", OUTSIDE);
    glutAddMenuEntry("Inside", INSIDE);
    
    glutCreateMenu(DoMainMenu);
    glutAddSubMenu(  "Axes",          axesmenu);
    glutAddSubMenu(  "Depth Cue",     depthcuemenu);
    glutAddSubMenu(  "Projection",    projmenu);
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
    glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);
    
    // open the window and set its title:
    MainWindow = glutCreateWindow(WINDOWTITLE);
    glutSetWindowTitle(WINDOWTITLE);
    
    // set the framebuffer clear values:
    glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);
    
    // setup the callback functions:
    // DisplayFunc -- redraw the window
    // ReshapeFunc -- handle the user resizing the window
    // KeyboardFunc -- handle a keyboard input
    // MouseFunc -- handle the mouse button going down or up
    // MotionFunc -- handle the mouse moving with a button down
    // PassiveMotionFunc -- handle the mouse moving with a button up
    // VisibilityFunc -- handle a change in window visibility
    // EntryFunc    -- handle the cursor entering or leaving the window
    // SpecialFunc -- handle special keys on the keyboard
    // SpaceballMotionFunc -- handle spaceball translation
    // SpaceballRotateFunc -- handle spaceball rotation
    // SpaceballButtonFunc -- handle spaceball button hits
    // ButtonBoxFunc -- handle button box hits
    // DialsFunc -- handle dial rotations
    // TabletMotionFunc -- handle digitizing tablet motion
    // TabletButtonFunc -- handle digitizing tablet button hits
    // MenuStateFunc -- declare when a pop-up menu is in use
    // TimerFunc -- trigger something to happen a certain time from now
    // IdleFunc -- what to do when nothing else is going on
    
    glutSetWindow(MainWindow);
    glutDisplayFunc(Display);
    glutReshapeFunc(Resize);
    glutKeyboardFunc(Keyboard);
    glutMouseFunc(MouseButton);
    glutMotionFunc(MouseMotion);
    glutPassiveMotionFunc(NULL);
    glutVisibilityFunc(Visibility);
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

// initialize the display lists that will not change:
// (a display list is a way to store opengl commands in
//  memory so that they can be played back efficiently at a later time
//  with a call to glCallList()
void InitLists() {
    float dx = BOXSIZE / 2.f;
    float dy = BOXSIZE / 2.f;
    float dz = BOXSIZE / 2.f;
    glutSetWindow(MainWindow);
    
    createCessnaWireframe();
    createCessnaPropeller();

//
//    // create helicopter
//    CessnaList = glGenLists(1);
//    glNewList(CessnaList, GL_COMPILE);
//
//    struct tri *tp;
//    float p01[3], p02[3], n[3];
//
//    glPushMatrix();
//    glTranslatef(0., -1., 0.);
//    glRotatef(97., 0., 1., 0.);
//    glRotatef(-15., 0., 0., 1.);
//    glBegin(GL_TRIANGLES);
//
//    for (i=0, tp = CESSNAtris; i < CESSNAntris; i++, tp++) {
//        p0 = &CESSNApoints[ tp->p0 ];
//        p1 = &CESSNApoints[ tp->p1 ];
////        p2 = &CESSNApoints[ tp->p2 ];
//
//        /* fake "lighting" from above:            */
//
//        p01[0] = p1->x - p0->x;
//        p01[1] = p1->y - p0->y;
//        p01[2] = p1->z - p0->z;
////        p02[0] = p2->x - p0->x;
////        p02[1] = p2->y - p0->y;
////        p02[2] = p2->z - p0->z;
//        Cross( p01, p02, n );
//        Unit( n, n );
//        n[1] = fabs( n[1] );
//        n[1] += .25;
//        if( n[1] > 1. )
//            n[1] = 1.;
//        glColor3f( 0., n[1], 0. );
//
//        glVertex3f( p0->x, p0->y, p0->z );
//        glVertex3f( p1->x, p1->y, p1->z );
////        glVertex3f( p2->x, p2->y, p2->z );
//    }
//    glEnd( );
//    glPopMatrix( );
//
//    glEndList();
//
//
//    // create the helicopter blade with radius BLADE_RADIUS and
//    //    width BLADE_WIDTH centered at (0.,0.,0.) in the XY plane
//    BladeList = glGenLists(1);
//    glNewList(BladeList, GL_COMPILE);
//
//    glBegin( GL_TRIANGLES );
//    glVertex2f(  BLADE_RADIUS,  BLADE_WIDTH/2. );
//    glVertex2f(  0., 0. );
//    glVertex2f(  BLADE_RADIUS, -BLADE_WIDTH/2. );
//
//    glVertex2f( -BLADE_RADIUS, -BLADE_WIDTH/2. );
//    glVertex2f(  0., 0. );
//    glVertex2f( -BLADE_RADIUS,  BLADE_WIDTH/2. );
//    glEnd();
//
//    glEndList();
//
//
    // create the axes
    AxesList = glGenLists(1);
    glNewList(AxesList, GL_COMPILE);
    glLineWidth(AXES_WIDTH);
    Axes(1.5);
    glLineWidth(1.);
    glEndList();
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

void createCessnaPropeller() {
    // propeller parameters:

    #define PROPELLER_RADIUS     1.0
    #define PROPELLER_WIDTH         0.4
    
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


// the keyboard callback:
void Keyboard(unsigned char c, int x, int y) {
    if (DebugOn)
        fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );
    
    switch (c) {
        case 'o': case 'O':
            WhichProjection = ORTHO;
            break;
            
        case 'p': case 'P':
            WhichProjection = PERSP;
            break;
            
        case 'q': case 'Q': case ESCAPE:
            DoMainMenu(QUIT);    // will not return here
            break;                // happy compiler
            
        case 'f': case 'F':
            Frozen = !Frozen;
            if (Frozen) glutIdleFunc(NULL);
            else glutIdleFunc(Animate);
            break;
            
        default:
            fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
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
        ActiveButton |= b;        // set the proper bit
    } else {
        ActiveButton &= ~b;        // clear the proper bit
    }
}


// called when the mouse moves while a button is down:
void MouseMotion(int x, int y) {
    if (DebugOn)
        fprintf(stderr, "MouseMotion: %d, %d\n", x, y);
    
    
    int dx = x - Xmouse;        // change in mouse coords
    int dy = y - Ymouse;
    
    if (ActiveButton & LEFT) {
        Xrot += (ANGFACT*dy);
        Yrot += (ANGFACT*dx);
    }
    
    
    if (ActiveButton & MIDDLE) {
        Scale += SCLFACT * (float) (dx - dy);
        
        // keep object from turning inside-out or disappearing:
        
        if (Scale < MINSCALE)
            Scale = MINSCALE;
    }
    
    Xmouse = x;            // new current position
    Ymouse = y;
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}


// reset the transformations and the colors:
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene
void Reset() {
    ActiveButton = 0;
    AxesOn = 0;
    DebugOn = 0;
    DepthCueOn = 0;
    Scale  = 1.0;
    WhichProjection = PERSP;
    Xrot = Yrot = 0.;
    Frozen = 0;
}


// called when user resizes the window:
void Resize(int width, int height) {
    if (DebugOn)
        fprintf(stderr, "ReSize: %d, %d\n", width, height);
    
    // don't really need to do anything since window size is
    // checked each time in Display():
    
    glutSetWindow(MainWindow);
    glutPostRedisplay();
}


// handle a change to the window's visibility:
void Visibility(int state) {
    if (DebugOn)
        fprintf(stderr, "Visibility: %d\n", state);
    
    if (state == GLUT_VISIBLE) {
        glutSetWindow(MainWindow);
        glutPostRedisplay();
    } else {
        // could optimize by keeping track of the fact
        // that the window is not visible and avoid
        // animating or redrawing it ...
    }
}



///////////////////////////////////////   HANDY UTILITIES:  //////////////////////////


// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {
    0.f, 1.f, 0.f, 1.f
};

static float xy[] = {
    -.5f, .5f, .5f, -.5f
};

static int xorder[] = {
    1, 2, -3, 4
};

static float yx[] = {
    0.f, 0.f, -.5f, .5f
};

static float yy[] = {
    0.f, .6f, 1.f, 1.f
};

static int yorder[] = {
    1, 2, 3, -2, 4
};

static float zx[] = {
    1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[] = {
    .5f, .5f, -.5f, -.5f, 0.f, 0.f
};

static int zorder[] = {
    1, 2, 3, 4, -5, 6
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
        int j = xorder[i];
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
        int j = yorder[i];
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
        int j = zorder[i];
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


// function to convert HSV to RGB
// 0.  <=  s, v, r, g, b  <=  1.
// 0.  <= h  <=  360.
// when this returns, call:
//        glColor3fv(rgb);

void HsvRgb(float hsv[3], float rgb[3]) {
    // guarantee valid input:
    
    float h = hsv[0] / 60.f;
    while (h >= 6.)    h -= 6.;
    while (h <  0.) h += 6.;
    
    float s = hsv[1];
    if (s < 0.)
        s = 0.;
    if (s > 1.)
        s = 1.;
    
    float v = hsv[2];
    if (v < 0.)
        v = 0.;
    if (v > 1.)
        v = 1.;
    
    // if sat==0, then is a gray:
    
    if (s == 0.0) {
        rgb[0] = rgb[1] = rgb[2] = v;
        return;
    }
    
    // get an rgb from the hue itself:
    
    float i = floor(h);
    float f = h - i;
    float p = v * (1.f - s);
    float q = v * (1.f - s*f);
    float t = v * (1.f - (s * (1.f-f)));
    
    float r, g, b;            // red, green, blue
    switch ((int) i) {
        case 0:
            r = v;    g = t;    b = p;
            break;
            
        case 1:
            r = q;    g = v;    b = p;
            break;
            
        case 2:
            r = p;    g = v;    b = t;
            break;
            
        case 3:
            r = p;    g = q;    b = v;
            break;
            
        case 4:
            r = t;    g = p;    b = v;
            break;
            
        case 5:
            r = v;    g = p;    b = q;
            break;
            
        default:
            r = 0;  g = 0;  b = 0;
    }
    
    
    rgb[0] = r;
    rgb[1] = g;
    rgb[2] = b;
}

float Dot(float v1[3], float v2[3]) {
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void Cross(float v1[3], float v2[3], float vout[3]) {
    float tmp[3];
    tmp[0] = v1[1]*v2[2] - v2[1]*v1[2];
    tmp[1] = v2[0]*v1[2] - v1[0]*v2[2];
    tmp[2] = v1[0]*v2[1] - v2[0]*v1[1];
    vout[0] = tmp[0];
    vout[1] = tmp[1];
    vout[2] = tmp[2];
}

float Unit(float vin[3], float vout[3]) {
    float dist = vin[0]*vin[0] + vin[1]*vin[1] + vin[2]*vin[2];
    if (dist > 0.0) {
        dist = sqrt( dist );
        vout[0] = vin[0] / dist;
        vout[1] = vin[1] / dist;
        vout[2] = vin[2] / dist;
    } else {
        vout[0] = vin[0];
        vout[1] = vin[1];
        vout[2] = vin[2];
    }
    return dist;
}
