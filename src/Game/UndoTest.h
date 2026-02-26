#pragma once

#include "Component.h"

#include "Mesh.h"

class UndoTest final : public Component
{
public:
#if EDITOR
    virtual std::string get_name() override;
#endif

    static std::shared_ptr<UndoTest> create();

    explicit UndoTest(AK::Badge<UndoTest>);

    virtual void awake() override;
    virtual void fixed_update() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    u8 _u8 = 0;
    u16 _u16 = 1;
    u32 _u32 = 2;
    u64 _u64 = 3;
    
    i8 _i8 = 4;
    i16 _i16 = 5;
    i32 _i32 = 6;
    i64 _i64 = 7;
    
    float _float = 8.0f;
    double _double = 9.0f;
    std::string _string = "10";
    bool _bool = true;

    glm::vec2 _vec2 = {12.0f, 12.0f};
    glm::vec3 _vec3 = {13.0f, 13.0f, 13.0f};
    glm::vec4 _vec4 = {14.0f, 14.0f, 14.0f, 14.0f};

    //std::shared_ptr<Mesh> _shared_mesh = {};
    //std::weak_ptr<Texture> _weak_texture = {};
};
