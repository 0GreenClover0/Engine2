#include "Editor.h"

#include "EngineDefines.h"

#if EDITOR
#include "imgui_extensions.h"
#include "imgui_stdlib.h"
#include <ImGuizmo.h>
#include <imgui.h>
#include <imgui_internal.h>
#endif

#include <algorithm>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "AK/ScopeGuard.h"

#include "Button.h"
#include "Camera.h"
#include "Collider2D.h"
#include "ComponentList.h"
#include "Cube.h"
#include "Curve.h"
#include "Debug.h"
#include "DebugDrawing.h"
#include "DebugInputController.h"
#include "DialoguePromptController.h"
#include "DirectionalLight.h"
#include "Ellipse.h"
#include "Engine.h"
#include "Entity.h"
#include "ExampleDynamicText.h"
#include "ExampleUIBar.h"
#include "Floater.h"
#include "FloatersManager.h"
#include "FloeButton.h"
#include "Game/Clock.h"
#include "Game/Credits.h"
#include "Game/Customer.h"
#include "Game/CustomerManager.h"
#include "Game/EndScreen.h"
#include "Game/EndScreenFoliage.h"
#include "Game/Factory.h"
#include "Game/FieldCell.h"
#include "Game/FieldGrid.h"
#include "Game/GameController.h"
#include "Game/HovercraftWithoutKeeper.h"
#include "Game/IceBound.h"
#include "Game/Jeep.h"
#include "Game/LevelController.h"
#include "Game/Lighthouse.h"
#include "Game/LighthouseKeeper.h"
#include "Game/LighthouseLight.h"
#include "Game/Path.h"
#include "Game/Player.h"
#include "Game/Player/PlayerInput.h"
#include "Game/Popup.h"
#include "Game/Port.h"
#include "Game/Ship.h"
#include "Game/ShipEyes.h"
#include "Game/ShipSpawner.h"
#include "Game/Thanks.h"
#include "Game/Truther.h"
#include "Game/UndoTest.h"
#include "Game/WheatOverlay.h"
#include "Globals.h"
#include "Grass.h"
#include "Input.h"
#include "Komiks/Cow.h"
#include "Komiks/CowManager.h"
#include "Komiks/JeepReflector.h"
#include "Komiks/UFO.h"
#include "Komiks/Wheat.h"
#include "Light.h"
#include "Model.h"
#include "NowPromptTrigger.h"
#include "Panel.h"
#include "Particle.h"
#include "ParticleSystem.h"
#include "PointLight.h"
#include "Quad.h"
#include "RendererDX11.h"
#include "SceneSerializer.h"
#include "ScreenText.h"
#include "Sound.h"
#include "SoundListener.h"
#include "Sphere.h"
#include "SpotLight.h"
#include "Sprite.h"
#include "Water.h"
// # Put new header here

namespace Editor
{

#if EDITOR
Editor::Editor(AK::Badge<Editor>)
{
    set_style();

    add_debug_window();
    add_content_browser();
    add_game();
    add_inspector();
    add_scene_hierarchy();
    add_history();

    m_last_second = glfwGetTime();

    load_assets();

    m_camera_entity = Entity::create_internal("Editor Camera");
    m_editor_camera = m_camera_entity->add_component_internal<Camera>(Camera::create());
    reset_camera();

    if (!Engine::is_game_running())
        Camera::set_main_camera(m_editor_camera);
}

Editor::~Editor()
{
    std::filesystem::path const copied_entity_path = m_copied_entity_path;

    if (std::filesystem::exists(copied_entity_path))
    {
        std::filesystem::remove(copied_entity_path);
    }
}

std::shared_ptr<Editor> Editor::create()
{
    auto editor = std::make_shared<Editor>(AK::Badge<Editor> {});

    m_instance = editor;

    Input::input->on_set_cursor_pos_event.attach(&Editor::mouse_callback, editor);

    return editor;
}

void Editor::draw()
{
    if (!m_rendering_to_editor)
        return;

    auto const windows_copy = m_editor_windows;
    for (auto& window : windows_copy)
    {
        switch (window->type)
        {
        case EditorWindowType::Debug:
            draw_debug_window(window);
            break;
        case EditorWindowType::Content:
            draw_content_browser(window);
            break;
        case EditorWindowType::Hierarchy:
            draw_scene_hierarchy(window);
            break;
        case EditorWindowType::Game:
            draw_game(window);
            break;
        case EditorWindowType::Inspector:
            draw_inspector(window);
            break;
        case EditorWindowType::History:
            draw_history(window);
            break;
        case EditorWindowType::Custom:
            std::cout << "Custom Editor windows are currently not supported.\n";
            break;
        }
    }
}

void Editor::set_scene(std::shared_ptr<Scene> const& scene)
{
    m_open_scene = scene;
}

void Editor::draw_debug_window(std::shared_ptr<EditorWindow> const& window)
{
    m_current_time = glfwGetTime();
    m_frame_count += 1;

    if (m_current_time - m_last_second >= 1.0)
    {
        m_average_ms_per_frame = 1000.0 / static_cast<double>(m_frame_count);
        m_frame_count = 0;
        m_last_second = glfwGetTime();
    }

    bool is_still_open = true;
    bool const open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);

    if (!is_still_open)
    {
        remove_window(window);
        ImGui::End();
        return;
    }

    if (!open)
    {
        ImGui::End();
        return;
    }

    draw_window_menu_bar(window);

    ImGui::Checkbox("Polygon mode", &m_polygon_mode_active);
    ImGui::SameLine();
    ImGui::Checkbox("Show newest logs", &m_always_newest_logs);
    ImGui::Text("Application average %.3f ms/frame", m_average_ms_per_frame);
    draw_scene_save();

    std::string const log_count = "Logs " + std::to_string(Debug::debug_messages.size());
    ImGui::Text(log_count.c_str());
    if (ImGui::Button("Clear log"))
    {
        Debug::clear();
    }
    if (ImGui::BeginListBox("Logs", ImVec2(-FLT_MIN, 0.0f)))
    {
        ImGuiListClipper clipper;
        clipper.Begin(Debug::debug_messages.size());
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
            {
                switch (Debug::debug_messages[i].type)
                {
                case DebugType::Log:
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                    break;
                case DebugType::Warning:
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 250, 0, 255));
                    break;
                case DebugType::Error:
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 55, 0, 255));
                    break;
                default:
                    std::unreachable();
                }

                ImGui::Text(Debug::debug_messages[i].text.c_str());
                ImGui::PopStyleColor();
            }
        }

        if (m_always_newest_logs)
        {
            ImGui::SetScrollHereY(1.0f); // Scroll to bottom so the latest logs are always shown
        }

        ImGui::EndListBox();
    }

    ImGui::End();

    Renderer::get_instance()->wireframe_mode_active = m_polygon_mode_active;
}

void Editor::draw_content_browser(std::shared_ptr<EditorWindow> const& window)
{
    bool is_still_open = true;
    bool const open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);

    if (!is_still_open)
    {
        remove_window(window);
        ImGui::End();
        return;
    }

    if (!open)
    {
        ImGui::End();
        return;
    }

    draw_window_menu_bar(window);

    draw_scene_save();

    ImGui::Text("Search bar");

    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::PushItemWidth(-FLT_MIN);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 5));
    ImGui::InputText("##filter", &m_content_search_filter);
    ImGui::PopStyleVar();
    ImGui::PopItemWidth();

    std::ranges::transform(m_content_search_filter, m_content_search_filter.begin(), [](u8 const c) { return std::tolower(c); });

    if (ImGui::BeginTabBar("AssetTabs", ImGuiTabBarFlags_None))
    {
        if (ImGui::BeginTabItem("Models"))
        {
            for (auto const& asset : m_assets)
            {
                std::string ui_name_lower(asset.path.c_str());
                std::ranges::transform(ui_name_lower, ui_name_lower.begin(), [](u8 const c) { return std::tolower(c); });
                if (m_content_search_filter.empty() || ui_name_lower.find(m_content_search_filter) != std::string::npos)
                {
                    if (asset.type == AssetType::Model
                        && ImGui::Selectable(
                            asset.path.substr(m_models_path.length(), asset.path.length() - m_models_path.length()).c_str()))
                    {
                        ImGui::SetClipboardText(asset.path.c_str());
                    }
                }
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Textures"))
        {
            int column_count =
                static_cast<int>(ImGui::GetContentRegionAvail().x / (m_thumbnail_size + ImGui::GetStyle().ItemSpacing.x)) - 1;

            column_count = std::max(column_count, 1);

            if (ImGui::BeginTable("TextureTable", column_count, ImGuiTableFlags_BordersInner | ImGuiTableFlags_SizingFixedSame))
            {
                u32 id = 0;
                for (auto const& asset : m_assets)
                {
                    std::string ui_name_lower(asset.path.c_str());
                    std::ranges::transform(ui_name_lower, ui_name_lower.begin(), [](u8 const c) { return std::tolower(c); });

                    if (m_content_search_filter.empty() || ui_name_lower.find(m_content_search_filter) != std::string::npos)
                    {
                        if (asset.type == AssetType::Texture)
                        {
                            ImGui::TableNextColumn();

                            std::shared_ptr<Texture> texture = m_textures_map.find(asset.path)->second;

                            float tex_w = static_cast<float>(texture->width);
                            float tex_h = static_cast<float>(texture->height);
                            float aspect = tex_w / tex_h;

                            ImVec2 image_size;
                            if (aspect > 1.0f)
                            {
                                image_size = ImVec2(m_thumbnail_size, m_thumbnail_size / aspect);
                            }
                            else
                            {
                                image_size = ImVec2(m_thumbnail_size * aspect, m_thumbnail_size);
                            }

                            ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
                            ImGui::InvisibleButton(("##imgbtn" + std::to_string(id)).c_str(), ImVec2(m_thumbnail_size, m_thumbnail_size));

                            bool hovered = ImGui::IsItemHovered();
                            bool clicked = ImGui::IsItemActive();

                            ImVec2 img_pos = ImVec2(cursor_pos.x + (m_thumbnail_size - image_size.x) * 0.5f,
                                                    cursor_pos.y + (m_thumbnail_size - image_size.y) * 0.5f);

                            ImDrawList* draw_list = ImGui::GetWindowDrawList();
                            draw_list->AddImage((ImTextureID)(intptr_t)texture->shader_resource_view, img_pos,
                                                ImVec2(img_pos.x + image_size.x, img_pos.y + image_size.y));

                            ImU32 overlay_color = IM_COL32(0, 0, 0, 0);

                            if (hovered)
                            {
                                overlay_color = IM_COL32(0, 0, 0, 255 * 0.75);
                            }

                            if (clicked)
                            {
                                overlay_color = IM_COL32(255, 255, 255, 255 * 0.75);
                            }

                            draw_list->AddRectFilled(cursor_pos, ImVec2(cursor_pos.x + m_thumbnail_size, cursor_pos.y + m_thumbnail_size),
                                                     overlay_color);

                            if (hovered && ImGui::IsMouseClicked(0))
                            {
                                ImGui::SetClipboardText(asset.path.c_str());
                            }
                        }
                    }

                    id++;
                }

                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Sounds"))
        {
            for (auto const& asset : m_assets)
            {
                std::string ui_name_lower(asset.path.c_str());
                std::ranges::transform(ui_name_lower, ui_name_lower.begin(), [](u8 const c) { return std::tolower(c); });
                if (m_content_search_filter.empty() || ui_name_lower.find(m_content_search_filter) != std::string::npos)
                {
                    if (asset.type == AssetType::Audio
                        && ImGui::Selectable(asset.path.substr(m_audio_path.length(), asset.path.length() - m_audio_path.length()).c_str()))
                    {
                        ImGui::SetClipboardText(asset.path.c_str());
                    }
                }
            }
            ImGui::EndTabItem();
        }

        bool const ctrl_pressed = ImGui::GetIO().KeyCtrl;

        if (ImGui::BeginTabItem("Scenes"))
        {
            for (auto const& asset : m_assets)
            {
                std::string ui_name_lower(asset.path.c_str());
                std::ranges::transform(ui_name_lower, ui_name_lower.begin(), [](u8 const c) { return std::tolower(c); });
                if (m_content_search_filter.empty() || ui_name_lower.find(m_content_search_filter) != std::string::npos)
                {
                    if (asset.type == AssetType::Scene
                        && ImGui::Selectable(asset.path.substr(m_scene_path.length(), asset.path.length() - m_scene_path.length()).c_str()))
                    {
                        std::filesystem::path file_path(asset.path);
                        std::string const filename = file_path.stem().string();

                        if (!m_append_scene && !ctrl_pressed)
                        {
                            MainScene::get_instance()->unload();
                            load_asset(filename, AssetType::Scene, AssetLoadType::Open);
                        }
                        else
                        {
                            load_asset(filename, AssetType::Scene, AssetLoadType::Load);
                        }
                    }
                }
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Prefabs"))
        {
            for (auto const& asset : m_assets)
            {
                std::string ui_name_lower(asset.path.c_str());
                std::ranges::transform(ui_name_lower, ui_name_lower.begin(), [](u8 const c) { return std::tolower(c); });
                if (m_content_search_filter.empty() || ui_name_lower.find(m_content_search_filter) != std::string::npos)
                {
                    if (asset.type == AssetType::Prefab
                        && ImGui::Selectable(
                            asset.path.substr(m_prefab_path.length(), asset.path.length() - m_prefab_path.length()).c_str()))
                    {
                        std::filesystem::path file_path(asset.path);
                        std::string const filename = file_path.stem().string();

                        if (!m_append_scene && !ctrl_pressed)
                        {
                            // We are entering prefab mode.

                            // TODO: Instead of unloading the current scene we might want to create a new editor tab.

                            MainScene::get_instance()->unload();

                            m_opened_asset_name = filename;
                            m_opened_asset_path = asset.path;

                            load_asset(filename, AssetType::Prefab, AssetLoadType::Open);
                        }
                        else
                        {
                            // Appending to the current scene.

                            load_asset(filename, AssetType::Prefab, AssetLoadType::Load);

                            m_is_scene_dirty = true;
                        }
                    }
                }
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();

        ImGui::End();
    }
}

void Editor::draw_game(std::shared_ptr<EditorWindow> const& window)
{
    bool is_still_open = true;
    bool open = false;
    if (Engine::is_game_running())
    {
        window->set_name("Game");
        open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);
    }
    else
    {
        window->set_name("Scene");
        open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);
    }

    if (!is_still_open)
    {
        remove_window(window);
        ImGui::End();
        return;
    }

    if (!open)
    {
        ImGui::End();
        return;
    }

    draw_window_menu_bar(window);

    if (ImGui::BeginMenuBar())
    {
        bool const is_game_running = Engine::is_game_running();
        if (is_game_running)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(120, 120, 0, 150));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 50));

            if (m_is_camera_options_locked)
            {
                ImGui::OpenPopup("Camera");
            }

            if (ImGui::BeginMenu("Camera"))
            {
                float fov_angle = glm::degrees(Camera::get_main_camera()->fov);
                ImGui::SliderFloat("Camera FoV", &fov_angle, 1.0f, 90.0f);
                Camera::get_main_camera()->fov = glm::radians(fov_angle);

                glm::vec3 position = Camera::get_main_camera()->entity->transform->get_local_position();
                ImGuiEx::InputFloat3("Position", glm::value_ptr(position));
                Camera::get_main_camera()->entity->transform->set_local_position(position);

                glm::vec3 rotation = Camera::get_main_camera()->entity->transform->get_euler_angles();
                ImGuiEx::InputFloat3("Rotation", glm::value_ptr(rotation));
                Camera::get_main_camera()->entity->transform->set_euler_angles(rotation);

                if (ImGui::Button("Reset camera"))
                {
                    reset_camera();
                }

                ImGui::SameLine();
                ImGui::Checkbox("Lock", &m_is_camera_options_locked);

                ImGui::EndMenu();
            }

            ImVec4 constexpr active_button = {0.2f, 0.5f, 0.4f, 1.0f};
            ImVec4 constexpr inactive_button = {0.05f, 0.05f, 0.05f, 0.54f};

            if (m_gizmo_snapping)
                ImGui::PushStyleColor(ImGuiCol_Button, active_button);
            else
                ImGui::PushStyleColor(ImGuiCol_Button, inactive_button);

            if (ImGui::Button("Gizmo snapping"))
            {
                switch_gizmo_snapping();
            }

            if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            {
                ImGui::SetTooltip("Shortcut: \\");
            }

            ImGui::Checkbox("Crop viewport", &m_viewport_crop);

            ImGui::PopStyleColor();
        }

        if (ImGui::Button("Play", ImVec2(50.0f, 20.0f)))
        {
            if (is_game_running)
            {
                m_open_scene = nullptr;
            }

            Engine::set_game_running(!is_game_running);

            if (is_game_running)
            {
                m_open_scene = MainScene::get_instance();
                Camera::set_main_camera(m_editor_camera);
            }
            else
            {
                Renderer::get_instance()->choose_main_camera(m_editor_camera);
            }
        }

        ImGui::PopStyleColor();

        if (Engine::is_game_paused())
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(120, 120, 0, 150));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 50));
        }

        if (is_game_running)
        {
            if (ImGui::Button("Pause", ImVec2(50.0f, 20.0f)))
            {
                Engine::set_game_paused(!Engine::is_game_paused());

                if (Engine::is_game_paused())
                {
                    Camera::set_main_camera(m_editor_camera);
                }
                else
                {
                    Renderer::get_instance()->choose_main_camera(m_editor_camera);
                }
            }
        }

        ImGui::PopStyleColor();

        if (ImGui::Checkbox("Debug drawings", &m_debug_drawings_enabled))
        {
            for (auto const& debug_drawing : m_debug_drawings)
            {
                debug_drawing->set_drawing_enabled(m_debug_drawings_enabled);
            }
        }

        ImGui::EndMenuBar();
    }

    auto window_size = ImGui::GetContentRegionAvail();
    float aspect = window_size.x / window_size.y;

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)
        && !ImGui::IsAnyItemHovered())
    {
        ImGui::OpenPopup("ViewportPopup");
    }

    if (m_viewport_crop)
    {
        if (aspect > 16.0f / 9.0f)
        {
            m_game_size = {window_size.x, window_size.x * 9.0f / 16.0f};
        }
        else
        {
            m_game_size = {window_size.y * 16.0f / 9.0f, window_size.y};
        }
    }
    else
    {
        if (aspect > 16.0f / 9.0f)
        {
            m_game_size = {window_size.y * 16.0f / 9.0f, window_size.y};
        }
        else
        {
            m_game_size = {window_size.x, window_size.x * 9.0f / 16.0f};
        }
    }

    auto window_position = ImGui::GetCursorPos();
    m_game_position.x = window_position.x + (window_size.x - m_game_size.x) / 2.0f;
    m_game_position.y = window_position.y + (window_size.y - m_game_size.y) / 2.0f;

    if (Renderer::renderer_api == Renderer::RendererApi::DirectX11)
    {
        ImGui::SetCursorPos(ImVec2(m_game_position.x, m_game_position.y));
        ImGui::Image(RendererDX11::get_instance_dx11()->get_render_texture_view(), ImVec2(m_game_size.x, m_game_size.y));
    }

    if (m_selected_entity.expired())
    {
        ImGui::End();
        return;
    }

    auto const camera = Camera::get_main_camera();
    auto const entity = m_selected_entity.lock();

    ImGuizmo::SetDrawlist();

    ImGuizmo::SetRect(m_game_position.x, m_game_position.y, m_game_size.x, m_game_size.y);

    bool was_transform_changed = false;
    glm::mat4 global_model = entity->transform->get_model_matrix();
    switch (m_operation_type)
    {
    case GuizmoOperationType::Translate:
        was_transform_changed = ImGuizmo::Manipulate(glm::value_ptr(camera->get_view_matrix()), glm::value_ptr(camera->get_projection()),
                                                     ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::WORLD, glm::value_ptr(global_model),
                                                     nullptr, m_gizmo_snapping ? glm::value_ptr(m_position_snap) : nullptr);
        break;
    case GuizmoOperationType::Scale:
        was_transform_changed = ImGuizmo::Manipulate(glm::value_ptr(camera->get_view_matrix()), glm::value_ptr(camera->get_projection()),
                                                     ImGuizmo::OPERATION::SCALE, ImGuizmo::MODE::WORLD, glm::value_ptr(global_model),
                                                     nullptr, m_gizmo_snapping ? glm::value_ptr(m_scale_snap) : nullptr);
        break;
    case GuizmoOperationType::Rotate:
        was_transform_changed = ImGuizmo::Manipulate(glm::value_ptr(camera->get_view_matrix()), glm::value_ptr(camera->get_projection()),
                                                     ImGuizmo::OPERATION::ROTATE, ImGuizmo::MODE::WORLD, glm::value_ptr(global_model),
                                                     nullptr, m_gizmo_snapping ? glm::value_ptr(m_rotation_snap) : nullptr);
        break;
    case GuizmoOperationType::None:
    default:
        break;
    }

    if (was_transform_changed)
    {
        entity->transform->set_model_matrix(global_model);

        m_is_scene_dirty = true;
    }

    if (ImGuizmo::IsUsingAny() != m_is_guizmo_in_use)
    {
        if (!m_is_guizmo_in_use)
        {
            switch (m_operation_type)
            {
            case GuizmoOperationType::Translate:
                begin_edit_value(&entity->transform->get_local_position_ref());
                set_currently_edited_value_label("Position");
                set_value_pointer_of_currently_edited_value(&entity->transform->get_local_position_ref());
                set_value_before_of_currently_edited_value(entity->transform->get_local_position());
                break;
            case GuizmoOperationType::Scale:
                begin_edit_value(&entity->transform->get_local_scale_ref());
                set_currently_edited_value_label("Scale");
                set_value_pointer_of_currently_edited_value(&entity->transform->get_local_scale_ref());
                set_value_before_of_currently_edited_value(entity->transform->get_local_scale());
                break;
            case GuizmoOperationType::Rotate:
                begin_edit_value(&entity->transform->get_euler_angles_ref());
                set_currently_edited_value_label("Rotation");
                set_value_pointer_of_currently_edited_value(&entity->transform->get_euler_angles_ref());
                set_value_before_of_currently_edited_value(entity->transform->get_euler_angles());
                break;
            case GuizmoOperationType::None:
            default:
                break;
            }
        }
        else
        {
            switch (m_operation_type)
            {
            case GuizmoOperationType::Translate:
                set_value_after_of_currently_edited_value(entity->transform->get_local_position());
                break;
            case GuizmoOperationType::Scale:
                set_value_after_of_currently_edited_value(entity->transform->get_local_scale());
                break;
            case GuizmoOperationType::Rotate:
                set_value_after_of_currently_edited_value(entity->transform->get_euler_angles());
                break;
            case GuizmoOperationType::None:
            default:
                break;
            }

            add_action_to_history();
            set_currently_edited_value_saved(true);
        }

        m_is_guizmo_in_use = ImGuizmo::IsUsingAny();
    }

    ImGui::End();
}

void Editor::draw_scene_hierarchy(std::shared_ptr<EditorWindow> const& window)
{
    bool is_still_open = true;
    bool const open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);

    if (!is_still_open)
    {
        remove_window(window);
        ImGui::End();
        return;
    }

    if (!open)
    {
        ImGui::End();
        return;
    }

    draw_window_menu_bar(window);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::Button("Add Entity"))
        {
            Entity::create("Entity");

            m_is_scene_dirty = true;
        }

        ImGui::EndMenuBar();
    }

    // Draw every entity without a parent, and draw its children recursively
    auto const entities_copy = m_open_scene->entities;
    for (auto const& entity : entities_copy)
    {
        if (!entity->transform->parent.expired())
            continue;

        draw_entity_recursively(entity->transform);
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)
        && !ImGui::IsAnyItemHovered())
    {
        ImGui::OpenPopup("HierarchyPopup");
    }

    if (ImGui::BeginPopup("HierarchyPopup", ImGuiPopupFlags_MouseButtonRight))
    {
        std::filesystem::path const copied_entity_path = m_copied_entity_path;
        bool const copied_entity_exists = std::filesystem::exists(copied_entity_path);

        if (!copied_entity_exists)
        {
            ImGui::BeginDisabled(true);
        }

        if (ImGui::Button("Paste"))
        {
            paste_entity();
            ImGui::CloseCurrentPopup();
        }

        if (!copied_entity_exists)
        {
            ImGui::EndDisabled();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

void Editor::draw_history(std::shared_ptr<EditorWindow> const& window)
{
    bool is_still_open = true;
    bool const open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);

    if (!is_still_open)
    {
        remove_window(window);
        ImGui::End();
        return;
    }

    if (!open)
    {
        ImGui::End();
        return;
    }

    draw_window_menu_bar(window);

    if (m_editor_history.size() == m_history_head)
    {
        ImGui::Text("--- Head ---");
    }

    for (i32 i = 0; i < m_editor_history.size(); i++)
    {
        std::string history_entry = m_editor_history[i]->to_string();
        if (ImGui::Selectable(history_entry.c_str(), false))
        {
            if (m_editor_history.size() - m_history_head > i)
            {
                for (i32 j = m_editor_history.size() - m_history_head - 1; j >= i; j--)
                {
                    m_editor_history[j]->apply_before();
                }
            }

            if (m_editor_history.size() - m_history_head < i)
            {
                for (i32 j = m_editor_history.size() - m_history_head; j < i; j++)
                {
                    m_editor_history[j]->apply_after();
                }
            }

            m_history_head = m_editor_history.size() - i;
        }

        if (m_editor_history.size() - i - 1 == m_history_head)
        {
            ImGui::Text("--- Head ---");
        }
    }

    if (ImGui::Selectable("Newest", false))
    {
        for (i32 i = m_editor_history.size() - m_history_head; i < m_editor_history.size(); i++)
        {
            m_editor_history[i]->apply_after();
        }

        m_history_head = 0;
    }

    ImGui::End();
}

void Editor::draw_entity_recursively(std::shared_ptr<Transform> const& transform)
{
    if (transform == nullptr || transform->entity.expired())
        return;

    auto const entity = transform->entity.lock();
    ImGuiTreeNodeFlags const node_flags =
        (!m_selected_entity.expired() && m_selected_entity.lock()->hashed_guid == entity->hashed_guid ? ImGuiTreeNodeFlags_Selected : 0)
        | (transform->children.empty() ? ImGuiTreeNodeFlags_Leaf : 0) | ImGuiTreeNodeFlags_OpenOnDoubleClick
        | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    if (!ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(entity->hashed_guid)), node_flags, "%s", ""))
    {
        if (ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            m_selected_entity = entity;
            m_is_selected_entity_being_renamed = false;
        }

        if (!draw_entity_popup(entity))
        {
            return;
        }

        entity_drag(entity);

        ImGui::SameLine();
        if (m_is_selected_entity_being_renamed && entity == m_selected_entity.lock())
        {
            ImGui::SetKeyboardFocusHere();
            if (ImGui::InputText("##empty", &entity->name, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                m_is_selected_entity_being_renamed = false;
            }
        }
        else
        {
            ImGui::Text(entity->name.c_str());
        }

        return;
    }

    entity_drag(entity);

    if (ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        m_selected_entity = entity;
        m_is_selected_entity_being_renamed = false;
    }

    if (!draw_entity_popup(entity))
    {
        ImGui::TreePop();
        return;
    }

    ImGui::SameLine();
    if (m_is_selected_entity_being_renamed && entity == m_selected_entity.lock())
    {
        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##empty", &entity->name, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            m_is_selected_entity_being_renamed = false;
        }
    }
    else
    {
        ImGui::Text(entity->name.c_str());
    }

    for (auto const& child : transform->children)
    {
        draw_entity_recursively(child);
    }

    ImGui::TreePop();
}

// Should be called just after an Entity item
void Editor::entity_drag(std::shared_ptr<Entity> const& entity)
{
    ImGuiDragDropFlags src_flags = 0;
    src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;

    if (ImGui::BeginDragDropSource(src_flags))
    {
        ImGui::Text((entity->name).c_str());
        ImGui::SetDragDropPayload("guid", entity->guid.data(), sizeof(i64) * 8);
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        std::string guid;

        if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("guid"))
        {
            guid.resize(sizeof(i64) * 8);
            memcpy(guid.data(), payload->Data, sizeof(i64) * 8);

            if (auto const reparent_entity = MainScene::get_instance()->get_entity_by_guid(guid))
            {
                reparent_entity->transform->set_parent(entity->transform);
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginDragDropTargetCustom(ImGui::GetCurrentWindow()->ContentRegionRect, ImGui::GetID("CustomTarget")))
    {
        std::string guid;

        if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("guid"))
        {
            guid.resize(sizeof(i64) * 8);
            memcpy(guid.data(), payload->Data, sizeof(i64) * 8);

            if (auto const reparent_entity = MainScene::get_instance()->get_entity_by_guid(guid))
            {
                reparent_entity->transform->set_parent(nullptr);
            }
        }
        ImGui::EndDragDropTarget();
    }
}

bool Editor::draw_entity_popup(std::shared_ptr<Entity> const& entity)
{
    if (!m_selected_entity.expired() && ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
    {
        if (m_selected_entity.lock() != entity)
        {
            m_selected_entity = entity;
        }

        if (ImGui::Button("Rename"))
        {
            m_is_selected_entity_being_renamed = true;
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return true;
        }

        if (ImGui::Button("Delete"))
        {
            delete_selected_entity();
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return false;
        }

        if (ImGui::Button("Copy"))
        {
            copy_selected_entity();
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return true;
        }

        if (ImGui::Button("Duplicate"))
        {
            copy_selected_entity();
            paste_entity();
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return true;
        }

        if (ImGui::Button("Add child"))
        {
            add_child_entity();
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return true;
        }

        if (ImGui::Button("Save as prefab"))
        {
            save_entity_as_prefab();
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return true;
        }

        ImGui::EndPopup();
    }

    return true;
}

void Editor::draw_window_menu_bar(std::shared_ptr<EditorWindow> const& window)
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            if (ImGui::BeginMenu("Add window"))
            {
                if (ImGui::MenuItem("Inspector"))
                {
                    add_inspector();
                }
                if (ImGui::MenuItem("Game"))
                {
                    add_game();
                }
                if (ImGui::MenuItem("Content"))
                {
                    add_content_browser();
                }
                if (ImGui::MenuItem("Hierarchy"))
                {
                    add_scene_hierarchy();
                }
                if (ImGui::MenuItem("Debug"))
                {
                    add_debug_window();
                }
                if (ImGui::MenuItem("History"))
                {
                    add_history();
                }

                ImGui::EndMenu();
            }

            if (window->type == EditorWindowType::Inspector)
            {
                if (ImGui::Button("Lock"))
                {
                    window->set_is_locked(!window->is_locked(), LockData {m_selected_entity});
                }
            }

            ImGui::EndMenu();
        }

        if (window->is_locked())
        {
            ImGui::Text("LOCKED");
        }

        ImGui::EndMenuBar();
    }
}

void Editor::load_assets()
{
    m_assets.clear();

    for (auto const& entry : std::filesystem::recursive_directory_iterator(m_models_path))
    {
        if (std::ranges::find(m_known_model_formats, entry.path().extension().string()) != m_known_model_formats.end())
        {
            m_assets.emplace_back(entry.path().string(), AssetType::Model);
        }
    }

    for (auto const& entry : std::filesystem::recursive_directory_iterator(m_scene_path))
    {
        if (std::ranges::find(m_known_scene_formats, entry.path().extension().string()) != m_known_scene_formats.end())
        {
            m_assets.emplace_back(entry.path().string(), AssetType::Scene);
        }
    }

    if (!std::filesystem::exists(m_prefab_path))
    {
        std::filesystem::create_directory(m_prefab_path);
    }

    for (auto const& entry : std::filesystem::recursive_directory_iterator(m_prefab_path))
    {
        if (std::ranges::find(m_known_scene_formats, entry.path().extension().string()) != m_known_scene_formats.end())
        {
            m_assets.emplace_back(entry.path().string(), AssetType::Prefab);
        }
    }

    for (auto const& entry : std::filesystem::recursive_directory_iterator(m_textures_path))
    {
        if (std::ranges::find(m_known_textures_formats, entry.path().extension().string()) != m_known_textures_formats.end())
        {
            m_assets.emplace_back(entry.path().string(), AssetType::Texture);

            TextureSettings texture_settings = {};
            texture_settings.wrap_mode_x = TextureWrapMode::ClampToBorder;
            texture_settings.wrap_mode_y = TextureWrapMode::ClampToBorder;
            texture_settings.flip_vertically = false;

            std::shared_ptr<Texture> texture =
                ResourceManager::get_instance().load_texture(entry.path().string(), TextureType::Diffuse, texture_settings);

            m_textures_map.insert({entry.path().string(), texture});
        }
    }

    for (auto const& entry : std::filesystem::recursive_directory_iterator(m_audio_path))
    {
        if (std::ranges::find(m_known_audio_formats, entry.path().extension().string()) != m_known_audio_formats.end())
        {
            m_assets.emplace_back(entry.path().string(), AssetType::Audio);
        }
    }
}

void Editor::draw_inspector(std::shared_ptr<EditorWindow> const& window)
{
    bool is_still_open = true;
    bool const open = ImGui::Begin(window->get_name().c_str(), &is_still_open, window->flags);

    if (!is_still_open)
    {
        remove_window(window);
        ImGui::End();
        return;
    }

    if (!open)
    {
        ImGui::End();
        return;
    }

    draw_window_menu_bar(window);

    if (window->is_locked() && window->get_locked_entity().expired())
    {
        window->set_is_locked(false, {});
    }

    std::weak_ptr<Entity> current_entity = window->get_locked_entity();

    if (current_entity.expired())
    {
        current_entity = m_selected_entity;
    }

    if (current_entity.expired())
    {
        ImGui::End();
        return;
    }

    auto const camera = Camera::get_main_camera();
    auto const entity = current_entity.lock();

    // TODO: When transform will be a component it should record it, until now changes in transform will be in history as last existed component in given entity
    //m_selected_component = entity->transform;

    ImGui::Text("Transform");
    ImGui::Spacing();

    glm::vec3& position = entity->transform->get_local_position_ref();
    glm::vec3 position_before = position;
    vec3_draw_editor("Position", position);

    if (AK::are_not_equal(position_before, position))
    {
        m_is_scene_dirty = true;
        entity->transform->set_dirty();
    }

    float const input_width = ImGui::CalcItemWidth() / 3.0f - ImGui::GetStyle().ItemSpacing.x * 0.66f;
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    if (ImGui::Button("Copy##1"))
    {
        ImGui::SetClipboardText(glm::to_string(position_before).c_str());
    }

    glm::vec3& rotation = entity->transform->get_euler_angles_ref();
    glm::vec3 rotation_before = rotation;
    vec3_draw_editor("Rotation", rotation);

    if (AK::are_not_equal(rotation_before, rotation))
    {
        m_is_scene_dirty = true;
        entity->transform->set_rotation(rotation);
    }

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    if (ImGui::Button("Copy##2"))
    {
        ImGui::SetClipboardText(glm::to_string(rotation).c_str());
    }

    glm::vec3& scale = entity->transform->get_local_scale_ref();
    glm::vec3 scale_before = scale;

    bool is_changing_finished = false;

    ImGui::PushItemWidth(input_width);

    ImGui::BeginDisabled(m_lock_scale && m_disabled_scale.x);
    ImGuiEx::InputFloat("##x", &scale.x);
    is_changing_finished |= ImGui::IsItemDeactivatedAfterEdit();
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(m_lock_scale && m_disabled_scale.y);
    ImGuiEx::InputFloat("##y", &scale.y);
    is_changing_finished |= ImGui::IsItemDeactivatedAfterEdit();
    ImGui::EndDisabled();

    ImGui::SameLine();

    ImGui::BeginDisabled(m_lock_scale && m_disabled_scale.z);
    ImGuiEx::InputFloat("Scale##z", &scale.z);
    is_changing_finished |= ImGui::IsItemDeactivatedAfterEdit();
    ImGui::EndDisabled();

    ImGui::PopItemWidth();

    if (AK::are_not_equal(scale_before, scale))
    {
        m_is_scene_dirty = true;
        entity->transform->set_dirty();

        if (m_lock_scale)
        {
            scale = update_locked_value(scale, scale_before);
        }

        entity->transform->set_local_scale(scale);

        begin_edit_value(&scale);
        set_currently_edited_value_label("Scale");
        set_value_pointer_of_currently_edited_value(&entity->transform->get_local_scale_ref());
        set_value_before_of_currently_edited_value(scale_before);
    }

    if (is_changing_finished)
    {
        set_value_after_of_currently_edited_value(scale);
        add_action_to_history();
        set_currently_edited_value_saved(true);
    }

    ImGui::SameLine();
    ImGui::Text("    | ");
    ImGui::SameLine();

    if (ImGui::Button("Copy##3"))
    {
        ImGui::SetClipboardText(glm::to_string(scale).c_str());
    }

    ImGui::SameLine();

    if (ImGui::Checkbox("LOCK", &m_lock_scale))
    {
        if (!m_lock_scale)
        {
            m_disabled_scale = {false, false, false};
        }
        else
        {
            m_lock_ratio = entity->transform->get_local_scale();

            m_disabled_scale.x = glm::epsilonEqual(m_lock_ratio.x, 0.0f, 0.0001f);

            m_disabled_scale.y = glm::epsilonEqual(m_lock_ratio.y, 0.0f, 0.0001f);

            m_disabled_scale.z = glm::epsilonEqual(m_lock_ratio.z, 0.0f, 0.0001f);
        }
    }

    auto const components_copy = entity->components;
    for (auto const& component : components_copy)
    {
        m_selected_component = component;

        ImGui::Spacing();
        std::string guid = "##" + component->guid;
        
        std::string name = component->get_name();
        std::string full_name = name;

        if (auto custom_name = m_component_custom_names.find(component->guid); custom_name != m_component_custom_names.end())
        {
            full_name += " " + custom_name->second;
        }

        std::string guid_id = "###" + component->guid;
        bool const component_open = ImGui::TreeNode((full_name + guid_id + "Component").c_str());

        std::string popup = "ComponentPopup" + (guid_id + "ComponentPopup");
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            ImGui::OpenPopup(popup.c_str());
        }

        if (ImGui::BeginPopup(popup.c_str(), ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::Button("Rename"))
            {
                ImGui::OpenPopup("RenameComponentPopup");
            }

            bool should_close_outer_popup = false;

            if (ImGui::BeginPopup("RenameComponentPopup"))
            {
                if (ImGui::InputText("##empty", get_component_custom_name_by_ptr(component->guid), ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    should_close_outer_popup = true;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            if (should_close_outer_popup)
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGuiDragDropFlags src_flags = 0;
        src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;

        if (ImGui::BeginDragDropSource(src_flags))
        {
            ImGui::Text((entity->name + " : " + name).c_str());
            ImGui::SetDragDropPayload("guid", component->guid.data(), sizeof(i64) * 8);
            ImGui::EndDragDropSource();
        }

        ImGui::Spacing();

        if (component_open)
        {
            bool enabled = component->enabled();
            ImGui::Checkbox("Enabled", &enabled);
            component->set_enabled(enabled);

            component->draw_editor();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TreePop();
        }
    }

    if (ImGui::BeginListBox("##empty", ImVec2(-FLT_MIN, -FLT_MIN)))
    {
        ImGui::Text("Search bar");

        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::PushItemWidth(-FLT_MIN);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 5));
        ImGui::InputText("##filter", &m_search_filter);
        ImGui::PopStyleVar();
        ImGui::PopItemWidth();

        std::ranges::transform(m_search_filter, m_search_filter.begin(), [](u8 const c) { return std::tolower(c); });

#define CONCAT_CLASS(name) class name
#define ENUMERATE_COMPONENT(name, ui_name)                                                                        \
    {                                                                                                             \
        std::string ui_name_lower(ui_name);                                                                       \
        std::ranges::transform(ui_name_lower, ui_name_lower.begin(), [](u8 const c) { return std::tolower(c); }); \
        if (m_search_filter.empty() || ui_name_lower.find(m_search_filter) != std::string::npos)                  \
        {                                                                                                         \
            if (ImGui::Button(ui_name, ImVec2(-FLT_MIN, 20)))                                                     \
                entity->add_component<CONCAT_CLASS(name)>(##name::create());                                      \
        }                                                                                                         \
    }
        ENUMERATE_COMPONENTS
#undef ENUMERATE_COMPONENT

        ImGui::EndListBox();
    }

    ImGui::End();
}

void Editor::draw_scene_save()
{
    bool open_save_scene_popup = false;

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save asset"))
            {
                if (Engine::is_game_running())
                {
                    Debug::log("Game is currently running. Asset has not been saved.", DebugType::Error);
                }
                else
                {
                    save_asset();
                }
            }

            if (ImGui::MenuItem("Save scene as"))
            {
                if (Engine::is_game_running())
                {
                    Debug::log("Game is currently running. Scene has not been saved.", DebugType::Error);
                }
                else
                {
                    open_save_scene_popup = true;
                }
            }

            if (ImGui::MenuItem("Load default scene"))
            {
                MainScene::get_instance()->unload();

                load_scene();
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }

    if (open_save_scene_popup)
    {
        ImGui::OpenPopup("SceneNamePopup");
    }

    if (ImGui::BeginPopup("SceneNamePopup"))
    {
        static std::string scene_name = "scene";

        ImGui::Text("Save scene as ...");

        if (ImGui::InputText("##empty", &scene_name, ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::string const new_opened_scene_path = get_scene_path(scene_name);
            m_opened_asset_path = new_opened_scene_path;
            m_opened_asset_name = scene_name;
            m_opened_asset_type = AssetType::Scene;
            save_asset();

            ImGui::CloseCurrentPopup();

            load_assets();

            scene_name = "scene";
        }

        if (ImGui::Button("Save"))
        {
            std::string const new_opened_scene_path = get_scene_path(scene_name);

            m_opened_asset_path = new_opened_scene_path;
            m_opened_asset_name = scene_name;
            m_opened_asset_type = AssetType::Scene;
            save_asset();

            ImGui::CloseCurrentPopup();

            load_assets();

            scene_name = "scene";
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void Editor::save_asset()
{
    [[unlikely]]
    if (m_opened_asset_path.empty())
    {
        Debug::log("Saving asset failed. Opened asset path is empty.", DebugType::Error);
        return;
    }

    switch (m_opened_asset_type)
    {
    case AssetType::Scene:
    {
        save_scene_as(m_opened_asset_path);
        break;
    }
    case AssetType::Prefab:
    {
    }
    case AssetType::Audio:
    case AssetType::Model:
    case AssetType::Texture:
    case AssetType::Unknown:
    {
        Debug::log("Cannot save this asset type.", DebugType::Error);
        break;
    }
    }

    m_is_scene_dirty = false;
    update_window_title();

    Debug::log("Asset " + m_opened_asset_path + " saved.", DebugType::Log);
}

void Editor::save_scene_as(std::string const& path)
{
    auto const scene_serializer = std::make_shared<SceneSerializer>(m_open_scene);
    SceneSerializer::set_instance(scene_serializer);
    ScopeGuard unset_instance = [&] { SceneSerializer::set_instance(nullptr); };
    scene_serializer->serialize(path);
}

glm::vec2 Editor::get_game_size() const
{
    // TODO: ImGui is REMOVED from game build so it's important to get window size from glfw or use fullscreen size.
    return m_game_size;
}

glm::vec2 Editor::get_game_position() const
{
    return m_game_position;
}

bool Editor::is_rendering_to_editor() const
{
    return m_rendering_to_editor;
}

void Editor::register_debug_drawing(std::shared_ptr<DebugDrawing> const& debug_drawing)
{
    m_debug_drawings.emplace_back(debug_drawing);
}

void Editor::unregister_debug_drawing(std::shared_ptr<DebugDrawing> const& debug_drawing)
{
    AK::swap_and_erase(m_debug_drawings, debug_drawing);
}

bool Editor::load_scene()
{
    return load_asset("scene", AssetType::Scene, AssetLoadType::Open);
}

bool Editor::load_scene_name(std::string const& name)
{
    auto const scene_serializer = std::make_shared<SceneSerializer>(m_open_scene);
    SceneSerializer::set_instance(scene_serializer);
    ScopeGuard unset_instance = [&] { SceneSerializer::set_instance(nullptr); };
    std::string const scene_to_load = get_scene_path(name);
    bool const deserialized = scene_serializer->deserialize(scene_to_load);

    if (deserialized)
    {
        m_opened_asset_path = scene_to_load;
        m_opened_asset_name = name;
    }

    return deserialized;
}

void Editor::set_style() const
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
    colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
    colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(8.00f, 8.00f);
    style.FramePadding = ImVec2(5.00f, 2.00f);
    style.CellPadding = ImVec2(6.00f, 6.00f);
    style.ItemSpacing = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    style.IndentSpacing = 25;
    style.ScrollbarSize = 15;
    style.GrabMinSize = 10;
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 1;
    style.TabBorderSize = 1;
    style.WindowRounding = 7;
    style.ChildRounding = 4;
    style.FrameRounding = 3;
    style.PopupRounding = 4;
    style.ScrollbarRounding = 9;
    style.GrabRounding = 3;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 4;
}

void Editor::camera_input() const
{
    float const current_speed = m_camera_speed * delta_time;

    if (Input::input->get_key(GLFW_KEY_W))
        m_camera_entity->transform->set_local_position(m_camera_entity->transform->get_local_position() +=
                                                       current_speed * m_editor_camera->get_front());

    if (Input::input->get_key(GLFW_KEY_S))
        m_camera_entity->transform->set_local_position(m_camera_entity->transform->get_local_position() -=
                                                       current_speed * m_editor_camera->get_front());

    if (Input::input->get_key(GLFW_KEY_A))
        m_camera_entity->transform->set_local_position(m_camera_entity->transform->get_local_position() -=
                                                       glm::normalize(glm::cross(m_editor_camera->get_front(), m_editor_camera->get_up()))
                                                       * current_speed);

    if (Input::input->get_key(GLFW_KEY_D))
        m_camera_entity->transform->set_local_position(m_camera_entity->transform->get_local_position() +=
                                                       glm::normalize(glm::cross(m_editor_camera->get_front(), m_editor_camera->get_up()))
                                                       * current_speed);

    if (Input::input->get_key(GLFW_KEY_E))
        m_camera_entity->transform->set_local_position(m_camera_entity->transform->get_local_position() +=
                                                       current_speed * glm::vec3(0.0f, 1.0f, 0.0f));

    if (Input::input->get_key(GLFW_KEY_Q))
        m_camera_entity->transform->set_local_position(m_camera_entity->transform->get_local_position() -=
                                                       current_speed * glm::vec3(0.0f, 1.0f, 0.0f));
}

void Editor::non_camera_input()
{
    if (Input::input->get_key_down(GLFW_KEY_W))
    {
        m_operation_type = GuizmoOperationType::Translate;
    }

    if (Input::input->get_key_down(GLFW_KEY_R))
    {
        m_operation_type = GuizmoOperationType::Scale;
    }

    if (Input::input->get_key_down(GLFW_KEY_E))
    {
        m_operation_type = GuizmoOperationType::Rotate;
    }

    if (Input::input->get_key_down(GLFW_KEY_G))
    {
        m_operation_type = GuizmoOperationType::None;
    }

    if (Input::input->get_key_down(GLFW_KEY_BACKSLASH))
    {
        switch_gizmo_snapping();
    }
}

void Editor::reset_camera()
{
    m_camera_entity->transform->set_local_position(m_camera_default_position);
    m_camera_entity->transform->set_euler_angles(m_camera_default_rotation);
    m_editor_camera->set_fov(glm::radians(m_camera_default_fov));
}

void Editor::switch_gizmo_snapping()
{
    m_gizmo_snapping = !m_gizmo_snapping;

    if (m_selected_entity.expired())
        return;

    auto const entity = m_selected_entity.lock();

    // FIXME: We should probably run this when selected entity changes as well.
    switch (m_operation_type)
    {
    case GuizmoOperationType::Translate:
        entity->transform->set_local_position(glm::round(entity->transform->get_local_position() * 100.0f) / 100.0f);
        break;
    case GuizmoOperationType::Scale:
        entity->transform->set_local_scale(glm::round(entity->transform->get_local_position() * 100.0f) / 100.0f);
        break;
    case GuizmoOperationType::Rotate:
        entity->transform->set_euler_angles(glm::round(entity->transform->get_euler_angles()));
        break;
    case GuizmoOperationType::None:
    default:
        break;
    }
}

void Editor::delete_selected_entity()
{
    if (!m_selected_entity.expired())
    {
        m_selected_entity.lock()->destroy_immediate();

        m_is_scene_dirty = true;
    }
}

void Editor::copy_selected_entity() const
{
    if (m_selected_entity.expired())
        return;

    auto const scene_serializer = std::make_shared<SceneSerializer>(m_open_scene);
    SceneSerializer::set_instance(scene_serializer);
    ScopeGuard unset_instance = [&] { SceneSerializer::set_instance(nullptr); };
    scene_serializer->serialize_this_entity(m_selected_entity.lock(), m_copied_entity_path);
}

void Editor::paste_entity()
{
    auto const scene_serializer = std::make_shared<SceneSerializer>(m_open_scene);
    SceneSerializer::set_instance(scene_serializer);
    ScopeGuard unset_instance = [&] { SceneSerializer::set_instance(nullptr); };
    scene_serializer->deserialize_this_entity("./.editor/copied_entity.txt");

    m_is_scene_dirty = true;
}

void Editor::add_child_entity()
{
    if (m_selected_entity.expired())
        return;

    auto const entity = m_selected_entity.lock();
    auto const child_entity = Entity::create("Child");
    child_entity->transform->set_parent(entity->transform);

    m_is_scene_dirty = true;
}

void Editor::save_entity_as_prefab()
{
    if (m_selected_entity.expired())
        return;

    auto const scene_serializer = std::make_shared<SceneSerializer>(m_open_scene);
    SceneSerializer::set_instance(scene_serializer);
    ScopeGuard unset_instance = [&] { SceneSerializer::set_instance(nullptr); };
    scene_serializer->serialize_this_entity(m_selected_entity.lock(), m_prefab_path + m_selected_entity.lock()->name + ".txt");

    load_assets();
}

bool Editor::load_prefab(std::string const& name)
{
    auto const scene_serializer = std::make_shared<SceneSerializer>(m_open_scene);
    SceneSerializer::set_instance(scene_serializer);
    ScopeGuard unset_instance = [&] { SceneSerializer::set_instance(nullptr); };
    std::shared_ptr<Entity> const entity = scene_serializer->deserialize_this_entity(m_prefab_path + name + ".txt");

    return entity != nullptr;
}

bool Editor::load_asset(std::string const& name, AssetType const type, AssetLoadType const load_type)
{
    switch (type)
    {
    case AssetType::Scene:
    {
        bool const result = load_scene_name(name);

        if (!result)
        {
            Debug::log("Could not load a scene.", DebugType::Error);
            return false;
        }

        break;
    }
    case AssetType::Prefab:
    {
        bool const result = load_prefab(name);

        if (!result)
        {
            Debug::log("Could not load a prefab.", DebugType::Error);
            return false;
        }

        break;
    }
    case AssetType::Audio:
    case AssetType::Model:
    case AssetType::Texture:
    case AssetType::Unknown:
    {
        Debug::log("Opening asset type " + std::to_string(static_cast<i32>(type)) + " is not supported,", DebugType::Error);
        return false;
    }
    }

    if (load_type == AssetLoadType::Open)
    {
        m_opened_asset_type = type;
        update_window_title();
    }

    return true;
}

std::string Editor::get_scene_path(std::string const& scene_name) const
{
    return m_scene_path + scene_name + m_scene_extension;
}

void Editor::update_window_title() const
{
    std::string title = Engine::window_title + " : " + asset_type_to_string(m_opened_asset_type) + " " + m_opened_asset_name;

    if (m_is_scene_dirty)
    {
        title += "*";
    }

    glfwSetWindowTitle(Engine::window->get_glfw_window(), title.c_str());
}

void Editor::mouse_callback(double const x, double const y)
{
    if (Engine::is_game_running() && !Engine::is_game_paused())
    {
        return;
    }

    if (m_mouse_just_entered)
    {
        m_last_mouse_position.x = x;
        m_last_mouse_position.y = y;
        m_mouse_just_entered = false;
    }

    double x_offset = x - m_last_mouse_position.x;
    double y_offset = m_last_mouse_position.y - y;
    m_last_mouse_position.x = x;
    m_last_mouse_position.y = y;

    if (!Input::input->get_key(GLFW_MOUSE_BUTTON_RIGHT))
        return;

    x_offset *= m_sensitivity;
    y_offset *= m_sensitivity;

    m_yaw += x_offset;
    m_pitch = glm::clamp(m_pitch + y_offset, -89.0, 89.0);

    m_camera_entity->transform->set_euler_angles(glm::vec3(m_pitch, -m_yaw, 0.0f));
}

glm::vec3 Editor::update_locked_value(glm::vec3 new_value, glm::vec3 const old_value) const
{
    if (glm::epsilonNotEqual(new_value.x, old_value.x, 0.0001f))
    {
        if (m_disabled_scale.x)
        {
            return old_value;
        }

        float const y = (m_lock_ratio.y / m_lock_ratio.x) * new_value.x;
        float const z = (m_lock_ratio.z / m_lock_ratio.x) * new_value.x;

        return {new_value.x, y, z};
    }

    if (glm::epsilonNotEqual(new_value.y, old_value.y, 0.0001f))
    {
        if (m_disabled_scale.y)
        {
            return old_value;
        }

        float const x = (m_lock_ratio.x / m_lock_ratio.y) * new_value.y;
        float const z = (m_lock_ratio.z / m_lock_ratio.y) * new_value.y;

        return {x, new_value.y, z};
    }

    if (glm::epsilonNotEqual(new_value.z, old_value.z, 0.0001f))
    {
        if (m_disabled_scale.z)
        {
            return old_value;
        }

        float const x = (m_lock_ratio.x / m_lock_ratio.z) * new_value.z;
        float const y = (m_lock_ratio.y / m_lock_ratio.z) * new_value.z;

        return {x, y, new_value.z};
    }

    return new_value;
}

void Editor::handle_input()
{
    auto const input = Input::input;

    if (input->get_key_down(GLFW_KEY_F1))
    {
        switch_rendering_to_editor();
    }

    if (input->get_key_down(GLFW_KEY_F2))
    {
        if (!ImGui::IsAnyItemActive() && !m_selected_entity.expired())
        {
            m_is_selected_entity_being_renamed = true;
        }
    }

    if (ImGui::GetIO().KeyCtrl && input->get_key_down(GLFW_KEY_D))
    {
        if (!ImGui::IsAnyItemActive())
        {
            copy_selected_entity();
            paste_entity();
        }
    }

    if (ImGui::GetIO().KeyCtrl)
    {
        if (!ImGui::GetIO().KeyShift && input->get_key_down(GLFW_KEY_Z))
        {
            undo();
        }

        if (input->get_key_down(GLFW_KEY_Y))
        {
            redo();
        }

        if (ImGui::GetIO().KeyShift && input->get_key_down(GLFW_KEY_Z))
        {
            redo();
        }
    }

    if (input->get_key_down(GLFW_KEY_F5))
    {
        Renderer::get_instance()->reload_shaders();
    }

    if (input->get_key_down(GLFW_KEY_DELETE))
    {
        if (!ImGui::IsAnyItemActive())
        {
            delete_selected_entity();
        }
    }

    if (!input->get_key(GLFW_MOUSE_BUTTON_RIGHT))
    {
        non_camera_input();
    }
    else
    {
        if (!Engine::is_game_running() || Engine::is_game_paused())
        {
            camera_input();
        }
    }

    if (ImGui::GetIO().KeyCtrl && input->get_key_down(GLFW_KEY_S))
    {
        save_asset();
    }
}

void Editor::run()
{
    bool const was_scene_dirty = m_is_scene_dirty;

    handle_input();
    draw();

    if (!was_scene_dirty && m_is_scene_dirty)
    {
        update_window_title();
    }
}

void Editor::set_docking_space() const
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void Editor::switch_rendering_to_editor()
{
    Renderer::get_instance()->switch_rendering_to_texture();
    m_rendering_to_editor = !m_rendering_to_editor;
    if (m_rendering_to_editor)
    {
        glfwSetInputMode(Engine::window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        GLFWvidmode const* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(Engine::window->get_glfw_window(), nullptr, 0, 0, Renderer::screen_width, Renderer::screen_height,
                             mode->refreshRate);
    }
    else
    {
        glfwSetInputMode(Engine::window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        GLFWvidmode const* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwSetWindowMonitor(Engine::window->get_glfw_window(), glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height,
                             mode->refreshRate);
    }
}

void Editor::add_debug_window()
{
    auto debug_window = std::make_shared<EditorWindow>(m_last_window_id, ImGuiWindowFlags_MenuBar, EditorWindowType::Debug);
    m_editor_windows.emplace_back(debug_window);
}

void Editor::add_content_browser()
{
    auto content_browser_window = std::make_shared<EditorWindow>(m_last_window_id, ImGuiWindowFlags_MenuBar, EditorWindowType::Content);
    m_editor_windows.emplace_back(content_browser_window);
}

void Editor::add_game()
{
    auto game_window = std::make_shared<EditorWindow>(m_last_window_id, ImGuiWindowFlags_MenuBar, EditorWindowType::Game);
    m_editor_windows.emplace_back(game_window);
}

void Editor::add_inspector()
{
    auto inspector_window = std::make_shared<EditorWindow>(m_last_window_id, ImGuiWindowFlags_MenuBar, EditorWindowType::Inspector);
    m_editor_windows.emplace_back(inspector_window);
}

void Editor::add_scene_hierarchy()
{
    auto hierarchy_window = std::make_shared<EditorWindow>(m_last_window_id, ImGuiWindowFlags_MenuBar, EditorWindowType::Hierarchy);
    m_editor_windows.emplace_back(hierarchy_window);
}

void Editor::add_history()
{
    auto history_window = std::make_shared<EditorWindow>(m_last_window_id, ImGuiWindowFlags_MenuBar, EditorWindowType::History);
    m_editor_windows.emplace_back(history_window);
}

void Editor::remove_window(std::shared_ptr<EditorWindow> const& window)
{
    auto const it = std::ranges::find(m_editor_windows, window);

    if (it != m_editor_windows.end())
        m_editor_windows.erase(it);
}

bool Editor::are_debug_drawings_enabled() const
{
    return m_debug_drawings_enabled;
}

std::string Editor::get_component_custom_name(std::string const& guid)
{
    auto const it = m_component_custom_names.find(guid);
    return it != m_component_custom_names.end() ? it->second : "";
}

std::string* Editor::get_component_custom_name_by_ptr(std::string const& guid)
{
    // This is a bit hacky but makes the job done.
    auto it = m_component_custom_names.find(guid);

    if (it == m_component_custom_names.end())
    {
        m_component_custom_names.insert({guid, ""});
        it = m_component_custom_names.find(guid);
    }

    return &it->second;
}

void Editor::set_component_custom_name(std::string const& guid, std::string const& custom_name)
{
    m_component_custom_names.insert({guid, custom_name});
}

bool Editor::does_edited_value_changed()
{
    return !m_currently_edited_value->is_values_equal();
}

void Editor::add_action_to_history()
{
    std::string name = m_selected_component.lock()->get_name();

    if (auto custom_name = m_component_custom_names.find(m_selected_component.lock()->guid); custom_name != m_component_custom_names.end())
    {
        name += " " + custom_name->second;
    }

    m_currently_edited_value->entity = m_selected_entity;
    m_currently_edited_value->component = name;
    m_editor_history.emplace_back(m_currently_edited_value);
}

bool Editor::is_currently_edited_value_saved()
{
    if (m_currently_edited_value == nullptr)
    {
        return true;
    }

    return m_currently_edited_value->is_saved;
}

void Editor::set_currently_edited_value_saved(bool is_saved)
{
    m_currently_edited_value->is_saved = is_saved;
}

void Editor::set_currently_edited_value_label(std::string label)
{
    m_currently_edited_value->label = label;
}

void Editor::undo()
{
    i32 undoId = m_editor_history.size() - m_history_head - 1;
    if (undoId >= 0)
    {
        m_editor_history[undoId]->apply_before();
        m_history_head = m_editor_history.size() - undoId;
        m_editor_history[undoId]->entity.lock()->transform->set_dirty();
        m_editor_history[undoId]->entity.lock()->transform->set_rotation(
            m_editor_history[undoId]->entity.lock()->transform->get_euler_angles());
        m_is_scene_dirty = true;
    }
}

void Editor::redo()
{
    i32 undoId = m_editor_history.size() - m_history_head;
    if (undoId < m_editor_history.size())
    {
        m_editor_history[undoId]->apply_after();
        m_history_head = m_editor_history.size() - undoId - 1;
        m_editor_history[undoId]->entity.lock()->transform->set_dirty();
        m_editor_history[undoId]->entity.lock()->transform->set_rotation(
            m_editor_history[undoId]->entity.lock()->transform->get_euler_angles());
        m_is_scene_dirty = true;
    }
}

template<>
std::string event_value_to_string(std::string const& v)
{
    return v;
}

template<>
std::string event_value_to_string(glm::vec2 const& v)
{
    return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")";
}

template<>
std::string event_value_to_string(glm::vec3 const& v)
{
    return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
}

template<>
std::string event_value_to_string(glm::vec4 const& v)
{
    return "(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ", " + std::to_string(v.w) + ")";
}

#endif
}
