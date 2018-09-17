//
//  main.cpp
//  SDL2_test
//
//  Created by 王登远 on 2018/9/10.
//  Copyright © 2018 王登远. All rights reserved.
//



//HW 0 - Moving Square
//Starter code for the first homework assignment.
//This code assumes SDL2 and OpenGL are both properly installed on your system

#include "glad.h"  //Include order can matter here

#if defined(__APPLE__) || defined(__linux__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <algorithm>
#ifndef geo_fig_cpp
#define geo_fig_cpp
#include "geo_fig.cpp"  //Include order can matter here
#endif
#define PI 3.141592653
#define Multi_square 1
using namespace std;

//Globals to store the state of the square (position, width, and angle)

//Name of image texture

float brighten_rate = 1;
//string textureName = "brick.ppm";
//string textureName = "/Users/wangdengyuan/Desktop/source code for xcode/SDL2_test/SDL2_test/test.ppm";




int screen_width = 400;
int screen_height = 400;
int window_w,window_h;
float g_mouse_down = false;
float g_clicked_x = -1;
float g_clicked_y = -1;
float g_lastCenter_x = -1;
float g_lastCenter_y = -1;
float g_clicked_angle = -1;
float g_clicked_size = -1;
int rotation_corner = -1;
bool g_bTranslate = false;
bool g_bRotate = false;
bool g_bScale = false;
void world_mouse_xy2screen_mouse_xy(float &m_x,float &m_y);
void RGB2CIELAB(float &a,float &b,float &c);
float Funf(float p1,float p2);
void CIELAB2RGB(float &a,float &b,float &c);
void Update_image_brightenness(unsigned char* img_data_tmp,unsigned char* img_data,int img_h,int img_w);


//////////////////////////
///  Begin your code here
/////////////////////////

//TODO: Read from ASCII (P6) PPM files
//Inputs are output variables for returning the image width and heigth
unsigned char* loadImage(int& img_w, int& img_h,string textureName){
    //Open the texture image file
    ifstream ppmFile;
   /* ofstream ppmFile_out;
    ppmFile_out.open("out_test.ppm");
    ppmFile_out.write("asdasd",3);
    ppmFile_out.close();*/
    cout<<textureName.c_str()<<endl;
    ppmFile.open(textureName.c_str());
    if (!ppmFile){
        printf("ERROR: Texture file '%s' not found.\n",textureName.c_str());
        exit(1);
    }

    //Check that this is an ASCII PPM (first line is P3)
    string PPM_style;
    ppmFile >> PPM_style; //Read the first line of the header
    if (PPM_style != "P3") {
        printf("ERROR: PPM Type number is %s. Not an ASCII (P3) PPM file!\n",PPM_style.c_str());
        exit(1);
    }

    //Read in the texture width and height
    ppmFile >> img_w >> img_h;
    unsigned char* img_data = new unsigned char[4*img_w*img_h];
    unsigned char* img_data_temp = new unsigned char[4*img_w*img_h];
    //Check that the 3rd line is 255 (ie., this is an 8 bit/pixel PPM)
    int maximum;
    ppmFile >> maximum;
    if (maximum != 255) {
        printf("ERROR: Maximum size is (%d) not 255.\n",maximum);
        exit(1);
    }
    
    //TODO:
    int x;
    int i = 0, j = 0, k = 0;
    while(ppmFile >> x){
    //    //Store the RGB pixel data from the file into an array
       // if(Debug_tag) cout << x<< img_data[(img_h)*img_w*4-1];
        if (k<2){
            img_data_temp[i*img_w*4 + j*4 + k] = x;
        }
        else{
            img_data_temp[i*img_w*4 + j*4 + k] = x;
            k = (k+1)%4;
            img_data_temp[i*img_w*4 + j*4 + k] = 255;  //Alpha
            if((j+1)==img_w)
                i=i+1;
            j = (j+1) % img_w;

        }
        k = (k+1)%4;
    }
    float tmp_rgb[3],range = 255;
    for (int i = 0; i < img_h; i++){
        for (int j = 0; j < img_w; j++){
            tmp_rgb[0] = img_data_temp[(img_h-i-1)*img_w*4 + (j)*4 ]/range;//Red
            tmp_rgb[1] = img_data_temp[(img_h-i-1)*img_w*4 + (j)*4 + 1]/range;//Green
            tmp_rgb[2] = img_data_temp[(img_h-i-1)*img_w*4 + (j)*4 + 2]/range;//Blue
            /*//printf("Origin:R:%f,G:%f,B:%f\n",tmp_rgb[0], tmp_rgb[1], tmp_rgb[2]);
            RGB2CIELAB(tmp_rgb[0], tmp_rgb[1], tmp_rgb[2]);
            tmp_rgb[0] = brighten_rate * tmp_rgb[0];
            CIELAB2RGB(tmp_rgb[0], tmp_rgb[1], tmp_rgb[2]);
            //printf("New:R:%f,G:%f,B:%f\n",tmp_rgb[0], tmp_rgb[1], tmp_rgb[2]);*/
            img_data[i*img_w*4 + j*4] = tmp_rgb[0]*range;  //Red
            img_data[i*img_w*4 + j*4 + 1] = tmp_rgb[1]*range; //Green
            img_data[i*img_w*4 + j*4 + 2] = tmp_rgb[2]*range; //Blue
            
             img_data[i*img_w*4 + j*4 + 3] = 255;  //Alpha
         }
     }
    delete [] img_data_temp;
    
    
    return img_data;
}
void RGB2CIELAB(float &a,float &b,float &c){
    // [ X ]   [  0.412453  0.357580  0.180423 ]   [ R ] **
    // [ Y ] = [  0.212671  0.715160  0.072169 ] * [ G ]
    //[ Z ]    [  0.019334  0.119193  0.950227 ]   [ B ].

    float T_rgb2xyz[3][3]={{0.412453,  0.357580,  0.180423},
                            {0.212671,  0.715160,  0.072169},
                            {0.019334,  0.119193,  0.950227}};
    float xyz[3];
    for(int i=0;i<3;i++)
        xyz[i] = T_rgb2xyz[i][0]*a+T_rgb2xyz[i][1]*b+T_rgb2xyz[i][2]*c;
    
    /*
     L* = 116 * (Y/Yn)1/3 - 16    for Y/Yn > 0.008856
     L* = 903.3 * Y/Yn             otherwise
     
     a* = 500 * ( f(X/Xn) - f(Y/Yn) )
     b* = 200 * ( f(Y/Yn) - f(Z/Zn) )
     where f(t) = t1/3      for t > 0.008856
     f(t) = 7.787 * t + 16/116    otherwise
     */
    
    float Xn=95.047,Yn=100.0,Zn=108.883;
    float L,A,B;
    L = 116.0*Funf(xyz[1],Yn)-16.0;
    A = 500.0*(Funf(xyz[0],Xn)-Funf(xyz[1],Yn));
    B = 200.0*(Funf(xyz[1],Yn)-Funf(xyz[2],Zn));
    
    a = L;
    b = A;
    c = B;
}
float Funf(float p1,float p2){
    double delta = 6.0/29.0;
    if (p1/p2>pow(delta,3.0))
    {
        return pow(p1/p2,1.0/3.0);
    }else{

       // cout<<p1/(3.0*p2*pow(delta,2.0))+4.0/29.0<<endl;
        return p1/(3.0*p2*pow(delta,2.0))+4.0/29.0;
    }
}
void CIELAB2RGB(float &a,float &b,float &c){
    /*
     X = Xn * ( P + a / 500 )^3
     Y = Yn * P^3
     Z = Zn * ( P - b / 200 )^3
     where P = (L + 16) / 116
     */
    float P = (a+16.0)/116.0,L=a,A=b,B=c;
    float Xn=95.047,Yn=100.0,Zn=108.883;
    float xyz[3];
    xyz[0] = Xn*pow(P+A/500.0,3.0);
    xyz[1] = Yn*pow(P,3.0);
    xyz[2] = Zn*pow(P-B/200.0,3.0);
    /*
     [ R ]   [  3.240479 -1.537150 -0.498535 ]   [ X ]
     [ G ] = [ -0.969256  1.875992  0.041556 ] * [ Y ]
     [ B ]   [  0.055648 -0.204043  1.057311 ]   [ Z ].*/
    float T_xyz2rgb[3][3] = {{3.240479, -1.537150, -0.498535},
                            {-0.969256,  1.875992,  0.041556 },
                            { 0.055648, -0.204043,  1.057311}};
    float rgb[3];
    for(int i=0;i<3;i++)
        rgb[i] = T_xyz2rgb[i][0]*xyz[0]+T_xyz2rgb[i][1]*xyz[1]+T_xyz2rgb[i][2]*xyz[2];
    a = rgb[0];
    b = rgb[1];
    c = rgb[2];
    /*float range = max(max(a,b),max(b,c));
    if (range>1)
    {
        a = 255.0*a/range;
        b = 255.0*b/range;
        c = 255.0*c/range;
    }*/
    if (a>1) a=1;
    if (b>1) b=1;
    if (c>1) c=1;

}
void Update_image_brightenness(unsigned char* img_data_tmp,unsigned char* img_data,int img_h,int img_w)
{
    float tmp_rgb[3],range = 255;
    for (int i = 0; i < img_h; i++){
        for (int j = 0; j < img_w; j++){
            tmp_rgb[0] = img_data[i*img_w*4 + j*4]/range;//Red
            tmp_rgb[1] = img_data[i*img_w*4 + j*4 + 1]/range;//Green
            tmp_rgb[2] = img_data[i*img_w*4 + j*4 + 2]/range;//Blue
            //printf("Origin:R:%f,G:%f,B:%f\n",tmp_rgb[0], tmp_rgb[1], tmp_rgb[2]);
            RGB2CIELAB(tmp_rgb[0], tmp_rgb[1], tmp_rgb[2]);
            tmp_rgb[0] = brighten_rate * tmp_rgb[0];
            CIELAB2RGB(tmp_rgb[0], tmp_rgb[1], tmp_rgb[2]);
            //printf("New:R:%f,G:%f,B:%f\n",tmp_rgb[0], tmp_rgb[1], tmp_rgb[2]);
            img_data_tmp[i*img_w*4 + j*4] = tmp_rgb[0]*range;  //Red
            img_data_tmp[i*img_w*4 + j*4 + 1] = tmp_rgb[1]*range; //Green
            img_data_tmp[i*img_w*4 + j*4 + 2] = tmp_rgb[2]*range; //Blue
            
            img_data_tmp[i*img_w*4 + j*4 + 3] = 255;  //Alpha
        }
    }
}
void world_mouse_xy2screen_mouse_xy(float &m_x,float &m_y){
    //input : m_x range [0 window_w],m_y [0 window_h]
    //output: m_x range [-1 1], m_y [-1 1]
    //window_w,window_h,screen_height,screen_width
    
    m_x = 2.0*m_x/(float)min(window_w,window_h)-1.0;
    m_y = 1.0-2.0*m_y/(float)min(window_w,window_h);
}
//TODO: Account for rotation by g_angle


//TODO: Choose between translate, rotate, and scale based on where the user clicked
// I've implemented the logic for translate and scale, but you need to add rotate


/////////////////////////////
/// ... below is OpenGL specifc code,
///     we will cover it in detail around Week 9,
///     but you should try to poke around a bit righ now.
///     I've annotated some parts with "TODO: Test ..." check those out.
////////////////////////////

// Shader sources
const GLchar* vertexSource =
"#version 150 core\n"
"in vec2 position;"
"in vec3 inColor;"
"in vec2 inTexcoord;"
"out vec3 Color;"
"out vec2 texcoord;"
"void main() {"
"   Color = inColor;"
"   gl_Position = vec4(position, 0.0, 1.0);"
"   texcoord = inTexcoord;"
"}";

const GLchar* fragmentSource =
"#version 150 core\n"
"uniform sampler2D tex0;"
"in vec2 texcoord;"
"out vec3 outColor;"
"void main() {"
"   outColor = texture(tex0, texcoord).rgb;"
"}";

bool fullscreen = false;

int main(int argc, char *argv[]){
    SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)
    
    //Ask SDL to get a fairly recent version of OpenGL (3.2 or greater)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    
    //Create a window (offsetx, offsety, width, height, flags)
    SDL_Window* window = SDL_CreateWindow("Dengyuan's Assignment0", 0, 0, screen_width, screen_height, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
    //TODO: Test your understanding: Try changing the title of the window to something more personalized.
    
    //The above window cannot be resized which makes some code slightly easier.
    //Below show how to make a full screen window or allow resizing
    //SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 0, 0, screen_width, screen_height, SDL_WINDOW_FULLSCREEN|SDL_WINDOW_OPENGL);
    //SDL_Window* window = SDL_CreateWindow("Dengyuan's Assignment0", 0, 0, screen_width, screen_height, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
    //SDL_Window* window = SDL_CreateWindow("My OpenGL Program",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,0,0,SDL_WINDOW_FULLSCREEN_DESKTOP|SDL_WINDOW_OPENGL); //Boarderless window "fake" full screen
    
    float aspect = screen_width/(float)screen_height; //aspect ratio (needs to be updated if the window is resized)
    geo_fig square1(0.5f,0.0f,0.3f,0*PI/180,true,true,screen_width),square2(-0.5f,0.0f,0.3f,0*PI/180,true,false,screen_width);
    geo_fig triangle(0.0f,0.5f,0.3f,45*PI/180,false,false,screen_width);
    geo_fig g_indicator(-0.7f,0.7f,0.1f,0*PI/180,true,false,screen_width);
    square1.updateVertices(); //set initial position of the square to match it's state
    if(Multi_square==1)
    {
        square2.updateVertices();
        triangle.updateVertices();
        g_indicator.updateVertices();
    }
    
    //Create a context to draw in
    SDL_GLContext context = SDL_GL_CreateContext(window);
    
    //OpenGL functions using glad library
    if (gladLoadGLLoader(SDL_GL_GetProcAddress)){
        printf("OpenGL loaded\n");
        printf("Vendor:   %s\n", glGetString(GL_VENDOR));
        printf("Renderer: %s\n", glGetString(GL_RENDERER));
        printf("Version:  %s\n", glGetString(GL_VERSION));
    }
    else {
        printf("ERROR: Failed to initialize OpenGL context.\n");
        return -1;
    }
    
    //// Allocate Texture 0 (Created in Load Image) ///////
    GLuint tex0,tex_T,tex_S,tex_R;
    glGenTextures(1, &tex0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);
    
    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //GL_LINEAR
    //TODO: Test your understanding: Try GL_LINEAR instead of GL_NEAREST on the 4x4 test image. What is happening?
    
    
    int img_w, img_h;
    unsigned char* img_data = loadImage(img_w,img_h,"goldy.ppm");
    unsigned char* img_data_Translate = loadImage(img_w,img_h,"test.ppm");
    unsigned char* img_data_Scale = loadImage(img_w,img_h,"brick.ppm");
    unsigned char* img_data_Rotate = loadImage(img_w,img_h,"goldy.ppm");
    unsigned char* img_data_temp = new unsigned char[4*img_w*img_h];
    printf("Loaded Image of size (%d,%d)\n",img_w,img_h);
    //memset(img_data,0,4*img_w*img_h); //Load all zeros
    //Load the texture into memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    //// End Allocate Texture ///////
    
    
    //Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
    GLuint vao;
    glGenVertexArrays(1, &vao); //Create a VAO
    glBindVertexArray(vao); //Bind the above created VAO to the current context
    
    
    //Allocate memory on the graphics card to store geometry (vertex buffer object)
    GLuint vbo;
    glGenBuffers(1, &vbo);  //Create 1 buffer called vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
    glBufferData(GL_ARRAY_BUFFER, sizeof(square1.vertices), square1.vertices, GL_DYNAMIC_DRAW); //upload vertices to vbo
    //GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
    //GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used
    
    
    //Load the vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    
    //Let's double check the shader compiled
    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (!status){
        char buffer[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
        printf("Vertex Shader Compile Failed. Info:\n\n%s\n",buffer);
    }
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    
    //Double check the shader compiled
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (!status){
        char buffer[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
        printf("Fragment Shader Compile Failed. Info:\n\n%s\n",buffer);
    }
    
    //Join the vertex and fragment shaders together into one program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor"); // set output
    glLinkProgram(shaderProgram); //run the linker
    
    glUseProgram(shaderProgram); //Set the active shader (only one can be used at a time)
    
    
    //Tell OpenGL how to set fragment shader input
    
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    //               Attribute, vals/attrib., type, normalized?, stride, offset
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7*sizeof(float), 0);
    glEnableVertexAttribArray(posAttrib); //Binds the VBO current GL_ARRAY_BUFFER
    
    GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(colAttrib);
    
    GLint texAttrib = glGetAttribLocation(shaderProgram, "inTexcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(5*sizeof(float)));
    
    
    //Event Loop (Loop forever processing each event as fast as possible)
    SDL_Event windowEvent;
    bool done = false;
    int action_index;//0 for none,1 for Translate, 2 for Scale, 3 for Rotation,
    while (!done){
        action_index = 0;
        while (SDL_PollEvent(&windowEvent)){  //Process input events (e.g., mouse & keyboard)
            if (windowEvent.type == SDL_QUIT) done = true;
            //List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
            //Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
                done = true; //Exit event loop
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) //If "f" is pressed
                fullscreen = !fullscreen;
            SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Set to full screen
        }
        SDL_GetWindowSize(window,&window_w,&window_h);
        //cout<<min(window_w,window_h)<<endl;
        
        square1.change_size(min(window_w,window_h));
        square2.change_size(min(window_w,window_h));
        triangle.change_size(min(window_w,window_h));
      //GLWidget::resizeGL(int width, int height)
        glViewport(0, 0, min(window_w,window_h), min(window_w,window_h));
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);;

        //screen_width = window_w;
        //screen_height = window_h;
        //printf("window_w:%d,window_h:%d\n",window_w,window_h);
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        if (state[SDL_SCANCODE_UP]==true)
        {
            brighten_rate = min(brighten_rate+0.05,2.0);
            Update_image_brightenness(img_data_temp,img_data,img_w, img_h);
            //Load the texture into memory
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data_temp);
            glGenerateMipmap(GL_TEXTURE_2D);
            //// End Allocate Texture ///////
        }
        if (state[SDL_SCANCODE_DOWN]==true)
        {
            brighten_rate = max(brighten_rate-0.05,0.0);
            Update_image_brightenness(img_data_temp, img_data,img_w, img_h);
            //Load the texture into memory
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data_temp);
            glGenerateMipmap(GL_TEXTURE_2D);
            //// End Allocate Texture ///////
        }
        int mx, my;

        if (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT)) { //Is the mouse down?
            float sc_mx=mx;
            float sc_my=my;
            world_mouse_xy2screen_mouse_xy(sc_mx, sc_my);
            //printf("mx:%d,my:%d\nsc_mx:%f,sc_my:%f\n",mx,my,sc_mx,sc_my);
            
            if (g_mouse_down == false){
                square1.mouseClicked(sc_mx,sc_my);
                if(Multi_square==1)
                {
                     square2.mouseClicked(sc_mx,sc_my);
                     triangle.mouseClicked(sc_mx,sc_my);
                }
                
            }
            else{
                square1.mouseDragged(sc_mx,sc_my);
                if(Multi_square==1)
                {
                    square2.mouseDragged(sc_mx,sc_my);
                    triangle.mouseDragged(sc_mx,sc_my);
                }
                
            }
            g_mouse_down = true;
        }
        else{
            g_mouse_down = false;
        }

        square1.self_animate();
        square2.self_animate();
        triangle.self_animate();
        if (square1.g_bTranslate||square2.g_bTranslate||triangle.g_bTranslate)
            action_index = 1;
        else if((square1.g_bScale ||square2.g_bScale||triangle.g_bScale))
            action_index = 2;
        else if((square1.g_bRotate ||square2.g_bRotate||triangle.g_bRotate))
            action_index = 2;
        // Clear the screen to white
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        switch (action_index) {
            case 1:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data_Translate);
                glGenerateMipmap(GL_TEXTURE_2D);
                break;
            case 2:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data_Scale);
                glGenerateMipmap(GL_TEXTURE_2D);
                break;
            case 3:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data_Rotate);
                glGenerateMipmap(GL_TEXTURE_2D);
                break;
            
            default:
                break;
        }
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_indicator.vertices), g_indicator.vertices, GL_DYNAMIC_DRAW); //upload vertices to vbo
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //Draw the two triangles (4 vertices) making up the square
        if(action_index!=0)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(square1.vertices), square1.vertices, GL_DYNAMIC_DRAW); //upload vertices to vbo
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //Draw the two triangles (4 vertices) making up the square
        
        if(Multi_square==1)
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(square2.vertices), square2.vertices, GL_DYNAMIC_DRAW); //upload vertices to vbo
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //Draw the two triangles (4 vertices) making up the square
            glBufferData(GL_ARRAY_BUFFER, sizeof(triangle.vertices), triangle.vertices, GL_DYNAMIC_DRAW); //upload vertices to vbo
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 3); //Draw the two triangles (4 vertices) making up the square
            /*cout<<sizeof(square1.vertices)/sizeof(float)<<endl;
            for(int i=0;i<sizeof(square1.vertices)/sizeof(float);i++)
            {
                if (i%7==0)
                    cout<<endl;
                printf("%f ",square1.vertices[i]);
                
            }*/
        }
        
        //TODO: Test your understanding: What shape do you expect to see if you change the above 4 to 3?  Guess, then try it!
        
        SDL_GL_SwapWindow(window); //Double buffering
    }
    
    delete [] img_data;
    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
    
    glDeleteBuffers(1, &vbo);
    
    glDeleteVertexArrays(1, &vao);
    
    
    //Clean Up
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}
