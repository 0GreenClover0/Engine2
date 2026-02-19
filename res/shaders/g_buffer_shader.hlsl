struct VS_Input
{
    float3 pos: POSITION;
    float3 normal : NORMAL;
    float2 UV : TEXCOORD0;
};

struct VS_Output
{
    float4 pixel_pos : SV_POSITION;
    float3 normal : NORMAL;
    float3 world_pos : POSITION;
    float2 UV : TEXCOORD;
};

struct PS_Output
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1; // Alpha channel holds whether something is blinking or not
    float4 diffuse : SV_Target2;
    float4 metallic : SV_Target3;
    float4 roughness : SV_Target4;
    float4 ao : SV_Target5;
};

cbuffer object_buffer : register(b0)
{
    float4x4 projection_view_model;
    float4x4 model;
    float4x4 projection_view;
};

cbuffer object_buffer : register(b10)
{
    float4x4 projection_view_model1;
    float4x4 model1;
    float4x4 projection_view1;
    bool is_glowing;
};

Texture2D albedo_texture : register(t0);
Texture2D normal_texture : register(t1);
Texture2D metallic_texture : register(t2);
Texture2D roughness_texture : register(t3);
Texture2D ambient_occlusion_texture : register(t4);
SamplerState obj_sampler_state : register(s0);

float3 get_normal_from_texture(VS_Output input)
{
    float3 tangent_normal = normal_texture.Sample(obj_sampler_state, input.UV).xyz * 2.0 - 1.0;

    float3 Q1  = ddx(input.world_pos);
    float3 Q2  = ddy(input.world_pos);
    float2 st1 = ddx(input.UV);
    float2 st2 = ddy(input.UV);

    float3 N = normalize(input.normal);
    float3 T = normalize(Q1 * st2.y - Q2 * st1.y);
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);

    return normalize(mul(tangent_normal, TBN));
}

VS_Output vs_main(VS_Input input)
{
    VS_Output output;

    output.world_pos = mul(model, float4(input.pos, 1.0f));
    output.UV = input.UV;
    output.normal = mul((float3x3)model, input.normal);
    output.pixel_pos = mul(projection_view_model, float4(input.pos, 1.0f));

    return output;
}

PS_Output ps_main(VS_Output input)
{
    PS_Output output;
    output.diffuse = albedo_texture.Sample(obj_sampler_state, input.UV);
    output.normal.xyz = get_normal_from_texture(input);
    output.metallic = metallic_texture.Sample(obj_sampler_state, input.UV).r;
    output.roughness = roughness_texture.Sample(obj_sampler_state, input.UV).r;
    output.ao = ambient_occlusion_texture.Sample(obj_sampler_state, input.UV).r;

    output.position.xyz = input.world_pos;
    output.normal.a = is_glowing ? 1.0f : -1.0f;
    output.position.a = 1.0f;

    return output;
}
