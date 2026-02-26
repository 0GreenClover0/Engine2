#include "UndoTest.h"
#include "SceneSerializer.h"

#if EDITOR
#include <imgui.h>
#endif

#if EDITOR
#include "imgui_extensions.h"
#endif

std::shared_ptr<UndoTest> UndoTest::create()
{
    return std::make_shared<UndoTest>(AK::Badge<UndoTest> {});
}

UndoTest::UndoTest(AK::Badge<UndoTest>)
{
}

void UndoTest::awake()
{
    set_can_tick(true);
}

void UndoTest::fixed_update()
{
}

#if EDITOR
void UndoTest::draw_editor()
{
    Component::draw_editor();

    u8_draw_editor(" U8: ", _u8);
    u16_draw_editor(" U16: ", _u16);
    u32_draw_editor(" U32: ", _u32);
    u64_draw_editor(" U64: ", _u64);
    i8_draw_editor(" I8: ", _i8);
    i16_draw_editor(" I16: ", _i16);
    i32_draw_editor(" I32: ", _i32);
    i64_draw_editor(" I64: ", _i64);
    float_draw_editor(" Float: ", _float);
    double_draw_editor(" Double: ", _double);
    string_draw_editor(" String: ", _string);
    bool_draw_editor(" Bool: ", _bool);
    vec2_draw_editor(" Vec2: ", _vec2);
    vec3_draw_editor(" Vec3: ", _vec3);
    vec4_draw_editor(" Vec4: ", _vec4);
}
#endif

#if EDITOR
std::string UndoTest::get_name()
{
    return "UndoTest";
}
#endif
