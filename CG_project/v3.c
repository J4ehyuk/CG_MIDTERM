/*
*  �ۼ��� : ������
*  �ۼ��� : 25.05.03
*  ���α׷��� : Midterm Project
*/

// �����Ҵ� ����

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
	float curX, curY;	// ���� ���콺 ��ǥ
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
float last_point_X;			// ������ ������ X ��ǥ
float last_point_Y;			// ������ ������ Y ��ǥ


int color_index = 0;		// ���� �� �ε���
int mouse_left_button = 0;	// ���콺 ���� ��ư�� ���ȴ���
int vertex_cnt = 0;			// ���� ������ ����

int status = -1;
int curX;
int curY;

// �ܹ��� ���� ����Ʈ
//==========================================================================================
Node* create_node(int mouse_x, int mouse_y) {
	Node* newnode = (Node*)malloc(sizeof(Node));
	if (!newnode) {
		printf("mem err \n");
		exit(1);
	}

	// ��� ������, ������ ���콺 ��ǥ�� ����
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

	temp->link = create_node(mouse_x, mouse_y); // ���콺 �̺�Ʈ�� �߻��� ���� ���콺 ��ġ ����
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

	// OpenGL ��ǥ��� ��ȯ
	*cvt_X = ((float)x / windowWidth) * (ORTHO_RIGHT - ORTHO_LEFT) - ORTHO_RIGHT;
	*cvt_Y = (1 - (float)y / windowHeight) * (ORTHO_TOP - ORTHO_BOTTOM) - ORTHO_TOP;
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT); // ȭ�� clear

	// 1. ��� ������ �׸���
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

	// 2. ù �������� ������ ������ �����ϱ�
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

	if (mouse_left_button == 1 && status == GLUT_DOWN) { // ���� ���콺 ��ư�� ������ ä �����Ǵ� ����

		if (vertex_cnt != 0) { // ù ��° ������ �ƴ� ���
			// ������ �������� ���� �������� line �׸���
			glBegin(GL_LINE_STRIP); {
				glVertex2f(last_point_X, last_point_Y);
				glVertex2f(curX, curY);
			}glEnd();
		}

	}
	glFlush();
}

void mouse(int button, int state, int x, int y) { // �޾ƿ� x, y�� ���� ���� (0, 0) ����

	float gl_X, gl_Y;
	convert_coordinate(x, y, &gl_X, &gl_Y);

	switch (button)
	{
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN) {
				printf("mouse left button pressed \n");

				mouse_left_button = 1;
				status = GLUT_DOWN; curX = gl_X; curY = gl_Y;
				glutPostRedisplay(); // display ȣ��
			}
			else if (state == GLUT_UP) {
				// 1. convex hall Ȯ��

				// 2. ���� ����Ʈ�� ����
				head = insert_node(&head, curX, curY);
				printf("���� ����! \n");

				// 3. ������ �������� ����
				last_point_X = gl_X; last_point_Y = gl_Y;

				// 4. ���� ���� ����
				vertex_cnt++;
				printf("vertex_cnt : %d \n", vertex_cnt);

				mouse_left_button = 0;

				status = GLUT_UP; curX = gl_X; curY = gl_Y;
				glutPostRedisplay(); // display ȣ��
			}
			break;

	} // end of switch
}

void mouse_motion(int x, int y) { // ���콺�� ������ ��, �۵�
	float gl_X, gl_Y;
	convert_coordinate(x, y, &gl_X, &gl_Y);

	if (mouse_left_button == 1) {
		status = GLUT_DOWN; curX = gl_X; curY = gl_Y;
		glutPostRedisplay(); // display ȣ��
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