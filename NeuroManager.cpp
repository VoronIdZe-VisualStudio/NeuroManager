#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <include/GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <vector>
#include <clocale>

#define _CRT_SECURE_NO_WARNINGS


#include "AIHelper.h"

#include "ProcessUtils.h"
#include "ToolManager.h"
#include "MemoryEditor.h" 


// где-то глобально или в синглтоне:
AIHelper aiHelper;

void RenderAIHelperUI() {
    static char processNameInput[256] = "TargetGame.exe";

    ImGui::InputText("Process Name", processNameInput, sizeof(processNameInput));
    if (ImGui::Button("Connect to Process")) {
        if (aiHelper.ConnectToProcessByName(processNameInput)) {
            aiHelper.AnalyzeProcess();
        }
        else {
            // Можно показать ошибку подключения
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Disconnect")) {
        aiHelper.DisconnectProcess();
    }

    ImGui::Separator();

    if (aiHelper.IsProcessConnected()) {
        ImGui::TextWrapped("%s", aiHelper.GetInfo().c_str());
    }
    else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not connected");
    }
}

std::vector<std::pair<DWORD, std::string>> processes;
static int selectedProcess = -1;
static HANDLE hProcess = NULL;
static DWORD currentPid = 0;

MemoryEditor memoryEditor;
bool showMemoryEditor = false;

ProcessUtils utils;
ToolManager toolManager;  // Экземпляр менеджера инструментов

void RefreshProcessList() {
    processes = utils.GetProcessList();
    selectedProcess = -1;
}

enum class Language {
    EN,
    RU
};

Language currentLanguage = Language::EN;

// Функция получения строки в зависимости от языка
const char* _(const char* en, const char* ru) {
    return currentLanguage == Language::EN ? en : ru;
}

void SetupImGuiStyle() {
    if (!ImGui::GetCurrentContext())
        return;

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.35f, 0.45f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.30f, 0.35f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.35f, 0.40f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.20f, 1.00f);
    style.FrameRounding = 3.0f;
    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(6, 4);
    style.ItemSpacing = ImVec2(8, 4);
}


int main() {
    setlocale(LC_ALL, "RU");

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1280, 720, "NeuroCheatManager", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    SetupImGuiStyle();

    std::vector<std::string> chatMessages;
    char processName[256] = "game.exe";
    char newToolName[256] = "";
    char chatInput[256] = "";

    RefreshProcessList();

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Выбор языка
        ImGui::Begin("Settings");
        if (ImGui::Button("English")) currentLanguage = Language::EN;
        ImGui::SameLine();
        if (ImGui::Button("Русский")) currentLanguage = Language::RU;
        ImGui::End();

        ImGui::Begin(_("NeuroCheatManager", "НейроЧитМенеджер"), nullptr, ImGuiWindowFlags_NoCollapse);

        // --- Чат ---
        ImGui::BeginChild("Chat", ImVec2(400, 300), true);
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), _("Chat with AI", "Чат с ИИ"));
        ImGui::Separator();

        ImGui::BeginChild("ChatHistory", ImVec2(0, 200), true);
        for (const auto& msg : chatMessages) {
            ImGui::TextWrapped("%s", msg.c_str());
        }
        ImGui::EndChild();

        if (ImGui::Button(_("Open Memory Editor", "Открыть Memory Editor"))) {
            showMemoryEditor = !showMemoryEditor;
            memoryEditor.SetOpen(showMemoryEditor);
        }
        memoryEditor.SetProcessHandle(hProcess);

        if (showMemoryEditor) {
            memoryEditor.RenderUI();
        }

        ImGui::InputText("##ChatInput", chatInput, sizeof(chatInput));
        ImGui::SameLine();
        if (ImGui::Button(_("Send", "Отправить"))) {
            if (strlen(chatInput) > 0) {
                chatMessages.push_back(std::string(currentLanguage == Language::EN ? "You: " : "Вы: ") + std::string(chatInput));
                chatMessages.push_back(currentLanguage == Language::EN ? "AI: Access" : "ИИ: Доступ");
                memset(chatInput, 0, sizeof(chatInput));
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // --- Инструменты ---
        ImGui::BeginChild("Tools", ImVec2(400, 300), true);
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.6f, 1.0f), _("Include tools", "Подключаемые инструменты"));
        ImGui::Separator();

        // Выводим инструменты из ToolManager
        for (const auto& tool : toolManager.GetTools()) {
            toolManager.RenderUI();
        }

        ImGui::InputText("##NewTool", newToolName, sizeof(newToolName));
        ImGui::SameLine();
        if (ImGui::Button(_("Add", "Добавить"))) {
            if (strlen(newToolName) > 0) {
                toolManager.AddTool(newToolName);
                memset(newToolName, 0, sizeof(newToolName));
            }
        }
        ImGui::EndChild();

      

        // --- Процессы ---
        ImGui::BeginChild("Process", ImVec2(0, 150), true);
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.4f, 1.0f), _("Process own", "Работа с процессом"));
        ImGui::Separator();

        ImGui::InputText(_("Name Process", "Имя процесса"), processName, sizeof(processName));
        if (ImGui::Button(_("Attach to process", "Подключиться к процессу"), ImVec2(150, 30))) {
            chatMessages.push_back(std::string(currentLanguage == Language::EN ? "Attached: " : "Подключено: ") + std::string(processName));
        }

        ImGui::SameLine();
        if (ImGui::Button(_("Dump memory", "Сделать дамп памяти"), ImVec2(150, 30))) {
            chatMessages.push_back(currentLanguage == Language::EN ? "Set Dump memory" : "Дамп памяти установлен");
        }
        ImGui::EndChild();

        ImGui::End();

        // --- Окно выбора процесса ---
        ImGui::Begin(_("Choose Process", "Выбор процесса"));

        if (ImGui::Button(_("Update list", "Обновить список"))) {
            RefreshProcessList();
        }

        ImGui::SameLine();
        if (hProcess != NULL) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), _("Attached to PID: %d", "Подключено к PID: %d"), currentPid);
        }
        else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), _("Not attached", "Не подключено"));
        }

        ImGui::BeginChild("ProcessList", ImVec2(0, 200), true);
        for (int i = 0; i < (int)processes.size(); i++) {
            char label[256];
            snprintf(label, sizeof(label), "%s (PID: %d)", processes[i].second.c_str(), processes[i].first);

            if (ImGui::Selectable(label, selectedProcess == i)) {
                selectedProcess = i;
            }
        }
        ImGui::EndChild();

        if (selectedProcess != -1) {
            if (hProcess == NULL) {
                if (ImGui::Button(_("Attach", "Подключиться"))) {
                    currentPid = processes[selectedProcess].first;
                    hProcess = ProcessUtils::OpenTargetProcess(currentPid);
                    if (hProcess) {
                        chatMessages.push_back(std::string(currentLanguage == Language::EN ? "Attached to process: " : "Подключено к процессу: ") + processes[selectedProcess].second);
                    }
                    else {
                        chatMessages.push_back(currentLanguage == Language::EN ? "Failed to attach process." : "Не удалось подключиться к процессу.");
                    }
                }
            }
            else {
                if (ImGui::Button(_("Detach", "Отключиться"))) {
                    ProcessUtils::CloseTargetProcess(hProcess);
                    hProcess = NULL;
                    currentPid = 0;
                    chatMessages.push_back(currentLanguage == Language::EN ? "Detached from process." : "Отключено от процесса.");
                }
            }
        }

        ImGui::End();

        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.07f, 0.07f, 0.09f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    if (hProcess != NULL) {
        ProcessUtils::CloseTargetProcess(hProcess);
        hProcess = NULL;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
