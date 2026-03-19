#define _CRT_SECURE_NO_WARNINGS
#include <GL/glut.h>
#include <stdbool.h>
#include <iostream>
#include <stddef.h> // 用于NULL

// 光栅矩阵参数
#define GRID_ROWS 80
#define GRID_COLS 80
#define CELL_SIZE 10
#define WINDOW_WIDTH (GRID_COLS * CELL_SIZE)
#define WINDOW_HEIGHT (GRID_ROWS * CELL_SIZE)

// 光栅矩阵：true=填充/轮廓，false=空白
bool grid_occupied[GRID_ROWS][GRID_COLS] = { false };

// ===================== 1. 活性边表核心结构 =====================
// 边结构体（活性边表的基础）
typedef struct Edge {
    int y_max;        // 边的最大y坐标（终止行）
    float x;          // 边当前的x坐标
    float dx_dy;      // 斜率倒数 (Δx/Δy)
    struct Edge* next;// 下一条边（链表）
} Edge;

// 边桶表（按y坐标存储所有边，下标=行号）
Edge* edge_table[GRID_ROWS] = { NULL };
// 活性边表（当前扫描行的有效边）
Edge* active_edge_table = NULL;

// ===================== 2. 活性边表辅助函数 =====================
// 创建新边节点
Edge* create_edge(int y_max, float x, float dx_dy) {
    Edge* e = new Edge;
    e->y_max = y_max;
    e->x = x;
    e->dx_dy = dx_dy;
    e->next = NULL;
    return e;
}

// 按x坐标插入边到链表（保持AET有序）
void insert_edge_sorted(Edge** head, Edge* new_edge) {
    if (*head == NULL || (*head)->x > new_edge->x) {
        new_edge->next = *head;
        *head = new_edge;
        return;
    }

    Edge* curr = *head;
    while (curr->next != NULL && curr->next->x <= new_edge->x) {
        curr = curr->next;
    }
    new_edge->next = curr->next;
    curr->next = new_edge;
}

// 清空边链表
void free_edge_list(Edge** head) {
    Edge* curr = *head;
    while (curr != NULL) {
        Edge* temp = curr;
        curr = curr->next;
        delete(temp);
    }
    *head = NULL;
}

// ===================== 3. 构建多边形边桶表（ET）=====================
// 添加多边形的一条边到边桶表
void add_polygon_edge(int x1, int y1, int x2, int y2) {
    // 确保y1 < y2（统一边的方向：从低y到高y）
    if (y1 > y2) {
        int tx = x1, ty = y1;
        x1 = x2; y1 = y2;
        x2 = tx; y2 = ty;
    }
    // 水平边跳过（扫描线填充不处理水平边）
    if (y1 == y2) { 
        return;
    }

    // 计算边的关键参数
    float dx_dy = (float)(x2 - x1) / (y2 - y1); // 斜率倒数
    Edge* e = create_edge(y2, x1, dx_dy);
    // 插入到边桶表的y1行
    insert_edge_sorted(&edge_table[y1], e);
}

// ===================== 4. 重置光栅矩阵 =====================
void reset_grid() {
    for (int i = 0; i < GRID_ROWS; i++)
        for (int j = 0; j < GRID_COLS; j++)
            grid_occupied[i][j] = false;
}

// ===================== 5. 活性边表+奇偶规则扫描线填充 =====================
void scanline_fill_aet() {
    // 逐行扫描（从最低y到最高y）
    for (int y = 0; y < GRID_ROWS; y++) {
        Edge* et_edge = edge_table[y];        // 步骤1：将当前y行的边从ET加入AET
        while (et_edge != NULL) {
            Edge* next = et_edge->next;
            insert_edge_sorted(&active_edge_table, et_edge);
            et_edge = next;
        }
        edge_table[y] = NULL; // 清空当前行ET
        Edge** curr = &active_edge_table; // 步骤2：移除AET中y_max <= 当前y的边（无效边）
        while (*curr != NULL) {
            if ((*curr)->y_max <= y) {
                Edge* temp = *curr;
                *curr = (*curr)->next;
                delete(temp);
            }
            else {
                curr = &(*curr)->next;
            }
        }
        Edge* aet_curr = active_edge_table;        // 步骤3：奇偶规则填充（遍历AET的边对）
        int pair_count = 0; // 奇偶计数器
        float x_start = 0.0f;

        while (aet_curr != NULL) {
            std::cout << "Row " << y << ": y_max=" << aet_curr->y_max << ", x=" << aet_curr->x << ", dx_dy=" << aet_curr->dx_dy << std::endl;
            if (pair_count % 2 == 0) {
                // 偶数：记录边对起始x
                x_start = aet_curr->x;
            }
            else {
                // 奇数：填充[x_start, x_end)区间（奇偶配对）
                int x1 = round(x_start);
                int x2 = round(aet_curr->x);
                if (x1 < x2)
                {
                    for (int x = x1; x < x2; x++) {
                        if (x >= 0 && x < GRID_COLS && y >= 0 && y < GRID_ROWS) {
                            if (grid_occupied[y][x] == false)
                            {
                                grid_occupied[y][x] = true;
                            }
                            else
                            {
                                grid_occupied[y][x] = false;
                            }
                            
                        }
                    }
                }
                else {
                    for (int x = x2; x < x1; x++) {
                        if (x >= 0 && x < GRID_COLS && y >= 0 && y < GRID_ROWS) {
                            if (grid_occupied[y][x] == false)
                            {
                                grid_occupied[y][x] = true;
                            }
                            else
                            {
                                grid_occupied[y][x] = false;
                            }
                        }
                    }
                }

            }
            pair_count++;
            aet_curr = aet_curr->next;
        }
        aet_curr = active_edge_table;// 步骤4：更新AET中边的x坐标（下一行的x = 当前x + dx_dy）
        while (aet_curr != NULL) {
            aet_curr->x += aet_curr->dx_dy;
            aet_curr = aet_curr->next;
        }
    }
    // 清空AET（释放内存）
    free_edge_list(&active_edge_table);
}

void scanline_trace_aet() {
    // 逐行扫描（从最低y到最高y）
    for (int y = 0; y < GRID_ROWS; y++) {
        Edge* et_edge = edge_table[y];        // 步骤1：将当前y行的边从ET加入AET
        while (et_edge != NULL) {
            Edge* next = et_edge->next;
            insert_edge_sorted(&active_edge_table, et_edge);
            et_edge = next;
        }
        edge_table[y] = NULL; // 清空当前行ET
        Edge** curr = &active_edge_table; // 步骤2：移除AET中y_max <= 当前y的边（无效边）
        while (*curr != NULL) {
            if ((*curr)->y_max <= y) {
                Edge* temp = *curr;
                *curr = (*curr)->next;
                delete(temp);
            }
            else {
                curr = &(*curr)->next;
            }
        }
        Edge* aet_curr = active_edge_table;        // 步骤3：奇偶规则填充（遍历AET的边对）
        int pair_count = 0; // 奇偶计数器
        float x_start = 0.0f;

        while (aet_curr != NULL) {
            std::cout << "Row " << y << ": y_max=" << aet_curr->y_max << ", x=" << aet_curr->x << ", dx_dy=" << aet_curr->dx_dy << std::endl;
            if (pair_count % 2 == 0) {
                // 偶数：记录边对起始x
                x_start = aet_curr->x;
            }
            else {
                // 奇数：填充[x_start, x_end)区间（奇偶配对）
                int x1 = round(x_start);
                int x2 = round(aet_curr->x);
                grid_occupied[y][x1] = true;
                grid_occupied[y][x2] = true;

            }
            pair_count++;
            aet_curr = aet_curr->next;
        }
        aet_curr = active_edge_table;// 步骤4：更新AET中边的x坐标（下一行的x = 当前x + dx_dy）
        while (aet_curr != NULL) {
            aet_curr->x += aet_curr->dx_dy;
            aet_curr = aet_curr->next;
        }
    }
    // 清空AET（释放内存）
    free_edge_list(&active_edge_table);
}
// 构建多边形（示例：四边形）
void build_polygon() {
    // 重置所有表和矩阵
    for (int i = 0; i < GRID_ROWS; i++) free_edge_list(&edge_table[i]);
    free_edge_list(&active_edge_table);
    reset_grid();

    // 定义四边形顶点（顺时针）：(20,20) → (60,20) → (60,60) → (20,60)
    int vertices[5][2] = { {30,20}, {70,50},{20,50}, {60,20},{45,70} };
    // 遍历所有边（闭合多边形：最后一个顶点连回第一个）
    for (int i = 0; i < 5; i++) {
        int j = (i + 1) % 5;
        add_polygon_edge(vertices[i][0], vertices[i][1], vertices[j][0], vertices[j][1]);
    }
}

// ===================== 6. 绘制与初始化 =====================
void display() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // 绘制光栅矩阵
    for (int row = 0; row < GRID_ROWS; row++) {
        for (int col = 0; col < GRID_COLS; col++) {
            float x = col * CELL_SIZE;
            float y = row * CELL_SIZE;

            if (grid_occupied[row][col]) {
                glColor3f(1.0f, 0.0f, 0.0f);
                glBegin(GL_QUADS);
                glVertex2f(x, y);
                glVertex2f(x + CELL_SIZE, y);
                glVertex2f(x + CELL_SIZE, y + CELL_SIZE);
                glVertex2f(x, y + CELL_SIZE);
                glEnd();
            }

            // 网格边框
            glColor3f(0.3f, 0.3f, 0.3f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + CELL_SIZE, y);
            glVertex2f(x + CELL_SIZE, y + CELL_SIZE);
            glVertex2f(x, y + CELL_SIZE);
            glEnd();
        }
    }
    glFlush();
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
    grid_occupied[center_row][center_col + y] = true;
    grid_occupied[center_row + y][center_col] = true;
    grid_occupied[center_row][center_col - y] = true;
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

//递归出点！！！！！！！！！！！！！！！！！！！！
void seed_filling1(int x, int y)
{
    if (x < 0 || x >= GRID_COLS || y < 0 || y >= GRID_ROWS)
        return;

    //std::cout << x << y << std::endl;
    if (grid_occupied[y][x] == true)
    {
        return;
    }
    grid_occupied[y][x] = true;
    if (x + 1 < GRID_COLS )seed_filling1(x + 1, y);
    if (y + 1 < GRID_ROWS )seed_filling1(x, y + 1);
    if (x - 1 >= 0 )seed_filling1(x - 1, y);
    if (y - 1 >= 0 )seed_filling1(x, y - 1);
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



void init() {
    // 正交投影
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    // 步骤1：构建多边形（生成边桶表）
    build_polygon();
    

    // 步骤2：活性边表+奇偶扫描线填充
    for (size_t i = 0; i < GRID_ROWS; i++)
    {
        Edge* curr = edge_table[i];
        while (curr != NULL) 
        {
			std::cout << "Row " << i << ": y_max=" << curr->y_max << ", x=" << curr->x << ", dx_dy=" << curr->dx_dy << std::endl;
            curr = curr->next;
        }
    }
    scanline_trace_aet();
    //Bresenham_circle(400, 400, 390);
    //seed_filling1(40, 40);
}

// ===================== 主函数 =====================
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("活性边表+奇偶规则 多边形扫描线填充");
    init();
    glutDisplayFunc(display);
    glutMainLoop();

    // 释放内存（防止泄漏）
    for (int i = 0; i < GRID_ROWS; i++) free_edge_list(&edge_table[i]);
    return 0;
}