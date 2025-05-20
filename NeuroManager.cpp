#include "imgui.h"
#include "backends\imgui_impl_glfw.h"
#include "backends\imgui_impl_opengl3.h"
#include <include\GLFW\glfw3.h>
#include <iostream>
#include <string>
#include <vector>

// Стиль интерфейса
void SetupImGuiStyle()
{
    // Проверяем, что контекст ImGui существует
    if (!ImGui::GetCurrentContext())
        return;

    ImGuiStyle& style = ImGui::GetStyle();

    // Цвета интерфейса (темная тема с синими акцентами)
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.35f, 0.45f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.30f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.35f, 0.40f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);

    // Закругление элементов
    style.FrameRounding = 3.0f;
    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;

    // Отступы и размеры
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 4);
}

int main() {
    // Инициализация GLFW и ImGui (как в предыдущих примерах)
    if (!glfwInit()) return 1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "NeuroCheatManager", NULL, NULL);
    // ... (остальная инициализация)
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Настройка стиля
    SetupImGuiStyle();

    // Данные приложения
    std::vector<std::string> chatMessages;
    std::vector<std::string> tools = { "Cheat Engine", "x64dbg", "IDA Pro" };
    char processName[256] = "game.exe";
    char newToolName[256] = "";
    char chatInput[256] = "";

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Главное окно
        ImGui::Begin("NeuroCheatManager", nullptr, ImGuiWindowFlags_NoCollapse);

        // Раздел 1: Чат с нейросетью
        ImGui::BeginChild("Chat", ImVec2(400, 300), true);
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Чат с AI");
        ImGui::Separator();

        // История чата
        ImGui::BeginChild("ChatHistory", ImVec2(0, 200), true);
        for (const auto& msg : chatMessages) {
            ImGui::TextWrapped("%s", msg.c_str());
        }
        ImGui::EndChild();

        // Ввод сообщения
        ImGui::InputText("##ChatInput", chatInput, sizeof(chatInput));
        ImGui::SameLine();
        if (ImGui::Button("Отправить")) {
            if (strlen(chatInput) > 0) {
                chatMessages.push_back("Вы: " + std::string(chatInput));
                // Здесь можно добавить логику ответа нейросети
                chatMessages.push_back("AI: Запрос обработан");
                memset(chatInput, 0, sizeof(chatInput));
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Раздел 2: Инструменты
        ImGui::BeginChild("Tools", ImVec2(400, 300), true);
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), "Подключенные инструменты");
        ImGui::Separator();

        // Список инструментов
        for (const auto& tool : tools) {
            ImGui::BulletText("%s", tool.c_str());
        }

        // Добавление нового инструмента
        ImGui::InputText("##NewTool", newToolName, sizeof(newToolName));
        ImGui::SameLine();
        if (ImGui::Button("Добавить")) {
            if (strlen(newToolName) > 0) {
                tools.push_back(newToolName);
                memset(newToolName, 0, sizeof(newToolName));
            }
        }
        ImGui::EndChild();

        // Раздел 3: Управление процессом
        ImGui::BeginChild("Process", ImVec2(0, 150), true);
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.4f, 1.0f), "Управление процессом");
        ImGui::Separator();

        ImGui::InputText("Имя процесса", processName, sizeof(processName));
        if (ImGui::Button("Подключиться", ImVec2(150, 30))) {
            // Логика подключения к процессу
            chatMessages.push_back("Подключение к процессу: " + std::string(processName));
        }

        ImGui::SameLine();
        if (ImGui::Button("Дамп памяти", ImVec2(150, 30))) {
            chatMessages.push_back("Создание дампа памяти");
        }
        ImGui::EndChild();

        ImGui::End();

        // Рендеринг
        ImGui::Render();
        glfwSwapBuffers(window);
    }

    // Очистка
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();  // Уничтожаем контекст в последнюю очередь

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}