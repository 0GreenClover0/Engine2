#pragma once

#include "Component.h"
#include "Cow.h"
#include "DialoguePromptController.h"
#include "Game/Path.h"
#include "Komiks/UFO.h"
#include "Sound.h"
#include <Game/FieldGrid.h>
#include <Game/Jeep.h>
#include <Game/WheatOverlay.h>
#include <ScreenText.h>

class UFO;

class CowManager final : public Component
{

public:
    static std::shared_ptr<CowManager> create();

    explicit CowManager(AK::Badge<CowManager>);

    static std::shared_ptr<CowManager> get_instance();

    void clear_map();

    void setup_level();
    virtual void awake() override;
    virtual void start() override;
    virtual void update() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

#if EDITOR
    virtual void custom_draw_editor() override;
#endif

    void get_spawn_paths();
    glm::vec2 get_random_position_with_minimal_distance(glm::vec3 current_position) const;

    std::weak_ptr<DialoguePromptController> dialogue_prompt_controller = {};
    std::weak_ptr<WheatOverlay> wheat_overlay = {};
    std::weak_ptr<ScreenText> clock_text_ref = {};
    std::weak_ptr<FieldGrid> field_grid = {};

    NON_SERIALIZED
    std::vector<std::weak_ptr<Path>> paths = {};

    NON_SERIALIZED
    std::vector<std::weak_ptr<Cow>> cows = {};

    NON_SERIALIZED
    std::weak_ptr<Truther> truther = {};

    NON_SERIALIZED
    std::weak_ptr<UFO> ufo = {};

    NON_SERIALIZED
    std::weak_ptr<Jeep> jeep = {};

    NON_SERIALIZED
    float time = 0.0f;

    NON_SERIALIZED
    bool has_level_ended = false;

    NON_SERIALIZED
    bool has_player_moved_this_level = false;

private:
    void spawn_truther();
    void spawn_cow();
    void spawn_ufo();
    void spawn_jeep();
    void activate_ufo();
    void activate_jeep();
    void change_jeep_direction();
    void end_level();

    void set_pattern(u32 id);

    inline static std::shared_ptr<CowManager> m_instance;

    float flash_at_start_timer = 3.5f;
    float m_event_time_threshold = 0.0f;

    std::weak_ptr<Entity> m_wasd_prompt = {};
    std::weak_ptr<Entity> m_shift_prompt = {};
    std::weak_ptr<Entity> m_space_prompt = {};

    float m_event_timer = 0.0f;

    bool m_space_shown = false;
    bool m_shift_shown = false;

    u32 m_level = 0;
    i32 m_frame_skip = 2;
};
