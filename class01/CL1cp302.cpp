#define _CRT_SECURE_NO_WARNINGS
#include <GL/glut.h>
#include <iostream>
#include <cmath>
using namespace std;

const int WIDTH = 800;
const int HEIGHT = 600;

// 可拖动直线
float lx1 = -250, ly1 = 150;
float lx2 = 250, ly2 = -150;
bool isDragging = false;

// ===================== 二次曲线：椭圆（固定不动）=====================
// 标准椭圆方程：x²/a² + y²/b² = 1
float ellipse_a = 180;
float ellipse_b = 120;

// ===================== 直线 ↔ 二次曲线 求交点（核心算法）=====================
// 输入：直线两点 p1,p2
// 输出：交点数组 pts，返回交点数量（0,1,2）
int lineEllipseIntersection(
    float x1, float y1, float x2, float y2,
    float pts[2][2]
) {
    // 直线参数方程：x = x1 + t*(x2-x1), y = y1 + t*(y2-y1)
    float dx = x2 - x1;
    float dy = y2 - y1;

    float a = ellipse_a;
    float b = ellipse_b;

    // 代入椭圆方程得到 At² + Bt + C = 0
    float A = (dx * dx) / (a * a) + (dy * dy) / (b * b);
    float B = 2 * ((x1 * dx) / (a * a) + (y1 * dy) / (b * b));
    float C = (x1 * x1) / (a * a) + (y1 * y1) / (b * b) - 1;

    float delta = B * B - 4 * A * C;
    if (delta < -1e-6) return 0;
    if (delta < 0) delta = 0;

    float sqrtD = sqrt(delta);
    int cnt = 0;

    // t1
    float t1 = (-B - sqrtD) / (2 * A);
    pts[cnt][0] = x1 + t1 * dx;
    pts[cnt][1] = y1 + t1 * dy;
    cnt++;

    // t2
    if (delta > 1e-6) {
        float t2 = (-B + sqrtD) / (2 * A);
        pts[cnt][0] = x1 + t2 * dx;
        pts[cnt][1] = y1 + t2 * dy;
        cnt++;
    }
    return cnt;
}

// ===================== 绘制 =====================
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // ---- 画椭圆（二次曲线，白色）----
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i += 5) {
        float r = i * 3.14159 / 180;
        float x = ellipse_a * cos(r);
        float y = ellipse_b * sin(r);
        glVertex2f(x, y);
    }
    glEnd();

    // ---- 画可拖动直线（黄色）----
    glColor3f(1, 1, 0);
    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex2f(lx1, ly1);
    glVertex2f(lx2, ly2);
    glEnd();

    // ---- 计算并画交点（红色）----
    float pts[2][2];
    int n = lineEllipseIntersection(lx1, ly1, lx2, ly2, pts);

    glColor3f(1, 0, 0);
    glPointSize(12);
    glBegin(GL_POINTS);
    for (int i = 0; i < n; i++) {
        glVertex2f(pts[i][0], pts[i][1]);
    }
    glEnd();

    glutSwapBuffers();
}

// ===================== 鼠标拖动 =====================
void mouse(int btn, int state, int x, int y) {
    if (btn == GLUT_LEFT_BUTTON)
        isDragging = (state == GLUT_DOWN);
}

void motion(int x, int y) {
    if (!isDragging) return;
    float wx = x - WIDTH / 2;
    float wy = HEIGHT / 2 - y;
    lx2 = wx;
    ly2 = wy;
    glutPostRedisplay();
}

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
    glutCreateWindow("直线拖动 + 二次曲线交点");
    init();
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMainLoop();
    return 0;
}