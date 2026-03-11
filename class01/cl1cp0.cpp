//#include <GL/glew.h>
//#include <GL/glut.h>
//#include <math.h>
//#include <iostream>
//
//// DDA核心绘制函数（GLUT渲染回调）
//void display() {
//    glClearColor(0.1f, 0.3f, 0.1f, 1.0f);
//
//    glClear(GL_COLOR_BUFFER_BIT);
//
//    // 固定管线设置
//    glColor3f(1.0f, 0.0f, 0.0f); // 红色
//    glPointSize(2.0f);            // 点大小
//
//    // DDA算法核心
//    int x1=100,y1=100,x2=700,y2=500,dx=x2-x1,dy=y2-y1;
//    int s=abs(dx)>abs(dy)?abs(dx):abs(dy);
//    float xi=(float)dx/s,yi=(float)dy/s,x=x1,y=y1;
//
//    // 固定管线绘制点
//    glBegin(GL_POINTS);
//    for (int i = 0; i <= s; i++) { glVertex2f((int)round(x), (int)round(y)); x += xi; y += yi; std::cout << x << " " << y << std::endl; }
//    glEnd();
//
//    glFlush(); // GLUT刷新缓冲区
//}
//
//int main(int argc, char** argv) {
//    // 初始化GLUT
//    glutInit(&argc, argv);
//    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
//    glutInitWindowSize(800, 600);
//    glutCreateWindow("DDA (GLUT+GLEW)");
//
//
//
//    // 设置正交投影（适配像素坐标）
//    glMatrixMode(GL_PROJECTION);
//    glLoadIdentity();
//    glOrtho(0, 800, 0, 600, -1, 1);
//    glMatrixMode(GL_MODELVIEW);
//
//    // 注册渲染回调
//    glutDisplayFunc(display);
//    // 启动GLUT主循环
//    glutMainLoop();
//
//    return 0;
//}





#include <GL/glew.h>
#include <GL/glut.h>
#include <math.h>
#include <stdbool.h>

// 网格参数（60行 × 80列）
#define GRID_ROWS 60
#define GRID_COLS 80
#define WINDOW_WIDTH 800   // 窗口宽度 = 网格列数 × 网格尺寸
#define WINDOW_HEIGHT 600  // 窗口高度 = 网格行数 × 网格尺寸
#define CELL_SIZE 10       // 每个网格的像素尺寸（800/80=10，600/60=10）

// 记录网格是否被直线穿过
bool grid_occupied[GRID_ROWS][GRID_COLS] = { false };
// 直线端点（像素坐标）
int x1 = 50, y1_pixel = 50, x2 = 750, y2 = 550;

// 重置网格状态
void reset_grid() {
    for (int i = 0; i < GRID_ROWS; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            grid_occupied[i][j] = false;
        }
    }
}

// DDA算法：追踪直线穿过的网格（核心逻辑）
void dda_grid_trace() {
    reset_grid(); // 先清空之前的标记

    // 转换为网格坐标（像素坐标 → 网格索引）
    float grid_x1 = (float)x1 / CELL_SIZE;
    float grid_y1 = (float)y1_pixel / CELL_SIZE;
    float grid_x2 = (float)x2 / CELL_SIZE;
    float grid_y2 = (float)y2 / CELL_SIZE;

    float dx = grid_x2 - grid_x1;
    float dy = grid_y2 - grid_y1;
    int steps = abs((int)dx) > abs((int)dy) ? abs((int)dx) : abs((int)dy);
    if (steps == 0) steps = 1; // 避免除以0

    float x_inc = dx / steps;
    float y_inc = dy / steps;
    float current_grid_x = grid_x1;
    float current_grid_y = grid_y1;

    // 迭代追踪穿过的网格
    for (int i = 0; i <= steps; i++) {
        // 取整得到当前所在网格的索引（行/列）
        int col = (int)floor(current_grid_x); // 列（x方向）
        int row = (int)floor(current_grid_y); // 行（y方向）

        // 边界检查：确保网格索引有效
        if (col >= 0 && col < GRID_COLS && row >= 0 && row < GRID_ROWS) {
            grid_occupied[row][col] = true; // 标记该网格被穿过
        }

        // 输出调试信息（可选：查看每一步穿过的网格）
        // printf("Step %d: Grid(%d, %d) | Float(%f, %f)\n", i, col, row, current_grid_x, current_grid_y);

        current_grid_x += x_inc;
        current_grid_y += y_inc;
    }
}

// 绘制网格和直线
void display() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 1. 绘制网格（边框+填充）
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            // 网格的像素坐标（左上角）
            float x = col * CELL_SIZE;
            float y = row * CELL_SIZE;

            if (grid_occupied[row][col]) {
                // 穿过的网格：红色填充
                glColor3f(1.0f, 0.0f, 0.0f);
                glBegin(GL_QUADS);
                glVertex2f(x, y);
                glVertex2f(x + CELL_SIZE, y);
                glVertex2f(x + CELL_SIZE, y + CELL_SIZE);
                glVertex2f(x, y + CELL_SIZE);
                glEnd();
            }

            // 所有网格：灰色边框
            glColor3f(0.3f, 0.3f, 0.3f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + CELL_SIZE, y);
            glVertex2f(x + CELL_SIZE, y + CELL_SIZE);
            glVertex2f(x, y + CELL_SIZE);
            glEnd();
        }
    }

    // 2. 绘制原始直线（白色，对比参考）
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2i(x1, y1_pixel);
    glVertex2i(x2, y2);
    glEnd();

    glFlush();
}

// 初始化函数
void init() {
    // 设置正交投影（严格匹配像素）
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    // 执行DDA网格追踪
    dda_grid_trace();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("DDA Grid Rasterization (60x80)");


    init();

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}