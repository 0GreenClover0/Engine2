#pragma once

#include "Component.h"
#include "Curve.h"
#include "CustomerManager.h"
#include "Engine.h"
#include "Factory.h"
#include "Lighthouse.h"
#include "Player.h"
#include "Port.h"

class ScreenText;
class Factory;
class Lighthouse;

enum class TutorialProgressAction
{
    PackageCollected,
    ShipEnteredPort,
    ShipEnteredControl,
    ShipDestroyed,
    ShipInFlashCollider,
    PirateDestroyed,
    KeeperLeftLighthouse,
    KeeperEnteredLighthouse,
    KeeperEnteredPort,
    LighthouseEnabled,
    GeneratorFueled,
    WorkshopUpgraded,
    LevelStarted,
    DialogEnded,
};

class LevelController final : public Component
{
public:
    static std::shared_ptr<LevelController> create();

    static std::shared_ptr<LevelController> get_instance();

    virtual void uninitialize() override;

    virtual void awake() override;
    virtual void update() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

#if EDITOR
    virtual void custom_draw_editor() override;
#endif

    void on_lighthouse_upgraded() const;

    void set_exiting_lighthouse(bool value);
    bool get_exiting_lighthouse();

    void check_tutorial_progress(TutorialProgressAction action);

    void progress_tutorial(i32 step = 1);

    void spawn_prompt(std::string const& prefab_name, glm::vec3 const& position, std::weak_ptr<Entity>& optional_ref);

    void spawn_mouse_prompt_on_new_level();
    void destroy_mouse_prompt();

    void end_level();

    NON_SERIALIZED
    bool is_started = false;
    NON_SERIALIZED
    bool is_ended = false;

    CUSTOM_EDITOR
    float map_time = 180.0f;
    CUSTOM_EDITOR
    u32 map_food = 20;
    CUSTOM_EDITOR
    i32 maximum_lighthouse_level = 10;
    NON_SERIALIZED
    float time = 0.0f;

    CUSTOM_EDITOR
    std::vector<std::weak_ptr<Factory>> factories = {};
    CUSTOM_EDITOR
    std::weak_ptr<Port> port = {};
    CUSTOM_EDITOR
    std::weak_ptr<Lighthouse> lighthouse = {};
    CUSTOM_EDITOR
    std::weak_ptr<CustomerManager> customer_manager = {};

    CUSTOM_EDITOR
    float playfield_width = 6.5f;
    CUSTOM_EDITOR
    float playfield_additional_width = 2.0f;
    CUSTOM_EDITOR
    float playfield_height = 4.8f;
    CUSTOM_EDITOR
    float playfield_y_shift = -2.1f;

    CUSTOM_EDITOR
    std::weak_ptr<Curve> ships_limit_curve = {};
    CUSTOM_EDITOR
    u32 ships_limit = 1;

    CUSTOM_EDITOR
    std::weak_ptr<Curve> ships_speed_curve = {};
    CUSTOM_EDITOR
    float ships_speed = 0.0f;

    CUSTOM_EDITOR
    std::weak_ptr<Curve> ships_range_curve = {};
    CUSTOM_EDITOR
    std::weak_ptr<Curve> ships_turn_curve = {};
    CUSTOM_EDITOR
    std::weak_ptr<Curve> ships_additional_speed_curve = {};
    CUSTOM_EDITOR
    std::weak_ptr<Curve> pirates_in_control_curve = {};

    CUSTOM_EDITOR
    bool is_tutorial = false;
    CUSTOM_EDITOR
    u32 starting_packages = 2;
    NON_SERIALIZED
    u32 tutorial_progress = 0;
    CUSTOM_EDITOR
    u32 tutorial_level = 1;
    NON_SERIALIZED
    u32 tutorial_spawn_path = 0;
    NON_SERIALIZED
    bool is_tutorial_dialogs_enabled = true;

private:
    inline static std::shared_ptr<LevelController> m_instance;

    std::weak_ptr<Player> player_ref = {};
    std::weak_ptr<ScreenText> clock_text_ref = {};
    std::weak_ptr<ScreenText> m_text = {};

    std::weak_ptr<Entity> m_story_mouse_prompt = {};
    std::weak_ptr<Entity> m_story_space_prompt = {};
    std::weak_ptr<Entity> m_story_second_space_prompt = {};
    std::weak_ptr<Entity> m_story_wasd_prompt = {};

    glm::vec3 m_mouse_prompt_pos = {3.863f, 2.0f, 1.168f};
    glm::vec3 m_space_prompt_pos = {3.741f, 2.0f, 1.826f};
    glm::vec3 m_second_space_prompt_pos = {2.244f, 2.0f, -3.885f};
    glm::vec3 m_wasd_prompt_pos = {2.436f, 2.0f, 1.152f};

    bool m_is_exiting_lighthouse_enabled = true;
};
