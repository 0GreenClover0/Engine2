#pragma once

#include "AK/Types.h"
#include "EngineDefines.h"
#include "MainScene.h"
#include "glm/gtc/type_ptr.hpp"

#include <memory>
#include <string>

#if EDITOR
#include <imgui.h>
#include <imgui_stdlib.h>
#include <Editor.h>

namespace ImGuiEx
{
template<class T>
bool draw_ptr(std::string const& label, std::weak_ptr<T>& ptr)
{
    std::string guid;

    if (!ptr.expired())
    {
        guid = ptr.lock()->guid;
    }
    else
    {
        guid = "nullptr";
    }

    ImGui::LabelText(label.c_str(), guid.c_str());

    bool value_changed = false;

    if (ImGui::BeginDragDropTarget())
    {
        if (ImGuiPayload const* payload = ImGui::AcceptDragDropPayload("guid"))
        {
            guid.resize(sizeof(i64) * 8);
            memcpy(guid.data(), payload->Data, sizeof(i64) * 8);

            if (auto const component = MainScene::get_instance()->get_component_by_guid(guid))
            {
                auto added_component = std::dynamic_pointer_cast<T>(component);

                if (added_component)
                {
                    value_changed = true;
                    ptr = added_component;
                }
            }
            else if constexpr (std::is_base_of_v<Entity, T>)
            {
                if (auto const entity = MainScene::get_instance()->get_entity_by_guid(guid))
                {
                    value_changed = true;
                    ptr = entity;
                }
            }
        }

        ImGui::EndDragDropTarget();
    }

    return value_changed;
}

inline bool InputFloat(char const* label, float* v)
{
    return ImGui::InputFloat(label, v, 0, 0, "%.3f", ImGuiInputTextFlags_CharsDecimal);
}

inline bool InputFloat2(char const* label, float v[2])
{
    return ImGui::InputFloat2(label, v, "%.3f", ImGuiInputTextFlags_CharsDecimal);
}

inline bool InputFloat3(char const* label, float v[3])
{
    return ImGui::InputFloat3(label, v, "%.3f", ImGuiInputTextFlags_CharsDecimal);
}

inline bool InputFloat4(char const* label, float v[4])
{
    return ImGui::InputFloat4(label, v, "%.3f", ImGuiInputTextFlags_CharsDecimal);
}

}

template<typename T>
inline void register_data_for_undo(T* v, std::string label, std::string componentName = "")
{
    if (ImGui::IsItemActive() and Editor::Editor::get_instance()->is_currently_edited_value_saved())
    {
        Editor::Editor::get_instance()->begin_edit_value(v);
        Editor::Editor::get_instance()->set_value_before_of_currently_edited_value(*v);
    }

    if (ImGui::IsItemDeactivated())
    {
        Editor::Editor::get_instance()->set_value_after_of_currently_edited_value(*v);
        if (Editor::Editor::get_instance()->does_edited_value_changed())
        {
            Editor::Editor::get_instance()->set_value_pointer_of_currently_edited_value(v);
            Editor::Editor::get_instance()->set_currently_edited_value_label(label);
            Editor::Editor::get_instance()->add_action_to_history(componentName);
        }
        
        Editor::Editor::get_instance()->set_currently_edited_value_saved(true);
    }
}

inline bool u8_draw_editor(std::string const& label, u8& value)
{
    bool is_changed = ImGui::InputScalar(label.c_str(), ImGuiDataType_U8, &value);
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool u16_draw_editor(std::string const& label, u16& value)
{
    bool is_changed = ImGui::InputScalar(label.c_str(), ImGuiDataType_U16, &value);
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool u32_draw_editor(std::string const& label, u32& value)
{
    bool is_changed = ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &value);
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool u64_draw_editor(std::string const& label, u64& value)
{
    bool is_changed = ImGui::InputScalar(label.c_str(), ImGuiDataType_U64, &value);
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool i8_draw_editor(std::string const& label, i8& value)
{
    bool is_changed = ImGui::InputScalar(label.c_str(), ImGuiDataType_S8, &value);
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool i16_draw_editor(std::string const& label, i16& value)
{
    bool is_changed = ImGui::InputScalar(label.c_str(), ImGuiDataType_S16, &value);
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool i32_draw_editor(std::string const& label, i32& value)
{
    bool is_changed = ImGui::InputScalar(label.c_str(), ImGuiDataType_S32, &value);
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool i64_draw_editor(std::string const& label, i64& value)
{
    bool is_changed = ImGui::InputScalar(label.c_str(), ImGuiDataType_S64, &value);
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool float_draw_editor(std::string const& label, float& value)
{
    bool is_changed = ImGuiEx::InputFloat(label.c_str(), &value);
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool double_draw_editor(std::string const& label, double& value)
{
    bool is_changed = ImGui::InputDouble(label.c_str(), &value);
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool string_draw_editor(std::string const& label, std::string& value)
{
    bool is_changed = ImGui::InputText(label.c_str(), &value);
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool rename_entity_draw_editor(std::string const& label, std::string& value)
{
    bool is_changed = ImGui::InputText(label.c_str(), &value, ImGuiInputTextFlags_EnterReturnsTrue);
    register_data_for_undo(&value, "Name", "entity");
    return is_changed;
}

inline bool bool_draw_editor(std::string const& label, bool& value)
{
    bool is_changed = ImGui::Checkbox(label.c_str(), &value);
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool vec2_draw_editor(std::string const& label, glm::vec2& value)
{
    bool is_changed = ImGuiEx::InputFloat2(label.c_str(), glm::value_ptr(value));
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool vec3_draw_editor(std::string const& label, glm::vec3& value)
{
    bool is_changed = ImGuiEx::InputFloat3(label.c_str(), glm::value_ptr(value));
    register_data_for_undo(&value, label);
    return is_changed;
}

inline bool vec4_draw_editor(std::string const& label, glm::vec4& value)
{
    bool is_changed = ImGuiEx::InputFloat4(label.c_str(), glm::value_ptr(value));
    register_data_for_undo(&value, label);
    return is_changed;
}

template<class T>
bool shared_ptr_draw_editor(std::string const& label, std::shared_ptr<T>& value)
{
    return ImGuiEx::draw_ptr(label, value);
}

template<class T>
bool weak_ptr_draw_editor(std::string const& label, std::weak_ptr<T>& value)
{
    return ImGuiEx::draw_ptr(label, value);
}

#endif
