//
//  triangle.cpp
//  SDL2_test
//
//  Created by 王登远 on 2018/9/16.
//  Copyright © 2018 王登远. All rights reserved.
//

#include "geo_fig.hpp"
#define PI 3.141592653
#define Debug_tag  0
using namespace std;
class geo_fig {
    public:
    float vertices[7*4] = {  //These values should be updated to match the square's state when it changes
        //  X     Y     R     G     B     U    V
        0.3f,  0.3f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
        0.3f, -0.3f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
        -0.3f,  0.3f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // top left
        -0.3f, -0.3f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom left
    };
    float last_screen_size;
    bool square_tag = true;
    bool animate_tag = false;
    float g_pos_x = 0.0f;
    float g_pos_y = 0.0f;
    float g_size = 0.6f;
    float g_angle = 0*PI/180;
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
    float R[4][4]={{cos(g_angle), -sin(g_angle), 0, 0},
        {sin(g_angle), cos(g_angle), 0, 0},
        {0,             0,          1, 0},
        {0,             0,          0, 1}};
    float T[4][4]={{1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}};
    geo_fig(){
        g_pos_x = 0.0f;
        g_pos_y = 0.0f;
        g_size = 0.6f;
        g_angle = 0*PI/180;
        R[0][0] = cos(g_angle);R[0][1] = -sin(g_angle);
        R[1][0] = sin(g_angle);R[1][1] = cos(g_angle);
        animate_tag = false;
        last_screen_size = 800;
    }
    geo_fig(float pos_x, float pos_y, float size, float angle, bool square, bool animate,float screen_size){
        T[0][3] = pos_x;g_pos_x = pos_x;
        T[1][3] = pos_y;g_pos_y = pos_y;
        g_size = size;
        g_angle = angle;
        R[0][0] = cos(g_angle);R[0][1] = -sin(g_angle);
        R[1][0] = sin(g_angle);R[1][1] = cos(g_angle);
        square_tag = square;
        animate_tag = animate;
        last_screen_size = screen_size;
    }
    void change_size(float size)
    {
        g_size =size*g_size/last_screen_size;
        last_screen_size = size;
        updateVertices();
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
    void mouseClicked(float m_x, float m_y){
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
        if (square_tag==true)
        {
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
        }
        else{
            // mouse clicked outside squre, then do nothing.
            if (x > absOffset_outside || y > absOffset_outside ||
                x+y<0) return; //TODO: Test your understanding: Why 1.05 and not 1?
            
            if (x < absOffset_inside && x > -absOffset_inside &&
                y < absOffset_inside && y > -absOffset_inside &&
                x+y>0){ //TODO: Test your understanding: What happens if you change .9 to .8?
                // mouse clicked inside the squre, then translating
                g_bTranslate = true;
                if (Debug_tag) printf("Translate!\n");
            }
            else if(near_corner(x,y)>0&&near_corner(x,y)!=3){
                g_bRotate = true;
                rotation_corner = near_corner(x,y);
                if (Debug_tag) printf("rotation!\n");
            }
            else{
                // mouse clicked at the edges, then scaling
                g_bScale = true;
                if (Debug_tag) printf("Scale!\n");
            }
        }
        
        
        
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
                        //cout<<r_y/r_x<<endl;
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
                //printf("theta0=%f,theta_offset=%f\n",180*theta0/PI,180*theta_offset/PI);
                
                g_angle = delta_theta+g_angle;
                while(g_angle>2*PI)
                g_angle = g_angle-2*PI;
                while(g_angle<0)
                g_angle = g_angle+2*PI;
                //cout<<g_angle<<endl;
                
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
    void self_animate()
    {
        if (animate_tag==true)
        {
            float delta_theta = PI/180;
            g_angle = delta_theta+g_angle;
            while(g_angle>2*PI)
            g_angle = g_angle-2*PI;
            while(g_angle<0)
            g_angle = g_angle+2*PI;
            //cout<<g_angle<<endl;
            
            R[0][0] = cos(g_angle);R[0][1] = -sin(g_angle);
            R[1][0] = sin(g_angle);R[1][1] = cos(g_angle);
            updateVertices();
        }
    }
};
