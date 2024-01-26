#include <gtk/gtk.h>

#define NAME_SIZE 16
#define queue_size 20
#define BUF_SIZE 128
#define GDK_KEY_UP 65362
#define GDK_KEY_DOWN 65364
#define GDK_KEY_LEFT 65361
#define GDK_KEY_RIGHT 65363

//object data structures
typedef struct location{
    int x;
    int y;
} location_t;

typedef struct user{
    char name[NAME_SIZE];
    int score;
    location_t base_loc;
    location_t user_loc;
}user_t;

typedef struct object_data{
    int map_width;
	int map_height;
    int timeout;
    int max_user;
    struct user * users;
    location_t * item_locations;
    location_t * block_locations;
}object_data_t;

enum entity {
	EMPTY = 0,
	BLOCK = -1,
	ITEM = -9, //item will be -10 ~ -110
	USER = 1, //user wil be 1 ~ 4
	BASE = 9, //base will be 10 ~ 30
};

enum spans {
	UP, 
	DOWN, 
	LEFT, 
	RIGHT
};

extern char user_color[8][20];

extern int ** map;  // cell
extern object_data_t Model; // model
extern char msg_info[BUF_SIZE];
extern char buf[BUF_SIZE];
extern int sock;
extern int my_id;
extern int num_item, num_block;
extern int current_num_item;

extern pthread_mutex_t mutx;
extern pthread_cond_t cond;
extern int event_array[queue_size];
extern int rear;
extern int front;


//GUI functions
GtkWidget *window;
GtkWidget *mat_main, *mat_changed_screen, *mat_board, *label_info, *label_me, *mat_fixed_screen, *mat_screen;
GtkWidget *mat_ans_btn, *mat_sol_btn;
GtkWidget *btn_solve, *btn_exit, *btn_next, *btn_prev;
GtkWidget *btn_auto, *btn_up, *btn_down, *btn_left, *btn_right;
GtkWidget *label_name;
GtkWidget ** label_score; 
GdkPixbuf *icon, *icon_block[2], *icon_fruit[11];
GdkPixbuf ** icon_player;
GdkPixbuf *create_pixbuf(const gchar * filename);
GtkWidget* create_entity(int id);
int load_icons();
int check_map_valid();
void set_window();
static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
void display_screen();
void add_mat_board();
void exit_game(GtkWidget* widget);
void gameover();

//for move handling
int check_validation(int cmd);
int move(int cmd, int movement);
void update_model(int id, int x, int y);
void update_cell();
int item_idxToId(int idx);
int item_idToIdx(int id);
void score_up(int user_idx);
gboolean handle_cmd(gpointer user_data) ;

//for networking
int recv_bytes(int sock_fd, void * buf, size_t len);
int send_bytes(int sock_fd, void * buf, size_t len);
void handle_timeout(int signum);
int parseJson(char * jsonfile);
void *recv_msg(void * arg);
void cannot_enter();