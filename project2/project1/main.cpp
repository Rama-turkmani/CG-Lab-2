#include <iostream>
#include <cmath>

// 1. تضمين المكتبات
// ملاحظة: يجب تضمين GLEW قبل GLFW دائماً
#define GLEW_STATIC // لأننا نستخدم مكتبة glew32s.lib (Static)
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// إعدادات النافذة
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// وضع العرض الحالي: 1=مثلث، 2=مستطيل، 3=مستطيل مع دوران
int currentMode = 1;

// --- 2. كود المظللات (Shaders) ---

// A. كود Vertex Shader (معدّل: يدعم مصفوفة التحويل)
// وظيفته: تحديد موقع الرؤوس مع تطبيق التحويلات (الدوران والإزاحة)
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform mat4 transform;\n" // مصفوفة التحويل
"void main()\n"
"{\n"
"   gl_Position = transform * vec4(aPos, 1.0);\n" // تطبيق التحويل على الموقع
"}\0";

// B. كود Fragment Shader (معدّل: يدعم تغيير اللون عبر uniform)
// وظيفته: تحديد لون البكسلات باستخدام لون يتم تمريره من البرنامج
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 ourColor;\n" // اللون يتم تمريره كـ uniform
"void main()\n"
"{\n"
"   FragColor = ourColor;\n"
"}\n\0";

// --- دوال مساعدة لحساب المصفوفات (بدون مكتبة GLM) ---

// مصفوفة الوحدة (Identity Matrix)
void mat4Identity(float m[16])
{
    for (int i = 0; i < 16; i++) m[i] = 0.0f;
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

// ضرب مصفوفتين 4×4 (ترتيب الأعمدة - Column Major)
void mat4Multiply(const float a[16], const float b[16], float result[16])
{
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            result[col * 4 + row] = 0.0f;
            for (int k = 0; k < 4; k++) {
                result[col * 4 + row] += a[k * 4 + row] * b[col * 4 + k];
            }
        }
    }
}

// مصفوفة الدوران حول المحور X
void mat4RotateX(float angle, float m[16])
{
    mat4Identity(m);
    float c = cosf(angle);
    float s = sinf(angle);
    m[5]  =  c;  m[9]  = -s;
    m[6]  =  s;  m[10] =  c;
}

// مصفوفة الدوران حول المحور Y
void mat4RotateY(float angle, float m[16])
{
    mat4Identity(m);
    float c = cosf(angle);
    float s = sinf(angle);
    m[0]  =  c;  m[8]  =  s;
    m[2]  = -s;  m[10] =  c;
}

// مصفوفة الدوران حول المحور Z
void mat4RotateZ(float angle, float m[16])
{
    mat4Identity(m);
    float c = cosf(angle);
    float s = sinf(angle);
    m[0] =  c;  m[4] = -s;
    m[1] =  s;  m[5] =  c;
}

// دالة لمعالجة تغيير حجم النافذة من قبل المستخدم
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// دالة Key Callback للتبديل بين المهام (أكثر موثوقية من glfwGetKey)
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, true);

        // مفاتيح Q, W, E للتبديل بين المهام
        if (key == GLFW_KEY_Q)
        {
            currentMode = 1;
            std::cout << ">> Mode 1 [Q]: Triangle only" << std::endl;
        }
        if (key == GLFW_KEY_W)
        {
            currentMode = 2;
            std::cout << ">> Mode 2 [W]: Rectangle only (no rotation)" << std::endl;
        }
        if (key == GLFW_KEY_E)
        {
            currentMode = 3;
            std::cout << ">> Mode 3 [E]: Rectangle with rotation (Bonus)" << std::endl;
        }
    }
}

// دالة لمعالجة المدخلات (احتياطي)
void processInput(GLFWwindow* window)
{
    // المعالجة الرئيسية تتم في key_callback
}

int main()
{
    // --- 3. تهيئة GLFW وإعداد النافذة ---
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // إصدار OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // النمط الحديث

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // خاص بأجهزة ماك
#endif

    // إنشاء كائن النافذة
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CG Assignment - Triangle & Rotating Rectangle", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // جعل هذه النافذة هي سياق الرسم الحالي
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // ربط دالة تغيير الحجم
    glfwSetKeyCallback(window, key_callback); // ربط دالة المفاتيح للتبديل بين الأوضاع

    // --- 4. تهيئة GLEW ---
    glewExperimental = GL_TRUE; // تفعيل التقنيات الحديثة
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // طباعة تعليمات التحكم في الكونسول
    std::cout << "========================================" << std::endl;
    std::cout << "  CG Assignment - Keyboard Controls:" << std::endl;
    std::cout << "  [Q] Triangle only" << std::endl;
    std::cout << "  [W] Rectangle only (no rotation)" << std::endl;
    std::cout << "  [E] Rectangle with rotation (Bonus)" << std::endl;
    std::cout << "  [ESC] Exit" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << ">> Mode 1 [Q]: Triangle only" << std::endl;

    // --- 5. بناء وتجميع برنامج الشيدر (Shader Program) ---

    // أ. تجميع Vertex Shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // فحص الأخطاء
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // ب. تجميع Fragment Shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // فحص الأخطاء
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // ج. ربط الشيدرز في برنامج واحد (Shader Program)
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // فحص أخطاء الربط
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // حذف الشيدرز المنفصلة لأننا ربطناها في البرنامج ولم نعد بحاجة لها
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ======================================================
    // --- 6أ. تعريف بيانات المثلث (Task 1) ---
    // ======================================================
    float triVertices[] = {
        -0.85f, -0.35f, 0.0f,  // الرأس السفلي الأيسر
        -0.35f, -0.35f, 0.0f,  // الرأس السفلي الأيمن
        -0.60f,  0.25f, 0.0f   // الرأس العلوي
    };

    // --- 7أ. إعداد VAO و VBO للمثلث ---
    unsigned int triVAO, triVBO;
    glGenVertexArrays(1, &triVAO);
    glGenBuffers(1, &triVBO);
    glBindVertexArray(triVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triVertices), triVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // ======================================================
    // --- 6ب. تعريف بيانات المستطيل (Task 2) ---
    // المستطيل مكون من 4 رؤوس و 6 فهارس (مثلثين)
    // الأبعاد: عرض 0.6 × ارتفاع 0.35 (فريدة لكل طالب)
    // ======================================================
    float rectVertices[] = {
        -0.30f,  0.175f, 0.0f,  // الزاوية العلوية اليسرى
         0.30f,  0.175f, 0.0f,  // الزاوية العلوية اليمنى
         0.30f, -0.175f, 0.0f,  // الزاوية السفلية اليمنى
        -0.30f, -0.175f, 0.0f   // الزاوية السفلية اليسرى
    };
    // ترتيب الفهارس لرسم مثلثين يشكلان المستطيل
    unsigned int rectIndices[] = {
        0, 1, 2,  // المثلث الأول (علوي أيمن)
        0, 2, 3   // المثلث الثاني (سفلي أيسر)
    };

    // --- 7ب. إعداد VAO و VBO و EBO للمستطيل ---
    unsigned int rectVAO, rectVBO, rectEBO;
    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);
    glGenBuffers(1, &rectEBO);
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertices), rectVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectIndices), rectIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // الحصول على مواقع المتغيرات (uniforms) في الشيدر
    int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    int colorLoc = glGetUniformLocation(shaderProgram, "ourColor");

    // --- 8. حلقة الرسم (Render Loop) ---
    while (!glfwWindowShouldClose(window))
    {
        // أ. معالجة المدخلات
        processInput(window);

        // ب. التنظيف (لون الخلفية: أزرق غامق)
        glClearColor(0.12f, 0.12f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        float identity[16];
        mat4Identity(identity);

        // ==============================
        // الوضع 1: رسم المثلث فقط (Task 1)
        // ==============================
        if (currentMode == 1)
        {
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, identity);
            // لون المثلث: أخضر فيروزي (لون فريد)
            glUniform4f(colorLoc, 0.18f, 0.80f, 0.65f, 1.0f);
            glBindVertexArray(triVAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        // ==============================
        // الوضع 2: رسم المستطيل بدون دوران (Task 2)
        // ==============================
        if (currentMode == 2)
        {
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, identity);
            // لون المستطيل: وردي أرجواني (لون فريد)
            glUniform4f(colorLoc, 0.85f, 0.25f, 0.55f, 1.0f);
            glBindVertexArray(rectVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // ==============================
        // الوضع 3: رسم المستطيل مع الدوران (Bonus)
        // الدوران على المحاور X, Y, Z بسرعات مختلفة
        // ==============================
        if (currentMode == 3)
        {
            float time = (float)glfwGetTime();

            // حساب مصفوفات الدوران لكل محور بزوايا مختلفة
            float rotX[16], rotY[16], rotZ[16];
            mat4RotateX(time * 0.7f, rotX);   // دوران حول X بسرعة 0.7 راديان/ثانية
            mat4RotateY(time * 0.5f, rotY);   // دوران حول Y بسرعة 0.5 راديان/ثانية
            mat4RotateZ(time * 1.0f, rotZ);   // دوران حول Z بسرعة 1.0 راديان/ثانية

            // دمج الدورانات: Rz × Ry × Rx
            float temp[16], finalTransform[16];
            mat4Multiply(rotZ, rotY, temp);
            mat4Multiply(temp, rotX, finalTransform);

            // لا حاجة لإزاحة - المستطيل في المنتصف بما أنه يُعرض لوحده
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, finalTransform);
            // لون المستطيل: وردي أرجواني (لون فريد)
            glUniform4f(colorLoc, 0.85f, 0.25f, 0.55f, 1.0f);
            glBindVertexArray(rectVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        // هـ. تبديل الـ Buffers ومعالجة الأحداث
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- 9. التنظيف النهائي ---
    glDeleteVertexArrays(1, &triVAO);
    glDeleteBuffers(1, &triVBO);
    glDeleteVertexArrays(1, &rectVAO);
    glDeleteBuffers(1, &rectVBO);
    glDeleteBuffers(1, &rectEBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}