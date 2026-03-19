#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <math.h>
#include <algorithm>
//#include <stdbool.h>

// 网格参数（60行 × 80列）
#define GRID_ROWS 80
#define GRID_COLS 80
#define WINDOW_WIDTH 800   // 窗口宽度 = 网格列数 × 网格尺寸
#define WINDOW_HEIGHT 800  // 窗口高度 = 网格行数 × 网格尺寸
#define CELL_SIZE 10       // 每个网格的像素尺寸（800/80=10）


bool grid_occupied[GRID_ROWS][GRID_COLS] = { false };

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
void dda_grid_trace(int x1,int y1,int x2,int y2) {
    reset_grid(); // 先清空之前的标记

    // 转换为网格坐标（像素坐标 → 网格索引）
    float grid_x1 = (float)x1 / CELL_SIZE;
    float grid_y1 = (float)y1 / CELL_SIZE;
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
        int col = (int)round(current_grid_x); // 列（x方向）
        int row = (int)round(current_grid_y); // 行（y方向）

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
void mid_point_trace(int x1, int y1, int x2, int y2) {
    reset_grid(); // 先清空之前的标记

    // 转换为网格坐标（像素坐标 → 网格索引）
    float grid_x1 = (float)x1 / CELL_SIZE;
    float grid_y1 = (float)y1 / CELL_SIZE;
    float grid_x2 = (float)x2 / CELL_SIZE;
    float grid_y2 = (float)y2 / CELL_SIZE;

    float dx = grid_x2 - grid_x1;
    float dy = grid_y2 - grid_y1;
    int steps = abs((int)dx) > abs((int)dy) ? abs((int)dx) : abs((int)dy);
    if (steps == 0) steps = 1; // 避免除以0

    int col = grid_x1;
    int row = grid_y1;
    // 迭代追踪穿过的网格
    for (int i = 0; i <= steps; i++) {

        // 边界检查：确保网格索引有效
        if (col >= 0 && col < GRID_COLS && row >= 0 && row < GRID_ROWS) {
            grid_occupied[row][col] = true; // 标记该网格被穿过
        }

        // 输出调试信息（可选：查看每一步穿过的网格）
        printf("Step %d: Grid(%d, %d) | Float(%f, %f)\n", i, col, row);

        if (dy <= dx)
        {
            if ((col - grid_x1 + 1) * (dy / dx) + grid_y1 - (row + 0.5) >= 0)
            {
                col++;
                row++;
            }
            else
            {
                col++;
            }
        }
        else
        {
            if ((row - grid_y1 + 1) * (dx / dy) + grid_x1 - (col + 0.5) >= 0)
            {
                col++;
                row++;
            }
            else
            {
                row++;
            }
        }

    }
}
void Bresenham_trace(int x1, int y1, int x2, int y2)
{
    reset_grid(); // 先清空之前的标记

    // 转换为网格坐标（像素坐标 → 网格索引）
    int grid_x1 = x1 / CELL_SIZE;
    int grid_y1 = y1 / CELL_SIZE;
    int grid_x2 = x2 / CELL_SIZE;
    int grid_y2 = y2 / CELL_SIZE;

    int dx = grid_x2 - grid_x1;
    int dy = grid_y2 - grid_y1;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    if (steps == 0) steps = 1; // 避免除以0

    int col = grid_x1;
    int row = grid_y1;
    if (dy <= dx)
    {
        int p = 2 * dy - dx;
        for (int i = 0; i <= steps; i++)
        {
            if (col >= 0 && col < GRID_COLS && row >= 0 && row < GRID_ROWS) {
                grid_occupied[row][col] = true; // 标记该网格被穿过
            }

            // 输出调试信息（可选：查看每一步穿过的网格）
            printf("Step %d: Grid(%d, %d)\n", i, col, row);

            if (p >= 0)
            {
                row++;
                p -= 2 * dx;
            }
            col++;
            p += 2 * dy;
        }
    }
    else
    {
        int p = 2 * dx - dy;
        for (int i = 0; i <= steps; i++)
        {
            if (col >= 0 && col < GRID_COLS && row >= 0 && row < GRID_ROWS) {
                grid_occupied[row][col] = true; // 标记该网格被穿过
            }

            // 输出调试信息（可选：查看每一步穿过的网格）
            printf("Step %d: Grid(%d, %d)\n", i, col, row);

            if (p >= 0)
            {
                col++;
                p -= 2 * dy;
            }
            row++;
            p += 2 * dx;
        }
    }

    // 迭代追踪穿过的网格
   
}


void Bresenham_circle(int cx, int cy, int r)
{
    reset_grid(); // 先清空之前的标记（和你的画线函数一致）

    // 转换为网格坐标（像素 → 网格索引，和你的画线函数逻辑完全一致）
    int center_col = cx / CELL_SIZE; // 圆心网格列（对应你的col）
    int center_row = cy / CELL_SIZE; // 圆心网格行（对应你的row）
    int grid_r = r / CELL_SIZE;     // 网格半径（整数）

    // 边界判断：半径为0时仅标记圆心
    if (grid_r == 0) {
        if (center_col >= 0 && center_col < GRID_COLS && center_row >= 0 && center_row < GRID_ROWS) {
            grid_occupied[center_row][center_col] = true;

        }
        printf("Step 0: Grid(%d, %d) (圆心，半径0)\n", center_col, center_row);
        return;
    }

    int x = 0;               // 初始x（1/8圆弧起点）
    int y = grid_r;          // 初始y
    int p = 3 - 2 * grid_r;  // Bresenham画圆初始误差项（纯整数，4x-6增量核心）
    int step = 0;            // 步骤计数（和你的画线函数调试输出一致）

    // 标记初始点（x=0, y=r）
    grid_occupied[center_row][center_col+y] = true;
    grid_occupied[center_row + y][center_col] = true;
    grid_occupied[center_row ][center_col - y] = true;
    grid_occupied[center_row - y][center_col] = true;
    printf("Step %d: Grid(x=%d, y=%d) (圆心偏移)\n", step++, x, y);

    // 循环绘制1/8圆弧（和你的画线函数for循环风格一致）
    while (x <= y)
    {
        x++; // x主方向步进

        // Bresenham 4x-6核心增量更新（纯整数运算）
        if (p < 0) {
            p += 4 * x + 6; // 选E点：增量4x+6
        }
        else {
            y--;            // 选SE点：y减1
            p += 4 * (x - y) + 10; // 增量4(x-y)+10（4x-4y+10，对应4x-6形式）
        }

        // 标记八对称网格（核心逻辑）
        grid_occupied[center_row + x][center_col + y] = true;
        grid_occupied[center_row + x][center_col - y] = true;
        grid_occupied[center_row - x][center_col + y] = true;
        grid_occupied[center_row - x][center_col - y] = true;
        grid_occupied[center_row + y][center_col + x] = true;
        grid_occupied[center_row - y][center_col + x] = true;
        grid_occupied[center_row + y][center_col - x] = true;
        grid_occupied[center_row - y][center_col - x] = true;
        
        // 调试输出（和你的画线函数格式一致）
        printf("Step %d: Grid(x=%d, y=%d) | 误差项p=%d\n", step++, x, y, p);
    }
}


void scanline_fill(int row0, int row1) {
    for (int row = row0+1; row < row1; row++) {
        int count = 0; 
        for (int col = 0; col < GRID_COLS; col++) {
            
            if (grid_occupied[row][col] && !grid_occupied[row][col+1]) {
                count = 1 - count;
            }
            else if (count == 1) {
                grid_occupied[row][col] = true;
            }
        }
    }
}



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
    Bresenham_trace(400, 400, 400, 700);
    //scanline_fill(1 , 79);
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