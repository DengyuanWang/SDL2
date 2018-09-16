//
//  main.cpp
//  SDL2_test
//
//  Created by 王登远 on 2018/9/10.
//  Copyright © 2018 王登远. All rights reserved.
//

/*
#include <iostream>
#include <SDL2/SDL.h>

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    return 0;
}
 */


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
#define PI 3.141592653
using namespace std;

//Name of image texture
string textureName = "goldy.ppm";
bool Debug_tag = true;
float brighten_rate = 1;
//string textureName = "brick.ppm";
//string textureName = "/Users/wangdengyuan/Desktop/source code for xcode/SDL2_test/SDL2_test/test.ppm";

//Globals to store the state of the square (position, width, and angle)
float g_pos_x = 0.0f;
float g_pos_y = 0.0f;
float g_size = 0.6f;
float g_angle = 0*PI/180;

float vertices[] = {  //These values should be updated to match the square's state when it changes
    //  X     Y     R     G     B     U    V
    0.3f,  0.3f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
    0.3f, -0.3f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
    -0.3f,  0.3f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // top left
    -0.3f, -0.3f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom left
};
float R[4][4]={{cos(g_angle), -sin(g_angle), 0, 0},
                {sin(g_angle), cos(g_angle), 0, 0},
                {0,             0,          1, 0},
                {0,             0,          0, 1}};
float T[4][4]={{1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 1}};

int screen_width = 800;
int screen_height = 800;

float g_mouse_down = false;
float g_clicked_x = -1;
float g_clicked_y = -1;
float g_lastCenter_x = -1;
float g_lastCenter_y = -1;
float g_clicked_angle = -1;
float g_clicked_size = -1;
int rotation_corner = -1;
void mouseClicked(float mx, float my);
void mouseDragged(float mx, float my);
void RelativeCoordinate2WorldCoordinate(float &x,float &y);
void WorldCoordinate2RelativeCoordinate(float &x,float &y);
void RGB2CIELAB(float &a,float &b,float &c);
float Funf(float p1,float p2);
void CIELAB2RGB(float &a,float &b,float &c);
void Update_image_brightenness(unsigned char* img_data_tmp,unsigned char* img_data,int img_h,int img_w);
bool g_bTranslate = false;
bool g_bRotate = false;
bool g_bScale = false;

//////////////////////////
///  Begin your code here
/////////////////////////

//TODO: Read from ASCII (P6) PPM files
//Inputs are output variables for returning the image width and heigth
unsigned char* loadImage(int& img_w, int& img_h){
    
    //Open the texture image file
    ifstream ppmFile;
   /* ofstream ppmFile_out;
    ppmFile_out.open("out_test.ppm");
    ppmFile_out.write("asdasd",3);
    ppmFile_out.close();*/
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
void RelativeCoordinate2WorldCoordinate(float &x,float &y)
{
   /* float x2,y2;
    float angle = g_angle;
    x2 = x*cos(angle) - y*sin(angle);
    y2 = x*sin(angle) + y*cos(angle);
    x = x2+g_pos_x;y = y2+g_pos_y;*/
    
    float v_world[4]={1,1,1,1},v_relative[4]={1,1,1,1},c[4][4];
    float sum = 0;
    v_relative[0]= x;v_relative[1]= y;
    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            sum = 0;
            for(int k=0;k<4;k++)
            {
                sum = sum+T[i][k]*R[k][j];
            }
            c[i][j]= sum;
        }
    }
    for(int i=0;i<4;i++)
    {
        sum = 0;
        for(int k=0;k<4;k++)
        {
            sum = sum+c[i][k]*v_relative[k];
        }
        v_world[i]= sum;
    }
    x = v_world[0];
    y = v_world[1];
    //v_world =T*R* v_relative;
    
}
void WorldCoordinate2RelativeCoordinate(float &x,float &y)
{
    /*float x2,y2;
    float angle = g_angle;
    x2 = (x-g_pos_x)*cos(angle) + (y-g_pos_y)*sin(angle);
    y2 = -(x-g_pos_x)*sin(angle) + (y-g_pos_y)*cos(angle);
    x = x2+g_pos_x;y = y2+g_pos_y;*/
    float R_reverse[4][4],T_reverse[4][4],c[4][4];
    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            R_reverse[i][j] = R[i][j];
            T_reverse[i][j] = T[i][j];
        }
        T_reverse[i][3] = -T_reverse[i][3];
    }
    T_reverse[3][3]=1;
    R_reverse[0][1] = -R_reverse[0][1];R_reverse[1][0] = -R_reverse[1][0];
    float v_world[4]={1,1,1,1},v_relative[4]={1,1,1,1};
    float sum = 0;
    v_world[0]= x;v_world[1]= y;
    for(int i=0;i<4;i++)
    {
        for(int j=0;j<4;j++)
        {
            sum = 0;
            for(int k=0;k<4;k++)
            {
                sum = sum+R_reverse[i][k]*T_reverse[k][j];
            }
           c[i][j]= sum;
        }
    }
    for(int i=0;i<4;i++)
    {
            sum = 0;
            for(int k=0;k<4;k++)
            {
                sum = sum+c[i][k]*v_world[k];
            }
            v_relative[i]= sum;
    }
    x = v_relative[0];
    y = v_relative[1];
    //v_relative =R^-1*T^-1* v_world;
}
//TODO: Account for rotation by g_angle
void updateVertices(){
    float vx = g_size;
    float vy =  g_size;
    //float Top_right_x,Top_right_y,Top_left_x,Top_left_y,Bottom_right_x,Bottom_right_y,Bottom_left_x,Bottom_left_y;
    
    vertices[0] = vx;  //Top right x
    vertices[1] = vy;  //Top right y
    
    vx = g_size;
    vy = - g_size;
    vertices[7] = vx;  //Bottom right x
    vertices[8] = vy;  //Bottom right y
    
    vx = - g_size;
    vy = + g_size;
    vertices[14] = vx;  //Top left x
    vertices[15] = vy;  //Top left y
    
    vx = - g_size;
    vy = - g_size;
    vertices[21]= vx;  //Bottom left x
    vertices[22] = vy;  //Bottom left y
    
    for(int i=0;i<4;i=i+1)
    {
        RelativeCoordinate2WorldCoordinate(vertices[0+i*7],vertices[1+i*7]);
    }
}

//TODO: Choose between translate, rotate, and scale based on where the user clicked
// I've implemented the logic for translate and scale, but you need to add rotate
void mouseClicked(float m_x, float m_y){
    int near_corner(float x,float y);
    printf("Clicked at %f, %f\n",m_x,m_y);
    g_clicked_x = m_x;
    g_clicked_y = m_y;
    g_lastCenter_x = g_pos_x;
    g_lastCenter_y = g_pos_y;
    g_clicked_angle = g_angle;
    g_clicked_size = g_size;
    
    g_bTranslate = false;
    g_bRotate = false;
    g_bScale = false;
    // x and y is the click position normalized to size of the square, with (-1,-1) at one corner (1,1) the other
    //   float x = m_x - g_pos_x;
    // float y = m_y - g_pos_y;
    float x = m_x;
    float y = m_y;
    WorldCoordinate2RelativeCoordinate(x,y);

    x = x / g_size;//norm by length 1
    y = y / g_size;//norm by length 1
    
    printf("Normalized click coord: %f, %f\n",x,y);
    float absOffset_outside = 1.05;
    float absOffset_inside = 0.9;
    // mouse clicked outside squre, then do nothing.
    if (x > absOffset_outside || y > absOffset_outside ||
            x < -absOffset_outside || y < -absOffset_outside) return; //TODO: Test your understanding: Why 1.05 and not 1?
    
    if (x < absOffset_inside && x > -absOffset_inside &&
            y < absOffset_inside && y > -absOffset_inside){ //TODO: Test your understanding: What happens if you change .9 to .8?
        // mouse clicked inside the squre, then translating
        g_bTranslate = true;
        if (Debug_tag) printf("Translate!\n");
    }
    else if(near_corner(x,y)>0){
        g_bRotate = true;
        rotation_corner = near_corner(x,y);
        if (Debug_tag) printf("rotation!\n");
    }
    else{
        // mouse clicked at the edges, then scaling
        g_bScale = true;
        if (Debug_tag) printf("Scale!\n");
    }
/*
 ((x < absOffset_outside && x > Offset_corner*absOffset_inside &&  y < absOffset_inside && y > Offset_corner*absOffset_inside)||
 (x > -absOffset_inside && x < -Offset_corner*absOffset_inside &&  y < absOffset_inside && y > Offset_corner*absOffset_inside)||
 (x < absOffset_inside && x > Offset_corner*absOffset_inside &&  y > -absOffset_inside && y < -Offset_corner*absOffset_inside)||
 (x > -absOffset_inside && x < -Offset_corner*absOffset_inside &&  y > -absOffset_inside && y < -Offset_corner*absOffset_inside))
 */


}
int near_corner(float x,float y)
{
    if ( 0.1>sqrt(pow(x-1,2)+pow(y-1,2))) return 1;
    if ( 0.1>sqrt(pow(x+1,2)+pow(y-1,2))) return 2;
    if ( 0.1>sqrt(pow(x+1,2)+pow(y+1,2))) return 3;
    if ( 0.1>sqrt(pow(x-1,2)+pow(y+1,2))) return 4;
    return 0;
}
//TODO: Update the position, rotation, or scale based on the mouse movement
//  I've implemented the logic for position, you need to do scaling and angle
//TODO: Notice how smooth draging the square is (e.g., there are no "jumps" when you click),
//      try to make your implementation of rotate and scale as smooth
void mouseDragged(float m_x, float m_y){
    float w_x,w_y,r_x,r_y;//world coordinate & relative coordinate
    w_x = m_x;w_y = m_y;
    r_x = m_x;r_y = m_y;
    WorldCoordinate2RelativeCoordinate(r_x,r_y);

    if (g_bTranslate){// translate is in world coordinate
        g_pos_x = w_x-g_clicked_x+g_lastCenter_x;
        g_pos_y = w_y-g_clicked_y+g_lastCenter_y;
        T[0][3] = w_x-g_clicked_x+g_lastCenter_x; T[1][3] = w_y-g_clicked_y+g_lastCenter_y;// set Translation matrix
    }
    
    if (g_bScale){// scale should be in relative coordinate
        //Compute the new size, g_size, based on the mouse positions
        // result: the edges will follow the mouse position to make sure the mouse is on the edge.

        if (abs(r_x)>abs(r_y)){
            g_size = abs(r_x);
        }
        else{
            g_size = abs(r_y);
        }
      
      // if(Debug_tag) cout<<"g_size"<<g_size<<endl;
        float min_size = 0.05f;
        g_size = max(g_size,min_size);
    }
    
    if (g_bRotate){// rotation should be in relative coordinate
        //Compute the new angle, g_angle, based on the mouse positions
        float r,theta_offset,theta0,delta_theta;
        r = sqrt(r_x*r_x+r_y*r_y);
        if (r>g_size/2) // mouse appoaching to the origin of coordinates, return to prevent math defination error
        {
            switch (rotation_corner) {
                case 1://Quadrant 1
                    theta_offset = PI/4;
                    break;
                case 2://Quadrant 2
                    theta_offset = 3*PI/4;
                    break;
                case 3://Quadrant 3
                    theta_offset = 5*PI/4;
                    break;
                case 4://Quadrant 4
                    theta_offset = 7*PI/4;
                    break;
                default:
                    printf("error in line 394,rotation_corner name wrong\n");
                    exit(0);
                    break;
            }
            if(r_x>0)
            {
                if(r_y>0){
                    theta0 = atan(r_y/r_x);
                    cout<<r_y/r_x<<endl;
                    if(Debug_tag)cout<<">0,>0"<<endl;
                }else if(r_y==0){
                    theta0 = 0;
                    if(Debug_tag)cout<<">0,=0"<<endl;
                }else{
                    theta0 = atan(r_y/r_x);
                    if(Debug_tag)cout<<">0,<0"<<endl;
                }
            }
            else if(r_x==0){
                if(r_y>0){
                    theta0 = PI/2;
                    if(Debug_tag)cout<<"=0,>0"<<endl;
                }else if(r_y<0){
                    theta0 = -PI/2;
                    if(Debug_tag)cout<<"=0,<0"<<endl;
                }else{
                    printf("error in line 413,x==0&&y==0,math defination error\n");
                    exit(0);
                }
            }
            else{//r_x<0
                if(r_y>0){
                    theta0 =  PI+atan(r_y/r_x);
                    if(Debug_tag)cout<<"<0,>0"<<endl;
                }else if(r_y==0){
                    theta0 = PI;
                    if(Debug_tag)cout<<"<0,=0"<<endl;
                }else{//r_y<
                    theta0 = PI+atan(r_y/r_x);
                    if(Debug_tag)cout<<"<0,<0"<<endl;
                }
            }
            delta_theta = theta0-theta_offset;
            printf("theta0=%f,theta_offset=%f\n",180*theta0/PI,180*theta_offset/PI);
            
            g_angle = delta_theta+g_angle;
            while(g_angle>2*PI)
                g_angle = g_angle-2*PI;
            while(g_angle<0)
                g_angle = g_angle+2*PI;
            cout<<g_angle<<endl;
            
            R[0][0] = cos(g_angle);R[0][1] = -sin(g_angle);
            R[1][0] = sin(g_angle);R[1][1] = cos(g_angle);
            /* R[4][4]={{cos(g_angle), -sin(g_angle), 0, 0},
             {sin(g_angle), cos(g_angle), 0, 0},
             {0,             0,          1, 0},
             {0,             0,          0, 1}};*/
        }

    }
    
    updateVertices();
}

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
    SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screen_width, screen_height, SDL_WINDOW_OPENGL);
    //TODO: Test your understanding: Try changing the title of the window to something more personalized.
    
    //The above window cannot be resized which makes some code slightly easier.
    //Below show how to make a full screen window or allow resizing
    //SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 0, 0, screen_width, screen_height, SDL_WINDOW_FULLSCREEN|SDL_WINDOW_OPENGL);
    //SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screen_width, screen_height, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
    //SDL_Window* window = SDL_CreateWindow("My OpenGL Program",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,0,0,SDL_WINDOW_FULLSCREEN_DESKTOP|SDL_WINDOW_OPENGL); //Boarderless window "fake" full screen
    
    float aspect = screen_width/(float)screen_height; //aspect ratio (needs to be updated if the window is resized)
    
    updateVertices(); //set initial position of the square to match it's state
    
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
    GLuint tex0;
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
    unsigned char* img_data = loadImage(img_w,img_h);
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW); //upload vertices to vbo
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
    while (!done){
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
            if (g_mouse_down == false){
                mouseClicked(2*mx/(float)screen_width - 1, 1-2*my/(float)screen_height);
            }
            else{
                mouseDragged(2*mx/(float)screen_width-1, 1-2*my/(float)screen_height);
            }
            g_mouse_down = true;
        }
        else{
            g_mouse_down = false;
        }
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW); //upload vertices to vbo
        
        
        // Clear the screen to white
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //Draw the two triangles (4 vertices) making up the square
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
