#define MEM_SIZE 1000
#define FRAME_SIZE 3

extern int FRAME_STORE_SIZE;
extern int VAR_STORE_SIZE;
extern int *frame_allocation_table;

extern size_t *last_used;
extern size_t time_counter;

extern struct frame_user **frame_users;
extern int *num_users_per_frame;

struct program_line {
    int allocated;
    char *line;
};

extern struct program_line linememory[MEM_SIZE];

void assert_linememory_is_empty(void);
size_t allocate_line(const char *line);
void free_line(size_t index);
const char *get_line(size_t index);
void reset_linememory_allocator(void);

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);

void init_frame_store(int framesize);
size_t find_free_frame(void);
void free_frame(size_t frame_num);
size_t allocate_line_at(size_t index, const char *line);

size_t select_victim_frame(void);