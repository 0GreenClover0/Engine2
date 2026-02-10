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

#endif

inline bool u8_draw_editor(std::string const& label, u8& value)
{
    return ImGui::InputScalar(label.c_str(), ImGuiDataType_U8, &value);
}

inline bool u16_draw_editor(std::string const& label, u16& value)
{
    return ImGui::InputScalar(label.c_str(), ImGuiDataType_U16, &value);
}

inline bool u32_draw_editor(std::string const& label, u32& value)
{
    return ImGui::InputScalar(label.c_str(), ImGuiDataType_U32, &value);
}

inline bool u64_draw_editor(std::string const& label, u64& value)
{
    return ImGui::InputScalar(label.c_str(), ImGuiDataType_U64, &value);
}

inline bool i8_draw_editor(std::string const& label, i8& value)
{
    return ImGui::InputScalar(label.c_str(), ImGuiDataType_S8, &value);
}

inline bool i16_draw_editor(std::string const& label, i16& value)
{
    return ImGui::InputScalar(label.c_str(), ImGuiDataType_S16, &value);
}

inline bool i32_draw_editor(std::string const& label, i32& value)
{
    return ImGui::InputScalar(label.c_str(), ImGuiDataType_S32, &value);
}

inline bool i64_draw_editor(std::string const& label, i64& value)
{
    return ImGui::InputScalar(label.c_str(), ImGuiDataType_S64, &value);
}

inline bool float_draw_editor(std::string const& label, float& value)
{
    return ImGuiEx::InputFloat(label.c_str(), &value);
}

inline bool double_draw_editor(std::string const& label, double& value)
{
    return ImGui::InputDouble(label.c_str(), &value);
}

inline bool string_draw_editor(std::string const& label, std::string& value)
{
    return ImGui::InputText(label.c_str(), &value);
}

inline bool bool_draw_editor(std::string const& label, bool& value)
{
    return ImGui::Checkbox(label.c_str(), &value);
}

inline bool vec2_draw_editor(std::string const& label, glm::vec2& value)
{
    return ImGuiEx::InputFloat2(label.c_str(), glm::value_ptr(value));
}

inline bool vec3_draw_editor(std::string const& label, glm::vec3& value)
{
    return ImGuiEx::InputFloat3(label.c_str(), glm::value_ptr(value));
}

inline bool vec4_draw_editor(std::string const& label, glm::vec4& value)
{
    return ImGuiEx::InputFloat4(label.c_str(), glm::value_ptr(value));
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
