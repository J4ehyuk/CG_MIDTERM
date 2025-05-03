/*
*  작성자 : 양재혁
*  작성일 : 25.05.03
*  프로그램명 : Midterm Project
*/

// 동적할당 해제

#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH	500
#define HEIGHT	500

#define ORTHO_LEFT		-100
#define ORTHO_RIGHT		100
#define ORTHO_BOTTOM	-100
#define ORTHO_TOP		100


typedef struct Node {
	float curX, curY;	// 현재 마우스 좌표
	struct Node* link;
}Node;

unsigned char PALETTE[16][3] = {
	  {255, 255, 255 },				// WHITE
	  {0, 255, 255},				// CYAN
	  {255, 0, 255},				// PURPLE
	  {0, 0, 255},					// BLUE
	  {192, 192, 192},				// LIGHT GRAY
	  {128, 128, 128},				// DARK GRAY
	  {0, 128, 128},				// DARK TEAL
	  {128, 0,128 },				// DARK PURPLE
	  {0, 0, 128},					// DARK BLUE
	  {255, 255, 0},				// YELLOW
	  {0, 255, 0},					// GREEN
	  {128, 128, 0},				// DARK YELLOW
	  {0, 128, 0},					// DARK GREEN
	  {255, 0, 0},					// RED
	  {128, 0, 0},					// DARK RED
	  {0, 0, 0},					// BLACK
};

Node* head = NULL;			// Linked List Head
float last_point_X;			// 마지막 정점의 X 좌표
float last_point_Y;			// 마지막 정점의 Y 좌표


int color_index = 0;		// 현재 색 인덱스
int mouse_left_button = 0;	// 마우스 왼쪽 버튼이 눌렸는지
int vertex_cnt = 0;			// 현재 정점의 개수

int status = -1;
int curX;
int curY;

// 단방향 연결 리스트
//==========================================================================================
Node* create_node(int mouse_x, int mouse_y) {
	Node* newnode = (Node*)malloc(sizeof(Node));
	if (!newnode) {
		printf("mem err \n");
		exit(1);
	}

	// 노드 생성시, 눌려진 마우스 좌표로 설정
	newnode->curX = mouse_x; newnode->curY = mouse_y;
	newnode->link = NULL;

	return newnode;
}

Node* insert_node(Node** head, int mouse_x, int mouse_y) {
	if (*head == NULL) {
		*head = create_node(mouse_x, mouse_y);
		return *head;
	}

	Node* temp = *head;
	while (temp->link != NULL) temp = temp->link;

	temp->link = create_node(mouse_x, mouse_y); // 마우스 이벤트때 발생한 현재 마우스 위치 전달
	return *head;
}

void delete_list(Node** head) {
	Node* temp = *head;
	Node* del = NULL; 

	while (temp) {
		del = temp;
		temp = temp->link;
		free(del);
	}
}
//==========================================================================================

void init() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_PROJECTION);
	gluOrtho2D(ORTHO_LEFT, ORTHO_RIGHT, ORTHO_BOTTOM, ORTHO_TOP);
	glMatrixMode(GL_MODELVIEW);
}

void convert_coordinate(int x, int y, float* cvt_X, float* cvt_Y) {
	int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
	int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

	// OpenGL 좌표계로 변환
	*cvt_X = ((float)x / windowWidth) * (ORTHO_RIGHT - ORTHO_LEFT) - ORTHO_RIGHT;
	*cvt_Y = (1 - (float)y / windowHeight) * (ORTHO_TOP - ORTHO_BOTTOM) - ORTHO_TOP;
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT); // 화면 clear

	// 1. 모든 정점들 그리기
	// ====================================================================================================================================
	Node* cur_node = head;

	glPointSize(5.0);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_POINTS); {
		while (cur_node) {
			glVertex2f(cur_node->curX, cur_node->curY);
			cur_node = cur_node->link;
		}
	}glEnd();
	// ====================================================================================================================================

	// 2. 첫 정점에서 마지막 정점을 연결하기
	// ====================================================================================================================================
	cur_node = head;

	glColor3f(1.0, 1.0, 1.0);

	glBegin(GL_LINE_STRIP); {
		while (cur_node) {
			glVertex2f(cur_node->curX, cur_node->curY);
			cur_node = cur_node->link;
		}
	}glEnd();
	// ====================================================================================================================================

	if (mouse_left_button == 1 && status == GLUT_DOWN) { // 왼쪽 마우스 버튼이 눌려진 채 유지되는 상태

		if (vertex_cnt != 0) { // 첫 번째 정점이 아닌 경우
			// 마지막 정점에서 현재 정점까지 line 그리기
			glBegin(GL_LINE_STRIP); {
				glVertex2f(last_point_X, last_point_Y);
				glVertex2f(curX, curY);
			}glEnd();
		}

	}
	glFlush();
}

void mouse(int button, int state, int x, int y) { // 받아온 x, y는 왼쪽 위가 (0, 0) 원점

	float gl_X, gl_Y;
	convert_coordinate(x, y, &gl_X, &gl_Y);

	switch (button)
	{
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN) {
				printf("mouse left button pressed \n");

				mouse_left_button = 1;
				status = GLUT_DOWN; curX = gl_X; curY = gl_Y;
				glutPostRedisplay(); // display 호출
			}
			else if (state == GLUT_UP) {
				// 1. convex hall 확인

				// 2. 연결 리스트에 삽입
				head = insert_node(&head, curX, curY);
				printf("정점 삽입! \n");

				// 3. 마지막 정점으로 갱신
				last_point_X = gl_X; last_point_Y = gl_Y;

				// 4. 정점 개수 증가
				vertex_cnt++;
				printf("vertex_cnt : %d \n", vertex_cnt);

				mouse_left_button = 0;

				status = GLUT_UP; curX = gl_X; curY = gl_Y;
				glutPostRedisplay(); // display 호출
			}
			break;

	} // end of switch
}

void mouse_motion(int x, int y) { // 마우스가 눌렸을 때, 작동
	float gl_X, gl_Y;
	convert_coordinate(x, y, &gl_X, &gl_Y);

	if (mouse_left_button == 1) {
		status = GLUT_DOWN; curX = gl_X; curY = gl_Y;
		glutPostRedisplay(); // display 호출
	}
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(100, 100);

	glutCreateWindow("Midterm Project");
	init();

	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(mouse_motion);


	glutMainLoop();

	return 0;
}