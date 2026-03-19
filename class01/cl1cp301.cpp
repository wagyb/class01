#define _CRT_SECURE_NO_WARNINGS
#include <GL/glut.h>
#include <iostream>
#include <cmath>

using namespace std;

// ===================== 坐标设定：屏幕中心为原点 =====================
const int WIDTH = 800;
const int HEIGHT = 600;

// 固定线段 1
float x1 = -200, y1_p = -100;
float x2 = 200, y2 = 150;

// 可拖动线段 2
float x3 = -150, y3 = 200;
float x4 = 150, y4 = -200;

bool isDragging = false;

// ===================== 【核心算法】两直线交点 =====================
// 输入：4个点，代表两条直线
// 输出：是否相交 + 交点坐标 (px, py)
bool lineIntersection(
    float x1, float y1_p, float x2, float y2,
    float x3, float y3, float x4, float y4,
    float& px, float& py
) {
    float denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1_p);

    // 平行 → 无交点
    if (fabs(denom) < 1e-6)
        return false;

    float ua = ((x4 - x3) * (y1_p - y3) - (y4 - y3) * (x1 - x3)) / denom;
    float ub = ((x2 - x1) * (y1_p - y3) - (y2 - y1_p) * (x1 - x3)) / denom;

    // 计算交点（直线交点，不是线段！）
    px = x1 + ua * (x2 - x1);
    py = y1_p + ua * (y2 - y1_p);

    return true;
}

// ===================== 辅助：判断点是否在线段上 =====================
bool pointOnSegment(float px, float py, float x1, float y1_p, float x2, float y2) {
    if (min(x1, x2) - 1 <= px && px <= max(x1, x2) + 1 &&
        min(y1_p, y2) - 1 <= py && py <= max(y1_p, y2) + 1)
        return true;
    return false;
}

// ===================== 绘制 =====================
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // 画固定线段 1（白色）
    glColor3f(1, 1, 1);
    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex2f(x1, y1_p);
    glVertex2f(x2, y2);
    glEnd();

    // 画可拖动线段 2（黄色）
    glColor3f(1, 1, 0);
    glBegin(GL_LINES);
    glVertex2f(x3, y3);
    glVertex2f(x4, y4);
    glEnd();

    // ========== 计算并绘制交点 ==========
    float px, py;
    bool hasIntersect = lineIntersection(x1, y1_p, x2, y2, x3, y3, x4, y4, px, py);

    if (hasIntersect) {
        bool onSeg1 = pointOnSegment(px, py, x1, y1_p, x2, y2);
        bool onSeg2 = pointOnSegment(px, py, x3, y3, x4, y4);

        if (onSeg1 && onSeg2) {
            glColor3f(0, 1, 0); // 真正线段相交：绿色
        }
        else {
            glColor3f(1, 0, 1); // 延长线交点：紫色
        }

        // 画交点
        glPointSize(10);
        glBegin(GL_POINTS);
        glVertex2f(px, py);
        glEnd();
    }
    else {
        cout << "两条直线平行，无交点" << endl;
    }

    glutSwapBuffers();
}

// ===================== 鼠标拖动：改变线段2 =====================
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) isDragging = true;
        else isDragging = false;
    }
}

void motion(int x, int y) {
    if (!isDragging) return;

    // 屏幕坐标 → 中心原点坐标
    float wx = x - WIDTH / 2;
    float wy = HEIGHT / 2 - y;

    // 拖动 → 修改线段2的一个端点
    x4 = wx;
    y4 = wy;

    glutPostRedisplay();
}

// ===================== 初始化 =====================
void init() {
    glClearColor(0.1, 0.1, 0.1, 1);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(-WIDTH / 2, WIDTH / 2, -HEIGHT / 2, HEIGHT / 2);
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_POINT_SMOOTH);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("直线拖拽 + 实时交点(含延长线)");
    init();

    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMainLoop();
    return 0;
}