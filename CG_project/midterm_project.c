/*
*  �ۼ��� : ������
*  �ۼ��� : 25.05.03
*  ���α׷��� : Midterm Project
*/

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

#include <math.h>

#include "LinkedList.h"

#define WIDTH	500
#define HEIGHT	500

#define ORTHO_LEFT		-100
#define ORTHO_RIGHT		100
#define ORTHO_BOTTOM	-100
#define ORTHO_TOP		100

#define MARGIN_OF_ERROR		1.0

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

int mouse_left_button = 0;	// ���콺 ���� ��ư�� ���ȴ���
int mouse_right_button = 0; // ���콺 ������ ��ư�� ���ȴ���

Node* cvx_h_verteces = NULL;	// convex hull�� �̷�� �������� ����Ʈ
int generated_poly = 0;			// �ٰ����� �����Ǿ�����

float std_x = 0, std_y = 0;		// ������ ��ǥ
Node* std_loc;					// �������� ��ġ

float drag_start_x, drag_start_y, drag_end_x, drag_end_y; // �巡�� ����, �� ��������
int drag_start_time, drag_end_time; 
int is_draged = 0;

float center_x, center_y;	// �����߽� ��ǥ
float spin = 0.0;			
float velocity;				// ȸ�� �ӵ�


int vertex_cnt = 0;			// ���� ������ ����

int status = -1;
float curX;
float curY;

int special_key = 0;
int color_index = 0;

void init();
void convert_coordinate(int x, int y, float* cvt_X, float* cvt_Y);
void display();
int ccw(float p0_x, float p0_y, float x1, float y1, float x2, float y2);
void unlink_node(Node** head, Node* cur);
void insert_after(Node** head, Node* compare, Node* cur);
Node* sort_vertices(Node** head, float p0_x, float p0_y, Node* p0_ptr);
int check_convex_hull(Node** head, float gl_X, float gl_Y);
int point_exist(Node** head, float gl_X, float gl_Y);
void mouse(int button, int state, int x, int y);
void mouse_motion(int x, int y);
int is_dragging_polygon(Node** head, float drag_start_x, float drag_start_y, float drag_end_x, float drag_end_y);
void calc_center(); 
void spin_poly_left();
void calc_spin_v();
void specialKeys(int key, int x, int y);


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

	Node* cur_node;
	if (is_draged == 0) { // �巡�� �Ǿ� ȸ���� ��쿡��, ������ ��� X
		// 1. ��� ������ �׸���
		// ====================================================================================================================================
		cur_node = head;

		glPointSize(5.0);
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_POINTS); {
			while (cur_node) {
				glVertex2f(cur_node->curX, cur_node->curY);
				cur_node = cur_node->next;
			}
		}glEnd();
		// ====================================================================================================================================
	}

	if (mouse_left_button == 1 && status == GLUT_DOWN) { // ���� ���콺 ��ư�� ������ ä �����Ǵ� ����

		if (vertex_cnt != 0) { // ù ��° ������ �ƴ� ���
			// ������ �������� ���� �������� line �׸���
			glBegin(GL_LINE_STRIP); {
				glVertex2f(last_point_X, last_point_Y);
				glVertex2f(curX, curY);
			}glEnd();
		}
	}
	else if (mouse_right_button == 1) {

		delete_list(&cvx_h_verteces); // ���� ���������� ���ĵ� ����Ʈ �޸� ����

		// 2. ���� ���� ����Ʈ���� �������� ã�Ƽ�, �� ���������� ���� ����Ʈ �������� ���� ��, ���
		//==============================================================================================================
		cvx_h_verteces = sort_vertices(&head, std_x, std_y, std_loc); 

		glColor3f(0.0, 1.0, 1.0);
		cur_node = cvx_h_verteces;
		glBegin(GL_POLYGON); {
			while (cur_node) {
				glVertex2f(cur_node->curX, cur_node->curY);
				cur_node = cur_node->next;
			}
		}glEnd();
		//==============================================================================================================

		mouse_right_button = 0;

		generated_poly = 1;
	}
	else if (is_draged == 1) {
		Node* temp = cvx_h_verteces; // �ٰ��� ���� ��, ������� ���ĵ� ���� ����Ʈ ���

		if (special_key == 1)
			glColor3f(PALETTE[color_index][0], PALETTE[color_index][1], PALETTE[color_index][2]);
		else
			glColor3f(0.0, 1.0, 1.0);

		glPushMatrix(); {
			// 1. �����߽����� �̵�
			glTranslatef(center_x, center_y, 0.0);

			// 2. ȸ�� ����
			glRotatef(spin, 0.0, 0.0, 1.0);

			// 3. ���� ����
			glTranslatef(-center_x, -center_y, 0.0);

			// 3. �׸���
			glBegin(GL_POLYGON); {
				while (temp) {
					glVertex2f(temp->curX, temp->curY);
					temp = temp->next;
				}
			}glEnd();
		}glPopMatrix();
		
		spin_poly_left(); // ��� ȸ��
	}
	glFlush();
}

int ccw(float p0_x, float p0_y, float x1, float y1, float x2, float y2) {
	// ������ p0�� �������� �� �� (x1, y1), (x2, y2)�� �ݽð� ���� ���θ� �Ǵ��ϴ� �Լ�
	/*
		��ȯ��:
		> 0  �� p0��(x1,y1)���� p0��(x2,y2) ������ �ݽð� ���� (left turn)
		< 0  �� �ð� ���� (right turn)
		== 0 �� ������ (colinear), �̶��� ���������� �� ����� ���� ���� ������ �Ǵ�
	*/

	/*
	 ���� ����:
	   (x1 - p0_x)*(y2 - p0_y) - (y1 - p0_y)*(x2 - p0_x)

	 �ǹ�:
	   ����� ����� �ݽð� ����, ������ �ð� ����, 0�̸� ������
	*/

	float ret = ( (x1 - p0_x) * (y2 - p0_y) ) - ( (y1 - p0_y) * (x2 - p0_x) );

	if (ret == 0) { // ���� ������ �������, �������� �� ����� ���� ����
		// �Ÿ� �������� ��
		float dx1 = x1 - p0_x, dy1 = y1 - p0_y;
		float dx2 = x2 - p0_x, dy2 = y2 - p0_y;

		float dist1 = dx1 * dx1 + dy1 * dy1;
		float dist2 = dx2 * dx2 + dy2 * dy2;

		return dist1 < dist2 ? 1 : -1;
	}
	
	return ret;
}

void unlink_node(Node** head, Node* cur) {
	// cur ��带 ���� ����Ʈ���� ��ũ ���� �ϴ� �Լ� (�޸� ���� X)

	if (!cur)
		return;

	if (cur->prev) { // cur->prev �� �����ϴ� ���
		cur->prev->next = cur->next;
	}
	else { // cur->prev �� head �� ���
		*head = cur->next;
	}


	if (cur->next) { // cur�� ������ ��Ұ� �ƴ� ���
		cur->next->prev = cur->prev;
	}

	cur->next = cur->prev = NULL;
}

void insert_after(Node** head, Node* compare, Node* cur) {
	// compare �ڿ� cur ��� ����
	if (compare == NULL) { // head �ڿ� �����ϴ� ��� (ù ��ҷ� ����)
		(*head)->prev = cur;
		cur->next = (*head);
		(*head) = cur;
		
	}
	else {
		if (compare->next) { // ����Ʈ �߰��� �����ϴ� ���
			cur->next = compare->next;
			cur->prev = compare;

			compare->next->prev = cur;
			compare->next = cur;

		}
		else { // ������ ��ҷ� �����ϴ� ���
			cur->next = compare->next;
			cur->prev = compare;
			compare->next = cur;
		}
	}

}

Node* sort_vertices(Node** head, float p0_x, float p0_y, Node* p0_ptr) {
	// p0_x, p0_y	: �������� ����ڰ� �߰��� ���� (�� ������ �����Ͽ� convex hull�� �Ǵ��� Ȯ��) 
	// p0_ptr		: �������� ����Ʈ ���ο� ���� -> p0_ptr != null / �������� ����Ʈ ���ο� ���� X -> p0_ptr == null 

	// 1. ���� ���� ����Ʈ�� ����
	//========================================================================================================================
	Node* sorted_list_head = NULL; // ���������� ���ĵ� ����Ʈ

	Node* cur = *head;
	while (cur) {
		// ��� ����
		if (cur != p0_ptr) { // ������ ���� ���� X
			insert_node(&sorted_list_head, cur->curX, cur->curY);
		}
		cur = cur->next;
	} // end of while

	//========================================================================================================================


	if (p0_ptr) { // p0_ptr != NULL -> �������� ����Ʈ ���ο� ����
		insert_node(&sorted_list_head, p0_x, p0_y); // ����ڰ� �������� �߰��� ���� ���� (convex hull �Ǵ��� ����)	

	}// else -> p0_ptr == NULL -> �������� ���� �߰��� ����
		

	// 2. ���� ���� ����
	//========================================================================================================================
	cur = sorted_list_head->next;	// ���� ���, �� ��° ��Һ��� ���� ����
	Node* cur_next;					// ���� ��� ���� ���
	Node* compare;					// ���� ���� ���� ���

	// �� ��° ������ ���� ����
	while (cur) {
		cur_next = cur->next;
		compare = cur->prev;

		if (p0_ptr) { 
			// 1. �������� ����Ʈ ���ο� �ִ� ���
			// ���� ����Ʈ���� �������� ����, sorted_list_head ���ο��� �������� ���� 
			// (p0_ptr�� ���� ����Ʈ���� ����)

			while (compare && ccw(p0_ptr->curX, p0_ptr->curY, compare->curX, compare->curY, cur->curX, cur->curY) > 0) {
				// cur ��尡 compare ��庸�� ���� ��� ����
				compare = compare->prev;
			}
		}
		else { 
			// 2. �������� ����ڰ� GLUT_UP�� ���� ���
			// ���� ����Ʈ�� sorted_list_head ���ο��� �������� ����
			// p0_x, p0_y�� ������

			while (compare && ccw(p0_x, p0_y, compare->curX, compare->curY, cur->curX, cur->curY) > 0) {
				// cur ��尡 compare ��庸�� ���� ��� ����
				compare = compare->prev;
			}
		}		

		if (compare != cur->prev) {
			unlink_node(&sorted_list_head, cur); // cur ��带 ��ũ ����
			insert_after(&sorted_list_head, compare, cur);  // �Ǵ� compare == NULL�̸� �� �տ� ����
		}

		cur = cur_next; // remove_node�� cur �� ��ġ�� �ٲ�Ƿ� cur_next ���
	}
	
	//========================================================================================================================

	return sorted_list_head;
}

int check_convex_hull(Node** head, float gl_X, float gl_Y) { // convex hull ����(1) �Ұ���(0)
	// �׶��� ��ĵ�� ����Ͽ� convex hull ã��

	if (vertex_cnt < 2) // �� 2�� ���ϸ� �׻� convex hull
		return 1;

	// 1. ������ ��, y��ǥ�� ���� �۰�, ������ x��ǥ�� ���� ���� �� �˻�
	// ==============================================================================================================
	float p0_x, p0_y;		// ���� ������ �� ��
	Node* p0_ptr = NULL;	// ���� ���� �� �ּ�

	p0_x = INFINITY; p0_y = INFINITY; // �ʱⰪ�� ���Ѵ� ������ ����

	// ��� �������� �����ϸ�, ������ �� �� Ž��
	Node* temp = *head;
	while (temp) {
		if (temp->curY < p0_y) {
			p0_x = temp->curX;
			p0_y = temp->curY;
			p0_ptr = temp;
		}
		else if (fabs(temp->curY - p0_y) < MARGIN_OF_ERROR) { // Y ��ǥ���� ���� ���, �������� 1.0
			if (temp->curX < p0_x|| fabs(temp->curX - p0_x) < MARGIN_OF_ERROR) { // X ��ǥ���� ��, �۰ų� ���� ��� ����, �������� 1.0
				p0_x = temp->curX;
				p0_y = temp->curY;
				p0_ptr = temp;
			}
		} 
		temp = temp->next;

	} // end of while

	
	// �޾ƿ� �Ű����� ������ ��
	if (gl_Y < p0_y) {
		p0_x = gl_X;
		p0_y = gl_Y;
		p0_ptr = NULL; // ���� �߰��� ������ �������� ���� null�� ǥ���� (���Ḯ��Ʈ ���ο� ������ �� ���� ���ٴ� ���� ǥ��)
	}
	else if (fabs(gl_Y - p0_y) < MARGIN_OF_ERROR) { // Y ��ǥ���� ���� ���, �������� 1.0
		if (gl_X < p0_x || fabs(gl_X - p0_x) < MARGIN_OF_ERROR) { // X ��ǥ���� ��, �۰ų� ���� ��� ����, �������� 1.0
			p0_x = gl_X;
			p0_y = gl_Y;
			p0_ptr = NULL;
		}
	}
	
	// ==============================================================================================================
	
	// 2. ���� ���� ��������, �ݽð� �������� ��� ������ ���� 
	// ==============================================================================================================
	Node* sorted_list;
	if (p0_ptr == NULL) {
		//printf("���� �߰��� ���� ���� �� �Դϴ�. \n");
		sorted_list = sort_vertices(head, gl_X, gl_Y, p0_ptr); // ���� ����Ʈ�� ���������� �����Ͽ� ���� ������ ����Ʈ
	}
	else {
		//printf("����Ʈ ������ ���� ���� �� �Դϴ�. \n");
		sorted_list = sort_vertices(head, gl_X, gl_Y, p0_ptr);
	}

	//printf("���� ���� �� ��ǥ : [%f, %f] \n", p0_x, p0_y);

	// ==============================================================================================================
	

	// 2. �ݽð� ������ �Ǵ��� Ȯ���ϸ�, üũ 
	// ==============================================================================================================
	Node* A;
	Node* B;
	Node* C; // C�� �߰������� convex hull�� �Ǵ��� Ȯ��

	// sorted_list�� ������ ��尡 ����.
	if (p0_ptr == NULL) { // �������� ���� �Է¹��� �� ��ġ
		Node first;
		first.curX = gl_X; first.curY = gl_Y;

		A = &first;
		B = sorted_list;
		C = B->next;
	}
	else { // �������� ���� ����Ʈ ���ο� ����
		A = p0_ptr;
		B = sorted_list;
		C = B->next; 
	}

	while (C) {
		int ret = ccw(A->curX, A->curY, B->curX, B->curY, C->curX, C->curY); // B�������� ���� �Ǵ�

		if (ret > 0) { // B�� C���� ���� -> �ð����
			return 0; // ���� �ٰ��� �ƴ�
		}
		A = B;
		B = C;
		C = C->next;
	}
	
	// ���콺 ������ Ŭ�� ��, �ٰ����� ����� ���� ����.
	if (p0_ptr == NULL) {
		std_x = gl_X; std_y = gl_Y; // ����
		std_loc = NULL;
	}
	else {
		std_loc = p0_ptr;
	}

	return 1; // convex hull ������
	// ==============================================================================================================
}

int point_exist(Node** head, float gl_X, float gl_Y) { // �̹� �����ϴ� �������� �˻�
	Node* temp = *head;
	while (temp) {
		if (fabs(temp->curX - gl_X) < MARGIN_OF_ERROR && fabs(temp->curY - gl_Y) < MARGIN_OF_ERROR) { // 1.0�� ���������� ����.
			return 1; // �̹� �����ϴ� ����
		}
		temp = temp->next;
	}
	return 0; // �������� �ʴ� ����
}

void mouse(int button, int state, int x, int y) { // �޾ƿ� x, y�� ���� ���� (0, 0) ����
	// ��ǥ ��ȯ
	float gl_X, gl_Y;
	convert_coordinate(x, y, &gl_X, &gl_Y);

	Node* temp;
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
			// �ٰ����� �����Ǿ��� ���
			// ======================================================================================================
			if (generated_poly == 1) { // �ٰ����� �����Ǿ����� ���� X
				// �巡�� ���� ��ǥ ����
				drag_start_x = gl_X;
				drag_start_y = gl_Y;

				// �巡�� ���� �ð� ����
				drag_start_time = glutGet(GLUT_ELAPSED_TIME);

				break;
			}
			// ======================================================================================================

			mouse_left_button = 1;
			status = GLUT_DOWN; curX = gl_X; curY = gl_Y;
			glutPostRedisplay(); // display ȣ��
		}
		else if (state == GLUT_UP) {

			// �ٰ����� �����Ǿ��� ���
			// ======================================================================================================
			if (generated_poly == 1) {  // �ٰ����� �����Ǿ����� ���� X
				// �巡�� ���� ��ǥ ����
				drag_end_x = gl_X;
				drag_end_y = gl_Y;

				// �巡�� ���� �ð� ����
				drag_end_time = glutGet(GLUT_ELAPSED_TIME);

				int ret = is_dragging_polygon(&head, drag_start_x, drag_start_y, drag_end_x, drag_end_y);

				if (ret) { // �ٰ��� ��ü�� �巡�� �� ���
					// 1. �ٰ��� �����߽� ���ϱ�
					calc_center();

					// 2. ȸ�� �ӵ� ���ϱ�
					calc_spin_v();

					is_draged = 1; // �ٰ����� ���ԵǼ� �巡�� �Ǿ���
					glutPostRedisplay();
				}

				break;
			}
			// ======================================================================================================

			if (point_exist(&head, gl_X, gl_Y)) { // ���� ������ �� �ٽ� ������ ���� x
				printf("�̹� �����ϴ� ���� �Դϴ�. \n");
			}
			else {
				// 1. convex hull Ȯ��
				if (check_convex_hull(&head, gl_X, gl_Y)) {
					// 2. ���� ����Ʈ�� ����
					head = insert_node(&head, curX, curY);

					// 3. ������ �������� ����
					last_point_X = gl_X; last_point_Y = gl_Y;

					// 4. ���� ���� ����
					vertex_cnt++;
					vertex_cnt++;
				}
				else {
					printf("������ �߰��� �� �����ϴ�. (Convex Hull ���� �Ұ�) \n");
				}
			}

			mouse_left_button = 0;

			status = GLUT_UP; curX = gl_X; curY = gl_Y;
			glutPostRedisplay(); // display ȣ��
		}
		break;

	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_UP) {
			mouse_right_button = 1;
			glutPostRedisplay();
		}
		break;

	case GLUT_MIDDLE_BUTTON: // �ٰ��� �ʱ�ȭ
		delete_list(&head);
		vertex_cnt = 0;
		generated_poly = 0;
		is_draged = 0;
		special_key = 0;
		glutPostRedisplay();

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

int is_dragging_polygon(Node** head, float drag_start_x, float drag_start_y, float drag_end_x, float drag_end_y) {
	// �ٰ����� ��� �������� �����Ͽ� �ּ� ��ǥ, �ִ� ��ǥ�� ���ϰ�, �巡�׿� ���ԵǾ� �ִ��� �Ǵ�

	float minX = INFINITY, minY = INFINITY;
	float maxX = -INFINITY, maxY = -INFINITY;

	Node* cur = *head;
	while (cur) {
		if (cur->curX < minX) minX = cur->curX;
		if (cur->curY < minY) minY = cur->curY;
		if (cur->curX > maxX) maxX = cur->curX;
		if (cur->curY > maxY) maxY = cur->curY;
		cur = cur->next;
	}

	// �巡�� �ڽ��� �ٰ����� �����ϴ��� Ȯ��
	return (drag_start_x <= minX && drag_start_y <= minY &&
		drag_end_x >= maxX && drag_end_y >= maxY);
}

void calc_center() {
	int cnt = 0;
	float sum_x = 0.0, sum_y = 0.0;
	Node* temp = head;

	while (temp) {
		sum_x += temp->curX;
		sum_y += temp->curY;
		cnt++;
		temp = temp->next;
	}

	center_x = sum_x / cnt;
	center_y = sum_y / cnt;	
}

void spin_poly_left() { // �ݽð� ���� ȸ��
	spin = spin + velocity;
	if (spin > 360.0) {
		spin = spin - 360.0;
		if (special_key == 1)
			color_index = (color_index + 1) % 16;
	}
	glutPostRedisplay();
}

void calc_spin_v() {
	// �巡�� �ӵ��� ���� ȸ�� �ӵ� ���

	float dx = drag_end_x - drag_start_x;
	float dy = drag_end_y - drag_start_y;

	float distance = sqrt(dx * dx + dy * dy); // ���� ���� �Ÿ�

	int time_diff = drag_end_time - drag_start_time; // ms, �巡�� �ð�

	if (time_diff > 0) {
		velocity = distance / (time_diff ); // �ȼ�/ms
		printf("Drag speed: %.2f \n", velocity);
	}
}

void specialKeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:// ���� ����Ű
		special_key = 1;
		glutPostRedisplay();
		break;
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
	glutSpecialFunc(specialKeys);

	glutMainLoop();

	return 0;
}