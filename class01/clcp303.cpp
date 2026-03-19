#define _CRT_SECURE_NO_WARNINGS
#include <GL/glut.h>
#include <cmath>
#include <iostream>
using namespace std;

const int WIDTH = 1000;
const int HEIGHT = 800;

float viewAngleX = 20.0f;
float viewAngleY = 30.0f;

// 平面1：位置 + 欧拉角
float px1 = 0, py1 = 0, pz1 = 0;
float rx1 = 0, ry1 = 20, rz1 = 0;

// 平面2：位置 + 欧拉角
float px2 = 0, py2 = 0, pz2 = 0;
float rx2 = 45, ry2 = 0, rz2 = 0;

bool dragging = false;
int lastX, lastY;
int selected = 0;

// ----------------------------------------------------
// 【工具】向量叉积
// ----------------------------------------------------
void cross(float a[3], float b[3], float res[3]) {
    res[0] = a[1] * b[2] - a[2] * b[1];
    res[1] = a[2] * b[0] - a[0] * b[2];
    res[2] = a[0] * b[1] - a[1] * b[0];
}

// ----------------------------------------------------
// 【工具】向量点积
// ----------------------------------------------------
float dot(float a[3], float b[3]) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

// ----------------------------------------------------
// 【工具】向量归一化
// ----------------------------------------------------
void normalize(float v[3]) {
    float len = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (len > 1e-6) {
        v[0] /= len;
        v[1] /= len;
        v[2] /= len;
    }
}

// ----------------------------------------------------
// 【核心1】从欧拉角计算平面的法向量和平面方程
// ----------------------------------------------------
void getPlaneFromEuler(float rx, float ry, float rz, float x, float y, float z,
    float normal[3], float& d)
{
    float rad = 3.14159f / 180.0f;
    float crx = cos(rx * rad);
    float srx = sin(rx * rad);
    float cry = cos(ry * rad);
    float sry = sin(ry * rad);

    // 初始法向量 (0,0,1)
    float n[3] = { 0, 0, 1 };

    // 绕 X 轴旋转
    float ny = n[1] * crx - n[2] * srx;
    float nz = n[1] * srx + n[2] * crx;
    n[1] = ny;
    n[2] = nz;

    // 绕 Y 轴旋转
    float nx = n[0] * cry + n[2] * sry;
    nz = -n[0] * sry + n[2] * cry;
    n[0] = nx;
    n[2] = nz;

    normal[0] = n[0];
    normal[1] = n[1];
    normal[2] = n[2];

    // 平面方程：ax + by + cz + d = 0
    float p[3] = { x, y, z };
    d = -dot(normal, p);
}

// ----------------------------------------------------
// 【核心2】两平面求交线
// ----------------------------------------------------
bool planeIntersection(
    float n1[3], float d1,
    float n2[3], float d2,
    float linePt[3],
    float lineDir[3]
) {
    // 1. 交线方向 = 法向量叉积
    cross(n1, n2, lineDir);

    float len = sqrt(lineDir[0] * lineDir[0] +
        lineDir[1] * lineDir[1] +
        lineDir[2] * lineDir[2]);
    if (len < 1e-6)
        return false;

    // 2. 求交线上一点
    float n1n1 = dot(n1, n1);
    float n2n2 = dot(n2, n2);
    float n1n2 = dot(n1, n2);
    float det = n1n1 * n2n2 - n1n2 * n1n2;

    float c1 = (-d1 * n2n2 + d2 * n1n2) / det;
    float c2 = (-d2 * n1n1 + d1 * n1n2) / det;

    linePt[0] = c1 * n1[0] + c2 * n2[0];
    linePt[1] = c1 * n1[1] + c2 * n2[1];
    linePt[2] = c1 * n1[2] + c2 * n2[2];

    return true;
}

// ----------------------------------------------------
// 绘制平面
// ----------------------------------------------------
void drawPlane(float rx, float ry, float rz, float x, float y, float z, float r, float g, float blue)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(ry, 0, 1, 0);
    glRotatef(rx, 1, 0, 0);

    glColor3f(r, g, blue);
    glBegin(GL_QUADS);
    glVertex3f(-2, -2, 0);
    glVertex3f(2, -2, 0);
    glVertex3f(2, 2, 0);
    glVertex3f(-2, 2, 0);
    glEnd();

    glPopMatrix();
}

// ----------------------------------------------------
// 绘制交线
// ----------------------------------------------------
void drawIntersection()
{
    float n1[3], d1;
    getPlaneFromEuler(rx1, ry1, rz1, px1, py1, pz1, n1, d1);

    float n2[3], d2;
    getPlaneFromEuler(rx2, ry2, rz2, px2, py2, pz2, n2, d2);

    float pt[3], dir[3];
    if (!planeIntersection(n1, d1, n2, d2, pt, dir))
        return;

    glColor3f(1, 0, 0);
    glLineWidth(4);
    glBegin(GL_LINES);
    glVertex3f(pt[0] - dir[0] * 5, pt[1] - dir[1] * 5, pt[2] - dir[2] * 5);
    glVertex3f(pt[0] + dir[0] * 5, pt[1] + dir[1] * 5, pt[2] + dir[2] * 5);
    glEnd();
}

// ----------------------------------------------------
// 显示
// ----------------------------------------------------
void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0, 0, -8);
    glRotatef(viewAngleX, 1, 0, 0);
    glRotatef(viewAngleY, 0, 1, 0);

    drawPlane(rx1, ry1, rz1, px1, py1, pz1, 1, 1, 0);
    drawPlane(rx2, ry2, rz2, px2, py2, pz2, 0, 1, 1);
    drawIntersection();

    glutSwapBuffers();
}

// ----------------------------------------------------
// 鼠标
// ----------------------------------------------------
void mouse(int btn, int state, int x, int y)
{
    if (btn == GLUT_LEFT_BUTTON) {
        dragging = (state == GLUT_DOWN);
        lastX = x;
        lastY = y;
    }
    if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        selected = (selected + 1) % 3;
        cout << "控制目标：0视角 1平面1 2平面2 → 当前：" << selected << endl;
    }
}

void motion(int x, int y)
{
    if (!dragging) return;

    int dx = x - lastX;
    int dy = y - lastY;
    lastX = x;
    lastY = y;

    if (selected == 0) {
        viewAngleX += dy * 0.3f;
        viewAngleY += dx * 0.3f;
    }
    else if (selected == 1) {
        rx1 += dy * 0.5f;
        ry1 += dx * 0.5f;
    }
    else if (selected == 2) {
        rx2 += dy * 0.5f;
        ry2 += dx * 0.5f;
    }

    glutPostRedisplay();
}

void init()
{
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    gluPerspective(50, (double)WIDTH / HEIGHT, 0.1, 100);
    glMatrixMode(GL_MODELVIEW);
}

// ----------------------------------------------------
// 主函数
// ----------------------------------------------------
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("3D 双平面可旋转 求交线（完美版）");
    init();
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMainLoop();
    return 0;
}