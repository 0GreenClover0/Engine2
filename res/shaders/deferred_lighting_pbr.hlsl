#include "ShadingDefines.h"
#include "halo.hlsl"

struct VS_Input
{
    float3 pos: POSITION;
    float3 normal : NORMAL;
    float2 UV : TEXCOORD;
};

struct VS_Output
{
    float4 pos: SV_Position;
    float2 UV : TEXCOORD0;
};

Texture2D pos_tex : register(t7);
Texture2D albedo_tex : register(t8);
Texture2D normal_tex : register(t9);
Texture2D metallic_tex : register(t10);
Texture2D roughness_tex : register(t11);
Texture2D ambient_occlusion_tex : register(t12);
Texture2D ssao_tex : register(t14);
Texture2D fog_tex : register(t16);

SamplerState obj_sampler_state : register(s0);
SamplerState repeat_sampler : register(s2);

VS_Output vs_main(VS_Input input)
{
    VS_Output output;
    output.pos = float4(input.pos, 1.0f);
    output.UV = input.UV;
    return output;
}

float4 ps_main(VS_Output input) : SV_Target
{
    float4 result = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 pos = pos_tex.Sample(obj_sampler_state, input.UV);
    float4 normal = normalize(normal_tex.Sample(obj_sampler_state, input.UV));
    float3 albedo = pow(albedo_tex.Sample(obj_sampler_state, input.UV).rgb, float3(2.2f, 2.2f, 2.2f));
    //float3 albedo = albedo_tex.Sample(obj_sampler_state, input.UV).rgb;
    float metallic = metallic_tex.Sample(obj_sampler_state, input.UV).r;
    float roughness = roughness_tex.Sample(obj_sampler_state, input.UV).r;
    float ambient_occlusion = ssao_tex.Sample(obj_sampler_state, input.UV).r + ambient_occlusion_tex.Sample(obj_sampler_state, input.UV).r;

    if (albedo.r == 0.0f && albedo.g == 0.0f && albedo.b == 0.0f)
    {
        discard;
    }

    float3 view_dir = normalize(camera_pos.xyz - pos.xyz);

    result.xyz += calculate_directional_light(directional_light, normal.xyz, view_dir, albedo, metallic, roughness, ambient_occlusion, pos.xyz, true);
    float fog_value = fog_tex.Sample(repeat_sampler, input.UV + time_ps / 100.0f).r;
    float3 scatter = 0.0f.xxx;

    result += 0.1f * fog_value;

    for (int point_light_index = 0; point_light_index < number_of_point_lights; point_light_index++)
    {
        if (!BELOW_WATER_HACK || pos.y > -0.1f)
        {
            scatter.xyz += calculate_scatter(point_lights[point_light_index], pos) * fog_value;
        }

        result.xyz += calculate_point_light(point_lights[point_light_index],normal.xyz, pos.xyz, view_dir, albedo, metallic, roughness, ambient_occlusion, point_light_index, RENDER_POINT_SHADOW_MAPS);
    }

    for (int spot_light_index = 0; spot_light_index < number_of_spot_lights; spot_light_index++)
    {
        if (!BELOW_WATER_HACK || pos.y > -0.1f)
        {
            scatter.xyz += calculate_scatter(spot_lights[spot_light_index], pos, spot_light_index) * fog_value;
        }

        result.xyz += calculate_spot_light(spot_lights[spot_light_index], normal.xyz, pos.xyz, view_dir, albedo, metallic, roughness, ambient_occlusion, spot_light_index, true);
    }

    // Normal alpha channel stores info whether glow should be applied
    if (normal.a > 0.0f)
    {
        result.xyz = glow(result.xyz, pos.xyz, normal.xyz).xyz;
    }

    result.xyz += scatter;
    result.xyz = exposure_tonemapping(result.xyz, exposure_strength);
    return gamma_correction(result.xyz, gamma_strength);
}
