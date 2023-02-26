
extern const unsigned int SCR_WIDTH;
extern const unsigned int SCR_HEIGHT;
extern Camera camera;
extern float lastX;
extern float lastY;
extern bool firstMouse;
extern float deltaTime;
extern float lastFrame;
extern glm::vec3 lightPos;
extern glm::vec3 lightColor;


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void create_sphere();
void create_tool();

