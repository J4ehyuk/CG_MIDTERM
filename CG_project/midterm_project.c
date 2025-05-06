/*
*  작성자 : 양재혁
*  작성일 : 25.05.03
*  프로그램명 : Midterm Project
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
float last_point_X;			// 마지막 정점의 X 좌표
float last_point_Y;			// 마지막 정점의 Y 좌표

int mouse_left_button = 0;	// 마우스 왼쪽 버튼이 눌렸는지
int mouse_right_button = 0; // 마우스 오른쪽 버튼이 눌렸는지

Node* cvx_h_verteces = NULL;	// convex hull을 이루는 정점순서 리스트
int generated_poly = 0;			// 다각형이 생성되었는지

float std_x = 0, std_y = 0;		// 기준점 좌표
Node* std_loc;					// 기준점의 위치

float drag_start_x, drag_start_y, drag_end_x, drag_end_y; // 드래그 시작, 끝 정점정보
int drag_start_time, drag_end_time; 
int is_draged = 0;

float center_x, center_y;	// 무게중심 좌표
float spin = 0.0;			
float velocity;				// 회전 속도


int vertex_cnt = 0;			// 현재 정점의 개수

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

	// OpenGL 좌표계로 변환
	*cvt_X = ((float)x / windowWidth) * (ORTHO_RIGHT - ORTHO_LEFT) - ORTHO_RIGHT;
	*cvt_Y = (1 - (float)y / windowHeight) * (ORTHO_TOP - ORTHO_BOTTOM) - ORTHO_TOP;
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT); // 화면 clear

	Node* cur_node;
	if (is_draged == 0) { // 드래그 되어 회전할 경우에는, 정점들 출력 X
		// 1. 모든 정점들 그리기
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

	if (mouse_left_button == 1 && status == GLUT_DOWN) { // 왼쪽 마우스 버튼이 눌려진 채 유지되는 상태

		if (vertex_cnt != 0) { // 첫 번째 정점이 아닌 경우
			// 마지막 정점에서 현재 정점까지 line 그리기
			glBegin(GL_LINE_STRIP); {
				glVertex2f(last_point_X, last_point_Y);
				glVertex2f(curX, curY);
			}glEnd();
		}
	}
	else if (mouse_right_button == 1) {

		delete_list(&cvx_h_verteces); // 기존 기준점으로 정렬된 리스트 메모리 해제

		// 2. 원본 정점 리스트에서 기준점을 찾아서, 그 기준점으로 원본 리스트 복제본을 정렬 후, 출력
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
		Node* temp = cvx_h_verteces; // 다각형 생성 시, 만들어진 정렬된 정점 리스트 사용

		if (special_key == 1)
			glColor3f(PALETTE[color_index][0], PALETTE[color_index][1], PALETTE[color_index][2]);
		else
			glColor3f(0.0, 1.0, 1.0);

		glPushMatrix(); {
			// 1. 무게중심으로 이동
			glTranslatef(center_x, center_y, 0.0);

			// 2. 회전 수행
			glRotatef(spin, 0.0, 0.0, 1.0);

			// 3. 원점 복귀
			glTranslatef(-center_x, -center_y, 0.0);

			// 3. 그리기
			glBegin(GL_POLYGON); {
				while (temp) {
					glVertex2f(temp->curX, temp->curY);
					temp = temp->next;
				}
			}glEnd();
		}glPopMatrix();
		
		spin_poly_left(); // 계속 회전
	}
	glFlush();
}

int ccw(float p0_x, float p0_y, float x1, float y1, float x2, float y2) {
	// 기준점 p0를 기준으로 두 점 (x1, y1), (x2, y2)의 반시계 방향 여부를 판단하는 함수
	/*
		반환값:
		> 0  → p0→(x1,y1)에서 p0→(x2,y2) 방향이 반시계 방향 (left turn)
		< 0  → 시계 방향 (right turn)
		== 0 → 일직선 (colinear), 이때는 기준점에서 더 가까운 점이 먼저 오도록 판단
	*/

	/*
	 외적 공식:
	   (x1 - p0_x)*(y2 - p0_y) - (y1 - p0_y)*(x2 - p0_x)

	 의미:
	   결과가 양수면 반시계 방향, 음수면 시계 방향, 0이면 일직선
	*/

	float ret = ( (x1 - p0_x) * (y2 - p0_y) ) - ( (y1 - p0_y) * (x2 - p0_x) );

	if (ret == 0) { // 서로 순서가 같은경우, 기준점에 더 가까운 점이 빠름
		// 거리 제곱으로 비교
		float dx1 = x1 - p0_x, dy1 = y1 - p0_y;
		float dx2 = x2 - p0_x, dy2 = y2 - p0_y;

		float dist1 = dx1 * dx1 + dy1 * dy1;
		float dist2 = dx2 * dx2 + dy2 * dy2;

		return dist1 < dist2 ? 1 : -1;
	}
	
	return ret;
}

void unlink_node(Node** head, Node* cur) {
	// cur 노드를 연결 리스트에서 링크 해제 하는 함수 (메모리 해제 X)

	if (!cur)
		return;

	if (cur->prev) { // cur->prev 가 존재하는 경우
		cur->prev->next = cur->next;
	}
	else { // cur->prev 가 head 인 경우
		*head = cur->next;
	}


	if (cur->next) { // cur이 마지막 요소가 아닌 경우
		cur->next->prev = cur->prev;
	}

	cur->next = cur->prev = NULL;
}

void insert_after(Node** head, Node* compare, Node* cur) {
	// compare 뒤에 cur 노드 삽입
	if (compare == NULL) { // head 뒤에 삽입하는 경우 (첫 요소로 삽입)
		(*head)->prev = cur;
		cur->next = (*head);
		(*head) = cur;
		
	}
	else {
		if (compare->next) { // 리스트 중간에 삽입하는 경우
			cur->next = compare->next;
			cur->prev = compare;

			compare->next->prev = cur;
			compare->next = cur;

		}
		else { // 마지막 요소로 삽입하는 경우
			cur->next = compare->next;
			cur->prev = compare;
			compare->next = cur;
		}
	}

}

Node* sort_vertices(Node** head, float p0_x, float p0_y, Node* p0_ptr) {
	// p0_x, p0_y	: 마지막에 사용자가 추가할 정점 (이 정점을 포함하여 convex hull이 되는지 확인) 
	// p0_ptr		: 기준점이 리스트 내부에 존재 -> p0_ptr != null / 기준점이 리스트 내부에 존재 X -> p0_ptr == null 

	// 1. 기존 정점 리스트를 복제
	//========================================================================================================================
	Node* sorted_list_head = NULL; // 기준점으로 정렬된 리스트

	Node* cur = *head;
	while (cur) {
		// 노드 복사
		if (cur != p0_ptr) { // 기준점 노드는 복사 X
			insert_node(&sorted_list_head, cur->curX, cur->curY);
		}
		cur = cur->next;
	} // end of while

	//========================================================================================================================


	if (p0_ptr) { // p0_ptr != NULL -> 기준점이 리스트 내부에 존재
		insert_node(&sorted_list_head, p0_x, p0_y); // 사용자가 마지막에 추가할 정점 삽입 (convex hull 판단을 위해)	

	}// else -> p0_ptr == NULL -> 기준점이 새로 추가할 정점
		

	// 2. 삽입 정렬 수행
	//========================================================================================================================
	cur = sorted_list_head->next;	// 현재 노드, 두 번째 요소부터 정렬 시작
	Node* cur_next;					// 현재 노드 다음 노드
	Node* compare;					// 현재 노드와 비교할 노드

	// 두 번째 노드부터 정렬 시작
	while (cur) {
		cur_next = cur->next;
		compare = cur->prev;

		if (p0_ptr) { 
			// 1. 기준점이 리스트 내부에 있는 경우
			// 원본 리스트에는 기준점이 있음, sorted_list_head 내부에는 기준점이 없음 
			// (p0_ptr이 원본 리스트에서 참조)

			while (compare && ccw(p0_ptr->curX, p0_ptr->curY, compare->curX, compare->curY, cur->curX, cur->curY) > 0) {
				// cur 노드가 compare 노드보다 빠른 경우 실행
				compare = compare->prev;
			}
		}
		else { 
			// 2. 기준점이 사용자가 GLUT_UP한 점인 경우
			// 원본 리스트와 sorted_list_head 내부에는 기준점이 없음
			// p0_x, p0_y가 기준점

			while (compare && ccw(p0_x, p0_y, compare->curX, compare->curY, cur->curX, cur->curY) > 0) {
				// cur 노드가 compare 노드보다 빠른 경우 실행
				compare = compare->prev;
			}
		}		

		if (compare != cur->prev) {
			unlink_node(&sorted_list_head, cur); // cur 노드를 링크 해제
			insert_after(&sorted_list_head, compare, cur);  // 또는 compare == NULL이면 맨 앞에 삽입
		}

		cur = cur_next; // remove_node시 cur 가 위치가 바뀌므로 cur_next 사용
	}
	
	//========================================================================================================================

	return sorted_list_head;
}

int check_convex_hull(Node** head, float gl_X, float gl_Y) { // convex hull 가능(1) 불가능(0)
	// 그라함 스캔을 사용하여 convex hull 찾기

	if (vertex_cnt < 2) // 점 2개 이하면 항상 convex hull
		return 1;

	// 1. 정점들 중, y좌표가 가장 작고, 같으면 x좌표가 가장 작은 점 검색
	// ==============================================================================================================
	float p0_x, p0_y;		// 정렬 기준이 될 점
	Node* p0_ptr = NULL;	// 정렬 기준 점 주소

	p0_x = INFINITY; p0_y = INFINITY; // 초기값을 무한대 값으로 설정

	// 모든 정점들을 접근하며, 기준이 될 점 탐색
	Node* temp = *head;
	while (temp) {
		if (temp->curY < p0_y) {
			p0_x = temp->curX;
			p0_y = temp->curY;
			p0_ptr = temp;
		}
		else if (fabs(temp->curY - p0_y) < MARGIN_OF_ERROR) { // Y 좌표값이 같은 경우, 오차범위 1.0
			if (temp->curX < p0_x|| fabs(temp->curX - p0_x) < MARGIN_OF_ERROR) { // X 좌표값을 비교, 작거나 같은 경우 실행, 오차범위 1.0
				p0_x = temp->curX;
				p0_y = temp->curY;
				p0_ptr = temp;
			}
		} 
		temp = temp->next;

	} // end of while

	
	// 받아온 매개변수 값과도 비교
	if (gl_Y < p0_y) {
		p0_x = gl_X;
		p0_y = gl_Y;
		p0_ptr = NULL; // 새로 추가될 정점이 기준점이 됨을 null로 표시함 (연결리스트 내부엔 기준이 될 점이 없다는 것을 표시)
	}
	else if (fabs(gl_Y - p0_y) < MARGIN_OF_ERROR) { // Y 좌표값이 같은 경우, 오차범위 1.0
		if (gl_X < p0_x || fabs(gl_X - p0_x) < MARGIN_OF_ERROR) { // X 좌표값을 비교, 작거나 같은 경우 실행, 오차범위 1.0
			p0_x = gl_X;
			p0_y = gl_Y;
			p0_ptr = NULL;
		}
	}
	
	// ==============================================================================================================
	
	// 2. 기준 점을 기준으로, 반시계 방향으로 모든 정점을 정렬 
	// ==============================================================================================================
	Node* sorted_list;
	if (p0_ptr == NULL) {
		//printf("새로 추가될 점이 기준 점 입니다. \n");
		sorted_list = sort_vertices(head, gl_X, gl_Y, p0_ptr); // 기존 리스트를 기준점으로 정렬하여 새로 복제한 리스트
	}
	else {
		//printf("리스트 내부의 점이 기준 점 입니다. \n");
		sorted_list = sort_vertices(head, gl_X, gl_Y, p0_ptr);
	}

	//printf("정렬 기준 점 좌표 : [%f, %f] \n", p0_x, p0_y);

	// ==============================================================================================================
	

	// 2. 반시계 방향이 되는지 확인하며, 체크 
	// ==============================================================================================================
	Node* A;
	Node* B;
	Node* C; // C를 추가함으로 convex hull이 되는지 확인

	// sorted_list엔 기준점 노드가 없다.
	if (p0_ptr == NULL) { // 기준점이 현재 입력받은 점 위치
		Node first;
		first.curX = gl_X; first.curY = gl_Y;

		A = &first;
		B = sorted_list;
		C = B->next;
	}
	else { // 기준점이 원본 리스트 내부에 존재
		A = p0_ptr;
		B = sorted_list;
		C = B->next; 
	}

	while (C) {
		int ret = ccw(A->curX, A->curY, B->curX, B->curY, C->curX, C->curY); // B기준으로 방향 판단

		if (ret > 0) { // B가 C보다 왼쪽 -> 시계방향
			return 0; // 볼록 다각형 아님
		}
		A = B;
		B = C;
		C = C->next;
	}
	
	// 마우스 오른쪽 클릭 시, 다각형을 만들기 위해 저장.
	if (p0_ptr == NULL) {
		std_x = gl_X; std_y = gl_Y; // 저장
		std_loc = NULL;
	}
	else {
		std_loc = p0_ptr;
	}

	return 1; // convex hull 유지됨
	// ==============================================================================================================
}

int point_exist(Node** head, float gl_X, float gl_Y) { // 이미 존재하는 정점인지 검사
	Node* temp = *head;
	while (temp) {
		if (fabs(temp->curX - gl_X) < MARGIN_OF_ERROR && fabs(temp->curY - gl_Y) < MARGIN_OF_ERROR) { // 1.0의 오차범위를 가짐.
			return 1; // 이미 존재하는 정점
		}
		temp = temp->next;
	}
	return 0; // 존재하지 않는 정점
}

void mouse(int button, int state, int x, int y) { // 받아온 x, y는 왼쪽 위가 (0, 0) 원점
	// 좌표 변환
	float gl_X, gl_Y;
	convert_coordinate(x, y, &gl_X, &gl_Y);

	Node* temp;
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN) {
			// 다각형이 생성되었을 경우
			// ======================================================================================================
			if (generated_poly == 1) { // 다각형이 생성되었으니 동작 X
				// 드래그 시작 좌표 저장
				drag_start_x = gl_X;
				drag_start_y = gl_Y;

				// 드래그 시작 시간 저장
				drag_start_time = glutGet(GLUT_ELAPSED_TIME);

				break;
			}
			// ======================================================================================================

			mouse_left_button = 1;
			status = GLUT_DOWN; curX = gl_X; curY = gl_Y;
			glutPostRedisplay(); // display 호출
		}
		else if (state == GLUT_UP) {

			// 다각형이 생성되었을 경우
			// ======================================================================================================
			if (generated_poly == 1) {  // 다각형이 생성되었으니 동작 X
				// 드래그 종료 좌표 저장
				drag_end_x = gl_X;
				drag_end_y = gl_Y;

				// 드래그 종료 시간 저장
				drag_end_time = glutGet(GLUT_ELAPSED_TIME);

				int ret = is_dragging_polygon(&head, drag_start_x, drag_start_y, drag_end_x, drag_end_y);

				if (ret) { // 다각형 전체를 드래그 한 경우
					// 1. 다각형 무게중심 구하기
					calc_center();

					// 2. 회전 속도 구하기
					calc_spin_v();

					is_draged = 1; // 다각형이 포함되서 드래그 되었음
					glutPostRedisplay();
				}

				break;
			}
			// ======================================================================================================

			if (point_exist(&head, gl_X, gl_Y)) { // 찍은 정점에 또 다시 찍으면 저장 x
				printf("이미 존재하는 정점 입니다. \n");
			}
			else {
				// 1. convex hull 확인
				if (check_convex_hull(&head, gl_X, gl_Y)) {
					// 2. 연결 리스트에 삽입
					head = insert_node(&head, curX, curY);

					// 3. 마지막 정점으로 갱신
					last_point_X = gl_X; last_point_Y = gl_Y;

					// 4. 정점 개수 증가
					vertex_cnt++;
					vertex_cnt++;
				}
				else {
					printf("정점을 추가할 수 없습니다. (Convex Hull 생성 불가) \n");
				}
			}

			mouse_left_button = 0;

			status = GLUT_UP; curX = gl_X; curY = gl_Y;
			glutPostRedisplay(); // display 호출
		}
		break;

	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_UP) {
			mouse_right_button = 1;
			glutPostRedisplay();
		}
		break;

	case GLUT_MIDDLE_BUTTON: // 다각형 초기화
		delete_list(&head);
		vertex_cnt = 0;
		generated_poly = 0;
		is_draged = 0;
		special_key = 0;
		glutPostRedisplay();

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

int is_dragging_polygon(Node** head, float drag_start_x, float drag_start_y, float drag_end_x, float drag_end_y) {
	// 다각형의 모든 정점들을 접근하여 최소 좌표, 최대 좌표를 구하고, 드래그에 포함되어 있는지 판단

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

	// 드래그 박스가 다각형을 포함하는지 확인
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

void spin_poly_left() { // 반시계 방향 회전
	spin = spin + velocity;
	if (spin > 360.0) {
		spin = spin - 360.0;
		if (special_key == 1)
			color_index = (color_index + 1) % 16;
	}
	glutPostRedisplay();
}

void calc_spin_v() {
	// 드래그 속도에 따른 회전 속도 계산

	float dx = drag_end_x - drag_start_x;
	float dy = drag_end_y - drag_start_y;

	float distance = sqrt(dx * dx + dy * dy); // 두점 사이 거리

	int time_diff = drag_end_time - drag_start_time; // ms, 드래그 시간

	if (time_diff > 0) {
		velocity = distance / (time_diff ); // 픽셀/ms
		printf("Drag speed: %.2f \n", velocity);
	}
}

void specialKeys(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:// 위쪽 방향키
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